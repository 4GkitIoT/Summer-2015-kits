/**
* Copyright (C) 2015 Orange
*
* This software is distributed under the terms and conditions of the 'Apache-2.0'
* license which can be found in the file 'LICENSE' in this package distribution
* or at 'http://www.apache.org/licenses/LICENSE-2.0'.
*/

#include "SocketServer.hpp"

#include "Config.hpp"

#include <limits>
#include <thread>
#include <functional>
#include <algorithm>

namespace SZ
{

SocketServer::SocketServer()
  : mServer(nullptr),
    mConn(nullptr),
    mServerThreadRunning(false)
{

}

SocketServer::~SocketServer()
{

}

SocketResult<int> SocketServer::Init()
{
    auto port = Config::Me()->DirectConnectionPort();
    mServer = nopoll_ctx_new();
    mConn = nopoll_listener_new(mServer, "0.0.0.0", port.c_str());
    if (!Connected())
    {
        ZLOGW() << "Could not create websocket server";
        return SocketResult<int>(SocketStatusCode::ConnectionError, errno);
    }

    ZLOGI() << "Created websocket server";
    return SocketResult<int>(SocketStatusCode::Correct, 0);
}

SocketResult<int> SocketServer::Send(noPollConn *clientConn, std::string data)
{
    ZLOGD() << "Directly sending to client socket: " << data;
    if (nopoll_conn_send_text(clientConn, data.c_str(), data.length()) < 0)
    {
        return SocketResult<int>(SocketStatusCode::SendError, errno);
    }
    return SocketResult<int>(SocketStatusCode::Correct, 0);
}

SocketResult<std::string> SocketServer::EnsureClientAllowed(const std::string &session)
{
    if (std::find(mAllowedSessions.begin(), mAllowedSessions.end(), session) == mAllowedSessions.end())
    {
        return SocketResult<std::string>(SocketStatusCode::SessionInvalidError, session);
    }
    return SocketResult<std::string>(SocketStatusCode::Correct, "");
}

void SocketServer::AddAllowedSession(const std::string &session)
{
    if (std::find(mAllowedSessions.begin(), mAllowedSessions.end(), session) != mAllowedSessions.end())
    {
        ZLOGI() << "New session request for " << session << " ignored; already active";
        return;
    }
    mAllowedSessions.push_back(session);
    if (mAllowedSessions.size() > MAX_ALLOWED_SESSIONS)
    {
        mAllowedSessions.erase(mAllowedSessions.begin());
    }
}

BoundConnection SocketServer::BindConnection(noPollConn *clientConn)
{
    return BoundConnection(this, clientConn);
}

void SocketServer::ServerThreadListen(std::function<bool (noPollConn *, std::string)> receiveCallback)
{
    mServerThreadRunning = true;
    while (true)
    {
        auto clientConn = nopoll_conn_accept(mServer, mConn);
        if (!nopoll_conn_is_ok(clientConn))
        {
            ZLOGW() << "Could not connect client";
            continue;
        }
        ZLOGI() << "Connected new client to websocket server; id: " << nopoll_conn_get_id(clientConn);

        std::thread clientThread(std::bind(&SocketServer::ClientThreadListen, this, receiveCallback, clientConn));
        clientThread.detach();
    }

    nopoll_conn_close(mConn);
    nopoll_ctx_unref(mServer);
    mServerThreadRunning = false;
}

void SocketServer::ClientThreadListen(std::function<bool (noPollConn *, std::string)> receiveCallback, noPollConn *clientConn)
{
    int connId = nopoll_conn_get_id(clientConn);
    ZLOGI() << "Started client connection thread for id: " << connId;
    while (true)
    {
        auto msgResult = AssembleFullMessage(clientConn);
        if (msgResult != SocketStatusCode::Correct)
        {
            break;
        }

        ZLOGD() << "Got message from client #" << connId;
        if (receiveCallback(clientConn, msgResult.Data()))
        {
            break;
        }
    }

    ZLOGD() << "Closing client connection; id: " << connId;
    nopoll_conn_close(clientConn);
}

SocketResult<std::string> SocketServer::AssembleFullMessage(noPollConn *clientConn)
{
    noPollMsg *msg;
    std::stringstream ss;
    do
    {
        msg = nopoll_conn_get_msg(clientConn);
        if (!msg) // either connection is broken or message is not ready
        {
            if (!nopoll_conn_is_ok(clientConn))
            {
                ZLOGI() << "Client connection broken";
                return SocketResult<std::string>(SocketStatusCode::ConnectionError, "");
            }
            nopoll_sleep(10000); // 10ms
            continue;
        }

        auto dataRaw = reinterpret_cast<const char*>(nopoll_msg_get_payload(msg));
        if (!dataRaw)
        {
            nopoll_sleep(10000);
            continue;
        }
        auto data = std::string(dataRaw);
        ss << data;
        ZLOGD() << "Received directly: " << data;
    }
    while (!nopoll_msg_is_final(msg));

    return SocketResult<std::string>(SocketStatusCode::Correct, ss.str());
}

SocketResult<int> SocketServer::Listen(std::function<bool (noPollConn*, std::string)> receiveCallback)
{
    if (mServerThreadRunning)
    {
        ZLOGW() << "Tried to start second instance of socket server";
        return SocketResult<int>(SocketStatusCode::ServerAlreadyRunningError, -1);
    }

    std::thread serverThread(std::bind(&SocketServer::ServerThreadListen, this, receiveCallback));
    serverThread.detach();

    return SocketResult<int>(SocketStatusCode::Correct, 0);
}

void SocketServer::Close()
{
    if (mServer != nullptr)
    {
        nopoll_ctx_unref(mServer);
        mConn = nullptr;
    }
}

bool SocketServer::Connected() const
{
    return mConn && nopoll_conn_is_ok(mConn);
}

}
