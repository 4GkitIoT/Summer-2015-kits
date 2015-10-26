/**
* Copyright (C) 2015 Orange
*
* This software is distributed under the terms and conditions of the 'Apache-2.0'
* license which can be found in the file 'LICENSE' in this package distribution
* or at 'http://www.apache.org/licenses/LICENSE-2.0'.
*/

#ifndef SZ_PERF_COUNTER_
#define SZ_PERF_COUNTER_

#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 8
    #define SZ_USE_CHRONO
    #include <chrono>
#else
    #include <sys/time.h>
#endif

#include <iostream>

namespace SZ
{
    class PerfCounter
    {
         std::string mPerfName;
#ifdef SZ_USE_CHRONO
         std::chrono::steady_clock::time_point mStart;
#else
         timeval mStart;
#endif
     protected:
         void PerfStart();
         void PerfEnd();
     public:
         PerfCounter(const std::string &name);
         ~PerfCounter();
    };
}
#endif // SZ_PERF_COUNTER_
