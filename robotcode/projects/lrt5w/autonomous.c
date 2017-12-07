#include "LRTUtilities.h"


//void AutonomousAbort(void){}
//void AutonomousInitialize(void){}
//void AutonomousReset(void){}
//char AutonomousRun(void){}
//char AutonomousStatus(void){}
//
char gDriveLimitedFlag;


#include "lrtUtilities.h"
#include "lrtMotorDrive.h"

#include "interrupts.h"
#include "lrtConnections.h"
#include "printf_lib.h"
#include "OIFeedback.h"
#include "lrtResultCodes.h"
//#include "lrtStallDetect.h"
#include "arm.h"
#include "TetraWhacker.h"
#include "lrtRobotMove.h"
#include "motorSIM.h"
#include "hook.h"

//void LRTautonomous(void);
char MoveRobot(void);
static void GetAutonomousFieldPosition(void);

//typedef struct { long left, right; } Point;
	
/***********************************************************************************/
struct autonomous {
	char returnValue;
	char taskPhase;
	char stallCount;	//number of times robot has stalled in routine

	char fieldStartPosition;
	unsigned alreadyRan:1;	//prevent running more than once
	unsigned tetraWhackSide:1;	//kLeft or kRight
	unsigned centerPosition:1;
	unsigned cornerPosition:1;
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
//set outputs to neutral-- bug in Master controller causes stale data
//on pwm's when switched to autonomous.
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

	GetAutonomousFieldPosition();	//may prevent Autonmous running if pos 0 or 7;

//	EnableStallDetect();

//gMotorDriveIsStalled=1;		//DEBUG

//	AllStop();
//	TeeBallReset();
//	mWinchRelayFwd=mWinchRelayRev=0;
	
//	gLegs.front.cmd = gLegs.rear.cmd = 255;	//don't do anything

}

/*********************************************************/
static void GetAutonomousFieldPosition(void)
{
	//build up the value 0-7 from each of  3  switches
	pAutonomous.fieldStartPosition = 0;
	if (mAutonomousSw0) pAutonomous.fieldStartPosition|=0x4;
	if (mAutonomousSw1) pAutonomous.fieldStartPosition|=0x2;
	if (mAutonomousSw2) pAutonomous.fieldStartPosition|=0x1;

#ifdef _SIMULATOR
	pAutonomous.fieldStartPosition = 3;	//for  testing only
#endif //_SIMULATOR

#ifdef SIM
	pAutonomous.fieldStartPosition = 3;	//for  testing only
#endif //SIM

/*
* field positions
*   Judges
  Auto loaders
* >        <
* 1        6
* >2      5<
* 3        4
* >        <
  Human loaders
*/
	pAutonomous.tetraWhackSide=kRight;	// correct for pos 3 and 6
	pAutonomous.cornerPosition=0;
	pAutonomous.centerPosition=0;
printf("start pos #%d",(int) pAutonomous.fieldStartPosition);
	switch (pAutonomous.fieldStartPosition)
	{
		case 1:
		case 4:
			pAutonomous.tetraWhackSide=kLeft;
		case 3:
		case 6:
printf("Corner Pos; Whack ");
if (pAutonomous.tetraWhackSide==kLeft) printf("left\n");
else printf("right\n");
			pAutonomous.cornerPosition=1;
			break;
		case 2:
		case 5:
			pAutonomous.centerPosition=1;
printf("CenterPos\n");
			break;
		case 0:
		case 7:
			printf("Autonomous Disabled\n");
			pAutonomous.returnValue = kResultNotRunning;
			break;	//should we not run autonomous w/ these settings?
	}
}

