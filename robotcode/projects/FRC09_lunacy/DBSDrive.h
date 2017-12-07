#ifndef DBSDRIVE_H
#define DBSDRIVE_H

#include "WPILib.h"
#include "Brake.h"
#include "DriveMethod.h"

class DBSDrive : public DriveMethod {
	Brake &m_leftBrake, &m_rightBrake;
	int m_maxBraking;
	
public:
	DBSDrive(
		SpeedController &leftDrive, SpeedController &rightDrive,
		Brake &leftBrake, Brake &rightBrake, int maxBraking
	);
	
	virtual DriveOutput DoArcadeDrive(float forward, float turn);
};

#endif
