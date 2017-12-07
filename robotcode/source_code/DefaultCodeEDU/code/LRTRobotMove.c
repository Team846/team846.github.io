#include "lrtUtilities.h"
#include "lrtRobotMove.h"
#include "interrupts.h"
#include "lrtConnections.h"
#include "printf_lib.h"
#include "lrtMotorDrive.h"

#define TORQUE_DRIVE 1		//for auto control only; see lrtControls.c too

char MoveRobot(void);

//robotTask gRobotTask={0};
robotTask gTask={0};


enum direction { forward,reverse };
enum turnDirection { CW,CCW };

/*****************************************************************************************/
#ifdef CUT
char MoveRobot(void)
{
	MoveDriveMotor(&gTaskLW);
	MoveDriveMotor(&gTaskRW);
//	return ((gTaskRW.returnValue!=0));
	//abort task of either times out
	if (kResultTimeOut==gTaskLW.returnValue || kResultTimeOut==gTaskRW.returnValue)
	{
		printf("timeout\n");
		*gTaskLW.pwm=*gTaskRW.pwm=127;	//stop motors
		return kResultTimeOut;
	}
	else	//both motors must finish task
		return ((gTaskLW.returnValue!=0) && (gTaskRW.returnValue!=0));
}


/***********************************************************************************/
void MoveDriveMotor(task1Wheel *t)
{
	char EMF;	//motor EMF
	char advancePhase;
	do {
		advancePhase=0;
		t->cyclesRemaining--;
		switch (t->phase)
		{
			case 0:	//initialization
				advancePhase=1;
				if (t->maxPWM < 0) t->maxPWM=-t->maxPWM; //hmm...really an error
				t->initialDirection = (t->destination>=0 ? forward:reverse);
				t->destination += t->e->position;	//add on the encoder's current postion to
					//get an absolute value for the destination position
				t->cyclesRemaining = t->maxCycles; //ticks to timeout
				t->returnValue=kResultRunning;
				break;

			case 1:
				// does distance remaining have same sign as the initial direction ?
				if ((t->e->position < t->destination) == (forward==t->initialDirection))
				{
					//should control velocity here. Oh well
#if TORQUE_DRIVE
					*t->pwm = TorqueDrive(
						(forward == t->initialDirection ? t->maxPWM : -t->maxPWM),t->e->velocity);	
#else
					*t->pwm = limitDriveMotorCurrent((forward == t->initialDirection ? t->maxPWM : -t->maxPWM),t->e->velocity);	
#endif				
				}
				else
				{	//passed destination; quit & coast
					advancePhase=1;
					*t->pwm = 127;	//coast
				}
					break;
			case 2:
				t->returnValue = kResultSuccess;
				break;
			default:
				t->returnValue=kResultError;
		}	//end switch

		if (advancePhase) t->phase++;

		if (0==t->cyclesRemaining && 0==t->returnValue)
		{
			*t->pwm=127;
			 t->returnValue=kResultTimeOut;	//timeout
		}
	} while (advancePhase);
}

#endif //CUT

/***********************************************************************************/
//set destination to angle in encoder ticks.
//e.g. turnETicks = degrees * kTurnTicksPerDegree
//apply opposite max power to left/right until degrees acheived.
//positive angle turns left (CCW)
//negative angle turns right (CW)
/***********************************************************************************/

