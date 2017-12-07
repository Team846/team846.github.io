#include "TorqueLimitedMotorOutput.h"
#include <cmath>
#include "util.h"

TorqueLimitedMotorOutput::TorqueLimitedMotorOutput(SpeedController *esc1, LRTEncoder *encoder1, Brake *brake1)
	: m_esc(esc1), m_encoder(encoder1), m_brake(brake1)
{
	m_target = 0;
	m_maxAccel = 0.10;
	m_maxBrake = 0.10;
	m_enabled = true;
	
	m_brakeToSlow = false;
}
TorqueLimitedMotorOutput::~TorqueLimitedMotorOutput() {
}

float TorqueLimitedMotorOutput::Get() {
	return m_target;
}
void TorqueLimitedMotorOutput::Set(float value) {
	m_target = value;
}

float TorqueLimitedMotorOutput::min(float a, float b) {
	return a < b ? a : b;
}

float TorqueLimitedMotorOutput::max(float a, float b) {
	return a > b ? a : b;
}

double TorqueLimitedMotorOutput::GetRawRate() {
	return m_encoder->GetRate();
}
double TorqueLimitedMotorOutput::GetNormalizedRate() {
	return m_encoder->GetNormalizedRate();
}


const static float kDeadBand = 4.0/128;
const static float kMinDutyCycle = 18.0/128; // intentionally higher
//const static float kMinDutyCycle = 16.0/128; // in order to start moving
// for turning in place, +-26.0/128 is the minimum to move

const static float kOverspeedTolerance = 0.00;
//const static float kOverspeedTolerance = 0.05;


float TorqueLimitedMotorOutput::RemoveMechanicalDeadband(float minPWMforMotion, float unscaledPWM) {
	bool isNegative = (unscaledPWM < 0);
	if (isNegative) {
		unscaledPWM = -unscaledPWM;
	}
	
	if (unscaledPWM < kDeadBand)
		return 0.0;
	
	float scaledPWM = minPWMforMotion + unscaledPWM*(1-minPWMforMotion);
	return (isNegative ? -scaledPWM : scaledPWM);
}

void TorqueLimitedMotorOutput::UpdateOutput(float maxAccelMultiplier) {
	const float boostedMaxAccel = m_maxAccel*maxAccelMultiplier;
	
	m_brake->SetCoast();
	if (!m_enabled) {
		m_esc->Set(m_target);
		return;
	}
	
	double wheel_speed = GetNormalizedRate();
	double setPoint = m_target;	//don't want to modify m_target.
	
	/*
	 * To reduce # of test cases,
	 * Reverse sense if wheel_speed is going backwards, and correct at end of routine
	 * Make Wheel_speed positive, but m_target may be positive or negative cases.
	 */
	
	bool runningBackwards = (wheel_speed<0);
	if (runningBackwards)
	{
		wheel_speed = -wheel_speed;	//wheel_speed is now positive definite.
		setPoint = -setPoint;
	}
	
	
	float output = setPoint;	//default condition.
	float error = setPoint - wheel_speed;
	
	// We normally just compare error to 0.0,
	// but this reduces jitter because it doesn't pulse
	// power on and off when wheel_speed ~= setPoint.
	if (error > -kOverspeedTolerance)	//Need to accelerate; limit traction as needed.
	{
		float maxOutput = wheel_speed + boostedMaxAccel; //max we may apply without slipping
		if (output > maxOutput) { //Check if Traction Limit needed
			output = maxOutput;
//			printf("TorqueLimited\n"); //print/display diagnostic -- should indicate on an LED or LCD [dg]
		}
	} else {
		if (setPoint >= 0.0) {	//We need to slow down.
			output = 0; //in case we forget to return.
			ApplyBrake(error);
//			printf("B slow:%g\n", error);
			return;
		}

		// Because setPoint < 0, we are reversing direction!
		if (wheel_speed >= boostedMaxAccel) {	//Slow down before accelerating in opposite direction.
			output = 0;
			ApplyBrake(error);
//			printf("B rev!:%g\n", error);
			return;
		}
		//Traction limited acceleration in opposite direction
		//maxOutput, setpoint, and output are < 0;
		float maxOutput = wheel_speed - boostedMaxAccel; //max we may apply in reverse without slipping
		if (output < maxOutput) {
			output = maxOutput;
//			printf("TorqueLimited Rev\n"); //print/display diagnostic -- should indicate on an LED or LCD [dg]
		}
	}
	
	if (runningBackwards)
		output = -output;

	output = RemoveMechanicalDeadband(kMinDutyCycle, output);
	m_esc->Set(output);
}

/* 
 * Braking: %T = %N * BDC  (Brake Duty Cycle)
 * So BDC = %T / %N for %N >= %T; 100% otherwise.
 */
void TorqueLimitedMotorOutput::ApplyBrake(float brakeError)
{
	float brakeTorque = brakeError * 1.0; //gain factor
	float brake_duty_cycle;
	float wheel_speed = GetNormalizedRate();
	
	//keep values positive; braking assumed to slow, regardless of direction.
	if (wheel_speed < 0) wheel_speed = -wheel_speed;
	if (brakeTorque < 0) brakeTorque = -brakeTorque;
	
	if (brakeTorque > m_maxBrake)	//limit braking to maxBrake
		brakeTorque = m_maxBrake;
//brakeTorque = m_maxBrake;
	
	brake_duty_cycle = 1.0;	//full brakes
	if (wheel_speed > brakeTorque)	//reduce braking if speed is high.
		brake_duty_cycle = brakeTorque / wheel_speed;	//duty cycles to brake.

	
	//code to apply brakes here!
	int brake8 = (int)(brake_duty_cycle * 8);
	m_brake->Set(brake8);
//	printf("\tAB:%f => %d\n",brake_duty_cycle, brake8);
	m_esc->Set(0);
}


void TorqueLimitedMotorOutput::SetTorqueLimitEnabled(bool value) {
	m_enabled = value;
}
void TorqueLimitedMotorOutput::SetMaxBrake(float value) {
	m_maxBrake = value;
}
float TorqueLimitedMotorOutput::GetMaxBrake() {
	return m_maxBrake;
}
void TorqueLimitedMotorOutput::SetMaxAccel(float value) {
	m_maxAccel = value;
}
float TorqueLimitedMotorOutput::GetMaxAccel() {
	return m_maxAccel;
}
