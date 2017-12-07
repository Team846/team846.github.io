// controls.c: Handles user controls and user options.
//

#include "common.h"
#include "controls.h"

struct userOption gUserOption = {1,0,1,0, 0};
char gUserByteOI;
static char sTurnRateLimited=0;

//lift gLift = {0,0,0,0,kFrontLiftActive};
//drive gDrive = {0,0,0,0};
//sensors gSensors = {0,0,0,0};

//***********************************************************************
static void ReverseRobotControls(void);
//static char JoystickTrimLED(unsigned char in);

//static void JoystickTankDrive_HalfTurn(void);
//static void JoystickTankDrive_ZSquaredTurn(void);
static void JoystickXYDrive(void);
static void JoystickXYDrive_HalfTurn(void);
static void JoystickXYDrive_ZSquaredTurn(void);
static void JoystickXYDrive_HighSpeedTurn(void);
static void TurnKnobDrive(void);

void doRamps(void);
//***********************************************************************

void controls(void)
{
//	gLED.LeftJoyY = JoystickTrimLED(mJoyLeftCIM);
//	gLED.RightJoyY = JoystickTrimLED(mJoyRightCIM);
//	gLED.RightJoyX = JoystickTrimLED(mJoyTurn);

//#define TURN_LIMITING
#ifdef TURN_LIMITING
	sTurnRateLimited = (mAbsolute(gRobotTurnSpeed) > 300);
#else
	sTurnRateLimited = 0;
#endif
//	gLED.turnRateLimited = sTurnRateLimited;

	JoystickXYDrive();
	
//		switch (gUserOption.DriveMethod)
//		{
//			case 1:
//				JoystickTankDrive_HalfTurn();
//				break;
//			case 2:
//				JoystickTankDrive_ZSquaredTurn();
//				break;
//			case 3:
//				JoystickXYDrive();
//				break;
//			case 4:
//				JoystickXYDrive_HalfTurn();
//				break;
//			case 5:
//				JoystickXYDrive_ZSquaredTurn();
//				break;
//			case 6:
//				JoystickXYDrive_HighSpeedTurn();
//				break;
//
//			default:
//				printf("Unknown DriveType!\r");
//		}
//	ReverseRobotControls();	// must be after DriveJoysticks
//	TurnKnobDrive();	// must be after ReverseRobotControls

	if (mBtnPrecisionDrive) {
		int fwd = ((int)gMotorSpeed.cimL + (int)gMotorSpeed.cimR)/2   * 7/10;
		int turn = ((int)gMotorSpeed.cimL - (int)gMotorSpeed.cimR)/2  * 7/10;
		gMotorSpeed.cimL = fwd + turn;
		gMotorSpeed.cimR = fwd - turn;
	}
	
	// Call in order of overriding.
	doActionBtns();
#ifdef ROBOT_2007
	doLiftBtns();
	doGripperBtns();
	doRamps();
#endif	// ROBOT_2007


	if (mBtnProxAim)
		aim(0);
	
	if (mBtnBrake)
		mCoast = 0;
	
	if (gUserOption.ServiceMode) {
		if (gLoop.onSecond)
			printf("SERVICE MODE controls\r\n");
		gMotorSpeed.cimL = mXboxLeft;
		gMotorSpeed.cimR = mXboxRight;
	}
	
#ifdef SAFE_TURN_TESTING
	gMotorSpeed.cimL = -gMotorSpeed.cimR;	// must be after everything
#endif
}

#ifdef ROBOT_2007
void doRamps(void) {
	static char deployBtnTime = 0;
	if (mBtnDeployRamps) {
		if (++deployBtnTime > (50/26.2))		// safety: must hold to activate
			mRelayRampsDeploy = 1;
	} else
		deployBtnTime = 0;
}
#endif //ROBOT_2007

//********************************************************************************

// called at initialization
void ReadUserOptionsFromEPROM(void)
{
	gUserOption.DriveMethod = EEPROM_read( kEPROMAdr_drive_method );
	gUserOption.RawDrive = EEPROM_read( kEPROMAdr_drive_type );
}

//********************************************************************************

void SetUserOptions(void)
{
	//must be in user mode with Meta button pressed to set options
	if (!user_display_mode || !mBtnMeta) return;
	
	if (mBtnDriveMethod && !gUserOption.oldDriveMethodBtn)
	{
		if (++gUserOption.DriveMethod > 6 || gUserOption.DriveMethod<=0)
			gUserOption.DriveMethod=1;
		EEPROM_write(kEPROMAdr_drive_method, gUserOption.DriveMethod);	//save across resets
	}
	if (mBtnRawDrive && !gUserOption.oldRawDriveBtn)
	{
		if (++gUserOption.RawDrive != 2)
			gUserOption.RawDrive=1;
		EEPROM_write(kEPROMAdr_drive_type, gUserOption.RawDrive);	//save across resets
	}
	if (mBtnServiceMode)
		gUserOption.ServiceMode = 1;
	
	if (gUserOption.ServiceMode)
		gUserByteOI = 210;
	else
		gUserByteOI = 100 * gUserOption.RawDrive + gUserOption.DriveMethod;

	gUserOption.oldDriveMethodBtn = mBtnDriveMethod;
	gUserOption.oldRawDriveBtn = mBtnRawDrive;
}

