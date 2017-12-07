/*******************************************************************************
*	TITLE:		controls.c 
******************************************************************************/

#include "ifi_aliases.h"
#include "ifi_default.h"
#include "ifi_utilities.h"
#include "connections.h"
#include "utilities.h"
#include "terminal.h"
#include <stdio.h>
#include "turret.h"
#include "controls.h"
#include "tracking.h" //
#include "interruptHandlers.h"
#include "MotorDrive.h"
#include "eeprom.h"
#include "LED.h"
#define kLiftSpeedRear 50
#define kLiftSpeedFront 50
#define kLiftSpeedLaunch 80


struct userOption gUserOption = {1,0,1,0};

pulse_response gTurnPulse;	//initiallized to zero's

char gBallLauncherSpeed=0;
char gUserByteOI;

static char gTurnRateLimited=0;

lift gLift = {0,0,0,0,kFrontLiftActive};	
drive gDrive = {0,0,0,0};
sensors gSensors = {0,0,0,0};

//***************** Local Prototypes *********************************
static void OperatorShiftGear(void);
static void doDriveJoysticks(void);
static void doBallLauncher(void);
static  void CheckOIforFrontRearLiftSelection(void);
static void ReverseRobotControls(void);
static char JoystickTrimLED(unsigned char in);

static void setDivertorRelayAndLifts(void);
//static void ballQueue(void);
//void checkDiverter(void);
//void getDirection (void);
//void driveLifts (void);
//static void ballShoot(void);
static void unjamRear (void);
static void unjamFront (void);
//void checkKills (void);
static void checkOIForKillSwitches (void);

static void sum_response( pulse_response *p, char delta_in );
//***********************************************************************
void controls(void)
{
	OperatorShiftGear();
	doDriveJoysticks();
	ReverseRobotControls();	//must be after the DriveJoysticks
	doBallLauncher();	//handles launcher speeds from camera etc.
	
	if (gToggleSweep && !(TARGET_IN_VIEW & GetRawTrackingState())) {
		TurretSweep();
	}
	MoveTurretManual();

if (gLoop.onSecondA)
	printf("ball in sec. pos = %d\r", (int) mBallin2nd);

if (gLoop.onSecondA)
	printf("mPWMTurretMotor=%d\r\r",(int)mPWMTurretMotor);


}
//********************************************************************************
void OperatorShiftGear(void)
{
	if (mSwHiGear) {
		gGearBox.cmdGearObjective=kHighGear;
		printf("mSwHiGear\r");
	}
	if (mSwLowGear) {
		gGearBox.cmdGearObjective=kLowGear;
		printf("mSwLowGear\r");
	}
}
//********************************************************************************

