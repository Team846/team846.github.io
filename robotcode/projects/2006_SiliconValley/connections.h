/********************************************************************************
* FILE NAME: connections.h <FRC VERSION>
*
* DESCRIPTION: 
*  This file ...
*
********************************************************************************/

#ifndef connections_h_
#define connections_h_

#define mSTOP 	127u
#define mON 	1
#define mOFF 	0
#define mMAX	254u


//**************  FRC CONNECTIONS *****************
//  PWM definitions
#define mPWMRightCIM		pwm01
#define mPWMLeftCIM			pwm02
#define mPWMFrontLift		pwm03
#define mPWMRearLift		pwm04
#define mPWMLaunchLift		pwm05
#define mPWMTurretMotor		pwm06
#define mPWMRightShooter	pwm07
#define mPWMLeftShooter		pwm08
#define mPWMCameraPanMotor	pwm09
#define mPWMCameraTiltMotor	pwm10

#define mFrontLiftPot	rc_ana_in01
#define mRearLiftPot	rc_ana_in02
#define mLaunchLiftPot	rc_ana_in03
//#define mCameraSimPot   rc_ana_in15
//#define mCameraTurretGain   rc_ana_in16

//Digital Inputs:
//Interrupts first
#define mLeftEncoderCount	rc_dig_in01
#define mRightEncoderCount	rc_dig_in02
#define mLeftShooterCount	rc_dig_in03
#define mRightShooterCount	rc_dig_in04
//End of Interrupts

#define mLeftInHiGear		rc_dig_in08
#define mLeftInLowGear		rc_dig_in09
#define mRightInHiGear		rc_dig_in10
#define mRightInLowGear		rc_dig_in11
//#define mSwPressure		rc_dig_in12
#define mLeftEncoderDir		rc_dig_in13
#define mRightEncoderDir	rc_dig_in14

//#define mTurretCWLimit  	rc_dig_in15	
//#define mTurretCCWLimit		rc_dig_in16

#define mTurretCWLimit  	0	
#define mTurretCCWLimit		0

//New section: modified in 10Ant
//Please edit this, I dont know what's going wrong
//WARNING, not enough digital inputs
#define mBallin2nd         rc_dig_in07

#define mBallinRear1		0 //rc_dig_in6
#define mBallinFront1		0 //rc_dig_in7
#define mBallinRear2		0 //rc_dig_in8
//End of this section.
//**************  END of FRC CONNECTIONS *****************


//**************  OPERATOR INTERFACE CONNECTIONS *****************
#define mRightCIMJoy		p1_y
#define mLeftCIMJoy			p2_y


//#define mTurretCWSw	p4_sw_top
//#define mTurretCCWSw	p4_sw_trig  
//#define mTurretSpeed  p3_y

#define mFrontLiftSw		p2_sw_aux1	//on left joystick
#define mRearLiftSw			p2_sw_aux2
#define mFrontLiftSpeed		((int) Get_Analog_Value(mFrontLiftPot)>>3)
#define mRearLiftSpeed		((int) Get_Analog_Value(mRearLiftPot)>>3)
#define mLaunchLiftSpeed	((int) Get_Analog_Value(mLaunchLiftPot)>>3)

#define mDiverterFront		relay2_fwd
#define mDiverterRear		relay2_rev
#define mBallLauncherspeed  p3_y
//#define mPump			relay3_fwd
#define mLaunchSwitch       p4_sw_aux1

//Debug purpose simulate via switch
//#define mSecondBallAvl     rc_dig_16
//#define mSecondBallAvl 		p3_sw_aux1


#define mRelayHiGear	relay1_fwd
#define mSwHiGear		p1_sw_trig	//right

#define mRelayLowGear	relay1_rev
#define mSwLowGear		p1_sw_top	//right



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
//New Controller definitions as of version 10Ant
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

#define mActivateCameraSw 		mControlSwitch1
#define mBallQueueSw			mControlSwitch2
#define mPanRightSw				mControlSwitch3
#define mPanLeftSw				mControlSwitch4
#define mUnjamFrontSw			mControlSwitch5
#define mUnjamRearSw			mControlSwitch6
#define mFireSw					mControlSwitch7
#define mFrontKillSw			mControlSwitch8
#define mRearKillSw				mControlSwitch9

#define mThreewayswitch			p4_x


#define mPanOffset		p3_x	 //this should be an OI port analog input [dg], not this neutral value


//LED connections
#define mLEDCameraStatus		Relay1_green
#define mLEDBallInNo2Pos		Relay2_green	//need to be properly selected
//***********************************

//**************  END of OPERATOR INTERFACE CONNECTIONS *****************
//
//typedef struct {
//
//	int left;
//	int right;
//
//} gControl;
//
//extern gControl gCtrl;
//
//
//


//
//#define sw1=p3_sw_trig = Camera/Human Control -- No needed with new verison software
//#define sw2=p3_sw_top = Que Next Ball
//#define sw3=p3_sw_aux1= pan_left
//#define sw4=p3_sw_aux2=pan_right
//#define sw5=p4_sw_trig = Unjam Front
//#define sw6=p4_sw_top= unjam rear
//#define sw7=p4_sw_aux1=Fire
//#define sw8=p4_sw_aux2=front kill
//#define sw9=p1_aux_2 = Real Kill
// 
//#define pot1= p3_x = pan offset
//#define pot2=p3_y = shooting speed
// 
//#define Threewayswitch = p4_x [Down=165, off=0, up=79]

enum { kDown=-1, kOff=0, kUp=1};

#endif //connections_h_	NO CODE BELOW THIS LINE
