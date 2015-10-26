/**
* Copyright (C) 2015 Orange
*
* This software is distributed under the terms and conditions of the 'Apache-2.0'
* license which can be found in the file 'LICENSE' in this package distribution
* or at 'http://www.apache.org/licenses/LICENSE-2.0'.
*/

#include "Config.hpp"

#include "LibJSON/jsonxx.h"
#include "JSONResult.hpp"

#include <fstream>

namespace SZ
{
Config Config::sMe;


Config *Config::Me()
{
    return &sMe;
}

template <typename T>
void WriteConf(jsonxx::Object obj, std::string key, T &targetVar)
{
    if (obj.has<T>(key))
    {
        targetVar = obj.get<T>(key);
        ZLOGI() << "Config loaded: " << key << " -> " << targetVar;
    }
    else
    {
        ZLOGW() << "No value defined for config key: " << key;
    }
}

void LoadLogLevel(jsonxx::Object &o, LogTag &target)
{
    std::string logLevelRaw;
    WriteConf(o, "logLevel", logLevelRaw);
    target = LogTag::Debug;
    if (!logLevelRaw.compare("info"))
    {
        target = LogTag::Info;
    }
    else if (!logLevelRaw.compare("warn"))
    {
        target = LogTag::Warn;
    }
    else if (!logLevelRaw.compare("error"))
    {
        target = LogTag::Error;
    }
    else if (!logLevelRaw.compare("debug"))
    {
        target = LogTag::Debug;
    }
    else
    {
        ZLOGW() << "Unknown value for log level: " << logLevelRaw;
    }
}

// has to be ptr to avoid cyclic deps with config->result->logger; TODO can be fixed somehow?
JSONResult<std::string>* Config::LoadConfig(const std::string &configPath)
{
    PerfStart();
    ZLOGD() << "Loading config from " << configPath;

    jsonxx::Object o;
    std::ifstream file(configPath);

    if (!file.is_open())
    {
        return new JSONResult<std::string>(JSONStatusCode::FileError, "Could not open: " + configPath);
    }

    std::ostringstream contents;
    contents << file.rdbuf();
    file.close();

    if (!o.parse(contents.str()))
    {
        return new JSONResult<std::string>(JSONStatusCode::ParseError, "Could not parse: " + configPath);
    }
    jsonxx::Number portRaw;
    jsonxx::Number connectionTimeoutRaw;

    LoadLogLevel(o, mLogLevel);
    WriteConf(o, "coloredLogs", mColoredLogs);
    WriteConf(o, "boardId", mBoardId);
    WriteConf(o, "serverHost", mHost);
    WriteConf(o, "serverPort", portRaw);
    WriteConf(o, "directPort", mDirectPort);
    WriteConf(o, "connectionTimeout", connectionTimeoutRaw);
    WriteConf(o, "modulesPath", mModulesPath);
    WriteConf(o, "modulesExt", mModulesExt);

    mPort = static_cast<unsigned short>(portRaw);
    mConnectionTimeout = static_cast<unsigned long>(connectionTimeoutRaw);

    PerfEnd();
    return new JSONResult<std::string>(JSONStatusCode::Correct, "");
}
}