void SetGearshiftRelays(void)
{
	mRelayHiGear=mRelayLowGear=0;
	if (gGearBox.cmdGearObjective==kLowGear)
		mRelayLowGear=1;
	if (gGearBox.cmdGearObjective==kHighGear)
		mRelayHiGear=1;
}
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
	if (!user_display_mode || !mMetaButton) return;
	
	if (mDriveMethodBtn && !gUserOption.oldDriveMethodBtn)
	{
		if (++gUserOption.DriveMethod > 6 || gUserOption.DriveMethod<=0)
			gUserOption.DriveMethod=1;
		EEPROM_write(kEPROMAdr_drive_method, gUserOption.DriveMethod);	//save across resets
	}
	if (mRawDriveBtn && !gUserOption.oldRawDriveBtn)
	{
		if (++gUserOption.RawDrive != 2)
			gUserOption.RawDrive=1;
		EEPROM_write(kEPROMAdr_drive_type, gUserOption.RawDrive);	//save across resets
	}
	
	gUserByteOI = 100 * gUserOption.RawDrive + gUserOption.DriveMethod;

	gUserOption.oldDriveMethodBtn = mDriveMethodBtn;
	gUserOption.oldRawDriveBtn = mRawDriveBtn;
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
void doDriveJoysticks(void)
{
	void doDriveJoysticks1(void);
	void doDriveJoysticks2(void);
	void doDriveJoysticks3(void);
	void doDriveJoysticks4(void);
	void doDriveJoysticks5(void);
	void doDriveJoysticks6(void);

	gLED.LeftJoyY = JoystickTrimLED(mLeftCIMJoy);
	gLED.RightJoyY = JoystickTrimLED(mRightCIMJoy);
	gLED.RightJoyX = JoystickTrimLED(mJoyTurn);

	gTurnRateLimited = (mAbsolute(gRobotTurnSpeed) > 300);
	gLED.turnRateLimited = gTurnRateLimited;
		
	switch (gUserOption.DriveMethod)
	{
		case 1:
			doDriveJoysticks1();	break;
		case 2:
			doDriveJoysticks2();	break;
		case 3:
			doDriveJoysticks3();	break;
		case 4:
			doDriveJoysticks4();	break;
		case 5:
			doDriveJoysticks5();	break;
		case 6:
			doDriveJoysticks6();	break;
		default:
			if (gLoop.onSecond)
				printf("Unknown DriveType!\r");
	}
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
	
	gLED.MotorsReversed = flags.controlsReversed;

}
//********************************************************************************
void doDriveJoysticks1(void)
{
	struct {
		int left;
		int right;
		int turn;
		int fwd;
	} drive = {0,0,0,0};

	drive.left = addDeadband(mLeftCIMJoy);
	drive.right = addDeadband(mRightCIMJoy);

	drive.fwd = (drive.left + drive.right) /2;
	drive.turn = (drive.left - drive.right)/2;

//scale turning (skip if maintaining)
	if (0) drive.turn /= 2;	//reduce turning input.
	
	if (gTurnRateLimited) drive.turn=0;

	drive.left = drive.fwd + drive.turn;
	drive.right = drive.fwd - drive.turn;

	gMotorSpeed.cimL = Limit127(drive.left);
	gMotorSpeed.cimR = Limit127(drive.right);	//actual PWM's handled in DriveMotors()
}
//***********************************************************************************************
// reduced turning rate
void doDriveJoysticks2(void)
{
	struct {
		int left;
		int right;
		int turn;
		int fwd;
	} drive = {0,0,0,0};
	char signOfTurn;

	drive.left = addDeadband(mLeftCIMJoy);
	drive.right = addDeadband(mRightCIMJoy);

	drive.fwd = (drive.left + drive.right) /2;
	drive.turn = (drive.left - drive.right)/2;

	signOfTurn=0;
	if (drive.turn<0) signOfTurn=1;

	drive.turn*=drive.turn;
	drive.turn >>= 7;	//divide by 128; drive.turn is positive definite
	
	if (signOfTurn) drive.turn = -drive.turn;
	
	if (gTurnRateLimited) drive.turn=0;

	drive.left =  drive.fwd + drive.turn;
	drive.right = drive.fwd - drive.turn;

	gMotorSpeed.cimL = Limit127(drive.left);
	gMotorSpeed.cimR = Limit127(drive.right);	//actual PWM's handled in DriveMotors()
}
//***********************************************************************************************
void doDriveJoysticks3(void)
{
	struct {
		int left;
		int right;
		int turn;
		int fwd;
	} drive = {0,0,0,0};

	drive.fwd = addDeadband(mJoyForward);
	drive.turn = -addDeadband(mJoyTurn);
	
	if (gTurnRateLimited) drive.turn=0;

	drive.left =  drive.fwd + drive.turn;
	drive.right = drive.fwd - drive.turn;

	gMotorSpeed.cimL = Limit127(drive.left);
	gMotorSpeed.cimR = Limit127(drive.right);	//actual PWM's handled in DriveMotors()
}
//***********************************************************************************************
void doDriveJoysticks4(void)
{
	struct {
		int left;
		int right;
		int turn;
		int fwd;
	} drive = {0,0,0,0};

	drive.fwd = addDeadband(mJoyForward);
	drive.turn = -addDeadband(mJoyTurn);

//scale turning (skip if maintaining)
	drive.turn /= 2;	//reduce turning input.
	
	if (gTurnRateLimited) drive.turn=0;

	drive.left = drive.fwd + drive.turn;
	drive.right = drive.fwd - drive.turn;

	gMotorSpeed.cimL = Limit127(drive.left);
	gMotorSpeed.cimR = Limit127(drive.right);	//actual PWM's handled in DriveMotors()
}
//********************************************************************************
void doDriveJoysticks5(void)
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
	
	if (gTurnRateLimited) drive.turn=0;

	drive.left = drive.fwd + drive.turn;
	drive.right = drive.fwd - drive.turn;

	gMotorSpeed.cimL = Limit127(drive.left);
	gMotorSpeed.cimR = Limit127(drive.right);	//actual PWM's handled in DriveMotors()
}

