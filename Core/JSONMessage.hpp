/**
* Copyright (C) 2015 Orange
*
* This software is distributed under the terms and conditions of the 'Apache-2.0'
* license which can be found in the file 'LICENSE' in this package distribution
* or at 'http://www.apache.org/licenses/LICENSE-2.0'.
*/

#ifndef SZ_JSON_MESSAGE_
#define SZ_JSON_MESSAGE_

#include <iostream>
#include <ctime>

#include "JSONResult.hpp"

namespace SZ
{
class JSONMessage
{
protected:
    std::string mModuleName;
    std::string mContent;
    std::string mSessionId;

public:
    JSONMessage() {}

    const std::string& Content() const { return mContent; }
    const std::string& Module() const { return mModuleName; }
    const std::string& SessionId() const { return mSessionId; }
};

class JSONServerMessage : public JSONMessage
{
public:
    JSONServerMessage() {}
    JSONResult<std::string> Decode(const std::string &msg);
    void InitManually(std::string module, std::string content)
    {
        mModuleName = module;
        mContent = content;
    }
};

class JSONClientMessage : public JSONMessage
{
    std::string mBoardId;
    long mTimestamp;
    std::string mStatus;

    void InitCommon(const std::string &module, const std::string &boardId, const std::string &status, const std::string &sessionId);
public:
    JSONClientMessage() {}
    void Init(const std::string &module, const std::string &content, const std::string &boardId, const std::string &status, const std::string &sessionId);

    template <typename TCode, typename TData>
    void InitCoreMessage(const std::string &boardId, const IResult<TCode, TData> &result, const std::string &extraMsg = std::string(), const std::string &session = std::string())
    {
        std::string status = extraMsg.length() ? extraMsg : "ERROR"; // TODO tbd what status should contain
        InitCommon("core", boardId, status, session);
        std::stringstream ss;
        ss << result;
        mContent = ss.str();
    }

    std::string Create();
};
}

#endif // SZ_JSON_MESSAGE_
