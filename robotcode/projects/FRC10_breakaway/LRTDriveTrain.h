//Author: Brian Axelrod (2010)

#include "DBSDrive.h"
//#include "PIDController.h"
#include "LRTConfig.h"
#ifndef _LRTPID_
#define _LRTPID_
#include "LRTPIDController.h"
#endif
/*#ifndef _LRTDSC_
#define _LRTDSC_
#include "LRTDriveSpeedControllers.h"
#endif*/

//Author: Brian Axelrod

class LRTDriveTrain : public DBSDrive //remove now that I am actually not using this (the subclassing)
{
private:
	float m_Speed;
	float m_TurningRate;
	float m_TurningCoef;
	float m_SpeedTrim;
	float m_TurnTrim;
	bool m_enabled;
	float m_vel, m_turn;
	/*double m_forwardSpeed, m_turningSpeed;
	double m_leftSpeed, m_rightSpeed;*/
	//this data will be stored when the proper functions are called
	SEM_ID m_semaphore;
	
	LRTPIDController *m_PIDvel, *m_PIDturn;//cannot use as instance
	LRTDriveEncoders *m_source;
	void UpdateOutput();
public:
	LRTDriveTrain(SpeedController &leftDrive, SpeedController &rightDrive,
			Brake &leftBrake, Brake &rightBrake, int maxBraking, LRTDriveEncoders* driveEncoders, bool isSquaredInputs);
//			LRTDriveSpeedControllers* driveControllers);//TODO get rid of duplicate
	void SetSpeedTrim(float speedtrim);
	void SetTurnTrim(float turntrim);
	void SetSpeed(float speed);
	void SetTurnRate(float turningRate);
	void SetTurningCoef(float turningCoef);
	void ArcadeDrive(float forward, float turn);
	void SetSpeedValues(float vel, float turn);
	void SetVel(float in);
	void SetTurn(float in);
};
