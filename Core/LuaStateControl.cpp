/**
* Copyright (C) 2015 Orange
*
* This software is distributed under the terms and conditions of the 'Apache-2.0'
* license which can be found in the file 'LICENSE' in this package distribution
* or at 'http://www.apache.org/licenses/LICENSE-2.0'.
*/

#include "LuaStateControl.hpp"

extern "C" {
    #include <lua5.2/lua.h>
    #include <lua5.2/lauxlib.h>
    #include <lua5.2/lualib.h>
}

namespace SZ
{
LuaStateControl *LuaStateControl::mMe = nullptr;

LuaStateControl::LuaStateControl()
{
    mState = luaL_newstate();
    luaL_openlibs(mState);
}

LuaStateControl::~LuaStateControl()
{
    if (mState != nullptr)
    {
        lua_close(mState);
        mState = nullptr;
    }
}

LuaStateControl *LuaStateControl::Me()
{
    if (mMe == nullptr)
    {
        mMe = new LuaStateControl();
    }
    return mMe;
}

lua_State *LuaStateControl::LoadState()
{
    //lua_settop(mState, 0);
    return mState;
}

void LuaStateControl::RegisterFunc(const std::string &name, int (*funPtr)(lua_State *))
{
    lua_register(mState, name.c_str(), funPtr);
}

void LuaStateControl::Destroy()
{
    if (mMe != nullptr)
    {
        delete mMe;
        mMe = nullptr;
    }
}

}
