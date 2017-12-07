/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2008. All Rights Reserved.							  */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in $(WIND_BASE)/WPILib.  */
/*----------------------------------------------------------------------------*/

#ifndef TORQUE_LIMITED_MOTOR_OUTPUT_H_
#define TORQUE_LIMITED_MOTOR_OUTPUT_H_

#include "LRTEncoder.h"
#include "SpeedController.h"
#include "LRTConnections.h"
#include "Brake.h"

class TorqueLimitedMotorOutput : public SpeedController {
public:
	TorqueLimitedMotorOutput(SpeedController *esc1, LRTEncoder *encoder1, Brake *brake1);
	virtual ~TorqueLimitedMotorOutput();
	
	float Get();
	void Set(float speed);
	
	float GetMaxAccel();
	void SetMaxAccel(float value);
	float GetMaxBrake();
	void SetMaxBrake(float value);
	void SetTorqueLimitEnabled(bool value);
	
	void UpdateOutput(float maxAccelMultiplier);

	double GetRawRate();
	double GetNormalizedRate();
	
	float min(float a, float b);
	float max(float a, float b);
	
protected:
	SpeedController *m_esc;
	LRTEncoder *m_encoder;
	Brake *m_brake;
	
	static const float m_deadband = 0.04;	//deadband;  do we need this?
	float m_target;
	float m_maxAccel;
	float m_maxBrake;
	bool m_enabled;
	
	bool m_brakeToSlow;
	
	void ApplyBrake(float brakeAmount);
	
	float RemoveMechanicalDeadband(float minPWMforMotion, float unscaledPWM);
};


#endif
