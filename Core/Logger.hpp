/**
* Copyright (C) 2015 Orange
*
* This software is distributed under the terms and conditions of the 'Apache-2.0'
* license which can be found in the file 'LICENSE' in this package distribution
* or at 'http://www.apache.org/licenses/LICENSE-2.0'.
*/

#ifndef SZ_LOGGER_
#define SZ_LOGGER_

#include <sstream>
#include <iostream>
#include <fstream>

#include "LogTag.hpp"
#include "Config.hpp"

namespace SZ
{
    template <typename TStream>
    class zstream
    {
        TStream &s;
        LogTag tag;
        std::string clr;

        bool CanLog() const { return Config::Me()->LogLevel() >= tag; }
        bool IsColored() const { return Config::Me()->ColoredLogs(); }
    public:
        zstream(TStream &str, LogTag tag_, std::string clr_)
          : s(str),
            tag(tag_),
            clr(clr_)
        {
        }

        template <typename T>
        zstream<TStream>& operator<<(const T &x)
        {
            if (CanLog())
            {
                bool colored = IsColored();
                if (colored)
                {
                    s << clr;
                }
                s << x;
                if (colored)
                {
                    s << "\033[m";
                }
            }
            return *this;
        }
    };

    template <typename streamType, LogTag tag, LogTarget target>
    class ZLog
    {
        static zstream<streamType> str;

        static std::string TagToString()
        {
            switch (tag)
            {
            case LogTag::Debug: return "debug";
            case LogTag::Info: return "info ";
            case LogTag::Warn: return "warn ";
            case LogTag::Error: return "error";
            default: return "unknown";
            }
        }

    public:
        static zstream<streamType>& S()
        {
            str << "\n[" << TagToString() << "] ";
            return str;
        }

        static zstream<streamType>& S_(const char *file, int line)
        {
            str << "\n[" << TagToString() << " " << file << "@" << line << "] ";
            return str;
        }
    };

    std::string LogColor(const LogTag &tag);
    std::ostream &LogStream(LogTarget target);

    template <typename streamType, LogTag tag, LogTarget target>
    zstream<streamType> ZLog<streamType, tag, target>::str(LogStream(target), tag, LogColor(tag));

}

#define ZLOGTARGET SZ::LogTarget::Console

#define ZLOGX(tag) (SZ::ZLog<std::ostream, tag, ZLOGTARGET>::S_(__FILE__, __LINE__))
#define ZLOGD() ZLOGX(SZ::LogTag::Debug)
#define ZLOGI() ZLOGX(SZ::LogTag::Info)
#define ZLOGW() ZLOGX(SZ::LogTag::Warn)
#define ZLOGE() ZLOGX(SZ::LogTag::Error)

#endif // SZ_LOGGER_
