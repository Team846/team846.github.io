//Author: Brian Axelrod (2010)



#include "LRTDriveTrain.h"
#include "LRTConfig.h"
#include "AsynchronousPrinter.h"
#include "LRTConsole.h"
//#include "Task.h"
#include "Synchronized.h"
#ifndef _LRTPID_
#define _LRTPID_
//#include "LRTPIDController.h"
#endif
#ifndef _LRTDSC_
#define _LRTDSC_
#include "LRTDriveSpeedControllers.h"
#endif
#include "LRTUtil.h"
//#include "DBSDrive.h"
LRTDriveTrain::LRTDriveTrain(SpeedController &leftDrive, SpeedController &rightDrive,
		Brake &leftBrake, Brake &rightBrake, int maxBraking, LRTDriveEncoders* driveEncoders,
		bool isSquaredInputs)
: DBSDrive(leftDrive, rightDrive,
	leftBrake, rightBrake, maxBraking, isSquaredInputs),
	m_semaphore(semMCreate(SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE))
{
	LRTConfig &loader = LRTConfig::GetInstance();
	m_TurningCoef = loader.Get<float>("DriveTrain_TurningCoef");
	m_SpeedTrim = loader.Get<float>("DriveTrain_SpeedTrim");
	m_TurnTrim = loader.Get<float>("DriveTrain_TurningTrim");
	m_Speed = 0;
	m_TurningRate = 0;
	m_source = driveEncoders;
	m_enabled = false;
		
	m_PIDvel = new LRTPIDController(loader.Get<float>("DriveTrain_V_P"),
									loader.Get<float>("DriveTrain_V_I"),
									loader.Get<float>("DriveTrain_V_D"),
									driveEncoders, this,
									 true);
	m_PIDvel->SetSetpoint(0);
	m_PIDvel->SetInputRange(-1,1);
	m_PIDvel->SetOutputRange(-1,1);
	m_PIDvel->Enable();
	
	m_PIDturn = new LRTPIDController(loader.Get<float>("DriveTrain_T_P"),
									loader.Get<float>("DriveTrain_T_I"),
									loader.Get<float>("DriveTrain_T_D"),
									driveEncoders, this,
									false);
	m_PIDturn->SetSetpoint(0);
	m_PIDturn->SetInputRange(-1,1);
	m_PIDturn->SetOutputRange(-1,1);
	m_PIDturn->Enable();
}

void LRTDriveTrain::SetSpeedTrim(float speedtrim)
{
	m_SpeedTrim = speedtrim;
}

void LRTDriveTrain::SetTurnTrim(float turntrim)
{
	m_TurnTrim = turntrim;
}

void LRTDriveTrain::SetSpeed(float speed)
{
	m_Speed = speed;
}

void LRTDriveTrain::SetTurnRate(float turningRate)
{
	m_TurningRate = turningRate;
}

void LRTDriveTrain::SetTurningCoef(float turningCoef)
{
	m_TurningCoef = turningCoef;
}

void LRTDriveTrain::ArcadeDrive(float forward, float turn)
{
	m_Speed = forward;
	m_TurningRate = turn;
	m_PIDturn->SetSetpoint(turn);
	m_PIDvel->SetSetpoint(forward);

//	m_PIDturn->Enable();
//	m_PIDvel->Enable();
	if (!m_enabled)
	{
		m_enabled = true;
	}
//	AsynchronousPrinter::Printf("Forward target = %f, turn target = %f\n", forward, turn);
	LRTConsole &m_console = LRTConsole::GetInstance();
	m_console.PrintEverySecond("Forward target = %f, turn target = %f\n", forward, turn);
//	m_PID->SetSetpoint(m_Speed, m_TurningRate);
//	m_PIDAhead->SetSetpoint(m_Speed);

}

//this function is called from the LRTPidController
void LRTDriveTrain::UpdateOutput()
{
	Synchronized sync(m_semaphore);
	LRTConsole &m_console = LRTConsole::GetInstance();
	//const double kMinToBrake = .1;//loag from config
	double temp = m_source->GetLeftSpeed();//load this from variable later
	int brkamt = (int)(LRTUtil::sign(temp - m_vel - m_turn) *(temp - m_vel - m_turn) )*8;
	m_leftBrake.Set(brkamt);
	temp = m_source->GetRightSpeed();
	brkamt = (int)(LRTUtil::sign(temp - m_vel + m_turn) *(temp - m_vel + m_turn) )*8;
	m_rightBrake.Set(brkamt);
	m_console.PrintEverySecond("Left = %f, Right = %f\n", m_vel + m_turn, m_vel - m_turn);
	m_leftDrive.Set(LRTUtil::clamp<float>(m_vel + m_turn, -1, 1));
	m_rightDrive.Set(LRTUtil::clamp<float>(m_vel - m_turn, -1, 1));
}

void LRTDriveTrain::SetVel(float in )
{
	Synchronized sync(m_semaphore);
//	m_semaphore
	m_vel = in;
	//update everything
	UpdateOutput();
}

void LRTDriveTrain::SetTurn(float in)
{
	Synchronized sync(m_semaphore);
	m_turn = in;
	UpdateOutput();
}
