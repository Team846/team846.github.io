// controls.c: Handles user controls and user options.
//
#include <stdio.h>

#include "common.h"
#include "controls.h"
#include "lcd.h"
#include "lift.h"
#include "hybrid.h"
#include "drive.h"

struct UserOption gUserOption =
{ 1, 1, 0 };
static char sTurnRateLimited = 0;

//***********************************************************************

static void ReverseRobotControls(void);
//static char JoystickTrimLED(unsigned char in);

static void JoystickXYDrive(void);
static void JoystickXYDrive_HalfTurn(void);
static void JoystickXYDrive_ZSquaredTurn(void);
static void JoystickXYDrive_HighSpeedTurn(void);
static void JoystickXYDrive_BrakedDrive(void);
static void TurnKnobDrive(void);
static void PrecisionDrive(Drive *drive);
static void ServiceMode(void);
static void JoystickXYDrive_TrackTurn(void);
//static void GyroTest(void);
//***********************************************************************

void Brake_Test(void) {	// only called from Disabled mode
	if (OI_BRAKE_LEFT)
	{
		gMotorSpeed.left=0;
		gMotorSpeed.brakeLeft=1;
		if (mRolling())
			printf("Braking LEFT\r");
	}
	if (OI_BRAKE_RIGHT)
	{
		gMotorSpeed.right=0;
		gMotorSpeed.brakeRight=1;
		if (mRolling())
			printf("Braking RIGHT\r");
	}
}

void Hat_Timer_Indicator(void) {
//	// !! to force boolean 0/1 interpretation
//	mOILEDHat1Red = mOILEDHat2Red = !!(gLoop.count & 0x01);
//	printf("ct:%d\r",gLoop.count & 0x01);
//	 FIXME: TEST
//	long gameTimeRemaining = 35000/26.2 - gLoop.enabledCount;
	long gameTimeRemaining = 120000/26.2 - gLoop.enabledCount;
	
	if (gameTimeRemaining > 0)
		gameTimeRemaining--;
	
	// !! LED expressions to force boolean 0/1 interpretation
	if (gameTimeRemaining <= 0) {
		mOILEDHat1Red = mOILEDHat2Red = 1;
	} else if (gameTimeRemaining < 5000/26.2) {
		mOILEDHat1Red = mOILEDHat2Red = !!(gLoop.count & 0x02);
	} else if (gameTimeRemaining < 10000/26.2) {
		mOILEDHat1Red = mOILEDHat2Red = !!(gLoop.count & 0x04);
	} else if (gameTimeRemaining < 20000/26.2) {
		mOILEDHat1Red = mOILEDHat2Red = !!(gLoop.count & 0x08);
	} else if (gameTimeRemaining < 30000/26.2) {
		mOILEDHat1Red = mOILEDHat2Red = !!(gLoop.count & 0x10);
	} else {
		mOILEDHat1Red = mOILEDHat2Red = 0;
	}
}

void controls(void)
{
	//	GyroTest();
//	RELAY_VACUUM=1;
	RELAY_VACUUM = (!OI_VACUUM_OFF);
	if (Fork_GetPos() > FORK_POS_VACUUMOFF)
		RELAY_VACUUM = 0;
//	printf("%c",(int)'0'+RELAY_VACUUM);

#ifdef TURN_LIMITING
	sTurnRateLimited = (mAbsolute(gRobotTurnSpeed) > 300);
#else
	sTurnRateLimited = 0;
#endif
	mOILEDTurnLimited = sTurnRateLimited;

	Hat_Timer_Indicator();
	
	Lift_Controls();

	switch (gUserOption.DriveMethod)
	{
	case 1:
		JoystickXYDrive();
		break;
	case 2:
		JoystickXYDrive_HalfTurn();
		break;
	case 3:
		JoystickXYDrive_ZSquaredTurn();
		break;
	case 4:
		JoystickXYDrive_HighSpeedTurn();
		break;
	case 5:
		JoystickXYDrive_BrakedDrive();
		break;
	case 6:
		JoystickXYDrive_TrackTurn();
		break;
	case 7:
		// Testing purposes
		HybridRun();
		if (mRolling())
			printf("GyrogainPot %d\r", OI_USERPOT3);
		break;
	default:
		printf("[!] Unknown drive method, resetting to 1\r");
		gUserOption.DriveMethod = 1;
	}

	// FIXME: put these two back
	ReverseRobotControls();	// must be after DriveJoysticks

	ServiceMode();

	if (OI_BRAKE)
	{
		gMotorSpeed.left=gMotorSpeed.right=0;
		gMotorSpeed.brakeLeft=gMotorSpeed.brakeRight=1;
		if (mRolling())
			printf("Braking button\r");
	}

	// ShiftControls must be last, because it reads gMotorSpeed to see if we're moving.
	Drive_ShiftControls();

#ifdef SAFE_TURN_TESTING
	gMotorSpeed.left = -gMotorSpeed.right; // must be after everything
#endif

}

