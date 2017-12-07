//Author: Brandon Liu and Brian Axelrod (2010)

#include "WPILib.h"
#include "Servo.h"
#include "LRTDriveEncoders.h"
#include <string>
#include "Synchronized.h"
#include "SpeedController.h"
#include <types/vxWind.h>

class LRTGearBox 
{
public:
	enum GearBoxState {kHighGear, kNeutralGear, kLowGear};
	
	LRTGearBox(std::string gearBoxName, SpeedController& esc, int servoNum);
	~LRTGearBox();
	
	void ApplyConfig();
	void ShiftTo(GearBoxState state);
	GearBoxState GetState();
	int GetPulseCount();
	
	void SetServiceMode(bool isServiceMode);
	bool IsServiceMode();
	
	void PulseGearBox();
	
	Servo m_shifterServo;
private:
	SpeedController& m_wheelEsc;
	volatile float m_highVal, m_neutralVal, m_lowVal;
	string m_name;
	Task m_taskPulser;
	volatile GearBoxState m_state;
	
	volatile int m_pulseCount;
	volatile bool m_isServiceMode;
	SEM_ID m_semaphore;
	
	volatile float m_servoPulseTime, m_servoRestTime;
};
