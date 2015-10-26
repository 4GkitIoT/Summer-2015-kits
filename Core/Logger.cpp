/**
* Copyright (C) 2015 Orange
*
* This software is distributed under the terms and conditions of the 'Apache-2.0'
* license which can be found in the file 'LICENSE' in this package distribution
* or at 'http://www.apache.org/licenses/LICENSE-2.0'.
*/

#include "Logger.hpp"

namespace SZ
{
std::ofstream _prv_logstream("sz.log");

std::string LogColor(const LogTag &tag)
{
    switch (tag)
    {
    default:
    case LogTag::Debug: return "\033[0;36;40m";
    case LogTag::Info: return "\033[0;37;40m";
    case LogTag::Warn: return "\033[1;33;40m";
    case LogTag::Error: return "\033[1;30;41m";
    }
}

std::ostream &LogStream(LogTarget target)
{
    switch (target)
    {
    default:
        // invalid target, fallback to console
    case LogTarget::Console:
        return std::cout;
    case  LogTarget::File:
        return _prv_logstream;
    }
}
}