//********************************************************************************
//doDriveJoysticks6() Adds mor turning at high speeds
void doDriveJoysticks6(void)
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

	if (gTurnRateLimited) drive.turn=0;

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

void doLifts(void)
{
//  // Conver belt movement and Diverter control
//	if(mFrontLiftSw)
//	{
//		//lift.front = mPanelSwitch3 ? mFrontLiftSpeed : -mFrontLiftSpeed;
//		lift.front = !mPanelSwitch1 ? mFrontLiftSpeed : -(mFrontLiftSpeed>>1);
//		mDiverterFront = 1;
//		mDiverterRear = 0;
//	}
//	else
//	{
//		lift.rear = !mPanelSwitch1 ? mRearLiftSpeed : -(mRearLiftSpeed>>1);
//		mDiverterFront = 0;
//		mDiverterRear = 1;
//	}
//
////	printf("Front//Rear\t%d//%d", mFrontLift, mRearLift);
//
//	if (mLaunchSwitch)
//	{
//		lift.launch = mLaunchLiftSpeed;
//		if (mPanelSwitch2)	//reverse
//			lift.launch = -lift.launch;
//	}
////	if 	( (mSecondBallAvl) || (mLaunchSwitch) )
////		lift.launch = 0;
//
//	if (p2_sw_aux1)
//	{
//		lift.front=lift.rear=0;
//		lift.launch = mLaunchLiftSpeed;
//	}
	
//	updateSensors();
//	checkDiverter();
	
	gLift.front = gLift.rear = gLift.launch = 0;	//clear until set otherwise [dg]
	CheckOIforFrontRearLiftSelection();

	//	driveLifts();

	setDivertorRelayAndLifts();	//call before ballShootOrQueue()
	ballLaunchOrQueue();	//pulses front/rear lifts off to add space between balls
//	ballQueue();
//	ballShoot();	//advance center lift - ignore 2nd ball sensor
	
	unjamFront();
	unjamRear();
	checkOIForKillSwitches();

	mPWMFrontLift  = 127u + gLift.front;
	mPWMRearLift   = 127u + gLift.rear;
	mPWMLaunchLift = 127u - gLift.launch;	//reverse polarity for actual motor rotation

}



