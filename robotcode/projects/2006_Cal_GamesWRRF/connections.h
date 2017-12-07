/********************************************************************************
* FILE NAME: connections.h <FRC VERSION>
*
* DESCRIPTION: 
*  This file ...
*
********************************************************************************/

#ifndef connections_h_
#define connections_h_

//#define mSTOP 	127u
//#define mON 	1
//#define mOFF 	0
//#define mMAX	254u


//**************  FRC CONNECTIONS *****************
//  PWM definitions
#ifdef __18F8520
	#define Robot2005
#elif defined(__18F8722)
	#define Robot2006
#else
	#error Unknown processor defined
#endif


#ifdef Robot2005
	#define mPWMcimLeft		pwm01
	#define mPWMfpriceLeft	pwm02
	#define mPWMcimRight	pwm03
	#define mPWMfpriceRight pwm04

	#define mCompressorRelay relay2_fwd	//on separate power circuit
#elif !defined(Robot2006)
#error undeclared Robot
#endif

#define mPWMRightCIM		pwm01
#define mPWMLeftCIM			pwm02
#define mPWMFrontLift		pwm03
#define mPWMRearLift		pwm04
#define mPWMLaunchLift		pwm05
#define mPWMTurretMotor		pwm06
#define mPWMLeftShooter		pwm07
#define mPWMRightShooter	pwm08
#define mPWMCameraPanMotor	pwm09
#define mPWMCameraTiltMotor	pwm10

#define mDiverterFront		relay2_fwd
#define mDiverterRear		relay2_rev
#define mPump				relay3_fwd

#define mFrontLiftPot	rc_ana_in01
#define mRearLiftPot	rc_ana_in02
#define mLaunchLiftPot	rc_ana_in03

#define mFrontLiftSpeed		((int) Get_Analog_Value(mFrontLiftPot)>>3)
#define mRearLiftSpeed		((int) Get_Analog_Value(mRearLiftPot)>>3)
#define mLaunchLiftSpeed	((int) Get_Analog_Value(mLaunchLiftPot)>>3)


//#define mCameraSimPot   rc_ana_in15
//#define mCameraTurretGain   rc_ana_in16

//Digital Inputs:
#define mLeftEncoderBCount	rc_dig_in01
#define mRightEncoderBCount	rc_dig_in02

//Interrupts - won't appear in code. Level changes on these pins
// will be handled in interruptHandlers.c & userRoutines_Fast.c
#define mLeftEncoderCount	rc_dig_in03
#define mRightEncoderCount	rc_dig_in04
#define mLeftShooterCount	rc_dig_in05
#define mRightShooterCount	rc_dig_in06	//assignments changed
//End of Interrupts

#define mLeftInHiGear		rc_dig_in08
#define mLeftInLowGear		rc_dig_in09
#define mRightInHiGear		rc_dig_in10

#define mRightInLowGear		rc_dig_in11
#define mBallin2nd			rc_dig_in17
#define mSwPressure			rc_dig_in18
//#define mLeftEncoderDir		rc_dig_in13
//#define mRightEncoderDir	rc_dig_in14

//#define mTurretCWLimit  	rc_dig_in15	
//#define mTurretCCWLimit		rc_dig_in16

#define mTurretCWLimit  	0	
#define mTurretCCWLimit		0



#define mBallinRear1		0 //rc_dig_in6
#define mBallinFront1		0 //rc_dig_in7
#define mBallinRear2		0 //rc_dig_in8
//End of this section.
//**************  END of FRC CONNECTIONS *****************


//**************  OPERATOR INTERFACE CONNECTIONS *****************
#define mRightCIMJoy		p1_y
#define mLeftCIMJoy			p2_y
#define mJoyForward			p2_y
#define mJoyTurn			p1_x


//#define mTurretCWSw	p4_sw_top
//#define mTurretCCWSw	p4_sw_trig  
//#define mTurretSpeed  p3_y

