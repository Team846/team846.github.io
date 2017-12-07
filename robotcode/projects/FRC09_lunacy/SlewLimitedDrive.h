/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2008. All Rights Reserved.							  */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in $(WIND_BASE)/WPILib.  */
/*----------------------------------------------------------------------------*/

#ifndef SLEW_LIMITED_H_
#define SLEW_LIMITED_H_

#include "CurrentSensor.h"
#include "SpeedController.h"
#include "Timer.h"

class SlewLimitedDrive {
public:
	SlewLimitedDrive(SpeedController *esc, CurrentSensor *current_sense);
	virtual ~SlewLimitedDrive();
	
	float Get();
	void Set(float speed);
	
	float GetMaxCurrent();
	void SetMaxCurrent(float value);
	
	void UpdateOutput();
	
protected:
	SpeedController *esc;
	CurrentSensor *current_sense;
	float m_maxCurrent;
	float m_maxDeltaPerSecond;
	float m_target;
	float m_lastOutput;
	Timer m_timer;
};

#endif
