//Author: David Liu (2009)

#ifndef DBSDRIVE_H
#define DBSDRIVE_H

#include "WPILib.h"
#include "Brake.h"
#include "DriveMethod.h"

class DBSDrive : public DriveMethod {
protected:
	Brake &m_leftBrake, &m_rightBrake;
	int m_maxBraking;
	
	const static float kJoystickDeadband = 0.02; //add to the config file when ready
	bool m_isSquaredInputs;
	
public:
	DBSDrive(
		SpeedController &leftDrive, SpeedController &rightDrive,
		Brake &leftBrake, Brake &rightBrake, int maxBraking, bool isSquaredInputs
	);
	
	void ApplyConfig();
	void SetSquaredInputsEnabled(bool enabled);
	virtual DriveOutput ComputeArcadeDrive(float forward, float turn);
};

#endif /* DBSDRIVE_H */
