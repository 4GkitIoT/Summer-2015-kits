/**
* Copyright (C) 2015 Orange
*
* This software is distributed under the terms and conditions of the 'Apache-2.0'
* license which can be found in the file 'LICENSE' in this package distribution
* or at 'http://www.apache.org/licenses/LICENSE-2.0'.
*/

#ifndef SZ_IRESULT_
#define SZ_IRESULT_

#include <iostream>

#include "Logger.hpp"

namespace SZ
{
    template <typename TCode, typename TData>
    class IResult
    {
    protected:
        TCode mCode;
        TData mData;
    public:
        IResult(TCode statusCode, TData data)
            : mCode(statusCode),
              mData(data)
        {}
        virtual ~IResult(){}

        const TCode& Code() const { return mCode; }
        const TData& Data() const { return mData; }

        bool operator==(const TCode& code) const { return mCode == code; }
        bool operator!=(const TCode& code) const { return mCode != code; }

        virtual bool Correct() const = 0;
    };

    template <typename TStream, typename TCode, typename TData>
    zstream<TStream>& operator<< (zstream<TStream>& stream, const IResult<TCode, TData> &result)
    {
        return stream << "[" << (int)result.Code() << ", " << result.Data() << "]";
    }

    template <typename TCode, typename TData>
    std::ostream& operator<< (std::ostream& stream, const IResult<TCode, TData> &result)
    {
        return stream << "[" << (int)result.Code() << ", " << result.Data() << "]";
    }
}
#endif // SZ_IRESULT_