#define mFrontLiftSw		p2_sw_aux1	//on left joystick
#define mRearLiftSw			p2_sw_aux2

#define mBallLauncherspeed  p3_y 
#define mLaunchSwitch       p4_sw_aux1

//Debug purpose simulate via switch
//#define mSecondBallAvl     rc_dig_16
//#define mSecondBallAvl 		p3_sw_aux1


#define mRelayHiGear		relay1_fwd
#define mSwHiGear			p1_sw_trig	//right

#define mRelayLowGear		relay1_rev
#define mSwLowGear			p1_sw_top	//right

#define mReverseRobot		p1_sw_aux1

//Debug Purpose
//#define mTurretCWLimit	p3_sw_trig
//#define mTurretCCWLimit  p3_sw_top
//Replace the above line once the Sensor is avl


//Use these macros for test access to panel switches
//
//#define mPanelSwitch1	p4_sw_top
//#define mPanelSwitch2	p4_sw_trig
//#define mPanelSwitch3	p3_sw_trig
//#define mPanelSwitch4	p3_sw_top
//#define mPanelSwitch5	p4_sw_aux1
//#define mPanelSwitch6	p3_sw_aux1
//#define mPanelSwitch7	p4_sw_aux2
//#define mPanelSwitch8	p1_sw_top
//#define mPanelSwitch9	p1_sw_trig
//#define mPanelSwitch10	p1_sw_aux1
//
//#define mPanelPot1	p3_y
//#define mPanelPot2	p3_x
//#define mPanelPot3	p4_x
//#define mPanelPot4	p4_y
////End of Panel controls

//**********************************
#define mControlSwitch1		p3_sw_trig
#define mControlSwitch2		p3_sw_top
#define mControlSwitch3		p3_sw_aux1
#define mControlSwitch4		p3_sw_aux2
#define mControlSwitch5		p4_sw_trig
#define mControlSwitch6		p4_sw_top
#define mControlSwitch7		p4_sw_aux1
#define mControlSwitch8		p4_sw_aux2
#define mControlSwitch9		p1_sw_aux2		//overlaps joystick - should disable joystick button [dg]
#define mControlSwitch10	1

#define mMetaButton		 		mControlSwitch1
#define mBallQueueSw			mControlSwitch2
#define mPanLeftSw				mControlSwitch3
#define mPanRightSw				mControlSwitch4
#define mUnjamFrontSw			mControlSwitch5
#define mUnjamRearSw			mControlSwitch6
#define mFireSw					mControlSwitch7
#define mFrontKillSw			mControlSwitch8
#define mRearKillSw				mControlSwitch9

#define mThreewayswitch			p4_x


#define mRawDriveBtn			mPanLeftSw		//must be in UserDisplay mode
#define mDriveMethodBtn			mPanRightSw		//must be in UserDisplay mode

#define mAutoTimeUpBtn			mUnjamFrontSw		//must be in UserDisplay mode
#define mAutoTimeDownBtn		mUnjamRearSw		//must be in UserDisplay mode

#define mAutoWrite1Btn			mUnjamFrontSw		//must be in UserDisplay mode
#define mAutoWrite2Btn			mUnjamRearSw		//must be in UserDisplay mode

#define mPanOffset		p3_x


//**************  END of OPERATOR INTERFACE CONNECTIONS *****************

//******************* DASH Board User Bytes *********************************************

#define kUB_CWSpeed		User_Byte1
#define kUB_CCWSpeed	User_Byte1

//#define xxx User_Byte3	/* This byte is now used for breaker panel byte 3 */
//#define xxx User_Byte4	/* This byte is now used for breaker panel byte 4 */
//#define xxx User_Byte5	/* This byte is now used for breaker panel byte 5 */
//#define xxx User_Byte6	/* This byte is now used for breaker panel byte 6 */





//********************************************************************************

enum { kDown=-1, kOff=0, kUp=1};

#endif //connections_h_	NO CODE BELOW THIS LINE
