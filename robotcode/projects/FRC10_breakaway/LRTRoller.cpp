//Author: David Liu (2010)

#include "LRTRoller.h"
#include "Jaguar.h"
#include "LRTConnections.h"
#include "LRTUtil.h"
#include "LRTConfig.h"

LRTRoller::LRTRoller(SpeedController& esc, LRTDriveEncoders& driveEncoders,
		LRTBallDetector& ballDetector)
: m_escRaw(esc)
, m_driveEncoders(driveEncoders)
, m_ballDetector(ballDetector)
, m_isEnabled(false)
, m_isReverse(false)
{
	ApplyConfig();
}

void LRTRoller::ApplyConfig() {
	LRTConfig &config = LRTConfig::GetInstance();
	
	string prefix("LRTRoller.");
	
	m_rollerAdjustment = config.Get<float>(prefix+"rollerAdjustment", 0.5);
	m_rollerForwardMultiplier = config.Get<float>(prefix + "rollerForwardMultiplier", 1.8);
	m_rollerReverseMultiplier = config.Get<float>(prefix + "rollerReverseMultiplier", 1.5);
}

float LRTRoller::GetRollerOutput() {
	return m_escRaw.Get();
}
void LRTRoller::SetRollerOutput(float speed) {
//	speed = -speed; // Polarity is backwards.
	m_escRaw.Set(LRTUtil::clamp<float>(speed, -1.0, 1.0));
}

void LRTRoller::SetEnabled(bool enabled) {
	m_isEnabled = enabled;
}

bool LRTRoller::GetEnabled() {
	return m_isEnabled;
}

void LRTRoller::SetReverse(bool reverse) {
	m_isReverse = reverse;
}

bool LRTRoller::IsReverse() {
	return m_isReverse;
}


float LRTRoller::ComputeSpeed() {
	float suck; //pos val is sucking ball in
	if (m_isReverse) {
		suck = -1.0;
		m_isReverse = false; //Force Driver to hold button down
	} else {
//		if (m_ballDetector.IsBallClose()) {
			float multiplier = m_driveEncoders.GetNormalizedRate() > 0 ? 
					m_rollerForwardMultiplier : m_rollerReverseMultiplier;
			suck = -multiplier * m_driveEncoders.GetNormalizedRate() + m_rollerAdjustment;
//		} else {
//			out = m_rollerAdjustment; // keep trying to suck a ball in.
//		}
	}
	return -suck; //motor is wired opposite polarity
}

void LRTRoller::ApplyOutput() {
	if (m_isEnabled) {
		SetRollerOutput(ComputeSpeed());
	}
	else {
		SetRollerOutput(0);
	}
	
}
