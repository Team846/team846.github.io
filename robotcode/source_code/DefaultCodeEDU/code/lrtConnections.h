/**************************
* Function mapping for Lynbrook Robotics robot
*
* maps predefined macros to user functions on Lynbrook's console
*
********************************************************/


#ifndef __lrtConnections_h_
#define __lrtConnections_h_
#include "ifi_picdefs.h"
#include "ifi_aliases.h"

void LRTConsoleMapping(void);

/******************* OI CONNECTIONS ************************************/
//Driver controls:
//port 4 is left joystick
//port 3 is right joystick

//port 2 is copilot console

#define mLeftDriveJoyStick p3_y
#define mRightDriveJoyStick p4_y
#define mTurnJoyStick p4_x	//used for single joystick operation

//#define mShiftLow p3_sw_aux1
//#define mShiftHigh p3_sw_aux2
//#define mShiftLow p3_sw_top
#define mShiftHigh p4_sw_top
#define mShiftLow p4_sw_trig
//#define mShiftHigh p4_sw_trig

#define mTetraLoadServiceFlag p4_sw_aux1

//#define mDescendStep	p4_sw_aux1
//#define mClimbStep	p4_sw_aux2
//#define mLowRate	p4_sw_trig		//reduce gain on joysticks
#define mLowRate	0		//reduce gain on joysticks
//#define mServoSwitch	p4_sw_top	//for testing

#define mDriverAbort	p3_sw_aux2	//abort any automated routine
//#define mRightJoyStickOnly p3_sw_trig
#define mRightJoyStickOnly 0

#define mBtnArmGroundLevel	p2_sw_aux1
#define mBtnArmLoadTetra	p2_sw_aux2
#define mBtnArmCarryTetra	p1_sw_aux1
#define mBtnArmRaiseTetra	p1_sw_aux2

#define mBtnHook			p1_sw_trig
#define mBtnHookRelease		p2_sw_trig

#define mAbortArm			p1_sw_top
#define mBashPlateOverride	p1_sw_top

//#define mAbortCopilot	p1_sw_aux1
#define mServoInTest		p4_x

//Copilot controls: Either Joystick or the Absolute Control arm is used.
#define mForearmJoy	p1_y
#define mShoulderJoy p2_y
#define mForearmAbs	p1_wheel	//=255 when using joystick
#define mShoulderAbs p1_aux	//=255 when using joystick



/******************* ROBOT CONNECTIONS ************************************/

//front leg is in ADC 0
//rear leg is on ADC 1

//pwm definitions

// PWM Outputs
//#define mPWMLeft pwm01
//#define mPWMRight pwm02

#define mPWMcimLeft pwm01
#define mPWMfpriceLeft pwm02
#define mPWMcimRight pwm03
#define mPWMfpriceRight pwm04
#define mPWMshoulder pwm05
#define mPWMforearm pwm06

#define mPWMsignalFlag pwm07


	//mLeftDriveJoyStick will directly drive mPWMTestOut
	// for testing & setup purposes.
#define mPWMTestOut pwm10


//RELAY Outputs
#define mGearShiftHighRelay relay1_fwd
#define mGearShiftLowRelay relay1_rev
#define mCompressorRelay relay2_fwd	//on separate power circuit
#define mHookRelay relay3_fwd	//0 must reflect disabled position (hook up)
#define mTetraWhackCW relay4_fwd
#define mTetraWhackCCW relay4_rev

//#define mPWMLegFront pwm03
//#define mPWMLegRear pwm04
//#define mRotateBoomPWM pwm05	//boom horizontal
//#define mRaiseBoomPWM pwm06		//boom vertical
//#define mExtendHookPWM pwm07
//#define mTeeBallServoRight pwm08
//#define mTeeBallServoLeft pwm09
//
//



// Robot Digital Inputs
//#define mEncoderLeftAin rc_dig_in01	//interrupt - used implicitly by interupt 1
//#define mEncoderRightAin rc_dig_in02  //interrupt - used implicitly by interupt 2
#define mEncoderLeftBin rc_dig_in03
#define mEncoderRightBin rc_dig_in04

#define mRightLowGearSw  (0==rc_dig_in05)
#define mRightHighGearSw (0==rc_dig_in06)
#define mLeftLowGearSw	 (0==rc_dig_in07)
#define mLeftHighGearSw  (0==rc_dig_in08)

#define mShoulderUpperLimitSw (0==rc_dig_in09)
#define mShoulderLowerLimitSw (0==rc_dig_in10)
#define mForeArmUpperLimitSw  (0==rc_dig_in11)
#define mForeArmLowerLimitSw  (0==rc_dig_in12)

#define mCompressorAtPressure rc_dig_in13

#define mAutonomousSw0	(0==rc_dig_in14)
#define mAutonomousSw1	(0==rc_dig_in15)
#define mAutonomousSw2	(0==rc_dig_in16)

#define mBashPlateLimit	(0==rc_dig_in18)


//ROBOT Analog Inputs (unsigned char const to be used with 
#define kA2DShoulderJoint rc_ana_in01
#define kA2DForearmJoint rc_ana_in02

#define kA2D1 rc_ana_in03
#define kA2D2 rc_ana_in04
#define kA2D3 rc_ana_in05



////unsigned char const
//#define kA2DLegFront rc_ana_in01
//#define kA2DLegRear rc_ana_in02
//
//#define mSwTeeBall	!rc_dig_in11
//#define mSwGenericAction	!rc_dig_in12

//#define mSwAutonomous	!rc_dig_in13

//Redefinitions if using the EDU board

#ifndef _FRC_BOARD	//then we are using the EDU board
//	#undef mSwTeeBall
//	#undef mSwGenericAction
//	#undef mClimbStep
//	#undef mDescendStep
	#undef mDriverAbort
	
//	#define mSwGenericAction !rc_dig_in01
//	#define mDriverAbort !rc_dig_in02
	#define mDriverAbort 0
//	#define mSwTeeBall !rc_dig_in03
//	#define mClimbStep	!rc_dig_in04
//	#define mDescendStep	!rc_dig_in05
#endif //_FRC_BOARD


//redefinititions to test on 2004 robot.
#ifdef _04Robot
#include "2004Robot.h"
#endif //_04Robot

#ifdef _SIMULATOR
#include "simulator.h"
#endif //_SIMULATOR

#endif // __lrtConnections_h_  --nothing below this line!!