//********************************************************************************

void DisplayUserOptions(void)
{
	if (user_display_mode)
		User_Mode_byte = gUserByteOI;
	else
		SendLEDs();
}

//********************************************************************************

#define TURNKNOB_THRESHOLD	2	// knob motion threshold
#define TURN_GAIN			0.4
#define TURN_TIMEOUT		(int) (2000 / 26.2)

#define ENCODER_TICKS_PER_360	940
#define JOYSTICK_IN_USE_THRESHOLD	5

static struct {
	long initialRobotBearing;
	int initialValue;
	int lastValue;
	int timeout;
	unsigned wasMoving :1;
	unsigned active :1;
} sKnob = {0, 0, -1, 0, 0};

/*
static void ResetTurnKnob(void)
{
	TurnKnobZeroValue = -1;
}*/

// TurnKnobDrive uses the Turn Knob to control absolute turning.
// Keeps the forward/back component from the joysticks. This makes it impossible to drive
// with tank drive, because you'd have to have both hands pushing forward/back on the
// joysticks and another hand on the turn knob! [dcl]
static void TurnKnobDrive(void)
{
#ifdef TURN_KNOB
	struct {
		int left;
		int right;
		int turn;
		int fwd;
	} drive = {0,0,0,0};

	int actualKnobValue, relativeKnobValue;
	long actualBearing, relativeBearing;
	long targetBearing;
	int bearingError;

	actualKnobValue = mTurnPot1;	// We'll need to change this for a continous dual wiper pot later.
	actualBearing = gEncoderLeft.posNow - gEncoderRight.posNow;

	if (sKnob.lastValue == -1)
		sKnob.lastValue = actualKnobValue;

	// Is the knob moving?
	if (mAbsDiff(sKnob.lastValue, actualKnobValue) > TURNKNOB_THRESHOLD)
	{			// Knob Is Moving
		if (!sKnob.active)		// Did it just start moving?
		{
			sKnob.initialRobotBearing = actualBearing;
			sKnob.initialValue = sKnob.lastValue;
		}

		sKnob.active = 1;
		sKnob.timeout = TURN_TIMEOUT;

		sKnob.lastValue = actualKnobValue;
	}

	if (sKnob.timeout > 0)
		sKnob.timeout--;
	else
		sKnob.active = 0;

	if (!sKnob.active)
		return;


	// If we've reached here, the knob is active.


	relativeBearing = actualBearing - sKnob.initialRobotBearing;
	
	if (gLoop.onSecondA)
		printf("Current Tick Difference %ld (relative), %ld (actual)\r\n", relativeBearing, actualBearing); // Beware of printf with longs.


	// Note that the Bearing variables don't yet hold 0-32767 bearings; we need to scale them first.
	
	// SCALE ACTUAL BEARING
	// FIXME: It would probably be better to % ENCODER_TICKS_PER_360 before doing the scaling, since this might risk overflow [dcl]
	relativeBearing = (long)(relativeBearing / ENCODER_TICKS_PER_360) << 16;	// 65536 <=> 360°
	if (relativeBearing >= 0)
		relativeBearing &= (65536-1);	// equivalent to mod 65536
	else
		relativeBearing = 65536 - ( (-relativeBearing) & (65536-1) );

	// SCALE TARGET BEARING - approx [-128, 127] to [0, 65536]
	if (relativeKnobValue >= 0)
		targetBearing = (unsigned int)( (long)(relativeKnobValue << 8) & (long)(65536-1) );	// equiv to: relKnobVal * 2^8  % 65536
	else
		targetBearing = 65536 - (unsigned int)( (long)((-relativeKnobValue) << 8) & (long)(65536-1) );
																							// equiv to: 65536 - ( (-relKnobVal) * 2^8  % 65536 )
	/*
	bearingError = mDivideBy128( (signed int)(
		(unsigned int)relativeBearing) -
		(unsigned int)targetBearing
	) ); // Magic with 2's complement representation
	// bearingError in [-256, 255]
*/
	if (gLoop.onSecondA) {
	//	printf("TurnPot1: %d; ZeroValue: %d\r\n", (int)mTurnPot1, (int)TurnKnobZeroValue);
		printf("Current Bearing %d\r\n", (int)relativeBearing); // Beware of printf with longs.
		printf("Target Bearing: %d\r\n", targetBearing);
		printf("Error: %d\r\n", bearingError);
	}


	// Read in current drive info, and add in our turn component.

	drive.left = gMotorSpeed.cimL;
	drive.right = gMotorSpeed.cimR;
	
	drive.fwd =  (drive.left + drive.right)/2;
	drive.turn = (drive.left - drive.right)/2;
	
	if (mAbsolute(drive.turn) > JOYSTICK_IN_USE_THRESHOLD) {
		// The joystick is being used, disable the turn knob.
		sKnob.active = 0;
		sKnob.timeout = 0;
		return;
	}
	
	// Set the turning component.
	drive.turn = -bearingError * TURN_GAIN;
	
	drive.left =  drive.fwd + drive.turn;
	drive.right = drive.fwd - drive.turn;
	
	gMotorSpeed.cimL = Limit127(drive.left);
	gMotorSpeed.cimR = Limit127(drive.right);	//actual PWM's handled in DriveMotors()
#endif // TURN_KNOB
}