void TurnRobotInitialize(int turnETicks, int maxCyclesTimeout, char maxPWMLeft, char maxPWMRight)
{
	extern robotTask gTask;	
//	gTask.phase = 0;
	//gTask.maxPWM = maxPWMswing;	//reduce turn power
	gTask.maxPWMLeft = maxPWMLeft;
	gTask.maxPWMRight = maxPWMRight;
	gTask.maxPWM = 0;	//not used

	gTask.cyclesRemaining = maxCyclesTimeout; //# of 26.2ms cycles to timeout

	//positive 'turnETicks' yields left (CCW) turn
	gTask.destination = turnETicks;  //tweak here as needed (e.g. 250/256)
	gTask.destination += EncoderRight.position-EncoderLeft.position;

	gTask.initialDirection = (turnETicks>=0 ? CCW:CW);

	//look for invalid parameters	
	gTask.returnValue=kResultRunning;
	if (gTask.maxPWMLeft > 0) {}
	else if (gTask.maxPWMRight > 0) {}
	else if (gTask.cyclesRemaining > 0) {}
	else gTask.returnValue = kResultError; //invalid parameter

	if (gTask.initialDirection == CW)
		gTask.maxPWMRight = -gTask.maxPWMRight;
	else
		gTask.maxPWMLeft = -gTask.maxPWMLeft;

}
/***********************************************************************************/
char TurnRobot(void)
{	
	extern robotTask gTask;	
	long positionAngle;
//to turn 'theta' degrees, the outside wheel must turn more than inside wheel:
//That is, Xout-Xin = d*theta*2Pi/360 where d is the axle length


	//Check if task already completed or improperly initialized
	if (gTask.returnValue != kResultRunning)
		return gTask.returnValue;
	if (gTask.cyclesRemaining<=0)
		return gTask.returnValue = kResultTimeOut;
		
	gTask.cyclesRemaining--;
	
	//calculate projected angle at end of this cycle:
	positionAngle = EncoderRight.projectedPos-EncoderLeft.projectedPos;
//	positionAngle = EncoderRight.position-EncoderLeft.position;
	
	// does positionAngle remaining have same sign as the initial direction?
	if ((positionAngle < gTask.destination) == (CCW==gTask.initialDirection))
	{
		DriveLeftMotors(gTask.maxPWMLeft);
		DriveRightMotors(gTask.maxPWMRight);

		gTask.returnValue = kResultRunning;
	}
	else{ 	//passed destination
		gTask.returnValue = kResultSuccess;
	}
	return gTask.returnValue;
}
/***********************************************************************************/



///***********************************************************************************/
//void StopRobotInitialize(void)
//{
//	gTaskLW.phase = gTaskRW.phase = 0;
//	gTaskLW.maxPWM = gTaskRW.maxPWM=5;		//80,127; 9/29/2004
//	gTaskLW.maxCycles = gTaskRW.maxCycles=2000/26.2;
//	
//	gTaskLW.returnValue = gTaskRW.returnValue = kResultRunning;
//}
//
//
//char StopRobotExecute(void)
//{
//PrintVelocity();		
//	StopDriveMotor(&gTaskLW);
//	StopDriveMotor(&gTaskRW);
//	if (1==gTaskLW.returnValue && 1==gTaskRW.returnValue)
//		return 1;
//	else
//		return 0; //not stopped yet
//}
///***********************************************************************************/
//void StopDriveMotor(task1Wheel *t)
//{
//	char advancePhase;
//	if (t->returnValue != kResultRunning)
//		return;
//
//	do {
//		advancePhase=0;
//		t->cyclesRemaining--;
//		switch (t->phase)
//		{
//			case 0:	//initialization
//				advancePhase=1;
//				if (t->maxPWM < 0) t->maxPWM=-t->maxPWM;
//				t->initialDirection = (t->e->velocity>=0 ? forward:reverse);
//				t->cyclesRemaining = t->maxCycles; //ticks to timeout
//			//	t->returnValue = kResultRunning;
//				break;
//
//			case 1:
//				// does velocity have same sign as the initial direction ?
//				if (0==t->e->velocity)
//				{
//					*t->pwm=127;
//					advancePhase=1; //finished stopping
//				}
//				if ((t->e->velocity >0) == (forward==t->initialDirection))
//				{
//					int power=(forward == t->initialDirection ? -t->maxPWM : t->maxPWM);
//					//apply max possible reverse power.
//					//?try predicting the velocity?
//#if TORQUE_DRIVE
//					*t->pwm = TorqueDrive(power,t->e->velocity);		
//#else
//					*t->pwm = limitDriveMotorCurrent(power,t->e->velocity);		
//#endif
//		printf("V=%d, %d->%d\n", (int) t->e->velocity, (int)power,*t->pwm-(int)127);
//				//	*t->pwm=127;
//
//				}
//				else {
//					//direction changed.  went past our stop
//					advancePhase=1;	//finished stopping
//					*t->pwm = 127;
//				}
//				break;
//			case 2:
//				t->returnValue = kResultSuccess;
//				break;
//			default:
//				t->returnValue= kResultError;
//		}	//end switch
//
//		if (advancePhase) t->phase++;
//
//		if (0==t->cyclesRemaining && 0==t->returnValue)
//		{
//			*t->pwm=127;
//			 t->returnValue = kResultTimeOut;
//		}
//	} while (advancePhase);
//}



