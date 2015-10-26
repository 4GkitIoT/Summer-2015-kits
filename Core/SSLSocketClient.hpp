/**
* Copyright (C) 2015 Orange
*
* This software is distributed under the terms and conditions of the 'Apache-2.0'
* license which can be found in the file 'LICENSE' in this package distribution
* or at 'http://www.apache.org/licenses/LICENSE-2.0'.
*/

#ifndef SZ_SSLSOCKET_CLIENT_
#define SZ_SSLSOCKET_CLIENT_

#include "SocketBase.hpp"
#include "SocketResult.hpp"
#include "PerfCounter.hpp"

#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>

namespace SZ
{
    class SSLSocketClient : public SocketBase
    {
        static constexpr auto TRUSTSTORE_LOCATION = "/etc/ssl/certs";
//        static constexpr auto TRUSTSTORE_LOCATION = "./truststore";
        BIO *mClient;
        SSL_CTX * mCtx;

        void CleanBio();
        void CleanCtx();
    public:
        SSLSocketClient();
        ~SSLSocketClient();
        SocketResult<int> Init() ;
        SocketResult<int> Listen(std::function<void(std::string)> receiveCallback);
//        SocketResult<int> Send(std::string data);
        bool SendThroughSocket(std::string &data) ;

        void Close() ;
        bool Connected() const ;

    };
}
#endif // SZ_SSLSOCKET_CLIENT_
