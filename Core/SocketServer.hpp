/**
* Copyright (C) 2015 Orange
*
* This software is distributed under the terms and conditions of the 'Apache-2.0'
* license which can be found in the file 'LICENSE' in this package distribution
* or at 'http://www.apache.org/licenses/LICENSE-2.0'.
*/

#ifndef SZ_SOCKETSERVER_
#define SZ_SOCKETSERVER_

#include "SocketBase.hpp"

#include <vector>
#include <atomic>

#include <nopoll.h>
#include <nopoll_ctx.h>

namespace SZ
{
    class BoundConnection;

    class SocketServer
    {
        static const int MAX_ALLOWED_SESSIONS = 50;
        noPollCtx *mServer;
        noPollConn *mConn;
        std::atomic<bool> mServerThreadRunning;
        std::vector<std::string> mAllowedSessions;

        bool SendThroughSocket(std::string &data);
        void ServerThreadListen(std::function<bool (noPollConn*, std::string)> receiveCallback);
        void ClientThreadListen(std::function<bool (noPollConn*, std::string)> receiveCallback, noPollConn *clientConn);
        SocketResult<std::string> AssembleFullMessage(noPollConn *clientConn);
    public:
        SocketServer();
        ~SocketServer();

        SocketResult<int> Init();
        SocketResult<int> Listen(std::function<bool(noPollConn*, std::string)> receiveCallback);
        SocketResult<int> Send(noPollConn *clientConn, std::string data);
        SocketResult<std::string> EnsureClientAllowed(const std::string &session);

        void AddAllowedSession(const std::string &session);
        void RemoveAllowedSession(const std::string &session);

        BoundConnection BindConnection(noPollConn *clientConn);

        void Close();
        bool Connected() const;
    };


    // helper wrapper class to allow sending errors like SocketClient
    class BoundConnection : public SocketBase
    {
        SocketServer *mServer;
        noPollConn *mBoundClient;
    public:
        BoundConnection(SocketServer *srv, noPollConn *conn)
            : mServer(srv),
              mBoundClient(conn)
        {}

        SocketResult<int> Init() { return SocketResult<int>(SocketStatusCode::Correct, 0); }
        SocketResult<int> Listen(std::function<void (std::string)>) { return SocketResult<int>(SocketStatusCode::Correct, 0); }
        void Close() {}
        bool Connected() const { return true; }

        // SocketBase interface
    protected:
        bool SendThroughSocket(std::string &data)
        {
            bool result = mServer->Send(mBoundClient, data).Code() == SocketStatusCode::Correct;
            data = "";
            return result;
        }
    };
}
#endif // SZ_SOCKETSERVER_
