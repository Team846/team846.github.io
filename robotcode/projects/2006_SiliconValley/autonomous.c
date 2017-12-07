#include "ifi_default.h"
#include "ifi_aliases.h"

#include "utilities.h"
#include "connections.h"
#include "resultCodes.h"
#include "tracking.h"
#include "controls.h"
#include "turret.h"

//#include "lrtRobotMove.h"
//#include "motorSIM.h"
#include <stdio.h>

char MoveRobot(void);
static void FireOnTargetLock(void);
//typedef struct { long left, right; } Point;
	
/***********************************************************************************/
struct autonomous {
	char returnValue;
	char taskPhase;
	char stallCount;	//number of times robot has stalled in routine

	unsigned alreadyRan:1;
	char fieldStartPosition;
}	pAutonomous={kResultNotRunning,0,0,0};


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
	
	char CameraTrackingState = Get_Tracking_State();

	//HARDWIRE shooting speed
	gBallLauncherSpeed = 75;	// on range {0,127}	

	if (gLoop.onSecond)
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

	CameraTrackingState = Get_Tracking_State();
	TurretError = get_pan_error();	
	MoveTurretAuto(TurretError);
	//MoveTurretManual();	//should go outside Autonomous

	

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
				gDriveGear = kHighGear;
			gLift.direction=kRearLiftActive;
	//			gLift.direction=kFrontLiftActive;	//this seems backwards
				printf("Shift into High Gear, turning on rear lift\r");

				advancePhase=1;
				wait=(500/26.2);
				break;
			case 2:
//				timer = (1500)/26.2;	//set timer for drive motors
//				timer = (1000)/26.2;	//set timer for drive motors
//				timer = (2300)/26.2;	//set timer for drive motors
				timer = (3600)/26.2;	//set timer for drive motors
//				timer = 1800;	//set timer for drive motors
				printf("Starting move for %d ticks\r", (int)timer);
				advancePhase=1;
				break;
			case 3:
				if (timer>0)
				{
					char speed=50;
					--timer;
					mPWMLeftCIM = removeESCDeadband(-speed);	//handles limiting too
					mPWMRightCIM = removeESCDeadband(speed);
				}
				else
					advancePhase=1;
				break;
			case 4:
				//FireOnTargetLock();
				//printf("CameraTrackingState=%d\r", (int) CameraTrackingState);
				if (CAMERA_ON_TARGET == CameraTrackingState)
				{
					fireTimer=(1000/26.2);    //decremented outside of loop and sets mFireSw=1 [sg]
				}
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
		mFireSw=1;
		printf("AutoFire!!!!\r");
	}

	if (kResultRunning != pAutonomous.returnValue)
		pAutonomous.alreadyRan = 1;

	doLifts();	//this should be called outside autonomous

	return pAutonomous.returnValue;
}
/*****************************************************************************************/

/*****************************************************************************************/
//FireOnTargetLock() fires on locked target
// If target is locked for at least kMinTargetTime (=2) cycles then it will fire
// when firing, lift will be held on for 1 sec since last target locked and held.


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
		printf("AutoFire!!!!\r");
	}
}
