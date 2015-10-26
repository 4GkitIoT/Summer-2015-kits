/**
* Copyright (C) 2015 Orange
*
* This software is distributed under the terms and conditions of the 'Apache-2.0'
* license which can be found in the file 'LICENSE' in this package distribution
* or at 'http://www.apache.org/licenses/LICENSE-2.0'.
*/

#ifndef SZ_MAIN_
#define SZ_MAIN_

#include "IResult.hpp"
#include "JSONMessage.hpp"
#include "SSLSocketClient.hpp"
#include "SocketServer.hpp"

namespace SZ
{
class CoreMessageHandler;

class App
{
    static constexpr auto FAILURE_COUNT_TO_RECONNECT_SCRIPT = 6;
    SSLSocketClient *mConn;
    SocketServer *mServerConn;
    CoreMessageHandler *mCoreHandler;
    std::atomic<int> mKeepAliveCounter;

    SocketBase* CurrentConn(); // to access "better" socket conn from lua call

    void RegisterCommonLuaFunctions();
    inline void Sleep(int seconds);
    const std::string& BoardId() const;

    void KeepAliveThread();
    bool HandleServerMessage(JSONServerMessage &msg);

    template <typename TCode, typename TData>
    void OnError(const IResult<TCode, TData> &result, std::string extraMsg, const std::string &session, SocketBase *conn = nullptr)
    {
        ZLOGW() << extraMsg << ": " << result;
        if (conn)
        {
            JSONClientMessage errorMsg;
            errorMsg.InitCoreMessage(BoardId(), result, extraMsg, session);
            conn->Send(errorMsg.Create());
        }
    }

    template <typename TCode, typename TData>
    bool EnsureCorrectness(const IResult<TCode, TData> &result, std::string extraMsg, const std::string &session, SocketBase *conn = nullptr)
    {
        if (result.Correct())
        {
            return true;
        }
        OnError(result, extraMsg, session, conn);
        return false;
    }
public:
    App();
    ~App();

    int Logic();
    int LuaCallSendToServer(lua_State *state);

    std::string OpenListeningSocket(const std::string &allowedSession);
};
}
#endif // SZ_MAIN_
