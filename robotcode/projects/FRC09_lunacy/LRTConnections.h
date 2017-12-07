/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2008. All Rights Reserved.							  */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in $(WIND_BASE)/WPILib.  */
/*----------------------------------------------------------------------------*/

#ifndef LRT_CONNECTIONS_H_
#define LRT_CONNECTIONS_H_

#define LRT_2008_ROBOT
//#define LRT_2009_ROBOT

#if !defined(LRT_2008_ROBOT) && !defined(LRT_2009_ROBOT)
#error Choose a robot to compile for: define LRT_200X_ROBOT
#endif

class LRTConnections {
public:
	const static UINT32 kPWMDriveLeft = 9;
	const static UINT32 kPWMDriveRight = 10;
	
	const static UINT32 kPWMShooter = 1;
	const static UINT32 kPWMPickupRear = 2;
	const static UINT32 kRelayPickupFront = 1;
	const static UINT32 kRelayLiftFront = 2;
	const static UINT32 kRelayLiftRear = 3;
	
	const static UINT32 kDIOEncoderLeftA = 11;
	const static UINT32 kDIOEncoderLeftB = 12;
	const static UINT32 kDIOEncoderRightA = 1;
	const static UINT32 kDIOEncoderRightB = 2;

//	const static UINT32 kDIOCurrentSenseResetLeft = 3;
	// FIX ME - don't need a second digital output for current sensor reset
//	const static UINT32 kDIOCurrentSenseResetRight = 4;
	
	const static UINT32 kDIOAutonomousLeft = 3;
	const static UINT32 kDIOAutonomousSpinOrCorner = 4;
	const static UINT32 kDIOAutonomousRight = 5;
	
	
	const static UINT32 kDIOBrakeLeft = 6;
	const static UINT32 kDIOBrakeRight = 7;
	//const static UINT32 kDIOShooterHallEffect = 8;
	const static UINT32 kDIOShooterEncoder = 8;
	
	const static UINT32 kDIOScrollUp = 13;
	const static UINT32 kDIOScrollDn = 14;
	
	
//	const static UINT32 kDIOShooterSpeedSensor = 5;
	
	const static UINT32 kAnalogCurrentSenseLeft = 6;
//	const static UINT32 kAnalogCurrentSenseRight = 7;
	
	const static UINT32 kAnalogTest = 7;	// for our analog testing
	
//	const static UINT32 kAutonomousSwitch = 8;	// analog input 8 reserved for battery voltage reading
	
	// Gyro+Accel = analog
	const static UINT32 kGyro = 1;
	const static UINT32 kLEFT_ACCEL_X = 2;
	const static UINT32 kLEFT_ACCEL_Y = 3;
	const static UINT32 kRIGHT_ACCEL_X = 4;
	const static UINT32 kRIGHT_ACCEL_Y = 5;
	
	
#if defined(LRT_2008_ROBOT)
	const static int kNumTicksPerRev = 100;
	const static float kDistancePerPulse = 3.14159 * 6.08 / kNumTicksPerRev;
#elif defined(LRT_2009_ROBOT)
	const static int kNumTicksPerRev = 128;
	const static float kDistancePerPulse = 3.14159 * 6.08 / kNumTicksPerRev;
#endif

};

#endif
