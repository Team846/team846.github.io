// Author: Karthik Viswanathan (2010)

#ifndef LRT_BALL_DETECTOR_H_
#define LRT_BALL_DETECTOR_H_

#include "LRTConnections.h"
#include "LRTUtil.h"

class LRTBallDetector {
private:
	AnalogChannel m_analog;
	int m_threshold;
	
public:
	LRTBallDetector();
	virtual ~LRTBallDetector();
	
	void ApplyConfig();
	INT32 GetThreshold();
	INT32 GetValue();
	bool IsBallClose();
};
#endif /* LRT_BALL_DETECTOR_H_ */
