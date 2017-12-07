/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2008. All Rights Reserved.							  */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in $(WIND_BASE)/WPILib.  */
/*----------------------------------------------------------------------------*/

#ifndef LRT_CONVEYORS_H_
#define LRT_CONVEYORS_H_

#include "SpeedController.h"
#include "Relay.h"
#include "Counter.h"
#include "DigitalInput.h"
#include "LRTConnections.h" // for LRT_200X_ROBOT

class LRTConveyors {
public:
	LRTConveyors();
	virtual ~LRTConveyors();
	
	void Init();
	void SetPickup(int state);
	void SetLift(int state);
	void SetAllOff();
	void SetShooterRaw(float pwm);
	void SetShooter(float speed); // use this one for real operation
	void SetShooter(float speed, float throttle);  // for determining closed loop gain

	float ReadShooterRawSpeed();
	float ReadShooterSpeed();
	
	void ApplyOutputs();

	void Shoot(float speed);
	void PreloadBalls();
	void NotShooting();
	
	const static float kShooterHighSpeed = 1.0;
	const static float kShooterLowSpeed = 0.7;
	const static float kShooterWaterfallSpeed = 0.2362;
	const static float kShooterGain = 2.0; //experimental
	const static float kShooterReverseSpeed = -0.6;

	const static int kTimeToClearLift = (int)(0.2*50);
	const static int kTimeoutForWaitingForShooterToGetToSpeed = (int)(0.2*50);
	
protected:
	
	SpeedController *m_pickupRear;
	Relay *m_pickupFront, *m_liftRear, *m_liftFront;
	SpeedController *m_shooter;
	
	Counter *m_shooterEncoder;
	DigitalInput *m_encraw;
	
	int m_pickupOn, m_liftOn;
	float m_shooterSpeed;
	
	bool m_shooterIsClear, m_shooterAtSpeed;
	int m_shootTime;
	
	const static float kShooterMinRPS = 0.5;
#if defined(LRT_2008_ROBOT)
	const static float kShooterMaxRPS = 169.5;
#elif defined(LRT_2009_ROBOT)
	const static float kShooterMaxRPS = 29.26;
#endif
	
//	const static float kIntakeOutboardValue = 1.0;
	const static float kIntakeInboardValue = 1.0;
//	const static float kUptakeOutboardValue = 1.0;
//	const static float kUptakeInboardValue = 1.0;
	
};

#endif
