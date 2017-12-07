#ifndef LRT_UTIL_H_
#define LRT_UTIL_H_

class LRTUtil {
public:
	static float clamp(float val, float min, float max) {
		if (val > max)
			return max;
		else if (val < min)
			return min;
		else
			return val;
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
};

#endif
