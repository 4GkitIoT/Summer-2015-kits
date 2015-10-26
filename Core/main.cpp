/**
* Copyright (C) 2015 Orange
*
* This software is distributed under the terms and conditions of the 'Apache-2.0'
* license which can be found in the file 'LICENSE' in this package distribution
* or at 'http://www.apache.org/licenses/LICENSE-2.0'.
*/

#include "main.hpp"

#include <iostream>
#include <thread>

#include "CoreMessageHandler.hpp"
#include "LuaStateControl.hpp"
#include "LuaModule.hpp"
#include "LuaResult.hpp"
#include "SocketServer.hpp"
#include "Config.hpp"
#include "Logger.hpp"

#ifndef SZ_USE_CHRONO
    #include <ctime>
#endif

SZ::App *globalApp = nullptr; // global for access from lua calls

namespace SZLuaCallbacks
{
int LuaSendToServer(lua_State *state)
{
    return globalApp->LuaCallSendToServer(state);
}
}

namespace SZ
{

inline void App::Sleep(int seconds)
{
#ifdef SZ_USE_CHRONO
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
#else
    timespec spec{seconds, 0L};
    nanosleep(&spec, nullptr);
#endif
}

const std::string &App::BoardId() const
{
    return Config::Me()->BoardId();
}

void App::KeepAliveThread()
{
    int singleSleepTime = 10; //[s]
    int sleepsBeforeSend = 6; // so keep alive interval == 1 min
    while (true)
    {
        Sleep(singleSleepTime);
        ++mKeepAliveCounter;
        ZLOGD() << "Keep alive thread checking: " << (int)mKeepAliveCounter << "/" << sleepsBeforeSend;
        if (mKeepAliveCounter >= sleepsBeforeSend) // TODO execute iff the connection is valid?
        {
            ZLOGI() << "Keep alive thread tries to send status";
            JSONServerMessage msg;
            msg.InitManually(LuaModule::MODULE_STATUS, "");
            HandleServerMessage(msg);
            mKeepAliveCounter = 0;
        }
    }
}

App::App()
  : mConn(nullptr),
    mServerConn(nullptr),
    mCoreHandler(new CoreMessageHandler()),
    mKeepAliveCounter(0)
{

}

App::~App()
{
    if (mConn)
    {
        delete mConn;
        mConn = nullptr;
    }
    if (mServerConn)
    {
        delete mServerConn;
        mServerConn = nullptr;
    }
    if (mCoreHandler)
    {
        delete mCoreHandler;
        mCoreHandler = nullptr;
    }
}

SocketBase *App::CurrentConn()
{
//    if (mServerConn && mServerConn->Connected())
//    {
//        return mServerConn;
//    }
    return mConn;
}

void App::RegisterCommonLuaFunctions()
{
    LuaStateControl::Me()->RegisterFunc("TestFunc", SZLuaCallbacks::LuaSendToServer);
}

int App::Logic()
{
    auto configResult = SZ::Config::Me()->LoadConfig("./config.json");
    if (!EnsureCorrectness(*configResult, "App config error", ""))
    {
        delete configResult;
        return -1;
    }
    delete configResult;

    RegisterCommonLuaFunctions();

    std::thread keepAlive(std::bind(&App::KeepAliveThread, this));
    keepAlive.detach();

    int connectionFailures = 0;

    while (true) // currently never leaves this loop
    {
        if (!mConn)
        {
            mConn = new SSLSocketClient();
        }
        else
        {
            mConn->Close(); // to restart client from scratch
        }

        if (!EnsureCorrectness(mConn->Init(), "Connection error", ""))
        {
            Sleep(3);
            ++connectionFailures;
            if (connectionFailures >= FAILURE_COUNT_TO_RECONNECT_SCRIPT)
            {
                connectionFailures = 0;
                SZ::LuaModule module;
                if (!EnsureCorrectness(module.Init(LuaModule::MODULE_MODEM_CONN), "Reconnecting script init error", "", nullptr))
                {
                    continue;
                }
                if (!EnsureCorrectness(module.Call("", ""), "Reconnecting script Lua call error", "", nullptr))
                {
                    continue;
                }
            }
            continue;
        }
        connectionFailures = 0;

        SZ::JSONClientMessage initialMessage;
        initialMessage.Init(LuaModule::MODULE_INIT, "", BoardId(), "OK", "");
        if (!EnsureCorrectness(mConn->Send(initialMessage.Create()), "Initial sending error", "", mConn))
        {
            Sleep(3);
            continue;
        }

        mConn->Listen([&](const std::string &command)
        {
            SZ::JSONServerMessage message;
            if (!EnsureCorrectness(message.Decode(command), "JSON decode error", "", mConn))
            {
                return;
            }

            HandleServerMessage(message);
            mKeepAliveCounter = 0;
        });

        delete mConn;
        mConn = nullptr;
    }

    keepAlive.join();
    SZ::LuaStateControl::Destroy();
    return 0;
}

bool App::HandleServerMessage(JSONServerMessage &message)
{
    auto &session = message.SessionId();
    SZ::JSONClientMessage response;
    if (mCoreHandler->CanHandle(&message))
    {
        auto callResult = mCoreHandler->Handle(this, &message);
        if (!EnsureCorrectness(callResult, "Core handler error", session, mConn))
        {
            return false;
        }
        response.Init(message.Module(), callResult.Data(), BoardId(), "OK", session);
    }
    else
    {
        SZ::LuaModule module;
        if (!EnsureCorrectness(module.Init(message.Module()), "Lua init error", session, mConn))
        {
            return false;
        }

        auto callResult = module.Call(message.Content(), session);
        if (!EnsureCorrectness(callResult, "Lua call error", session, mConn))
        {
            return false;
        }

        response.Init(message.Module(), callResult.Data(), BoardId(), "OK", session);
    }


    if (!EnsureCorrectness(mConn->Send(response.Create()), "Sending error", session, nullptr))
    {
        return false;
    }
    return true;
}

int App::LuaCallSendToServer(lua_State *state)
{
    auto conn = CurrentConn(); // TODO connection?
    if (!conn || !conn->Connected())
    {
        ZLOGI() << "Connection is invalid; aborting send";
        return -1;
    }

    std::string module = LuaModule::LoadRawStringParam(state, 1);
    std::string data = LuaModule::LoadRawStringParam(state, 2);
    std::string session = LuaModule::LoadRawStringParam(state, 3);

    SZ::JSONClientMessage message;
    message.Init(module, data, BoardId(), "OK", session);

    if (!EnsureCorrectness(conn->Send(message.Create()), "Sending error", session, conn))
    {
        return -1;
    }
    return 0;
}

std::string App::OpenListeningSocket(const std::string &allowedSession)
{
    if (!mServerConn)
    {
        mServerConn = new SocketServer();
    }

    if (!mServerConn->Connected())
    {
        if (!EnsureCorrectness(mServerConn->Init(), "DIRECT server init error", "", mConn))
        {
            return "";
        }
    }

    mServerConn->AddAllowedSession(allowedSession);
    auto result = mServerConn->Listen([&](noPollConn *conn, const std::string &command)
    {
        auto boundConnection = mServerConn->BindConnection(conn);

        SZ::JSONServerMessage message;
        if (!EnsureCorrectness(message.Decode(command), "DIRECT JSON decode error", "", &boundConnection))
        {
            return false;
        }

        if (!EnsureCorrectness(mServerConn->EnsureClientAllowed(message.SessionId()), "DIRECT Session check", message.SessionId(), &boundConnection))
        {
            return true;
        }

        SZ::LuaModule module;
        if (!EnsureCorrectness(module.Init(message.Module()), "DIRECT Lua init error", message.SessionId(), &boundConnection))
        {
            return false;
        }

        auto callResult = module.Call(message.Content(), message.SessionId());
        if (!EnsureCorrectness(callResult, "DIRECT Lua call error", message.SessionId(), &boundConnection))
        {
            return false;
        }

        SZ::JSONClientMessage response;
        response.Init(message.Module(), callResult.Data(), BoardId(), "OK", message.SessionId());
        auto responseText = response.Create();
        if (!EnsureCorrectness(mServerConn->Send(conn, responseText), "DIRECT send error", message.SessionId(), nullptr))
        {
            return false;
        }

        return false;
    });

    if (!EnsureCorrectness(result, "DIRECT server init error (listening)", "", mConn))
    {
        return "";
    }

    return "ok"; // TODO better return status
}
}

int main()
{
    globalApp = new SZ::App();
    int result = globalApp->Logic();
    delete globalApp;
    globalApp = nullptr;
    return result;
}
