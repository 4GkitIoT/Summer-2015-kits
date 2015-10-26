/**
* Copyright (C) 2015 Orange
*
* This software is distributed under the terms and conditions of the 'Apache-2.0'
* license which can be found in the file 'LICENSE' in this package distribution
* or at 'http://www.apache.org/licenses/LICENSE-2.0'.
*/

#include "CoreMessageHandler.hpp"

#include "JSONMessage.hpp"
#include "main.hpp"

namespace SZ
{
bool CoreMessageHandler::CanHandle(const JSONMessage *msg)
{
    return msg && msg->Module().find("core:") != std::string::npos;
}

CoreMessageResult CoreMessageHandler::Handle(App *app, const JSONMessage *msg)
{
    auto &type = msg->Module();
    if (!type.compare("core:openSocket"))
    {
        auto res = app->OpenListeningSocket(msg->SessionId());
        return CoreMessageResult(res.empty() ? -2 : 0, res);
    }

    return CoreMessageResult(-1, "Unhandled message type");
}
}
