/**
* Copyright (C) 2015 Orange
*
* This software is distributed under the terms and conditions of the 'Apache-2.0'
* license which can be found in the file 'LICENSE' in this package distribution
* or at 'http://www.apache.org/licenses/LICENSE-2.0'.
*/

#ifndef SZ_JSON_RESULT_
#define SZ_JSON_RESULT_

#include <iostream>

#include "IResult.hpp"

namespace SZ
{
    enum class JSONStatusCode
    {
        Correct,
        FileError,
        ParseError,
        MissingField
    };

    template <typename TData>
    class JSONResult : public IResult<JSONStatusCode, TData>
    {
    public:
        JSONResult(JSONStatusCode statusCode, TData data)
            : IResult<JSONStatusCode, TData>(statusCode, data)
        {}

        bool Correct() const { return IResult<JSONStatusCode, TData>::Code() == JSONStatusCode::Correct; }
    };

    class CoreMessageResult : public IResult<int, std::string>
    {
    public:
        CoreMessageResult(int code, std::string data)
            : IResult<int, std::string>(code, data)
        {}

        bool Correct() const { return IResult<int, std::string>::Code() == 0; }
    };
}
#endif // SZ_JSON_RESULT_
