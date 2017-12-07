// connections.h: Connections information

#ifndef connections_h_
#define connections_h_

#if !defined(__18F8520) && !defined(__18F8722)
	#error Unknown processor target
#endif

#if !defined(ROBOT_2005) && !defined(ROBOT_2006) && !defined(ROBOT_2007)
	#error Unknown robot target
#endif


//**************  FRC CONNECTIONS *****************

// PWM outputs
#ifdef ROBOT_2006
	#define mPWMRightCIM		pwm01
	#define mPWMLeftCIM			pwm02
#elif defined ROBOT_2007
	#define mPWMLeftCIM			pwm11
	#define mPWMRightCIM		pwm12
	
	#define mPWMLift			pwm03
#endif


// Relay outputs
#ifdef ROBOT_2007
	#define mRelayArmUp			relay1_fwd
	#define mRelayArmDown		relay1_rev
	#define mRelayFingerOpen	relay2_fwd
	#define mRelayFingerClose	relay2_rev
	#define mRelayRampsDeploy	relay4_fwd
#endif
#define mPump					relay3_fwd


// Analog Inputs
#ifdef ROBOT_2007
	#define mLiftPotPort		rc_ana_in01
	
	#define mUserPot1Port		rc_ana_in16
#endif

#define mProximityLeftPort		rc_ana_in10
#define mProximityRightPort		rc_ana_in11
#define mProximityBumperPort	rc_ana_in12


// Digital Inputs
#ifdef ROBOT_2006
	#define mLeftEncoderBCount	rc_dig_in01
	#define mRightEncoderBCount	rc_dig_in02
#elif defined ROBOT_2007
	#define mLeftEncoderBCount	rc_dig_in07
	#define mRightEncoderBCount	rc_dig_in08
	
	#define mSwGripperTrap		(!rc_dig_in15)
	#define mSwLimitLiftLow		(!rc_dig_in16)
	#define mSwLimitLiftHigh	(!rc_dig_in17)
#endif

#ifdef SERIAL_LED
	#define mLEDDataOut			rc_dig_in10		// Configure as output
	#define mLEDClock			rc_dig_in11		// Configure as output
	#define mLEDDataReady		rc_dig_in12		// Configure as output
#endif

#define mCoast					rc_dig_in13		// Configure as output

#define mSwPressure				rc_dig_in18

// Interrupts
#define mLeftEncoderCount		rc_dig_in03
#define mRightEncoderCount		rc_dig_in04

//**************  END of FRC CONNECTIONS *****************



//**************  OPERATOR INTERFACE CONNECTIONS *****************

#ifdef CONTROLS_2007
	#define mXboxLeft			p2_y
	#define mXboxRight			p2_x
	#define mXboxA				p2_sw_trig
	#define mXboxB				p2_sw_top
	#define mXboxX				p2_sw_aux1
	#define mXboxY				p2_sw_aux2
	#define mXboxLT				((p2_wheel & 0x80) == 0)
	#define mXboxRT				((p2_wheel & 0x40) == 0)
	#define mXboxLB				((p2_wheel & 0x20) == 0)
	#define mXboxRB				((p2_wheel & 0x10) == 0)
	#define mXboxBack			((p2_aux & 0x80) == 0)
	#define mXboxStart			((p2_aux & 0x40) == 0)
	#define mXboxCenterX		((p2_aux & 0x20) == 0)
	#define mXboxDUp			((p2_aux & 0x10) == 0)
	
	#define mBtnPanelA			p1_sw_trig
	#define mBtnPanelB			p1_sw_top
	#define mBtnPanelC			p1_sw_aux1
	#define mBtnPanelD			p1_sw_aux2

//	#define mJoyLeftCIM			mXboxLeft
//	#define mJoyRightCIM		mXboxRight
	#define mJoyForward			mXboxLeft
	#define mJoyTurn			mXboxRight



	#define mBtnMeta			mXboxCenterX
	#define mReverseRobot		0//p1_sw_aux1

	#define mSwXboxManualMode	p4_sw_aux1
		
