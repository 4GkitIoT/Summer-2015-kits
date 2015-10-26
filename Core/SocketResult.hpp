/**
* Copyright (C) 2015 Orange
*
* This software is distributed under the terms and conditions of the 'Apache-2.0'
* license which can be found in the file 'LICENSE' in this package distribution
* or at 'http://www.apache.org/licenses/LICENSE-2.0'.
*/

#ifndef SZ_SOCKET_RESULT_
#define SZ_SOCKET_RESULT_

#include <iostream>

#include "IResult.hpp"

namespace SZ
{
    enum class SocketStatusCode
    {
        Correct,
        ConnectionError,
        ReceiveError,
        SendError,
        ServerAlreadyRunningError,
        MessageTooLongError,
        SSLError,
        SessionInvalidError
    };

    template <typename TData>
    class SocketResult : public IResult<SocketStatusCode, TData>
    {
    public:
        SocketResult(SocketStatusCode statusCode, TData data)
            : IResult<SocketStatusCode, TData>(statusCode, data)
        {}

        bool Correct() const { return IResult<SocketStatusCode, TData>::Code() == SocketStatusCode::Correct; }
    };
}
#endif // SZ_SOCKET_RESULT_