//********************************************************************************

// called at initialization
void ReadUserOptionsFromEPROM(void)
{
	gUserOption.DriveMethod = EEPROM_ReadByte(EEPROM_DRIVE_METHOD);
	gUserOption.ServoDrive = EEPROM_ReadByte(EEPROM_SERVO_DRIVE);
}

//********************************************************************************

void SetUserOptions(void)
{
	static struct
	{
		char DriveMethodBtn;
		char ServoDriveBtn;
	} debounce =
	{ 0, 0 };

	//	printf("User %d / Meta %d\r", user_display_mode, OI_META);


	//Must be in user mode with Meta button pressed to set options
	if (!user_display_mode)
		return;
	if (!OI_META)
		return;
	if (!OI_XBOX_CONNECTED)
		return; //look for xbox powering up/down, sending all btns down.

	if (OI_EPROM_DRIVEMETHOD && !debounce.DriveMethodBtn)
	{
		if (++gUserOption.DriveMethod > 7 || gUserOption.DriveMethod <= 0)
			gUserOption.DriveMethod = 1;
		EEPROM_WriteByte(EEPROM_DRIVE_METHOD, gUserOption.DriveMethod);
	}
	if (OI_EPROM_SERVODRIVE && !debounce.ServoDriveBtn)
	{
		gUserOption.ServoDrive = !gUserOption.ServoDrive;
		EEPROM_WriteByte(EEPROM_SERVO_DRIVE, gUserOption.ServoDrive);
	}
	if (OI_EPROM_SERVICEMODE) // enter service mode until robot reset
		gUserOption.ServiceMode = 1;

	if (gUserOption.ServiceMode)
		gUserByteOI = 255;
	else
		gUserByteOI = 100 * gUserOption.ServoDrive + gUserOption.DriveMethod;

	debounce.DriveMethodBtn = OI_EPROM_DRIVEMETHOD;
	debounce.ServoDriveBtn = OI_EPROM_SERVODRIVE;
}

//********************************************************************************

void PrecisionDrive(Drive *drive)
{
	drive->fwd = drive->fwd * 7/10;
	drive->turn = drive->turn * 7/10;
}

//********************************************************************************

void ServiceMode(void)
{
	if (gUserOption.ServiceMode)
	{
		if (mRolling())
			printf("SERVICE MODE controls\r\n");
		gMotorSpeed.left = OI_JOYFWD -127;
		gMotorSpeed.right = OI_JOYTURN-127;
	}
}

//********************************************************************************

static void ReverseRobotControls(void)
{
	static struct
	{
		unsigned btnDownBefore : 1;
		unsigned controlsReversed : 1;
	} flags =
	{ 0, 0 };

	int temp;

	if (OI_REVERSE && !flags.btnDownBefore)
		flags.controlsReversed = !flags.controlsReversed;
	flags.btnDownBefore = OI_REVERSE;

	if (disabled_mode)
		flags.controlsReversed = 0; //reset on disable


	//reverse the motor globals.  Must be called after Joystick commands.
	if (flags.controlsReversed)
	{
		temp = gMotorSpeed.left;
		gMotorSpeed.left = -gMotorSpeed.right;
		gMotorSpeed.right = -temp;
		
		temp = gMotorSpeed.brakeLeft;
		gMotorSpeed.brakeLeft = gMotorSpeed.brakeRight;
		gMotorSpeed.brakeRight = temp;
	}

	mOILEDMotorsReversed = flags.controlsReversed;
}

//***********************************************************************************************

void printDrive(Drive drive)
{
	printf("Fwd %d Turn %d Left %d Right %d\r", drive.fwd, drive.turn,
			drive.left, drive.right);
}

void JoystickXYDrive(void)
{
	Drive drive =
	{ 0, 0, 0, 0 };

	drive.fwd = addDeadband(OI_JOYFWD);
	drive.turn = -addDeadband(OI_JOYTURN);

	if (sTurnRateLimited)
		drive.turn=0;

	drive.left = drive.fwd + drive.turn;
	drive.right = drive.fwd - drive.turn;

	gMotorSpeed = drive; //actual PWM's handled in Drive_Do()

	if (gLoop.f.printB)
		printDrive(drive);
}

//***********************************************************************************************

void JoystickXYDrive_HalfTurn(void)
{
	Drive drive =
	{ 0, 0, 0, 0 };

	drive.fwd = addDeadband(OI_JOYFWD);
	drive.turn = -addDeadband(OI_JOYTURN);

	drive.turn /= 2; //reduce turning input.

	if (sTurnRateLimited)
		drive.turn=0;

	drive.left = drive.fwd + drive.turn;
	drive.right = drive.fwd - drive.turn;

	gMotorSpeed = drive; //actual PWM's handled in Drive_Do()
}

