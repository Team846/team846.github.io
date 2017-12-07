#ifndef LRT_UTIL_H_
#define LRT_UTIL_H_

#include "../General.h"
#include <string>
#include <math.h>
#include <sstream>

class Util
{
public:
    template <typename T> static T Clamp(T val, T min, T max)
    {
        if(val > max)
            return max;
        else if(val < min)
            return min;
        else
            return val;
    }

    template <typename T> static T Rescale(T val, T min0, T max0, T min1,
            T max1)
    {
        if(max0 == min0)
            return min1;
        val = Clamp<T>(val, min0, max0);
        return (val - min0) * (max1 - min1) / (max0 - min0) + min1;
    }

    template <typename T> static std::string ToString(T val)
    {
        std::stringstream ss;
        ss << val;
        return ss.str();
    }

    template <typename T> static int Sign(T val)
    {
        if(val > 0)
            return 1;
        else if(val < 0)
            return -1;
        return 0;
    }

    template <typename T> static T Abs(T val)
    {
        if(val < 0)
            return -val;
        return val;
    }

    template <typename T> static void MinMaxMean(T val[], int n, T* minOut,
            T* maxOut, T* meanOut)
    {
        if(n == 0)
        {
            *minOut = 0;
            *maxOut = 0;
            *meanOut = 0;
            return;
        }

        T min = val[0], max = val[0];
        T mean = 0;
        for(int i = 0; i < n; ++i)
        {
            mean += val[i];
            if(val[i] < min)
                min = val[i];
            if(val[i] > max)
                max = val[i];
        }
        mean /= n;

        *minOut = min;
        *maxOut = max;
        *meanOut = mean;
    }

    template<typename T> static T AddDeadband(T raw, T deadbandSize)
    {
        if(Util::Abs<T>(raw) < deadbandSize)
            return 0;
        return Util::Sign<T>(raw)
                * Util::Rescale<T>(Util::Abs<T>(raw), deadbandSize, 1, 0, 1);
    }

    template<typename T>
    static T ValWithAbsMax(T val1, T val2)
    {
        if(Util::Abs<T>(val1) > Util::Abs<T>(val2))
            return val1;
        return val2;
    }

    template<typename T>
    static T Max(T val1, T val2)
    {
        if(val1 > val2)
            return val1;
        return val2;
    }

    template<typename T>
    static T Min(T val1, T val2)
    {
        if(val1 < val2)
            return val1;
        return val2;
    }

    template <typename T>
    static T PowPreseveSign(T val, int power)
    {
        T result = pow(val, power);
        if(Sign<T>(val) == Sign<T>(result))
            return result;
        else
            return -result;
    }

    static void Die();
    static void Die(const char* message);
    static bool Assert(bool test, const char* message);
};

#endif
