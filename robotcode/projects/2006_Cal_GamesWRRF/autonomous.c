#include "ifi_default.h"
#include "ifi_aliases.h"
#include "ifi_utilities.h"

#include "utilities.h"
#include "connections.h"
#include "resultCodes.h"
#include "tracking.h"
#include "controls.h"
#include "turret.h"
#include "MotorDrive.h"
#include "eeprom.h"
#include "LED.h"
//#include "lrtRobotMove.h"
//#include "motorSIM.h"
#include <stdio.h>

char MoveRobot(void);
static void FireOnTargetLock(void);
//static void AutonomousTurretSweep(void);

//typedef struct { long left, right; } Point;
	
/***********************************************************************************/
struct autonomous {
	char returnValue;
	char taskPhase;
	char stallCount;	//number of times robot has stalled in routine

	unsigned alreadyRan:1;
	unsigned dontLaunchBalls:1;	//saved from OI when disabled
	
	unsigned char distance;		//saved to eeprom
	
		
}	pAutonomous={kResultNotRunning,0,0,0,0};


/*******************************************************************/
//called to allow multiple autonomous runs
void AutonomousReset(void)
{
	pAutonomous.alreadyRan = 0;
	pAutonomous.returnValue=kResultNotRunning;
}
/*******************************************************************/
char AutonomousStatus(void)
{
	return pAutonomous.returnValue;	
}
/*******************************************************************/
void AutonomousAbort(void)
{
	pAutonomous.returnValue=kResultNotRunning;	
	pAutonomous.alreadyRan = 1;	
}

/*******************************************************************/
void AutonomousInitialize(void)
{
	if (pAutonomous.returnValue==kResultRunning)
		return;	//do nothing
	if (pAutonomous.alreadyRan)	//must be cleared before running again.
	{
		pAutonomous.returnValue = kResultError;
		return;
	}
	pAutonomous.returnValue=kResultRunning;	//prevent running twice
	pAutonomous.stallCount=0;	//prevent running twice
	pAutonomous.taskPhase=0;
}


/***********************************************************************************/
char AutonomousRun(void)
{
	enum { kHaltBeforePhase = 100 };	//for debugging; stop at phase #
	overlay char advancePhase;	//when set true, state machine will advance to next state
	overlay char result;
	overlay int turnETicks;	//turn measured in encoder ticks (R-L) so that positive = CCW (left turn)
	overlay int distance;
	overlay int timer;
	char TurretError;
	static unsigned char fireTimer = 0;
	static char stallReturnPhase = -1;
	static unsigned char wait = 0;	//wait this number of cycles; max 255, or about 6 sec
	
	char CameraTrackingState;

	//HARDWIRE shooting speed
	//gBallLauncherSpeed = 75;	// on range {0,127}	

	if (gLoop.onSecondA)
		printf("Autonomous phase: %d\r", (int)pAutonomous.taskPhase);
	
//	if (gLoop.onSecond)
//	{
//		printf("PanLeft: %d; PanOffset: %d; ShootingSpeed: %d\r", (int)mPanLeftSw, (int)mPanOffset, (int)mBallLauncherspeed);
//	}
	
	if (!autonomous_mode)
	{
		AutonomousAbort();	//added to avoid loops in autonomous
		printf("left autonomous mode early\r");
	}

//	if (pAutonomous.returnValue != kResultRunning)
//	{
//		printf("Auto Not Running\r");
//		return pAutonomous.returnValue;
//	}

	CameraTrackingState = GetRawTrackingState();
	TurretError = get_pan_error();
	
	if (pAutonomous.dontLaunchBalls)	//in case we are going toward own goal
		; //do nothing
	else
	{
		//if (TARGET_IN_VIEW & CameraTrackingState)
			MoveTurretAuto(TurretError);
		//else
		//	TurretSweep();
	}
			

	do {
		result = advancePhase=0;	//clear advancePhase command and result



		if (wait)
		{
			wait--;
			printf("waiting...\r");
			continue;	//skip loop	(advancePhase is 0, so will come back next cycle)
		}
		switch (pAutonomous.taskPhase) {
			case 0:		//initialization; set first destination
				printf("\Starting autonomous mode. ");
				PrintTime();			
				advancePhase=1;
				break;
			case 1:
				gGearBox.cmdGearObjective = kHighGear;
	//			gLift.direction=kRearLiftActive;
				gLift.direction=kFrontLiftActive;	//this seems backwards
				printf("Shift into High Gear, turning on rear lift\r");

				advancePhase=1;
//				wait=(400/26.2);
				wait=0;
				break;
			case 2:
//				timer = (3600)/26.2;	//set timer for drive motors (too far -- up ramp)
//				timer = (2000)/26.2;	//set timer for drive motors (too far -- up ramp)
//				timer = (1600)/26.2;	//set timer for drive motors (too far -- up ramp)
				timer = (1200)/26.2;	//set timer for drive motors (too far -- up ramp)
//				timer = (1400)/26.2;	//set timer for drive motors (too far -- up ramp)
//				timer = (1500)/26.2;	//set timer for drive motors (too far -- up ramp)

				//distance is in tenths of second. E.g. 1200ms->120 on display
				timer = (int) pAutonomous.distance*100/262;
				
				printf("Starting move for %d ticks\r", (int)timer);
				advancePhase=1;
				break;
			case 3:
				if (timer>0)
				{
					char speed=(0.90*127);
					--timer;
					gMotorSpeed.cimL = gMotorSpeed.cimR = speed;
//					mPWMLeftCIM = removeESCDeadband(-speed);	//handles limiting too
//					mPWMRightCIM = removeESCDeadband(speed);
				}
				else
					advancePhase=1;
				break;
			case 4:
				//FireOnTargetLock();
				//printf("CameraTrackingState=%d\r", (int) CameraTrackingState);
				//if ((CAMERA_ON_TARGET == CameraTrackingState)||(mAbsolute(TurretError)<2))
//				if (CAMERA_ON_TARGET == CameraTrackingState)
				if ((TARGET_IN_VIEW & CameraTrackingState) && (mAbsolute(TurretError)<5))
				{
					fireTimer=(1000/26.2);    //decremented outside of loop and sets mFireSw=1 [sg]
				}
				if (pAutonomous.dontLaunchBalls)	//in case we are going toward own goal
					; // do nothing
				else if (!(TARGET_IN_VIEW & CameraTrackingState))
					TurretSweep();
				
				//idle here
				
				break;

			default:
				if (pAutonomous.returnValue == kResultRunning)
					pAutonomous.returnValue = kResultNotRunning;
				break;	//quit;
		}	//end of switch


		if (result==kResultTimeOut)
		{	
			pAutonomous.returnValue = kResultTimeOut;
			printf("Timeout\r");
		}
		if (advancePhase)	//diagnostics
		{
			printf("\rEnd of AutoPhase=%d\r",(int)pAutonomous.taskPhase);
			PrintTime();
//			PrintDistanceFeet();
//			PrintVelocity();
		}
		if (advancePhase)
		{
			if (pAutonomous.taskPhase >= kHaltBeforePhase)	//For testing
			{
				pAutonomous.returnValue = kResultNotRunning;
				break;	//exit the main loop
			}

			pAutonomous.taskPhase++;
		}
	} while (advancePhase);
	
	if(fireTimer>0)
	{
		fireTimer--;
		mFireSw=1;		//doLifts will do the actual firing
		
		if (gLoop.onSecondB)
			printf("AutoFire!!!!\rAutoFire!!!!\r");
	}

	if (kResultRunning != pAutonomous.returnValue)
		pAutonomous.alreadyRan = 1;


	return pAutonomous.returnValue;
}
/*****************************************************************************************/