//********************************************************************************

static void ReverseRobotControls(void)
{
	static struct {
		unsigned btnDownBefore : 1;
		unsigned controlsReversed : 1;
	} flags = {0,0};
		
	overlay int temp;
		
	if (mReverseRobot && !flags.btnDownBefore)
		flags.controlsReversed = !flags.controlsReversed;
	flags.btnDownBefore = mReverseRobot;

	if (disabled_mode) flags.controlsReversed = 0;	//reset on disable
	
 
	//reverse the motor globals.  Must be called after Joystick commands.
	if (flags.controlsReversed)
	{
		temp = gMotorSpeed.cimL;
		gMotorSpeed.cimL = -gMotorSpeed.cimR;
		gMotorSpeed.cimR = -temp;
	}
	
//	gLED.MotorsReversed = flags.controlsReversed;

}

//***********************************************************************************************

void JoystickXYDrive(void)
{
	struct {
		int left;
		int right;
		int turn;
		int fwd;
	} drive = {0,0,0,0};

	drive.fwd = addDeadband(mJoyForward);
	drive.turn = -addDeadband(mJoyTurn);
	
	if (sTurnRateLimited) drive.turn=0;

	drive.left =  drive.fwd + drive.turn;
	drive.right = drive.fwd - drive.turn;

	gMotorSpeed.cimL = Limit127(drive.left);
	gMotorSpeed.cimR = Limit127(drive.right);	//actual PWM's handled in DriveMotors()
}

//***********************************************************************************************

void JoystickXYDrive_HalfTurn(void)
{
	struct {
		int left;
		int right;
		int turn;
		int fwd;
	} drive = {0,0,0,0};

	drive.fwd = addDeadband(mJoyForward);
	drive.turn = -addDeadband(mJoyTurn);

	drive.turn /= 2;	//reduce turning input.
	
	if (sTurnRateLimited) drive.turn=0;

	drive.left = drive.fwd + drive.turn;
	drive.right = drive.fwd - drive.turn;

	gMotorSpeed.cimL = Limit127(drive.left);
	gMotorSpeed.cimR = Limit127(drive.right);	//actual PWM's handled in DriveMotors()
}

//********************************************************************************

void JoystickXYDrive_ZSquaredTurn(void)
{
	struct {
		int left;
		int right;
		int turn;
		int fwd;
	} drive = {0,0,0,0};
	char signOfTurn;	

	drive.fwd = addDeadband(mJoyForward);
	drive.turn = -addDeadband(mJoyTurn);

	signOfTurn=0;
	if (drive.turn<0) signOfTurn=1;

	drive.turn*=drive.turn;
	drive.turn >>= 7;	//divide by 128; drive.turn is positive definite
	
	if (signOfTurn) drive.turn = -drive.turn;
	
	if (sTurnRateLimited) drive.turn=0;

	drive.left = drive.fwd + drive.turn;
	drive.right = drive.fwd - drive.turn;

	gMotorSpeed.cimL = Limit127(drive.left);
	gMotorSpeed.cimR = Limit127(drive.right);	//actual PWM's handled in DriveMotors()
}

//********************************************************************************

