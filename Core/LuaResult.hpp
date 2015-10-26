/**
* Copyright (C) 2015 Orange
*
* This software is distributed under the terms and conditions of the 'Apache-2.0'
* license which can be found in the file 'LICENSE' in this package distribution
* or at 'http://www.apache.org/licenses/LICENSE-2.0'.
*/

#ifndef SZ_LUA_RESULT_
#define SZ_LUA_RESULT_

#include <iostream>

#include "IResult.hpp"

namespace SZ
{
    enum class LuaStatusCode
    {
        Correct,
        ModuleError,
        PrimingCallError,
        CallError,
        OtherError
    };

    template <typename TData>
    class LuaResult : public IResult<LuaStatusCode, TData>
    {
    public:
        LuaResult(LuaStatusCode statusCode, TData data)
            : IResult<LuaStatusCode, TData>(statusCode, data)
        {}

        bool Correct() const { return IResult<LuaStatusCode, TData>::Code() == LuaStatusCode::Correct; }
    };
}
#endif // SZ_LUA_RESULT_
