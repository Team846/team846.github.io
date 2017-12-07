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

#define kLiftSpeedRear 50
#define kLiftSpeedFront 50
#define kLiftSpeedLaunch 80


char gDriveGear=kHighGear;
char gBallLauncherSpeed=0;


lift gLift = {0,0,0,0,kFrontLiftActive};	//seems reversed
drive gDrive = {0,0,0,0};
sensors gSensors = {0,0,0,0};


//***************** Local Prototypes *********************************
static void OperatorShiftGear(void);
static void doDriveJoysticks(void);
static  void CheckOIforFrontRearLiftSelection(void);


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

//***********************************************************************
void controls(void)
{
	OperatorShiftGear();
	doDriveJoysticks();
	doLifts();

	// Set Ball Shooter Speed Control controlled in userRoutines

//	gBallLauncherSpeed= (254 - mBallLauncherspeed)>>1;





	// Turret Motor Movement, when camera code need to plug the pan adustment
//	if (0) {
//		//Move the turret as needed
//		//turret error is on {-127,127}
//		//TurretError = ((int) Get_Analog_Value(mCameraSimPot)>>2) -127;
//		TurretError = get_pan_error();	
//		MoveTurretAuto(TurretError);
//	} else
//		
	MoveTurretManual();



if (gLoop.onSecond)
	printf("ball in sec. pos = %d\r", (int) mBallin2nd);

		


if (gLoop.onSecond)
	printf("mPWMTurretMotor=%d\r\r",(int)mPWMTurretMotor);


}
//********************************************************************************
void OperatorShiftGear(void)
{
	if (mSwHiGear) {
		gDriveGear=kHighGear;
		printf("mSwHiGear\r");
	}
	if (mSwLowGear) {
		gDriveGear=kLowGear;
		printf("mSwLowGear\r");
	}
}
//********************************************************************************

void SetGearshiftRelays(void)
{
	mRelayHiGear=mRelayLowGear=0;
	if (gDriveGear==kLowGear)
		mRelayLowGear=1;
	if (gDriveGear==kHighGear)
		mRelayHiGear=1;
}
//********************************************************************************
void doDriveJoysticks(void)
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
	
	drive.left = drive.fwd + drive.turn;
	drive.right = drive.fwd - drive.turn;

	mPWMLeftCIM = removeESCDeadband(-drive.left);	//handles limiting too
	mPWMRightCIM = removeESCDeadband(drive.right);
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
//********************************************************************************
//This needs to be replaced with a closed loop control sys.
void ControlBallLauncherSpeed(void)
{
	char speed=gBallLauncherSpeed;
	if (gBallLauncherSpeed == kBallLauncherSlow)
		speed = 30;	//set this to the minumum ball speed on range {0,127}

	mPWMRightShooter = mPWMLeftShooter = 127u + speed;
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

	//periodically turn off the rear/front lifts if queuing or launching to give space between balls
	if (!mFireSw && !(mBallQueueSw && !mBallin2nd))
	{
		count=0;	//reset counter & quit, leaving front rear lifts alone.
		return;
	}	
	
	//if ()
	//{
	//	count=0;
	//	return;
	//}
	
	if (gLoop.onSecond) {printf ("Firing button: %d%t Queuing button: %d%t\r", (int)mFireSw, (int)mBallQueueSw);}
	//Else we need to turn off the front/rear lifts periodically while running the launch lift
	gLift.launch = kLiftSpeedLaunch;
	count++;
	if (count & 0x20)	//apply pulsed power to front/rear lift to put space between balls
		gLift.front=gLift.rear = 0;	//turn lifts off
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
	if (gLoop.onSecond)	//this looks OK to me [dg].  Check by printing. Need to verify switch operation
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
void readBallLauncherSpeed(void)
{
	unsigned char launcherSpeed = mBallLauncherspeed;
	if (gLoop.onSecond)	
		printf("mBallLauncherspeed=%d\r", (int)launcherSpeed);
	calibrateInput( &launcherSpeed, 18);	//set second arg should be used to calibrate low value pot.

	gBallLauncherSpeed= (254 - launcherSpeed)>>1;
	gBallLauncherSpeed = LimitRange(gBallLauncherSpeed,20,127);

	if (gLoop.onSecond)	
		printf("gBallLauncherSpeed=%d\r", (int)gBallLauncherSpeed);

}
