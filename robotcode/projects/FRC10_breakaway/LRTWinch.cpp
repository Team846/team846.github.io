//Author: Brandon Liu (2010)

#include "LRTWinch.h"
#include "LRTConnections.h"
#include "Jaguar.h"
#include "LRTDriverStationLCD.h"

LRTWinch::LRTWinch(ProxiedCANJaguar& esc) :
	m_esc(esc) 
	, m_winchState(kIdle)
{
	m_esc.ConfigNeutralMode(CANJaguar::kNeutralMode_Brake);
}

void LRTWinch::RetractWinch() {
	m_winchState = kRetracting;
}
void LRTWinch::ReleaseWinch() {
	m_winchState = kReleasing;
}

void LRTWinch::ApplyOutput() {
	LrtLcd::GetInstance().print(LrtLcd::kWinchLine,
			"Winch %.3f A", GetCurrent());
	
	switch (m_winchState) {
	case kRetracting:
		m_esc.Set(1);
		break;
	case kReleasing:
		m_esc.Set(-1);
		break;
	case kIdle:
		m_esc.Set(0);
		break;
	}
	m_winchState = kIdle; //Force driver to keep the button down.
}

float LRTWinch::GetCurrent() {
	return m_esc.GetCurrent();
}