/***********************************************************************************/
void MoveRobotInitialize(int distanceETicks, int maxCyclesTimeout, char maxPWMswing)
{
	extern robotTask gTask;

//	gTask.phase = 0;
	gTask.maxPWM = maxPWMswing;
	gTask.cyclesRemaining = maxCyclesTimeout; //# of 26.2ms cycles to timeout

	gTask.initialDirection = (distanceETicks>=0 ? forward:reverse);

	// target location is length + (1/2) sum(left/right encoders)
    //faster to compute location = 2 * length + sum(left/right encoders)
	gTask.destination = distanceETicks + distanceETicks;
	gTask.destination +=EncoderRight.position+EncoderLeft.position;

	//look for invalid parameters	
	gTask.returnValue=kResultRunning;
	if (gTask.maxPWM > 0) {}
	else if (gTask.cyclesRemaining > 0) {}
	else gTask.returnValue = kResultError; //invalid parameter
} // End of MoveRobotInitialize(...)




/********************************************************************************
* FUNCTION: MoveRobotForward()
*
* DESCRIPTION: 
*  Call MoveRobotInitialize(...) first
*  If initial destination is positive, robot is moved forward
*  until the SUM of the left and right encoders is at least
*  destinationETicks.
*  Robot power is set to neutral at end of run, but robot may still coast
********************************************************************************/
char MoveRobotForward(void)
{	
	extern robotTask gTask;
	long positionSum;	//sum of left and right encoder positions
	
//	mPWMLeft = mPWMRight = 127;	//default if error or success
	
	//Check if task already completed or improperly initialized
	if (gTask.returnValue != kResultRunning)
		return gTask.returnValue;

	if (gTask.cyclesRemaining<=0)
		return gTask.returnValue = kResultTimeOut;
		
	gTask.cyclesRemaining--;
	

	//calculate projected location at end of this cycle:
	positionSum = EncoderRight.projectedPos+EncoderLeft.projectedPos;
	
	// does distance remaining have same sign as the initial direction?
	if ((positionSum < gTask.destination) == (forward==gTask.initialDirection))
	{
		struct { int left,right; } drivePower;
		
		drivePower.right = (forward==gTask.initialDirection?gTask.maxPWM:-gTask.maxPWM);
		drivePower.left = drivePower.right;
		
#ifdef MatchLeftRight	//this seemed to fail in tests; need to retest				
		if (forward == gTask.initialDirection)
		{
			if (EncoderLeft.velocity < EncoderRight.velocity)
				drivePower.right=0;
			else if (EncoderLeft.velocity > EncoderRight.velocity)
				drivePower.left=0;	
		} else
		{	//direction is reverse; velocities should be negative
			if (EncoderLeft.velocity < EncoderRight.velocity)
				drivePower.left=0;
			else if (EncoderLeft.velocity > EncoderRight.velocity)
				drivePower.right=0;	
		}
#endif //MatchLeftRight
		DriveLeftMotors(drivePower.left);
		DriveRightMotors(drivePower.right);
	}
	else
	{	//passed (or will pass) destination
		gTask.returnValue = kResultSuccess;	//success
	}
	return gTask.returnValue;
}
//end of MoveRobotFwd()

/***********************************************************************************/

char RobotStopped(char maxVelocity)
{
	return (mAbsolute(EncoderLeft.velocity) <= maxVelocity) &&
	(mAbsolute(EncoderRight.velocity) <= maxVelocity);	
}
