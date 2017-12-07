//Author: David Liu (2010)

#ifndef LRTROLLER_H_
#define LRTROLLER_H_

#include "LRTConnections.h"
#include "LRTDriveEncoders.h"
#include "Relay.h"
#include "Counter.h"
#include "DigitalInput.h"
#include "ClosedLoopSpeedController.h"
#include "SafeCANJaguar.h"
#include "LRTBallDetector.h"

class LRTRoller {
public:
	LRTRoller(SpeedController& esc, LRTDriveEncoders& driveEncoders, LRTBallDetector& detector);
	
	void ApplyConfig();
	
	void Init();
	float GetRollerOutput();
	void SetRollerOutput(float speed);

	void SetEnabled(bool enabled);
	bool GetEnabled();
	
	void SetReverse(bool reverse);
	bool IsReverse();

	float ComputeSpeed();
	void ApplyOutput();
protected:
	
	SpeedController& m_escRaw;
	LRTDriveEncoders& m_driveEncoders;
	LRTBallDetector& m_ballDetector;
	float m_rollerAdjustment;
	float m_rollerForwardMultiplier;
	float m_rollerReverseMultiplier;
	
	bool m_isEnabled;
	bool m_isReverse;
};

#endif // LRTROLLER_H_
