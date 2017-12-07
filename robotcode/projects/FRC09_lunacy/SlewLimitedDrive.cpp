#include "SlewLimitedDrive.h"
#include <cmath>

SlewLimitedDrive::SlewLimitedDrive(SpeedController *esc1, CurrentSensor *current_sense1)
	: esc(esc1), current_sense(current_sense1)
{
	m_timer.Start();
}
SlewLimitedDrive::~SlewLimitedDrive() {
}

float SlewLimitedDrive::Get() {
	return m_target;
}
void SlewLimitedDrive::Set(float value) {
	m_target = value;
}

float SlewLimitedDrive::GetMaxCurrent() {
	return m_maxCurrent;
}
void SlewLimitedDrive::SetMaxCurrent(float value) {
	m_maxCurrent = value;
}

// TODO: move this
float sign(float val) {
	if (val == 0.0)
		return 0.0;
	else if (val > 0)
		return 1.0;
	else
		return -1.0;
}

void SlewLimitedDrive::UpdateOutput() {
	double elapsed_time = m_timer.Get();
	m_timer.Reset();
	
	// FIXME: this will have issues with sudden reversals of power..?
	float output;
	float max_delta = m_maxDeltaPerSecond * elapsed_time;
	if (fabs(m_target) > fabs(m_lastOutput) + max_delta) {
		// increasing power
		if (current_sense->GetCurrent() <= m_maxCurrent)
			output = m_lastOutput + max_delta * sign(m_target);
		
		//if (current_sense->GetCurrent() <= m_maxSlipCurrent) //we are probably slipping
		//	output = 0; //stop and slew up again
		//else
		//	output = m_lastOutput + max_delta * sign(m_target);
	}
	else {
		// unlimited decrease delta
		output = m_target;
	}
	
	// TODO: incorporate current sensor data
	
	/**
	 * int m_maxSlipCurrent;
	 * int m_maxTractionCurrent;
	 * 
	 * so, if the current is under the maxSlipCurrent, then we're probably slipping
	 * if we're slipping, set the ouput to zero, then slew up again. but only if we
	 * haven't met our target speed yet. If we're at target speed, then current probably
	 * isn't at its max.
	 * 
	 * if (current_sense->GetCurrent() <= m_maxSlipCurrent)
	 * 		output = 0;
	 */
	
	esc->Set(output);
	m_lastOutput = output;
}