/*****************************************************************************************/
//FireOnTargetLock() fires on locked target
// If target is locked for at least kMinTargetTime (=2) cycles then it will fire
// when firing, lift will be held on for 1 sec since last target locked and held.

//#define _CUT
#ifdef _CUT

#define kMinTargetTime	2 //cycles

void FireOnTargetLock(void)
{
	static struct {
		char targeted;	//cycles camera has held target
		char fire;		//cycles lift is held to  fire; will fire if >0
	} timer ={0,0};


	if (Get_Tracking_State() == CAMERA_ON_TARGET)
	{
		if (++timer.targeted >= kMinTargetTime)
		{
			timer.targeted = kMinTargetTime;	//cap max value of 'targeted'timer.fire = (1000)/26.2;
			timer.fire = (1000/26.2);	//reset fire timer
		}
	}
	else
		timer.targeted=0;	//reset

	if (timer.fire>0)
	{
		timer.fire--;
		mFireSw=1;			//hold down the 'fire' btn on the OI
		
		if (gLoop.onSecondB)
			printf("AutoFire!!!!\rAutoFire!!!!\r");
	}
}
#endif //_CUT
//********************************************************************

void ReadAutonomousDistanceEPROM(void)
{
	pAutonomous.distance = EEPROM_read( kEPROMAdr_Autonomous );
}
//********************************************************************************

#if 1	//select method 1 or 2
void SetAutonomousDistance(void)
{
	static char dirty=0;

	//must be in user mode, disabled
	if (!user_display_mode || !disabled_mode) return;	
	
	if (mAutoWrite1Btn && mAutoWrite2Btn)
	{
		pAutonomous.distance = ~mBallLauncherspeed;
		dirty=1;
	}
	if (mAutoWrite1Btn || mAutoWrite2Btn)
		gUserByteOI = pAutonomous.distance;
	else if (dirty)
	{
		EEPROM_write(kEPROMAdr_Autonomous, pAutonomous.distance);	//save across resets
		dirty=0;	
	}
}
//********************************************************************************
#else	//second method 
void SetAutonomousDistance(void)
{
	static unsigned char keyDownTime=0;
	static char dirty=0;
#define kAutoRepeatTime ((1000/26.2))

	if (dirty &&  ((gLoop.secondz>>1) & 0x1))	//write every other second
	{
		EEPROM_write(kEPROMAdr_Autonomous, pAutonomous.distance);	//save across resets
		dirty=0;
	}
if (gLoop.onSecond)
	printf("U%d D%d Q%d Up%d Dwn%d  %d\r", (int) user_display_mode,(int) disabled_mode,
		(int)mBallQueueSw,(int)mAutoTimeUpBtn, (int)mAutoTimeDownBtn, (int)keyDownTime);
		
		
	//must be in user mode, disabled with Meta button pressed to set options
	if (!user_display_mode || !disabled_mode) return;
	if (!mBallQueueSw) return;

	
	if (mAutoTimeUpBtn)
	{
		if (++keyDownTime==1 || keyDownTime >= kAutoRepeatTime)
			pAutonomous.distance++;
			dirty=1;
	}
	else if (mAutoTimeDownBtn)
	{
		if (++keyDownTime==1 || keyDownTime >= kAutoRepeatTime)
			pAutonomous.distance--;
			dirty=1;
	}
	else
		keyDownTime=0;
	if (keyDownTime==255) keyDownTime=254;	//so's we don't wrap
	
	gUserByteOI = pAutonomous.distance;
}
#endif //end of second method



//******************************************************************************************
void AutonomousSaveOIControls(void)
{
	pAutonomous.dontLaunchBalls = (kOff==threeWaySwitch());
}
