//Author: Brandon Liu (2010)

#ifndef LRT_KICKER_H_
#define LRT_KICKER_H_

#include "Relay.h"
#include "LRTBallDetector.h"
#include "WPILib.h"

class LRTKicker {
public:
	LRTKicker(LRTBallDetector& detector);
	
	void ApplyConfig();
	
	void UnwindPulse();
	void Release();
	void Stop();
	
	void Disable();
	void Enable();
	
	bool GetSense();
	int GetSenseRaw();
	
	const static Relay::Value kKickerWindup = Relay::kReverse;
	const static Relay::Value kKickerReverse = Relay::kForward;
	
protected:
	static void KickerTaskRunner(UINT32 kicker_ptr);
	void KickerTask();
	
	Task m_task;
	SEM_ID m_semaphore;
	
	LRTBallDetector& m_detector;
	Relay m_relay;
	DigitalInput m_sense;
	
	volatile enum KickerState {kReleaseAndWind, kUnwindPulse, kIdle} m_state;
	volatile double m_delayPastSense;
	volatile double m_kickerReleaseTime;
	
	const static double kKickerPollIntervalWhenActive = 0.001;
	const static double kKickerPollIntervalWhenIdle = 0.020;
	const static bool kKickerSenseDepressed_LogicLevel = false; // goes LOW when limitsw pressed
	
	const static double kKickerUnwindPulseLength = 0.010;
};

#endif /* LRT_KICKER_H_ */
