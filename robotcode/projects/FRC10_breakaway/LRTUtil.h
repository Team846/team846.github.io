//Author: David Liu (2009,2010)

#ifndef LRT_UTIL_H_
#define LRT_UTIL_H_

#include <string>
#include <sstream>

class LRTUtil {
public:
	template <typename T>
	static T clamp(T val, T min, T max) {
		if (val > max)
			return max;
		else if (val < min)
			return min;
		else
			return val;
	}
	
	template <typename T>
	static T rescale(T val, T min0, T max0, T min1, T max1) {
		if (max0 == min0)
			return min1;
		val = clamp<T>(val, min0, max0);
		return (val - min0) / (max0 - min0) * (max1 - min1) + min1;
	}

	
	template <typename T>
	static std::string toString(T val) {
		std::stringstream ss;
		ss << val;
		return ss.str();
	}
	
	static double set0ifInf(double val)
	{
		if(val > 1e6)
			return 0;
		else if (val < -1e6)
			return 0;
		else
			return val;
	}
	

	template <typename T>
	static int sign (T val)
	{
		if (val > 0)
			return 1;
		else if (val < 0)
			return -1;
		return 0;
	}
	
	template <typename T>
	static T abs( T val )
	{
		if( val < 0 )
			return -val;
		return val;
	}

	template <typename T>
	static void minMaxMean( T val[], int n, T& minOut, T& maxOut, T& meanOut )
	{
		if (n == 0) {
			minOut = 0; maxOut = 0; meanOut = 0; return;
		}

		T min = val[0], max = val[0];
		T mean = 0;
		for (int i = 0; i < n; ++i) {
			mean += val[i];
			if (val[i] < min) min = val[i];
			if (val[i] > max) max = val[i];
		}
		mean /= n;

		minOut = min;
		maxOut = max;
		meanOut = mean;
	}
	

	template<typename T>
	static T addDeadband(T raw, T deadbandSize) {
		if (LRTUtil::abs<T>(raw) < deadbandSize) {
			return 0;
		}
		return LRTUtil::sign<T>(raw)
			* LRTUtil::rescale<T>(LRTUtil::abs<T>(raw), deadbandSize, 1, 0, 1);
	}
	
	template<typename T>
	static T absMax(T val1, T val2) {
		if( LRTUtil::abs<T>( val1 ) > LRTUtil::abs<T>( val2 ) )
			return val1;
		return val2;
	}
};

#endif
