/**
* Copyright (C) 2015 Orange
*
* This software is distributed under the terms and conditions of the 'Apache-2.0'
* license which can be found in the file 'LICENSE' in this package distribution
* or at 'http://www.apache.org/licenses/LICENSE-2.0'.
*/

#include "JSONMessage.hpp"

#include "LibBase64/base64.h"
#include "LibJSON/jsonxx.h"

namespace SZ
{

JSONResult<std::string> JSONServerMessage::Decode(const std::string &msg)
{
    jsonxx::Object obj;
    if (!obj.parse(msg))
    {
        return JSONResult<std::string>(JSONStatusCode::ParseError, ""); // TODO error message?
    }

    if (!obj.has<std::string>("module"))
    {
        return JSONResult<std::string>(JSONStatusCode::MissingField, "Module name not specified in received message");
    }
    mModuleName = obj.get<std::string>("module");

    if (!obj.has<std::string>("content"))
    {
        return JSONResult<std::string>(JSONStatusCode::MissingField, "Content not specified in received message");
    }
    std::string rawContent = obj.get<std::string>("content");

    mContent = base64_decode(rawContent);

    if (obj.has<std::string>("session"))
    {
        mSessionId = obj.get<std::string>("session");
    }

    return JSONResult<std::string>(JSONStatusCode::Correct, "");
}

void JSONClientMessage::InitCommon(const std::string &module, const std::string &boardId, const std::string &status, const std::string &sessionId)
{
    mModuleName = module;
    mBoardId = boardId;
    mStatus = status;
    mTimestamp = time(nullptr); // TODO is that enough or std::chrono would be better?
    mSessionId = sessionId;
}

void JSONClientMessage::Init(const std::string &module, const std::string &content, const std::string &boardId, const std::string &status, const std::string &sessionId)
{
    InitCommon(module, boardId, status, sessionId);
    mContent = content;
}

std::string JSONClientMessage::Create()
{
    // TODO errors?
    jsonxx::Object root;
    root << "boardId" << mBoardId;
    root << "module" << mModuleName;
    root << "time" << mTimestamp;
    root << "content" << base64_encode(reinterpret_cast<const unsigned char*>(mContent.c_str()), mContent.length());
    root << "status" << mStatus;
    root << "session" << mSessionId;

    return root.json();
}

}
