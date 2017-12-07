#ifndef ARCADEDRIVE_H
#define ARCADEDRIVE_H

#include "WPILib.h"
#include "Brake.h"
#include "DriveMethod.h"

class SimpleArcadeDrive : public DriveMethod {
	bool m_squaredInputs;
public:
	SimpleArcadeDrive::SimpleArcadeDrive(
		SpeedController &leftDrive, SpeedController &rightDrive,
		bool squared
	)
	: DriveMethod(leftDrive, rightDrive), m_squaredInputs(squared)
	{
	}
		
	void setSquaredInputs(bool squared) {
		m_squaredInputs = squared;
	}
	virtual DriveOutput ComputeArcadeDrive(float forward, float turn);
};

#endif
