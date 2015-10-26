/**
* Copyright (C) 2015 Orange
*
* This software is distributed under the terms and conditions of the 'Apache-2.0'
* license which can be found in the file 'LICENSE' in this package distribution
* or at 'http://www.apache.org/licenses/LICENSE-2.0'.
*/

#include "SocketBase.hpp"

#include "Config.hpp"

#include <sstream>

namespace SZ
{

SocketBase::SocketBase()
  : PerfCounter("Establishing connection")
{

}

SocketBase::~SocketBase()
{
}

SocketResult<int> SocketBase::Send(std::string data)
{
    static auto delim = std::string(MSG_DELIM);
    if (!data.empty())
    {
        data.append(delim);
        while (!data.empty())
        {
            if (!SendThroughSocket(data))
            {
                ZLOGW() << "Failed to send message: " << data;
                return SocketResult<int>(SocketStatusCode::SendError, 0);
            }
        }
    }
    return SocketResult<int>(SocketStatusCode::Correct, 0);
}

std::string SocketBase::TrimResponse(const std::string &response)
{
    auto first = response.find_first_not_of(" \r\n\t");
    auto last = response.find_last_not_of(" \r\n\t");
    if (first == std::string::npos)
    {
        first = 0;
    }
    if (last == std::string::npos)
    {
        last = response.length();
    }
    return response.substr(first, last - first + 1);
}
}
