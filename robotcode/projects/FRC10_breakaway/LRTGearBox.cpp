//Author: Brandon Liu and Brian Axelrod (2010)

#include "WPILib.h"
#include "Servo.h"
#include "LRTConfig.h"
#include "LRTGearBox.h"
#include "LRTDriveEncoders.h"
#include "PIDController.h"
#include "Task.h"
#include <string>
#include <sstream>
#include <types/vxTypesOld.h>
#include "Synchronized.h"

//Used as an entry point for the Task object
static void TaskEntry(UINT32 gbox) {
	LRTGearBox* gearbox = (LRTGearBox*)gbox;
	gearbox->PulseGearBox();
}

void LRTGearBox::ApplyConfig()
{
	LRTConfig &config = LRTConfig::GetInstance();
	
	std::string prefix = "LRTGearBox." + m_name + ".";
	
	m_neutralVal = config.Get<float>(prefix + "neutralVal");
	m_highVal = config.Get<float>(prefix + "highVal");
	m_lowVal =  config.Get<float>(prefix + "lowVal");
	
	prefix = "LRTGearBox.";
	
	m_servoPulseTime = config.Get<float>(prefix + "servoPulseTime");
	m_servoRestTime = config.Get<float>(prefix + "servoRestTime");
}
LRTGearBox::LRTGearBox(std::string gearBoxName, SpeedController& esc, int servoNum)
: m_shifterServo(servoNum)
, m_wheelEsc(esc)
, m_name(gearBoxName)
, m_taskPulser(("LRTGearBox." + m_name + ".taskPulser").c_str(), (FUNCPTR)TaskEntry)
, m_state(LRTGearBox::kHighGear)
, m_semaphore(semMCreate(SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE))
{
	ApplyConfig();
	m_taskPulser.Start((UINT32)this);
}

LRTGearBox::~LRTGearBox() {
	semDelete(m_semaphore);
}

void LRTGearBox::ShiftTo(GearBoxState state)
{
	{
		Synchronized sync(m_semaphore);
		m_state = state;
	}
	//this should fix the shifting mechanism but the pulser is still broken
//	switch(m_state) {
//	case kHighGear:
//		m_servo.Set(m_highVal);
//		break;
//	case kNeutralGear:
//		m_servo.Set(m_neutralVal);
//		break;
//	case kLowGear:
//		m_servo.Set(m_lowVal);
//		break;
//	}
}

LRTGearBox::GearBoxState LRTGearBox::GetState()
{
	{
		Synchronized sync(m_semaphore);
		return m_state;
	}
}

int LRTGearBox::GetPulseCount() {
	{
		Synchronized sync(m_semaphore);
		return m_pulseCount;
	}
}

void LRTGearBox::SetServiceMode(bool isServiceMode) {
	{ Synchronized sync(m_semaphore);
		m_isServiceMode = isServiceMode;
	}
}

bool LRTGearBox::IsServiceMode() {
	{ Synchronized sync(m_semaphore);
		return m_isServiceMode;
	}
}

void LRTGearBox::PulseGearBox() {
	{
		Synchronized sync(m_semaphore);
		m_pulseCount = 0;
	}
	while (true) {
		//don't pulse unless there's a signal being sent to the esc
		//but override the interlock in service mode
		if (m_isServiceMode || 
				(!m_isServiceMode && m_wheelEsc.Get() != 0)) 
		{	Synchronized sync(m_semaphore);
			m_pulseCount++;
			switch(m_state) {
			case kHighGear:
				m_shifterServo.Set(m_highVal);
				break;
			case kNeutralGear:
				m_shifterServo.Set(m_neutralVal);
				break;
			case kLowGear:
				m_shifterServo.Set(m_lowVal);
				break;
			}
		}
//		}
		float pulseTime, restTime;
		{ Synchronized s(m_semaphore);
			pulseTime = m_servoPulseTime;
			restTime = m_servoRestTime;
		}
		Wait(pulseTime);
		m_shifterServo.SetRaw(0); //set the PWM to 0, or no motor output
		Wait(restTime);
	}
}

