//Author: Brian Axelrod

#ifndef LRTAUTON_H_
#define LRTAUTON_H_

#include "WPILib.h"
#include "LRTKicker.h"
#include "LRTRoller.h"
#include "LRTConfig.h"

class LRTAuton {
private:
	int m_autonState, m_speed, m_numBallsInZone;
	float m_movementTimeBetweenKicks, m_movementTimeInCloseZone, m_waitTimeForKick;
	bool m_isInCloseZone;
	SpeedController &m_escLeft, &m_escRight;
	Task m_autonTask;
	LRTKicker &m_kicker;
	string m_prefix;
	LRTConfig &m_config;

	void AutonTask();
	static void CallAutonTask(UINT32 obj) {
		LRTAuton* object = (LRTAuton*) obj;
		object->AutonTask();
	}
	
public:
	LRTAuton(SpeedController& left, SpeedController& right, LRTKicker& kicker);
	
	void ApplyConfig();
	void StartAuton();
	void StopAuton();
	void SetIsInCloseZone( bool in );
	bool GetIsInCloseZone();
};

#endif
