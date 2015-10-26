/**
* Copyright (C) 2015 Orange
*
* This software is distributed under the terms and conditions of the 'Apache-2.0'
* license which can be found in the file 'LICENSE' in this package distribution
* or at 'http://www.apache.org/licenses/LICENSE-2.0'.
*/

#ifndef SZ_COREMESSAGEHANDLER_
#define SZ_COREMESSAGEHANDLER_

#include "JSONResult.hpp"

namespace SZ
{
    class JSONMessage;
    class App;

    class CoreMessageHandler
    {
    public:
        bool CanHandle(const JSONMessage *msg);
        CoreMessageResult Handle(App *app, const JSONMessage *msg);
    };
}
#endif // SZ_COREMESSAGEHANDLER_