//	#define mBtnReadyPickup		(mSwXboxManualMode ? 0 : mXboxA)
//	#define mBtnLiftL0			(mSwXboxManualMode ? mXboxA : 0)
//	#define mBtnLiftL1			mXboxB
//	#define mBtnLiftL2			mXboxX
//	#define mBtnLiftL3			mXboxY
	
	#define mBtnPrecisionDrive	mXboxRB
	
	#define mBtnScore			mXboxDUp	//(mSwXboxManualMode ? mXboxDUp : mXboxRT)
	//#define mBtnAutoAim			(mSwXboxManualMode ? 0 : mXboxRB)
	//#define mBtnUnspoil			(mSwXboxManualMode ? 0 : mXboxLT)
	#define mBtnAbortAction		mXboxCenterX
	
	#define mBtnAutoAim			(mSwXboxManualMode ? 0 : mXboxLT)
	#define mBtnAutoDistance	(mSwXboxManualMode ? 0 : mXboxLB)
	
	//#define mBtnLiftUp			(mSwXboxManualMode ? p3_sw_trig||mXboxLB	: p3_sw_trig)
	//#define mBtnLiftDown		(mSwXboxManualMode ? p3_sw_top ||mXboxRB	: p3_sw_top )
	#define mBtnLiftUp			p3_sw_trig
	#define mBtnLiftDown		p3_sw_top
	#define mBtnArmUp			(mSwXboxManualMode ? p3_sw_aux1||mXboxBack	: p3_sw_aux1)
	#define mBtnArmDown			(mSwXboxManualMode ? p3_sw_aux2||mXboxStart	: p3_sw_aux2)
	//#define mBtnFingerOpen		(mSwXboxManualMode ? p4_sw_trig||mXboxLT	: p4_sw_trig)
	//#define mBtnFingerClose		(mSwXboxManualMode ? p4_sw_top ||mXboxRT	: p4_sw_top )
	#define mBtnFingerOpen		(p4_sw_trig||mXboxLT)
	#define mBtnFingerClose		(p4_sw_top ||mXboxRT)
	
	#define mBtnDeployRamps		p4_sw_aux2
	
	#define mBtnSetLiftPreset	(mXboxBack && mXboxStart && mBtnMeta)
	
	#define mBtnLiftL0			(mBtnPanelD  || mXboxA) //(mSwXboxManualMode ? mXboxA : 0))
	#define mBtnLiftL1			(mBtnPanelC  || mXboxB)
	#define mBtnLiftL2			(mBtnPanelB  || mXboxX)
	#define mBtnLiftL3			(mBtnPanelA  || mXboxY)
	
	#define mBtnBrake			(mBtnMeta && mXboxRT)
	
	#define mBtnProxAim			0
	#define mBtnPotLift			0
	#define mBtnLiftPlus12		0

//	#define mBtnProxAim			mBtnPanelA
//	#define mBtnPotLift			mBtnPanelB
//	#define mBtnLiftPlus12		mBtnPanelC
//	#define mBtnSomething		mBtnPanelD



	#define mUserOIPot1			p3_x
	#define mUserOIPot2			p3_y
	#define mUserOIPot3			p3_wheel
	
	#define mServoDriveGainPot	mUserOIPot1
	#define mAutonomousDistPot	mUserOIPot3
	
	// Drive Option Buttons
	#define mBtnRawDrive		(mBtnMeta && mXboxLB)
	#define mBtnDriveMethod		(mBtnMeta && mXboxRB)
	#define mBtnServiceMode		(disabled_mode && mBtnMeta && mXboxLB && mXboxRB)
	
	// Autonomous Option Buttons
	#define mBtnShowAutonomousMode		(disabled_mode && mBtnPanelA)
	#define mBtnSetAutonomousMode		(mBtnShowAutonomousMode && p3_sw_trig)	// LiftUp
	#define mBtnShowAutonomousDistSide	(disabled_mode && mBtnPanelC)
	#define mBtnShowAutonomousDistCtr	(disabled_mode && mBtnPanelD)
	#define mBtnSetAutonomousDist		(disabled_mode && p3_sw_top)			// LiftDn
#else
	#define mJoyRightCIM		p1_y
	#define mJoyLeftCIM			p2_y
	#define mJoyForward			p2_y
	#define mJoyTurn			p1_x
	
	#define mReverseRobot		p1_sw_aux1
	#define mBtnMeta			0
	
	#define mBtnRawDrive		(mBtnMeta && p3_sw_aux1)	// Pan Left
	#define mBtnDriveMethod		(mBtnMeta && p3_sw_aux2)	// Pan Right
#endif

#ifdef TURN_KNOB
	#ifdef CONTROLS_2007
		#define mTurnPot1		p4_y
		#define mTurnPot2		p3_y
	#else
		#define mTurnPot1		p3_x		// Pan Offset on 2006
		#define mTurnPot2		127u
	#endif
#endif

//**************  END of OPERATOR INTERFACE CONNECTIONS *****************


#ifdef SERIAL_LED
//	#define mSLEDGripperArmFwd		gSerialLED.green1
//	#define mSLEDGripperArmRev		gSerialLED.red1
//	#define mSLEDGripperGripFwd 	gSerialLED.green2
//	#define mSLEDGripperGripRev		gSerialLED.red2
	
	#define mSLEDLiftStalled		gSerialLED.red4
	#define mSLEDLiftCooling		gSerialLED.green4
	
//	#define mSLEDGripperTrap		gSerialLED.red5
	
	#define mSLEDLiftLimitLow		gSerialLED.red6
	#define mSLEDLiftLimitHigh		gSerialLED.green6
	
	#define mSLEDProximityLeft0		gSerialLED.red7
	#define mSLEDProximityLeft1		gSerialLED.green7
	#define mSLEDProximityRight0	gSerialLED.red8
	#define mSLEDProximityRight1	gSerialLED.green8
#endif

#endif //connections_h_	NO CODE BELOW THIS LINE