/***********************************************************************************/
char AutonomousRunCornerPos(void)
{
//	enum { kHaltBeforePhase = 9 };	//for debugging; stop at phase #
	enum { kHaltBeforePhase = 100 };	//for debugging; stop at phase #
	overlay char advancePhase;
	overlay char result;
	overlay int turnETicks;	//turn measured in encoder ticks (R-L) so that positive = CCW (left turn)
	overlay int distance;
	overlay int forearmPos, shoulderPos;
	overlay unsigned char timer;
	static char stallReturnPhase = -1;
	static unsigned char wait = 0;	//wait this number of cycles; max 255, or about 6 sec
	
if (gLoop.onSecond)
{
		printf("Autonomous phase: %d\n", (int)pAutonomous.taskPhase);
}
	if (!autonomous_mode)
	{
		AutonomousAbort();	//added to avoid loops in autonomous
		printf("left autonomous mode early\n");
	}

	if (pAutonomous.returnValue != kResultRunning)
	{
		printf("Auto Not Running\n");
		return pAutonomous.returnValue;
	}

////	User_Byte5=EncoderLeft.velocity;
////	User_Byte6=EncoderRight.velocity;
	
	//MoveForearmTimedRun();	//run any pending arm tasks.
//	result = ArmMoveRun();

	do {
		result = advancePhase=0;	//clear advancePhase command and result
//		OIShowPhaseWithLEDs(pAutonomous.taskPhase);

		if (wait)
		{
			wait--;
			printf("waiting...\n");
			continue;	//skip loop	(advancePhase is 0, so will come back next cycle)
		}
		switch (pAutonomous.taskPhase) {
			case 0:		//initialization; set first destination
printf("\Starting autonomous mode. ");
PrintTime();			
				advancePhase=1;
				break;
			case 1:
				ShiftLowGear();
				printf("Shift & Move arm in auto\n");
				//MoveForearmTimedStart( 3000/26.2, 127 );
				forearmPos = kForeArmLevel + mDegrees2BitsRobot(45);
			//	shoulderPos = kShoulderVertical + mDegrees2BitsRobot(-5);
				shoulderPos = kShoulderVertical + mDegrees2BitsRobot(-15);
				
				printf("Arm move to %d / %d (forearm/shoulder)\n", (int) forearmPos, (int)shoulderPos);
				ArmMoveInitialize(forearmPos, shoulderPos);

				advancePhase=1;
				wait=38;
				break;
			case 2:
				if (gGearBox.shifting)
					DoShift();
				advancePhase=1;
				break;
			case 3:
				MoveRobotInitialize(kEncoderTicksPerFt*0.5, 2000/26.2, 50);
				advancePhase=1;
				break;
			case 4:
				result = MoveRobotForward();	//do the move in the last case-
				if (result)
					advancePhase=1;
				break;
			case 5:
				//printf("whacktetrastart\n");
				timer = 2000/26.2;
				advancePhase=1;
				break;
			case 6:
				if (0!=timer)
				{
					timer--;
					//activate the relay on the pneumatic rotary actuator
					mTetraWhackCW=(pAutonomous.tetraWhackSide==kRight);
					mTetraWhackCCW = !mTetraWhackCW;
				}
				else
					advancePhase=1;
				break;
			case 7:
				MoveRobotInitialize(kEncoderTicksPerFt*4.5, 3000/26.2, 50);
				advancePhase=1;
				break;
			case 8:
				if (MoveRobotForward() != kResultRunning)	//done, whether or not successful
					advancePhase=1;
				break;
			case 9:		//restore the tetrawhacker to its upright position
				timer = 250/26.2;
				advancePhase=1;
				break;
			case 10:
				if (0!=timer)
				{
					timer--;
					//activate opposite relays on the pneumatic rotary actuator
					mTetraWhackCW= !(pAutonomous.tetraWhackSide==kRight);
					mTetraWhackCCW = !mTetraWhackCW;
				}
				else
				{
					//release valves on pneumatic rotary actuator
					mTetraWhackCW = mTetraWhackCCW = 0;
					advancePhase=1;
				}
				break;

			case 11:
				printf("Start turn\n");
				//neg is CW; pos is CCW
				if (kLeft==pAutonomous.tetraWhackSide)
				{
					turnETicks = -(30L+90+30-3) * kTurnTicksPerDegree;	//turn CW
					printf("turn clockwise\n");
				}
				else
				{
					turnETicks = (30L+90+30-3) * kTurnTicksPerDegree;
					printf("turn counter clockwise\n");
				}

				TurnRobotInitialize(turnETicks, (3000/26.2), 75, 75);
				advancePhase=1;
				break;
			case 12:
				result = TurnRobot();
				if (result!=kResultRunning)
					advancePhase=1;
				break;
			case 13:
				MoveRobotInitialize(kEncoderTicksPerFt*7, 4000/26.2, 80);
				advancePhase=1;
				break;
			case 14:
				result = MoveRobotForward();
				if (result!=kResultRunning)
					advancePhase=1;
				break;
			case 15:
				wait=38;
				advancePhase=1;
				break;
			case 16:
			//	gHookPosition = kHookDown;
				pAutonomous.returnValue = kResultSuccess;
				printf("Autonomous mode completed. ");
				PrintTime();
				//don't advancephase
				break;


			default:
				if (pAutonomous.returnValue == kResultRunning)
					pAutonomous.returnValue = kResultNotRunning;
				break;	//quit;
		}	//end of switch


		if (result==kResultTimeOut)
		{	
			pAutonomous.returnValue = kResultTimeOut;
			printf("Timeout\n");
		}
		if (advancePhase)	//diagnostics
		{
			printf("\nEnd of AutoPhase=%d\n",(int)pAutonomous.taskPhase);
			PrintTime();
			PrintDistanceFeet();
			PrintVelocity();
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

	if (kResultRunning != pAutonomous.returnValue)
		pAutonomous.alreadyRan = 1;

	return pAutonomous.returnValue;
}
/*****************************************************************************************/



char AutonomousRunCenterPos(void)
{
	return pAutonomous.returnValue = kResultNotRunning;
}
/*****************************************************************************************/


char AutonomousRun(void)
{
	if (pAutonomous.centerPosition)
		AutonomousRunCenterPos();
	else if (pAutonomous.cornerPosition)
		AutonomousRunCornerPos();
	else 
		pAutonomous.returnValue = kResultNotRunning;
	return pAutonomous.returnValue;
}


/*
 * Reset the tetra whacker when processor reset and autonomous
 * was not active (this situation should not happen in field play)
 * This is to ensure the whacker is easily rotated without having to
 * cycle through the autonomous mode.
 * 
 * power robot, or press reset, and keep robot 'undisabled' and out of 
 * autonomous.
 */


void WhackTetraReset(void)
{
	static struct {
		unsigned alreadyRan:1;
		char timer;
	}wtr;	//whackTetraReset
	
	if (wtr.alreadyRan) return;
	if (disabled_mode) return;	//don't run until enabled
	if (autonomous_mode)	//don't run if autonomous ever was active
	{
		wtr.alreadyRan=1;
		return;
	}

	//set both relays to off
	mTetraWhackCCW = mTetraWhackCCW = 0;
	if (wtr.timer >= 38/4)
	{
		wtr.alreadyRan=1;
		return;
	}

	if (0==wtr.timer)	//need to initalize
	{
		GetAutonomousFieldPosition();
		if (!pAutonomous.cornerPosition)
		{
			wtr.alreadyRan=1;
			return;
		}
	}
	
	wtr.timer++;
	mTetraWhackCCW=(pAutonomous.tetraWhackSide==kRight);
	mTetraWhackCW = !mTetraWhackCCW;
}