//********************************************************************************

int SquaredCurve128(int z128)
{
	int squared = (z128*z128) >>7; //positive definite	
	return z128 >=0 ? squared : -squared; //restore sign
	//	return mDivideBy128(z128 >= 0 ? z128*z128 : -z128*z128);
}

void JoystickXYDrive_ZSquaredTurn(void)
{
	struct
	{
		int left;
		int right;
		int turn;
		int fwd;
	} drive =
	{ 0, 0, 0, 0 };

	drive.fwd = addDeadband(OI_JOYFWD);
	drive.turn = -addDeadband(OI_JOYTURN);

	drive.turn = SquaredCurve128(drive.turn);

	if (sTurnRateLimited)
		drive.turn=0;

	drive.left = drive.fwd + drive.turn;
	drive.right = drive.fwd - drive.turn;

	gMotorSpeed = drive; //actual PWM's handled in Drive_Do()
}

//********************************************************************************


// Adds more turning at high speeds
void JoystickXYDrive_HighSpeedTurn(void)
{
	Drive drive;

	drive.fwd = addDeadband(OI_JOYFWD);
	drive.turn = -addDeadband(OI_JOYTURN);

	if (OI_PRECISION)
		PrecisionDrive(&drive);
	//	if (sTurnRateLimited) drive.turn=0;

	drive.turn = SquaredCurve128(drive.turn);
	HighSpeedTurn(&drive);

	gMotorSpeed = drive; //actual PWM's handled in Drive_Do()
}

#define MAX_BRAKING 3

void JoystickXYDrive_BrakedDrive(void)
{
	Drive drive;
	drive.fwd = addDeadband(OI_JOYFWD);
	drive.turn = -addDeadband(OI_JOYTURN);

	if (OI_PRECISION)
		PrecisionDrive(&drive);

	// Positive turn = right turn

	BrakedDrive(&drive, MAX_BRAKING);

	gMotorSpeed = drive; //actual PWM's handled in Drive_Do()
}

//********************************************************************************
static long targetBearing = 0;
#define MAX_WINDUP (DRIVE_TICKS_PER_ROBOT_REV/12)	//30 degrees
void Controls_Init(void)
{
	targetBearing = encoder_diff();
}

void JoystickXYDrive_TrackTurn(void)
{
	Drive drive;
	int turnRate;
	const long currentBearing = encoder_diff();
	drive.fwd = addDeadband(OI_JOYFWD);
	turnRate = -addDeadband(OI_JOYTURN);
	turnRate = mDivideBy4(turnRate); //on range of {-31,31} (Max Turn rate is then 31*38 ticks/sec)

	if (turnRate == 0)
	{
		if (mAbsolute(drive.fwd)<=1)
		{
			// not driving
			targetBearing = currentBearing;
		}
		// otherwise, don't reset bearing (drive straight)
	}
	else
	{
		if (mSign(targetBearing-currentBearing) != mSign(turnRate))
		{
			targetBearing = currentBearing;
		}
	}
	//	if (turnRate == 0) targetBearing=encoder_diff();

	// Positive turn = right turn
	targetBearing += turnRate; //may need to scale this
	mLimitRange(targetBearing, currentBearing-MAX_WINDUP, currentBearing+MAX_WINDUP);
	drive.turn = computeTurn(targetBearing);
	BrakedDrive(&drive, 8);

	gMotorSpeed = drive; //actual PWM's handled in Drive_Do()
	//	PrintDrive(&gMotorSpeed);
}
//********************************************************************************

#define TURNKNOB_THRESHOLD	2	// knob motion threshold
#define TURN_GAIN			0.4
#define TURN_TIMEOUT		(int) (2000 / 26.2)

#define ENCODER_TICKS_PER_360	940
#define JOYSTICK_IN_USE_THRESHOLD	5

