//Author: Karthik Viswanathan (2010)

#include "WPILib.h"
#include "LRTConnections.h"
#include "LRTDriveEncoders.h"

LRTDriveEncoders::LRTDriveEncoders()
: m_encoderLeft(LRTConnections::kDioEncoderLeftA, LRTConnections::kDioEncoderLeftB, false, CounterBase::k1X)
, m_encoderRight(LRTConnections::kDioEncoderRightA, LRTConnections::kDioEncoderRightB, false, CounterBase::k1X)
{
	m_encoderLeft.SetDistancePerPulse(1); // want to stay with ticks/second
	m_encoderRight.SetDistancePerPulse(1);
	
	m_encoderLeft.Start();
	m_encoderRight.Start();
}

LRTDriveEncoders::~LRTDriveEncoders() {
	
}

double LRTDriveEncoders::GetNormalizedRate() {
	return LRTUtil::clamp<double>(GetForwardSpeed() / kMaxEncoderRate, -1.0, 1.0);
}

double LRTDriveEncoders::GetForwardSpeed() {
	return ( m_encoderLeft.GetRate() + m_encoderRight.GetRate() ) / 2;
}

double LRTDriveEncoders::GetTurningSpeed() {
//	return ( GetRightWheelDist() - GetLeftWheelDist() ) / kTrackLength;
	return (m_encoderLeft.GetRate() - m_encoderRight.GetRate()) / kTrackLength;
}

double LRTDriveEncoders::GetTurningSpeedPercent() {
	return GetTurningSpeed() / (2 * GetMaxRate());
}

double LRTDriveEncoders::GetLeftSpeed() {
	return m_encoderLeft.GetRate();
}

double LRTDriveEncoders::GetRightSpeed() {
	return m_encoderRight.GetRate();
}

double LRTDriveEncoders::GetLeftWheelDist() {
	// - Get() returns pulses / second
	// - kPulsesPerRevolution is in pulses / revolution
	// - kWheelDiameter * kPi is the circumference / revolution 
	// in centimeters
	
	// ( pulses / second ) / ( pulses / revolution )
	// * ( circumference / revolution ) = centimeter distance 
	return m_encoderLeft.Get() / kPulsesPerRevolution * kWheelDiameter * kPi;
}

double LRTDriveEncoders::GetRightWheelDist() {
	// see GetLeftWheelDist() for caclulation explanation
	return m_encoderRight.Get() / kPulsesPerRevolution * kWheelDiameter * kPi;
}

double LRTDriveEncoders::GetLeftWheelSpeed() {
	return GetLeftWheelDist() / m_encoderLeft.GetPeriod();
}

double LRTDriveEncoders::GetLeftWheelSpeedPercent() {
	return LRTUtil::clamp<double>(GetLeftSpeed() / kMaxEncoderRate, -1.0, 1.0);
}

double LRTDriveEncoders::GetRightWheelSpeed() {
	return GetRightWheelDist() / m_encoderRight.GetPeriod();
}

double LRTDriveEncoders::GetRightWheelSpeedPercent() {
	return LRTUtil::clamp<double>(GetRightSpeed() / kMaxEncoderRate, -1.0, 1.0);
}

double LRTDriveEncoders::GetMaxRate() {
	// same as GetLeftWheelDist() except using a 
	// max pulses / second
//	return kMaxEncoderRate / kPulsesPerRevolution * kWheelDiameter * kPi;
	return kMaxEncoderRate;
}

Encoder& LRTDriveEncoders::GetLeftEncoder() {
	return m_encoderLeft;
}
Encoder& LRTDriveEncoders::GetRightEncoder() {
	return m_encoderRight;
}
