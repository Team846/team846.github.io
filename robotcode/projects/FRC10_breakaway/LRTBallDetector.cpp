// Author: Karthik Viswanathan (2010)

#include "WPILib.h"
#include "LRTBallDetector.h"
#include "LRTConfig.h"
#include "LRTDriverStationLCD.h"

LRTBallDetector::LRTBallDetector() :
	m_analog(LRTConnections::kAnalogBallProximity) {
	ApplyConfig();
}

LRTBallDetector::~LRTBallDetector() {
}

void LRTBallDetector::ApplyConfig() {
	LRTConfig &config = LRTConfig::GetInstance();
	m_threshold = config.Get<int>("LRTBallDetector.threshold");
}

INT32 LRTBallDetector::GetValue() {
	return m_analog.GetAverageValue();
}

INT32 LRTBallDetector::GetThreshold() {
	return m_threshold;
}

bool LRTBallDetector::IsBallClose() {
	return GetValue() > m_threshold;
}