/*****************************************************************************
Notes:
					      ___
				   | a |/`   `\
			  ___  |   | b |   | 
			/`   `\|   |   |   |
		   |   |\__c_!_d__/|   |
		   Front            Rear
Assuming that's our robot:
	-Sensors are designated by letters
	-! is the diverter
	-Assume that if the sensor detects something, it'll be 1, else 0
	-"a" is mBallin2nd
	-"b" is mBallinRear2
	-"c" is mBallinFront1
	-"d" is mBallinRear1

-Whereever there's a variable called insertXXXXXHere, that means
	there probably should be a global setting for that to be determined
	later.
-Uses "gLift.direction", ( 1 for using the rear, 0 for using the front ).
	-"gLift.current" is the PWM value that goes into the side specified by
		"gLift.direction". It can be overriden by things that directly modify
		the "gLift.front" or "gLift.rear" variables.

-Order is:

updateSensors();
checkDiverter();
getDirection();
ballQueue();
ballShoot();
driveLifts();
unjamFront();
unjamRear();
checkOIForKillSwitches();


-checkDiverter() before getDirection()
-getDirection() before driveLifts()
-unjamFront() and unjamRear() after driveLifts()
-ballQueue() after getDirection()
-ballQueue() before ballShoot()
-Check Kill Switches last. **
******************************************************************************/
void updateSensors(void)
{
	static char oldBallinRear2 = 0;
	static char oldBallinRear1 = 0;


	if (!oldBallinRear2 && mBallinRear2)
	{
		gSensors.PassedRear2 = 1;
		gSensors.PassedRear1 = 0;
	}
	if (!oldBallinRear1 && mBallinRear1)
	{
		gSensors.PassedRear1 = 1;
		gSensors.PassedRear2 = 0;
	}

	oldBallinRear2 = mBallinRear2;
	oldBallinRear1 = mBallinRear1;
}
////********************************************************************************
//void ballQueue(void)
//{
//	static char state = 2;
//
//	if (gLoop.onSecond)
//		printf("the queueing switch: %d\r", mBallQueueSw);
//	if (mBallQueueSw && !mBallin2nd)
//	{
//		if (0 && gLift.direction)
//		{
//			//The current sequence keeps track of a state for the rear
//			//	-If a ball reaches the lower sensor, the rear reactivates
//			//		at a slower speed
//			//	-If a ball reaches the higher sensor, the rear shuts off
//			switch (state)
//			{
//				case 1:
//					gLift.current = 0;
//					if (gSensors.PassedRear1) 
//						state++;
//					break;
//				case 2:
//					gLift.current = kLiftSpeed>>1;
//					if (gSensors.PassedRear2)
//						state--;
//					break;
//				default:
//					break;
//			}
//		}
//		gLift.launch = kLiftSpeed;
//		gLift.current = 0;
//	}
//}
void ballLaunchOrQueue(void)
{	
	static unsigned char count=0;
	char operateLaunch=0;
	
	if (mFireSw)
	{
		if (!gLauncherBelowSpeed) operateLaunch=1;	//don't fire if below speed
		if  (mMetaButton) operateLaunch=1;		//override camera lockout
	}
	else if (mBallQueueSw && !mBallin2nd) operateLaunch=1;


	//periodically turn off the rear/front lifts if queuing or launching to give space between balls
	if (operateLaunch)
	{
		//Else we need to turn off the front/rear lifts periodically while running the launch lift
		gLift.launch = kLiftSpeedLaunch;
		count++;
		if ((count & 0x18)==0x18)	//apply pulsed power to front/rear lift to put space between balls
		{
			if (gLift.direction==kFrontLiftActive)
				gLift.front= -kLiftSpeedLaunch/2;	//reverse at half speed
			else
				gLift.rear = -kLiftSpeedLaunch/2;	//reverse at half speed
		}
	}
	else
		count=0;	//reset counter & quit, leaving front & rear lifts alone.
}


//********************************************************************************
//void checkDiverter(void)
//{
//  if(mFrontLiftSw && !mBallinRear1)
//  {
//	mDiverterFront = 1;
//	mDiverterRear = 0;
//  }
//  else if (mRearLiftSw && !mBallinFront1)
//  {
//	mDiverterFront = 0;
//	mDiverterRear = 1;
//  }
//}
//********************************************************************************
void setDivertorRelayAndLifts(void)
{
	if (gLift.direction==kRearLiftActive)
	{
		gLift.rear=kLiftSpeedRear;
		gLift.front=0;
		mDiverterFront = 0;
		mDiverterRear = 1;
	}
	else
	{
		gLift.front = kLiftSpeedFront;
		gLift.rear=0;
		mDiverterFront = 1;
		mDiverterRear = 0;
	}	
}
//********************************************************************************
void CheckOIforFrontRearLiftSelection(void)
{
	if (mFrontLiftSw)
		gLift.direction=kFrontLiftActive;
	if (mRearLiftSw)
		gLift.direction=kRearLiftActive;	
}
//********************************************************************************
//void getDirection (void)
//{
//	if (mDiverterFront)
//		gLift.direction = 0;
//	else
//		gLift.direction = 1;
//}
//********************************************************************************
//void driveLifts (void)
//{
//	gLift.current = kLiftSpeed; //replace this line
//
//	if (gLift.direction)
//	{
//		gLift.rear = gLift.current;
//		gLift.front = 0;
//	}
//	else
//	{
//		gLift.front = gLift.current;
//		gLift.rear = 0;
//	}
//}
//********************************************************************************
//void ballShoot(void)
//{
//
//	//consider taking this if statement out entirely and use only last
//	//	part of this section just in case our sensor stops functioning
//	if (mFireSw)
//	{
////		gLift.current = 0;
//		gLift.launch = kLiftSpeedLaunch;
//		gLift.rear = gLift.front = 0;	//dg - avoid jams
//	}
//	else
//		gLift.launch  = 0;	//don't see this as necessary [dg]
//}
//********************************************************************************
void unjamRear (void)
{

//	if (mControlSwitch1 && gLift.direction==kFrontLiftActive)
	if (mUnjamRearSw)
		gLift.rear = -kLiftSpeedRear;
}
//********************************************************************************
void unjamFront (void)
{


//	if (mControlSwitch1 && gLift.direction==kRearLiftActive)
	if (mUnjamFrontSw)
		gLift.front = -(kLiftSpeedFront >> 1);
}


