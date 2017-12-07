//Author: David Liu

#include "ClosedLoopSpeedController.h"
#include "LRTUtil.h"

ClosedLoopSpeedController::ClosedLoopSpeedController(SpeedController& esc, CounterBase& encoder, float maxRate,
		bool isClosedLoopEnabled, float p, float i, float d)
: m_esc(esc), m_encoder(encoder), m_maxRate(maxRate)
, m_isClosedLoopEnabled(isClosedLoopEnabled)
, m_controller(p,i,d, (PIDSource*)this, (PIDOutput*)this)
{
	SetClosedLoopEnabled(isClosedLoopEnabled);
	
	// range of pwm values
	m_controller.SetInputRange( -1.0, 1.0 );
}

ClosedLoopSpeedController::~ClosedLoopSpeedController() {
	
}

void ClosedLoopSpeedController::Set(float speed) {
	m_setPoint = speed;
	m_controller.SetSetpoint(m_setPoint);
	
	if (!m_isClosedLoopEnabled)
		m_esc.Set(speed);
}
float ClosedLoopSpeedController::Get() {
	return m_setPoint;
}

void ClosedLoopSpeedController::SetClosedLoopEnabled(bool enabled) {
	m_isClosedLoopEnabled = enabled;
	// this might be a double safety (two levels of enable/disable)
	if (enabled)
		m_controller.Enable();
	else
		m_controller.Disable();
}
bool ClosedLoopSpeedController::IsClosedLoopEnabled() {
	return m_isClosedLoopEnabled;
}

PIDController& ClosedLoopSpeedController::GetPIDController() {
	return m_controller;
}

void ClosedLoopSpeedController::SetPGain(float pGain) {
	m_controller.SetPID(pGain, m_controller.GetI(), m_controller.GetD());
}

float ClosedLoopSpeedController::GetMaxRate() {
	return m_maxRate;
}
void ClosedLoopSpeedController::SetMaxRate(float maxRate) {
	m_maxRate = maxRate;
}

SpeedController& ClosedLoopSpeedController::GetSpeedController() {
	return m_esc;
}
CounterBase& ClosedLoopSpeedController::GetEncoder() {
	return m_encoder;
}

void ClosedLoopSpeedController::PIDWrite(float correction) {
	// FIXME direction detection is not used... strange behavior may ensue when changing directions.
	
	if (!m_isClosedLoopEnabled)
		return;
	
	float output = m_setPoint + correction;
	
	if (output != 0 && LRTUtil::sign(output) != LRTUtil::sign(m_setPoint))
		output = 0; // don't allow power to reverse back and forth over the 0 point.
	
	m_esc.Set(output);
}
double ClosedLoopSpeedController::PIDGet() {
	return m_encoder.GetPeriod() / m_maxRate;
}
