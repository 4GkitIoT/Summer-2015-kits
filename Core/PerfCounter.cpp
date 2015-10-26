/**
* Copyright (C) 2015 Orange
*
* This software is distributed under the terms and conditions of the 'Apache-2.0'
* license which can be found in the file 'LICENSE' in this package distribution
* or at 'http://www.apache.org/licenses/LICENSE-2.0'.
*/

#include "PerfCounter.hpp"

#include "Logger.hpp"

namespace SZ
{

void PerfCounter::PerfStart()
{
#ifdef SZ_USE_CHRONO
    mStart = std::chrono::steady_clock::now();
#else
    gettimeofday(&mStart, nullptr);
#endif
}

void PerfCounter::PerfEnd()
{
#ifdef SZ_USE_CHRONO
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - mStart).count();
#else
    timeval end;
    gettimeofday(&end, nullptr);
    auto millis = (end.tv_sec * 1000 + end.tv_usec / 1000) - (mStart.tv_sec * 1000 + mStart.tv_usec / 1000);
#endif
    ZLOGD() << mPerfName << " took: " << millis << "ms";

}
PerfCounter::PerfCounter(const std::string &name)
    : mPerfName(name)
{

}

PerfCounter::~PerfCounter()
{
}
}
