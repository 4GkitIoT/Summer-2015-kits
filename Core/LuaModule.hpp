/**
* Copyright (C) 2015 Orange
*
* This software is distributed under the terms and conditions of the 'Apache-2.0'
* license which can be found in the file 'LICENSE' in this package distribution
* or at 'http://www.apache.org/licenses/LICENSE-2.0'.
*/

#ifndef SZ_LUA_MODULE_
#define SZ_LUA_MODULE_

#include "LuaResult.hpp"
#include <iostream>

#include "PerfCounter.hpp"

class lua_State;

namespace SZ
{
    class LuaModule : public PerfCounter
    {
        lua_State *mState;
        std::string mName;
        std::string mMethod;

        LuaResult<int> OpenModule();
        void ResolveModuleName(const std::string &moduleName);
    public:
        // default hardcoded lua method name
        static constexpr auto CMD_ONMESSAGE = "onmessage";
        // hardcoded  modules that core sends requests with
        static constexpr auto MODULE_INIT = "init";
        static constexpr auto MODULE_STATUS = "status";
        static constexpr auto MODULE_MODEM_CONN = "modemReconnect";

        LuaModule();
        ~LuaModule();
        LuaResult<int> Init(const std::string &moduleName);
        LuaResult<std::string> Call(const std::string &command, const std::string &sessionId);

        static std::string LoadRawStringParam(lua_State *state, int num);
    };
}
#endif // SZ_LUA_MODULE_