//********************************************************************************
void checkOIForKillSwitches (void)
{
	if (gLoop.onSecondA)	//this looks OK to me [dg].  Check by printing. Need to verify switch operation
		printf("checkOIForKillSwitches(): mFrontKillSw=%d, mRearKillSw=%d\r", (int) mFrontKillSw,  (int) mRearKillSw);
	
	if (mFrontKillSw)
		gLift.front = 0;
	
	if (mRearKillSw)
		gLift.rear = 0;
}
//********************************************************************************
// threeWaySwitch() returns kDown=-1, kOff=0, or kUp=1
// [Down=165, off=0, up=79] - uses 33K ohm and 66K (actually 2-33K) on an analog input
char threeWaySwitch(void)
{
	overlay char result;
	if (mThreewayswitch < 40) 
		result=kOff;
	else if (mThreewayswitch < 120)
		result=kUp;
	else
		result=kDown;
	return result;
}

//********************************************************************************
void doBallLauncher(void)
{
	char LaunchServiceSwitch = threeWaySwitch();
#ifdef Robot2005
	LaunchServiceSwitch=kOff;
#endif
	switch (LaunchServiceSwitch)
	{
		//These cases are used in AutonomousSaveOIControls() and off position *MUST* correspond.
		case kDown:	//camera control
			gBallLauncherSpeed = GetBallLauncherSpeedFromCamera();
			break;
		case kOff:	//off
			gBallLauncherSpeed=0;		//already off (default)
			break;
		case kUp:	//manual control
			gBallLauncherSpeed = readBallLauncherSpeedFromOI();
			break;
	}
}
//********************************************************************************
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
//********************************************************************************
static void sum_response( pulse_response *p, char delta_in )
{
	overlay char i;
	overlay int sum=0;
	overlay char *x;
	++p->index;
	p->index &= sizeof(p->pulse) - 1;		//wrap index; sizeof p->pulse must be power of 2
	p->pulse[p->index] = delta_in;		//overwrite data from xx cycles ago.
	
	//sum up values.
	//  We could just subtract old value and add new value to running sum,
	//  but if somehow an error occured, the output to the motors would be set to non-zero
	//  with no joystick input.  So, I see this method this as safer albeit slower.

	x = p->pulse;
	for (i=sizeof(p->pulse)-1; i>=0; i--)
		sum += *(x++);
	p->sum = sum;
}
//********************************************************************************
static void sum_response2( pulse_response *p_in, char delta_in )
{
	overlay char i;
	overlay int sum=0;
	overlay char *x;
	overlay pulse_response *p = p_in;
	++p->index;
	p->index &= sizeof(p->pulse) - 1;		//wrap index; sizeof p->pulse must be power of 2
	p->pulse[p->index] = delta_in;		//overwrite data from xx cycles ago.
	
	//sum up values.
	//  We could just subtract old value and add new value to running sum,
	//  but if somehow an error occured, the output to the motors would be set to non-zero
	//  with no joystick input.  So, I see this method this as safer albeit slower.

	x = p->pulse;
	for (i=sizeof(p->pulse)-1; i>=0; i--)
		sum += *(x++);
	p->sum = sum;
}
//********************************************************************************
static char JoystickTrimLED(unsigned char in)
{
#define kDeadBand 4
	if (in> (int) 127+kDeadBand)
		return (gLoop.count>>1)&1;
	if (in< (int) 127-kDeadBand)
//		return 10 == (10 & gLoop.count); 
		return 1;
	return 0;
}
//********************************************************************************
