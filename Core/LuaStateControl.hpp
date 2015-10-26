/**
* Copyright (C) 2015 Orange
*
* This software is distributed under the terms and conditions of the 'Apache-2.0'
* license which can be found in the file 'LICENSE' in this package distribution
* or at 'http://www.apache.org/licenses/LICENSE-2.0'.
*/

#ifndef SZ_LUA_STATE_CONTROL_
#define SZ_LUA_STATE_CONTROL_

#include "LuaResult.hpp"
#include <iostream>

class lua_State;

namespace SZ
{

    class LuaStateControl
    {
        static LuaStateControl* mMe;

        lua_State *mState;

        LuaStateControl();
        ~LuaStateControl();
    public:
        static LuaStateControl* Me();
        static void Destroy();

        lua_State* LoadState();
        void RegisterFunc(const std::string &name, int (*funPtr)(lua_State*));
    };
}
#endif // SZ_LUA_STATE_CONTROL_