// Adds more turning at high speeds
void JoystickXYDrive_HighSpeedTurn(void)
{
	struct {
		int left;
		int right;
		int turn;
		int fwd;
	} drive = {0,0,0,0};

	struct {
		unsigned turn:1;
		unsigned fwd:1;
	} sign = {0,0};
	int reduction;

	drive.fwd = addDeadband(mJoyForward);
	drive.turn = -addDeadband(mJoyTurn);

	if (sTurnRateLimited) drive.turn=0;

	if (drive.fwd <0)
	{
		drive.fwd = -drive.fwd;
		sign.fwd  = 1;
	}


	if (drive.turn<0) sign.turn = 1;
	drive.turn*=drive.turn;
	drive.turn >>= 7;	//divide by 128; drive.turn is positive definite

	//reduce fwd by alpha * abs(normalized turn), 0<=alpha<1;  alpha defined as (kFwdReduction/128)
	//both drive.fwd and .turn are positive here.
	#define kFwdReduction 64L
	reduction = ((long)drive.fwd * drive.turn * kFwdReduction) >> 14;	//divide by 128*128
	drive.fwd -= reduction;

	if (sign.fwd) drive.fwd = -drive.fwd;
	if (sign.turn) drive.turn = -drive.turn;
	
	drive.left = drive.fwd + drive.turn;
	drive.right = drive.fwd - drive.turn;

	gMotorSpeed.cimL = Limit127(drive.left);
	gMotorSpeed.cimR = Limit127(drive.right);	//actual PWM's handled in DriveMotors()
}

//********************************************************************************

// threeWaySwitch() returns kDown=-1, kOff=0, or kUp=1
// [Down=165, off=0, up=79] - uses 33K ohm and 66K (actually 2-33K) on an analog input
/*char threeWaySwitch(void)
{
	overlay char result;
	if (mThreewayswitch < 40) 
		result=kOff;
	else if (mThreewayswitch < 120)
		result=kUp;
	else
		result=kDown;
	return result;
}*/

//********************************************************************************

/* May contain useful calibration[dcl]
char readBallLauncherSpeedFromOI(void)
{
	unsigned char input = mBallLauncherspeed;	//need a copy to modify
	unsigned char speed;

//	if (gLoop.onSecond)	
//		printf("mBallLauncherspeed=%d\r", (int)launcherSpeed);
//		//low value of 18 drifts with time (and temperature?)
//	
	calibrateInput( &input, 18);	//set second arg should be used to calibrate low value pot.

	speed= (254 - input)>>1;	//invert and put on {0,127}
	speed =  LimitRange(speed,33,127);	//set min speed			
//	if (gLoop.onSecond)
//		printf("ShootingSpeed=%d\r",(int)speed);

	return speed;
}
*/

//********************************************************************************

//static char JoystickTrimLED(unsigned char in)
//{
//#define kDeadBand 4
//	if (in> (int) 127+kDeadBand)
//		return (gLoop.count>>1)&1;
//	if (in< (int) 127-kDeadBand)
////		return 10 == (10 & gLoop.count); 
//		return 1;
//	return 0;
//}

//********************************************************************************

//********************************************************************************

#if 0

void JoystickTankDrive_HalfTurn(void)
{
	struct {
		int left;
		int right;
		int turn;
		int fwd;
	} drive = {0,0,0,0};

	drive.left = addDeadband(mJoyLeftCIM);
	drive.right = addDeadband(mJoyRightCIM);

	drive.fwd = (drive.left + drive.right) /2;
	drive.turn = (drive.left - drive.right)/2;

	//scale turning (skip if maintaining)		// [!!!] FIXME why is this if(0)? [dcl]
	if (0) drive.turn /= 2;	//reduce turning input.
	
	if (sTurnRateLimited) drive.turn=0;

	drive.left = drive.fwd + drive.turn;
	drive.right = drive.fwd - drive.turn;

	gMotorSpeed.cimL = Limit127(drive.left);
	gMotorSpeed.cimR = Limit127(drive.right);	//actual PWM's handled in DriveMotors()
}

//***********************************************************************************************

void JoystickTankDrive_ZSquaredTurn(void)
{
	struct {
		int left;
		int right;
		int turn;
		int fwd;
	} drive = {0,0,0,0};
	char signOfTurn;

	drive.left = addDeadband(mJoyLeftCIM);
	drive.right = addDeadband(mJoyRightCIM);

	drive.fwd = (drive.left + drive.right) /2;
	drive.turn = (drive.left - drive.right)/2;

	signOfTurn=0;
	if (drive.turn<0) signOfTurn=1;

	drive.turn*=drive.turn;
	drive.turn >>= 7;	//divide by 128; drive.turn is positive definite
	
	if (signOfTurn) drive.turn = -drive.turn;
	
	if (sTurnRateLimited) drive.turn=0;

	drive.left =  drive.fwd + drive.turn;
	drive.right = drive.fwd - drive.turn;

	gMotorSpeed.cimL = Limit127(drive.left);
	gMotorSpeed.cimR = Limit127(drive.right);	//actual PWM's handled in DriveMotors()
}

#endif // 0
