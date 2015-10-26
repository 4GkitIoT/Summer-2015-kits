/**
* Copyright (C) 2015 Orange
*
* This software is distributed under the terms and conditions of the 'Apache-2.0'
* license which can be found in the file 'LICENSE' in this package distribution
* or at 'http://www.apache.org/licenses/LICENSE-2.0'.
*/

#ifndef SZ_CONFIG_
#define SZ_CONFIG_

#include <iostream>
#include "LogTag.hpp"
#include "PerfCounter.hpp"

class lua_State;

namespace SZ
{
    template <typename T>
    class JSONResult;


    class Config : public PerfCounter
    {
        static Config sMe;

        std::string mBoardId;
        std::string mHost;
        unsigned short mPort;
        std::string mDirectPort;
        unsigned long mConnectionTimeout;
        std::string mModulesPath;
        std::string mModulesExt;
        LogTag mLogLevel;
        bool mColoredLogs;

        Config()
          : PerfCounter("Loading config"),
            mLogLevel(LogTag::Debug), // to ensure logging before json config is loaded
            mColoredLogs(false)
        {}
    public:
        static Config* Me();

        JSONResult<std::string>* LoadConfig(const std::string &configPath);

        const std::string& BoardId() const { return mBoardId; }
        const std::string& ConnectionHost() const { return mHost; }
        unsigned short ConnectionPort() const { return mPort; }
        const std::string& DirectConnectionPort() const { return mDirectPort; }
        unsigned long ConnectionTimeout() const { return mConnectionTimeout; }
        const std::string& ModulesPath() const { return mModulesPath; }
        const std::string& ModulesExt() const { return mModulesExt; }
        LogTag LogLevel() const { return mLogLevel; }
        bool ColoredLogs() const { return mColoredLogs; }
    };
}
#endif // SZ_CONFIG_
