//Author: Karthik Viswanathan (2010)

#ifndef LRTENCODER_H_
#define LRTENCODER_H_

#include "Encoder.h"
#include "LRTUtil.h"
#include "LRTConnections.h"

class LRTDriveEncoders {
private:
	Encoder m_encoderLeft, m_encoderRight;
	// floats to prevent integer division
	static const float kWheelDiameter = 8.0; // in
//	static const float kPulsesPerRevolution = 128.0;
	// output sprocket:wheel sprocket = 36:48
	static const float kPi = 3.14159;
	static const float kTrackLength = 23.0; // in
	
public:
	LRTDriveEncoders();
	virtual ~LRTDriveEncoders();

#if defined(LRT_2010_ROBOT)
//	const static float kMaxEncoderRate = 170;
	static const float kPulsesPerRevolution = 333.3;//for practice robot [BA]
	const static float kMaxEncoderRate = 3333;
	//high gear, good library (3.0/r1718)
//	const static float kMaxEncoderRate = 335; //high gear, bad library (3.1)
//	const static float kMaxEncoderRate = 130; //low  gear
#elif defined(LRT_2010_ROBOT_PRACTICE) 
	static const float kPulsesPerRevolution = 480.0;//for practice robot [BA]
	const static float kMaxEncoderRate = 4800; // ticks/second
#elif defined(LRT_2009_ROBOT)
	const static float kMaxEncoderRate = 130; // ticks / second
//	const static float kMinDutyCycle = 20.0/128; // in order to start moving
#endif

	double GetNormalizedRate();
	double GetForwardSpeed();
	double GetTurningSpeed();
	double GetTurningSpeedPercent();
	double GetLeftSpeed();
	double GetRightSpeed();
	double GetLeftWheelDist();
	double GetRightWheelDist();
	double GetLeftWheelSpeed();
	double GetLeftWheelSpeedPercent();
	double GetRightWheelSpeed();
	double GetRightWheelSpeedPercent();
	double GetMaxRate();
	Encoder& GetLeftEncoder();
	Encoder& GetRightEncoder();
};
#endif /*LRTENCODER_H_*/
