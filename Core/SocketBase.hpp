/**
* Copyright (C) 2015 Orange
*
* This software is distributed under the terms and conditions of the 'Apache-2.0'
* license which can be found in the file 'LICENSE' in this package distribution
* or at 'http://www.apache.org/licenses/LICENSE-2.0'.
*/

#ifndef SZ_SOCKET_BASE_
#define SZ_SOCKET_BASE_

#include <iostream>
#include <functional>

#include "SocketResult.hpp"
#include "PerfCounter.hpp"

namespace SZ
{
    class SocketBase : public PerfCounter
    {
    protected:
        static constexpr auto MSG_DELIM = "\x1E\x1E\x1E";
        static constexpr auto MSG_MAX_SIZE = 0x2fffff;

        std::string TrimResponse(const std::string &response);
        virtual bool SendThroughSocket(std::string &data) = 0;
    public:
        SocketBase();
        virtual ~SocketBase();
        virtual SocketResult<int> Init() = 0;
        virtual SocketResult<int> Listen(std::function<void(std::string)> receiveCallback) = 0;
        SocketResult<int> Send(std::string data);

        virtual void Close() = 0;
        virtual bool Connected() const = 0;
    };
}
#endif // SZ_SOCKET_BASE_
