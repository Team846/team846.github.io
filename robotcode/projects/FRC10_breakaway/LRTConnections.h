//Author: David Liu (2009,2010)

#ifndef LRT_CONNECTIONS_H_
#define LRT_CONNECTIONS_H_

#include "WPILib.h"

//#define LRT_2009_ROBOT
#define LRT_2010_ROBOT
//#define LRT_2010_ROBOT_PRACTICE

#if !defined(LRT_2009_ROBOT) && !defined(LRT_2010_ROBOT) && !defined (LRT_2010_ROBOT_PRACTICE)
#error Choose a robot to compile for: define LRT_20XX_ROBOT
#endif

/*
 * 7 Motors
 * 5 Jaguars (drive train 2x, winch, lift righting, roller)
 * 2 Relays (lift, kicker)
 */

/*
 * Sensors:
 * 2 Encoders for left and right drive
 * Encoder for roller
 * Encoder for winch
 * Ball proximity sensor
 * Rotational pot for kicker
 * Rotational pot for lift opener
 * Position sensor for lift raiser tbd
 */

class LRTConnections {
public:

	// PWM Outputs
	
	// Jaguars
//	const static UINT32 kPwmDriveLeft = 9;
//	const static UINT32 kPwmDriveRight = 10;
//	const static UINT32 kPwmRoller = 1;
//	const static UINT32 kPwmWinch = 2;
//	const static UINT32 kPwmLiftOpener = 3;
	
	const static UINT32 kCanDriveLeft = 2;
//	const static UINT32 kCanDriveRight = 5; // testing only
	const static UINT32 kCanDriveRight = 3;
	const static UINT32 kCanRoller = 4;
	const static UINT32 kCanWinch = 6;
//	const static UINT32 kCanArm = 3; // testing only
	const static UINT32 kCanArm = 5;
	
	// Servos
	const static UINT32 kPwmShifterLeft = 8;
	const static UINT32 kPwmShifterRight = 10;
	
	// Relay Outputs
	const static UINT32 kRelayKicker = 8;
	const static UINT32 kRelayLiftRaiser = 7;

	// Digital Inputs
	const static UINT32 kDioEncoderLeftA = 1;
	const static UINT32 kDioEncoderLeftB = 2;
	const static UINT32 kDioEncoderRightA = 13;
	const static UINT32 kDioEncoderRightB = 14;

	const static UINT32 kDioKickerSense = 3;
	
	const static UINT32 kDioEncoderRoller = 8;
	
	// Digital Outputs
	const static UINT32 kDioBrakeLeft = 6;
	const static UINT32 kDioBrakeRight = 7;

	// Analog Inputs
	const static UINT32 kAnalogGyro = 1;
	const static UINT32 kAnalogBallProximity = 2;
	const static UINT32 kAnalogRotPotKicker = 6;
	const static UINT32 kAnalogRotPotArm = 7;
	const static UINT32 kAnalogRotPotLift = 5;
	
	const static int kMaxBraking = 8; // out of 8
	
#if defined(LRT_2009_ROBOT)
	const static int kNumTicksPerRev = 128;
	const static float kDistancePerPulse = 3.14159 * 6.08 / kNumTicksPerRev;
#elif defined(LRT_2010_ROBOT)
	const static int kNumTicksPerRev = 333;
	const static float kDistancePerPulse = 3.14159 * 6.08 / kNumTicksPerRev;
#endif

};

#endif
