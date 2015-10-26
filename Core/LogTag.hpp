/**
* Copyright (C) 2015 Orange
*
* This software is distributed under the terms and conditions of the 'Apache-2.0'
* license which can be found in the file 'LICENSE' in this package distribution
* or at 'http://www.apache.org/licenses/LICENSE-2.0'.
*/

#ifndef SZ_LOGTAG_
#define SZ_LOGTAG_

namespace SZ
{
    enum class LogTag
    {
        Error,
        Warn,
        Info,
        Debug
    };

    enum class LogTarget
    {
        Console,
        File
    };
}
#endif // SZ_LOGTAG_
