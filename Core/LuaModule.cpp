/**
* Copyright (C) 2015 Orange
*
* This software is distributed under the terms and conditions of the 'Apache-2.0'
* license which can be found in the file 'LICENSE' in this package distribution
* or at 'http://www.apache.org/licenses/LICENSE-2.0'.
*/

#include "LuaModule.hpp"

#include "LuaStateControl.hpp"
#include "Config.hpp"

extern "C" {
    #include <lua5.2/lua.h> 
    #include <lua5.2/lauxlib.h> 
    #include <lua5.2/lualib.h> 
}

namespace SZ
{

LuaModule::LuaModule()
    : PerfCounter("Lua handling"),
      mState(nullptr)
{
}

LuaModule::~LuaModule()
{
//    if (mState)
//    {
//        lua_close(mState);
//    }
}

LuaResult<int> LuaModule::OpenModule()
{
    std::string filename = Config::Me()->ModulesPath();
    filename.append(mName);
    filename.append(Config::Me()->ModulesExt());
    ZLOGD() << "Opening module: " << filename;
    int result = luaL_loadfile(mState, filename.c_str());
    if (result != 0)
    {
        ZLOGD() << "Lua init error: " << lua_tostring(mState, -1);
        return LuaResult<int>(LuaStatusCode::ModuleError, result);
    }
    result = lua_pcall(mState, 0, 0, 0); // priming
    if (result != 0)
    {
        ZLOGD() << "Lua priming error: " << lua_tostring(mState, -1);
    }
    return LuaResult<int>(result == 0 ? LuaStatusCode::Correct : LuaStatusCode::PrimingCallError, result);
}

void LuaModule::ResolveModuleName(const std::string &moduleName)
{
    auto separatorPos = moduleName.find(";");
    if (separatorPos == std::string::npos)
    {
        mName = moduleName;
        mMethod = CMD_ONMESSAGE;
    }
    else
    {
        mName = moduleName.substr(0, separatorPos);
        mMethod = moduleName.substr(separatorPos + 1);
    }
}

LuaResult<int> LuaModule::Init(const std::string &moduleName)
{
    mState = LuaStateControl::Me()->LoadState();
    ResolveModuleName(moduleName);
    return OpenModule();
}

LuaResult<std::string> LuaModule::Call(const std::string &command, const std::string &sessionId)
{
    try
    {
        PerfStart();
        lua_getglobal(mState, mMethod.c_str());
        lua_pushstring(mState, command.c_str());
        lua_pushstring(mState, sessionId.c_str());
        int callResult = lua_pcall(mState, 2, 1, 0);
        if (callResult != 0)
        {
            return LuaResult<std::string>(LuaStatusCode::CallError, lua_tostring(mState, -1));
        }
        std::string result = lua_tostring(mState, -1);
        PerfEnd();
        return LuaResult<std::string>(LuaStatusCode::Correct, result);
    }
    catch (...)
    {
        PerfEnd();
        return LuaResult<std::string>(LuaStatusCode::OtherError, "");
    }
}

std::string LuaModule::LoadRawStringParam(lua_State *state, int num)
{
    // error checking etc
    return lua_tostring(state, num);
}
}