static struct
{
	long initialRobotBearing;
	int initialValue;
	int lastValue;
	int timeout;
	unsigned wasMoving :1;
	unsigned active :1;
} sKnob =
{ 0, 0, -1, 0, 0 };

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
	struct
	{
		int left;
		int right;
		int turn;
		int fwd;
	}drive =
	{	0,0,0,0};

	int actualKnobValue, relativeKnobValue;
	long actualBearing, relativeBearing;
	long targetBearing;
	int bearingError;

	actualKnobValue = mTurnPot1; // We'll need to change this for a continous dual wiper pot later.
	actualBearing = gEncoderLeft.posNow - gEncoderRight.posNow;

	if (sKnob.lastValue == -1)
	sKnob.lastValue = actualKnobValue;

	// Is the knob moving?
	if (mAbsDiff(sKnob.lastValue, actualKnobValue) > TURNKNOB_THRESHOLD)
	{ // Knob Is Moving
		if (!sKnob.active) // Did it just start moving?

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

	if (gLoop.printA)
	printf("Current Tick Difference %ld (relative), %ld (actual)\r\n", relativeBearing, actualBearing); // Beware of printf with longs.


	// Note that the Bearing variables don't yet hold 0-32767 bearings; we need to scale them first.

	// SCALE ACTUAL BEARING
	// FIXME: It would probably be better to % ENCODER_TICKS_PER_360 before doing the scaling, since this might risk overflow [dcl]
	relativeBearing = (long)(relativeBearing / ENCODER_TICKS_PER_360) << 16; // 65536 <=> 360°
	if (relativeBearing >= 0)
	relativeBearing &= (65536-1); // equivalent to mod 65536

	else
	relativeBearing = 65536 - ( (-relativeBearing) & (65536-1) );

	// SCALE TARGET BEARING - approx [-128, 127] to [0, 65536]
	if (relativeKnobValue >= 0)
	targetBearing = (unsigned int)( (long)(relativeKnobValue << 8) & (long)(65536-1) ); // equiv to: relKnobVal * 2^8  % 65536

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
	if (gLoop.f.onSecondA)
	{
		//	printf("TurnPot1: %d; ZeroValue: %d\r\n", (int)mTurnPot1, (int)TurnKnobZeroValue);
		printf("Current Bearing %d\r\n", (int)relativeBearing); // Beware of printf with longs.
		printf("Target Bearing: %d\r\n", targetBearing);
		printf("Error: %d\r\n", bearingError);
	}

	// Read in current drive info, and add in our turn component.

	drive.left = gMotorSpeed.left;
	drive.right = gMotorSpeed.right;

	drive.fwd = (drive.left + drive.right)/2;
	drive.turn = (drive.left - drive.right)/2;

	if (mAbsolute(drive.turn) > JOYSTICK_IN_USE_THRESHOLD)
	{
		// The joystick is being used, disable the turn knob.
		sKnob.active = 0;
		sKnob.timeout = 0;
		return;
	}

	// Set the turning component.
	drive.turn = -bearingError * TURN_GAIN;

	gMotorSpeed = drive; //actual PWM's handled in Drive_Do()
#endif // TURN_KNOB
}

//********************************************************************************


//void turnInPlace(int angle10)
//{
//	int gain256= OI_USERPOT2;
//	long error;
//	struct
//	{
//		int left;
//		int right;
//		int turn;
//		int fwd;
//	} drive =
//	{ 0, 0, 0, 0 };
//
//	error = gain256 * (angle10 - Get_Gyro_Angle());
//
//	printf("Gyro: %d\r", (int)Get_Gyro_Angle());
//	printf("error: %d\r", (int) error);
//	printf("Angle10: %d\r\r", (int) angle10);
//
//	drive.turn = Limit127(mDivideBy256(error));
//	mLimitRange(drive.turn,-40,40);
//
//	drive.fwd = 0;
//	drive.left = drive.fwd + drive.turn;
//	drive.right = drive.fwd - drive.turn;
//
//	if (0 && gLoop.f.onSecond)
//	{
//		stdout_serial_port = SERIAL_PORT_TWO;
//		ClearLCD();
//		printf("E:%d G:%d", (int) (angle10 -Get_Gyro_Angle()), (int)gain256);
//		Write_Serial_Port_Two(254);
//		Write_Serial_Port_Two(192); //new line
//		printf("Angle10: %d", (int)Get_Gyro_Angle());
//
//		stdout_serial_port = SERIAL_PORT_ONE;
//	}
//
//	gMotorSpeed = drive; //actual PWM's handled in Drive_Do()
//}

//static void GyroTest(void)
//{
//
//	static struct
//	{
//		unsigned XboxA :1;
//		unsigned XboxB :1;
//	} old =
//	{ 0, 0 };
//
//	static int timer=0;
//	static int angle10=0;
//
//	if (OI_XBOX_A && !old.XboxA)
//	{
//		angle10=900;
//		Reset_Gyro_Angle();
//		timer = (4000/26.2);
//	}
//
//	if (OI_XBOX_B && !old.XboxB)
//	{
//		angle10=-900;
//		Reset_Gyro_Angle();
//		timer = (4000/26.2);
//	}
//
//	if (timer >=0)
//	{
//		timer--;
//		turnInPlace(angle10);
//	}
//
//	old.XboxA = OI_XBOX_A;
//	old.XboxB = OI_XBOX_B;
//}

void noWarnings(void)
{
//	GyroTest();
	JoystickXYDrive();
	JoystickXYDrive_HalfTurn();
	JoystickXYDrive_HighSpeedTurn();
	JoystickXYDrive_ZSquaredTurn();
	ReverseRobotControls();
	TurnKnobDrive();
//	PrecisionDrive();
}

//**********************************************************