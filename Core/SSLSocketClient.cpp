/**
* Copyright (C) 2015 Orange
*
* This software is distributed under the terms and conditions of the 'Apache-2.0'
* license which can be found in the file 'LICENSE' in this package distribution
* or at 'http://www.apache.org/licenses/LICENSE-2.0'.
*/

#include "SSLSocketClient.hpp"

#include "Config.hpp"

#include <string>
#include <sstream>

namespace SZ
{

SSLSocketClient::SSLSocketClient()
  : mClient(nullptr),
    mCtx(nullptr)
{

}

SSLSocketClient::~SSLSocketClient()
{
    Close();
    ERR_free_strings();
    EVP_cleanup();
}

void SSLSocketClient::CleanBio()
{
    if (mClient != nullptr)
    {
        BIO_free_all(mClient);
        mClient = nullptr;
    }
}

void SSLSocketClient::CleanCtx()
{
    if (mCtx != nullptr)
    {
        SSL_CTX_free(mCtx);
        mCtx = nullptr;
    }
}

SocketResult<int> SSLSocketClient::Init()
{
    PerfStart();
    std::string host = SZ::Config::Me()->ConnectionHost();
    unsigned short port = SZ::Config::Me()->ConnectionPort();
//    unsigned long timeout = SZ::Config::Me()->ConnectionTimeout();

    auto path = host + ":" + std::to_string(port);
    auto pathRaw = const_cast<char*>(path.c_str());

    try
    {
        ERR_load_BIO_strings();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();
        SSL_library_init();

        mCtx = SSL_CTX_new(SSLv23_client_method());

        if(!SSL_CTX_load_verify_locations(mCtx, nullptr, TRUSTSTORE_LOCATION))
        {
            ZLOGE() << "Error loading trust store: " << ERR_error_string(ERR_get_error(), nullptr);
            CleanCtx();
            return SocketResult<int>(SocketStatusCode::SSLError, (int)ERR_get_error());
        }

        mClient = BIO_new_ssl_connect(mCtx);
        if (mClient == nullptr)
        {
            ZLOGE()  << "Error creating new connection: " << ERR_error_string(ERR_get_error(), nullptr);
            CleanCtx();
            return SocketResult<int>(SocketStatusCode::ConnectionError, (int)ERR_get_error());
        }

        SSL *ssl;
        BIO_get_ssl(mClient, &ssl);
        SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
        BIO_set_conn_hostname(mClient, pathRaw);

        if(BIO_do_connect(mClient) <= 0)
        {
            ZLOGE()  << "Error establishing connection: " << ERR_error_string(ERR_get_error(), nullptr);
            CleanBio();
            CleanCtx();
            return SocketResult<int>(SocketStatusCode::ConnectionError, (int)ERR_get_error());
        }

#ifndef _WIN32 // openssl on windows can't easily access cert store so just skip verification for debugging
        long verifyResult = SSL_get_verify_result(ssl);
        if(verifyResult != X509_V_OK)
        {

            ZLOGW() << "Certificate verification error: " << verifyResult;
            CleanBio();
            CleanCtx();
            return SocketResult<int>(SocketStatusCode::SSLError, (int)verifyResult);
        }
#endif

        PerfEnd();
        return SocketResult<int>(SocketStatusCode::Correct, 0);
    }
    catch(...)
    {
        return SocketResult<int>(SocketStatusCode::ConnectionError, -1);
    }
}

SocketResult<int> SSLSocketClient::Listen(std::function<void (std::string)> receiveCallback)
{
    ZLOGI() << "Listening...";

    const int bufferLen = 1024;
    char buffer[bufferLen];
    std::stringstream response;
    while (true)
    {
        int read = BIO_read(mClient, buffer, bufferLen - 1); // this call blocks until some data is read
        if (read < 0) // error
        {
            ZLOGE() << "receive failed";
            return SocketResult<int>(SocketStatusCode::ReceiveError, 0);
        }
        else if (read == 0) // client disconnected
        {
            ZLOGI() << "connection has closed";
            return SocketResult<int>(SocketStatusCode::Correct, 0);
        }

        response << std::string(buffer).substr(0, read);
        auto responseAsString = response.str();

        auto delimPos = responseAsString.find(MSG_DELIM);
        if (delimPos == std::string::npos)
        {
            if (responseAsString.length() > MSG_MAX_SIZE)
            {
                ZLOGW() << "message too long (delim error or ddos attempt?)";
                return SocketResult<int>(SocketStatusCode::MessageTooLongError, 0);
            }

            ZLOGD() << "delim " << std::string(MSG_DELIM) << "; " << responseAsString;
            ZLOGD() << "received incomplete message; total size: " << responseAsString.length();
            continue; // ignore this msg as it's not completed
        }

        std::string foo = MSG_DELIM;

        int partStart = 0, partEnd = 0;
        while ((partEnd = responseAsString.find(MSG_DELIM, partStart)) != std::string::npos)
        {
            receiveCallback(responseAsString.substr(partStart, partEnd - partStart));
            partStart = partEnd + foo.length();
        }

        response.str("");
        if (partStart > 0)
        {
            // return non-consumed part of message back to response buffer
            response << responseAsString.substr(partStart);
        }
    }
    return SocketResult<int>(SocketStatusCode::Correct, 0);
}

bool SSLSocketClient::SendThroughSocket(std::string &data)
{
    if (!data.empty())
    {
        ZLOGD() << "Sending to server: " << data;
        BIO_write(mClient, data.c_str(), data.length());
        data.clear();
    }
    return true;
}

void SSLSocketClient::Close()
{
    CleanBio();
    CleanCtx();
}

bool SSLSocketClient::Connected() const
{
    return mClient && mCtx;
}
}
