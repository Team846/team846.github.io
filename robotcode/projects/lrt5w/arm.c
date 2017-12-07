#include "arm.h"
#include "ifi_utilities.h"
#include "lrtConnections.h"
#include "lrtMotorDrive.h"
#include "lrtUtilities.h"
#include "printf_lib.h"
#include "lrtResultCodes.h"
#include "xferProfiles.h"
/*
 * Arm control Overview
 * Each pass through the loop, the arm positions are read with a call to
 * GetArmPositions();
 * If an automated arm routine is running, then the arms controls are locked
 *   To unlock the arms, once the routine is stopped,
 *      if under joystick control, the controls must be neutral
 *		if under absolute control, then the controls must pass near the
 *		current arm position
 * 
 * If in an automated routine, arms are given setpoints and pwm's are calc'd 
 * to go to those set points.
 * If in joystick user control, arms are commanded up or down
 * If in absolute user control, arms are commanded to setpoints

 * In Absolute modes, Arms may be controlled using PID or alternately,
 * by commands to go toward the setpoint, stop once that position is reached,
 * and await new changes in the input.
*/

#ifdef _04Robot
	//forearm pot seems damaged on old robot.
	enum armLimits { kShoulderLow=100, kShoulderHigh=900, kForearmLow=600, kForearmHigh=900 };
#else
//stops within ~43 of limit on forearam
//	enum armLimits { kShoulderLow=340, kShoulderHigh=640, kForearmLow=75, kForearmHigh=950 };
	enum armLimits { kShoulderLow=340, kShoulderHigh=600, kForearmLow=75, kForearmHigh=960 };
#endif

pidData gpid,	//scratch area
	gShoulderPID = { 0,0,0, 0,0,0},
	gForearmPID = { 1,0,0, 0,0,0};

typedef struct {
	unsigned char cyclesRemaining;
	char direction; // either kUp or kDown
	char power127;
}	timedArmTask;

timedArmTask pTimedForearmTask;
/*******************************************************************************/
arms gArm;


int mapForearmInput(unsigned char inputOI);
int mapShoulderInput(unsigned char inputOI);
void armCommand(arm *theArm, int armInput);

void queryWhichArmInterface(void);
int MoveArm(int distance, char direction);

/********************************************************************************
* FUNCTION: GetArmPositions()
*
* DESCRIPTION: Read the analog positions of the arms at the beginning
* of each loop.
*
********************************************************************************/

void GetArmPositions(void)
{
	gArm.forearm.oldPosition = gArm.forearm.curPosition;
	gArm.shoulder.oldPosition = gArm.shoulder.curPosition;
#ifdef _SIMULATOR
	gArm.forearm.curPosition = -1;	//can't call Get_Analog_Value within simulator
	gArm.shoulder.curPosition = -1;
#else	//_SIMULATOR
	gArm.forearm.curPosition = Get_Analog_Value(kA2DForearmJoint);
	gArm.shoulder.curPosition = 1024-Get_Analog_Value(kA2DShoulderJoint);
#endif		//_SIMULATOR

	if (1&& gLoop.onSecond)
		printf("shoulderA2D=%d, forearmA2D=%d\n", (int)gArm.shoulder.curPosition, (int)gArm.forearm.curPosition);

//good place to get values for k
//#define _TunePID
#ifdef _TunePID
//KD =Get_Analog_Value(kA2D1);
//KI =Get_Analog_Value(kA2D2);
//KD =Get_Analog_Value(kA2D3);
#endif //_TunePID
}


/********************************************************************************
* FUNCTION: queryWhichArmInterface()
*
* DESCRIPTION: Determine whether joysticks or control arm is installed.
* Could be more complete at the expense of more code.
*  this should be adequate
********************************************************************************/
void queryWhichArmInterface(void)
{	
	//This should be carefully checked against definitions in lrtConnections.h 
	gArm.useJoyStick=1;
	gArm.controlConnectedToOI=1;
	if (p1_x==127 && p1_y==127 && p2_wheel==127 && p2_aux==127 && p2_x==127 && p2_y==127)
	{
		gArm.useJoyStick=0;
		if (p1_aux==127 && p1_wheel==127)
			gArm.controlConnectedToOI=0;	//all inputs neutral; no control connected
				//this might cause  trouble  if using custom control and inputs right  at 127.
	}
}



void lockUserArmControls(void)
{
	gArm.forearm.locked=1;
//	gArm.forearm.lastUnlockedValue=mForearm;	//record last known user command

	gArm.shoulder.locked=1;
//	gArm.shoulder.lastUnlockedValue=mShoulder;	//record last known user command
}

/********************************************************************************
* FUNCTION: checkArmLocks() 
*
* DESCRIPTION: unlocks each arm if 
 * 1) input has moved
*  2) input is near curposition value
* Different tolerances on each arm.
*
********************************************************************************/


void updateArmLocksAbsolute(void)
{
	if (gArm.forearm.locked)
			//check to see if input changed
		if (!isWithinRange(mForearmAbs, gArm.forearm.lastOIcmd, 8))	//input moved?
			if (isWithinRange(gArm.forearm.curInput, gArm.forearm.curPosition, mDegrees2BitsRobot(20)))
				gArm.forearm.locked=0;		//unlock this arm	


	if (gArm.shoulder.locked)
			//check to see if input changed
		if (!isWithinRange(mShoulderAbs, gArm.shoulder.lastOIcmd, 8)) //input moved?
			if (isWithinRange(gArm.shoulder.curInput, gArm.shoulder.curPosition, mDegrees2BitsRobot(10)))
				gArm.shoulder.locked=0;		//unlock this arm	

	if (gLoop.onSecond2)
	{
		if (gArm.forearm.locked) printf("forearm control locked\n");
		if (gArm.shoulder.locked) printf("shoulder control locked\n");
	}
	gArm.forearm.lastOIcmd = mForearmAbs;		//save inputs for next time
	gArm.shoulder.lastOIcmd = mShoulderAbs;
}	
/********************************************************************************/


/********************************************************************************
* FUNCTION: UserControlArmsAbsolute()
*
* DESCRIPTION: Reads inputs as absolute positions
* If input change is > some small change, then issues a new move command
********************************************************************************/


void UserControlArmsAbsolute(void)
{
	if (gArm.commandInProgress) return;
	
	gArm.forearm.curInput = mapForearmInput(mForearmAbs);
	gArm.shoulder.curInput = mapShoulderInput(mShoulderAbs);

	updateArmLocksAbsolute();

	if (gLoop.onSecond)
	{
		printf("UserControlArmsAbsolute move to %d / %d --> %d / %d mForearmAbs/mShoulderAbs-->Forearm/shoulder\n",
			(int) mForearmAbs,(int) mShoulderAbs,(int)  gArm.forearm.curInput, (int)gArm.shoulder.curInput);
	}
	if (gLoop.onHalfSecond)
	{
		if (gArm.forearm.locked)
			printf("forearm locked\n");
		if (gArm.shoulder.locked)
			printf("shoulder  locked\n");

	}
	
	OperateArms();
return;

	
	if (!gArm.forearm.locked)
	{
		if(gLoop.onSecond)
			printf("Move Forearm\n");
		armCommand(&gArm.forearm, gArm.forearm.curInput);
		gPWM.forearm = MoveArm(gArm.forearm.distance, gArm.forearm.direction);
	}
	if (!gArm.shoulder.locked)
	{

		if(gLoop.onSecond)
			printf("Move Shoulder\n");
		armCommand(&gArm.shoulder, gArm.shoulder.curInput);
		gPWM.shoulder = MoveArm(gArm.shoulder.distance, gArm.shoulder.direction);
	}
}



/********************************************************************************/

#define kThreshold mDegrees2BitsRobot(10)

void armCommand(arm *theArm, int armInput)
{
	overlay int deltaInput = theArm->setpoint - armInput;
	theArm->setpoint = armInput;
	theArm->distance = theArm->setpoint - theArm->curPosition;

	if (deltaInput < -kThreshold || deltaInput > kThreshold)
	{
		//recalculate direction
		theArm->direction = theArm->distance > 0 ? kUp: kDown;
	}
}
/********************************************************************************/

void UserControlArmsJoystick(void)
{

//	if (gLoop.onSecond)
//	{	
//		printf("InUserControlArms()\n");
//		printf("cmd in progress=%d\n",(int)gArm.commandInProgress);
//		printf("forearm.locked=%d\n",(int) gArm.forearm.locked);
//		printf("shoulder.locked=%d\n",(int) gArm.shoulder.locked);
//		printf("mforearm in=%d\n",(int) mForearmJoy);
//		printf("mshoulder in=%d\n",(int) mShoulderJoy);
//	}


	if (gArm.commandInProgress) return;

	gPWM.forearm = addDeadband(mForearmJoy);
	gPWM.shoulder = addDeadband(mShoulderJoy);



//Various profiles to change joystick -> output sensitivity.
//	gPWM.forearm = profile(gPWM.forearm, xferPower4_5);	//a smooth curve
//	gPWM.shoulder = profile(gPWM.shoulder, xferPower4_5);

//	gPWM.forearm = profile(gPWM.forearm, xfer96_4);	//a piecewiselinear curve
//	gPWM.shoulder = profile(gPWM.shoulder, xfer96_4);

//	gPWM.forearm = piecewiselinear(gPWM.forearm); //a piecewise linear function
//	gPWM.shoulder = piecewiselinear(gPWM.shoulder);


	//unlock arms if no command in progress and either joystick is at center.
	if (gPWM.forearm == 0) gArm.forearm.locked = 0;
	if (gPWM.shoulder == 0) gArm.shoulder.locked = 0;

#define kMinPower 5
	if (!gArm.forearm.locked)
	{
		//set min power (could be done in addDeadBand() )
		if (gPWM.forearm < kMinPower && gPWM.forearm > - kMinPower)
			gPWM.forearm = 0;
	//	if (gPWM.forearm < 0)	//moved to removePWMDeadband()
	//		gPWM.forearm = mDivideBy4(gPWM.forearm);
	}
	if (!gArm.shoulder.locked)
	{
		if (gPWM.shoulder < kMinPower && gPWM.shoulder > - kMinPower)
			gPWM.shoulder = 0;
	//	if (gPWM.shoulder < 0) 	//moved to removePWMDeadband()
	//		gPWM.shoulder = mDivideBy4(gPWM.shoulder);
	}

	
//	if (gLoop.onSecond)
//	{
//		printf("gPWM.forearm in=%d\n",(int) gPWM.forearm);
//		printf("gPWM.shoulder in=%d\n",(int) gPWM.shoulder);
//
//	}
}
/********************************************************************************/

void ApplyArmLimitSwitches(void)
{
	//switches should go high when limited (interrupted), or disconnected
	//motors should be connected such that positive pwm -> up movement

	
	if (1 && gLoop.onSecond)
	{
		if (mShoulderUpperLimitSw ||mShoulderLowerLimitSw)
			printf("ArmLmtShldr\n");
		if (mForeArmUpperLimitSw || mForeArmLowerLimitSw)
			printf("LmtForeArm\n");

		if (gArm.shoulder.curPosition < kShoulderLow) printf("L shlder pot lmt\n");
		if (gArm.shoulder.curPosition > kShoulderHigh) printf("H shlder pot lmt\n");
		if (gArm.forearm.curPosition < kForearmLow) printf("L forearm pot lmt\n");
		if (gArm.forearm.curPosition > kForearmHigh) printf("H forearm pot lmt\n");
		printf("PWM: shoulder =%d   forearm=%d\n", (int)gPWM.shoulder, (int)gPWM.forearm); 
	};

//return;		// ignore limit switches; not installed;

#ifdef _04Robot
	//ignore limit switches - connected to other parts  on that robot
#else //_04Robot
	if (mShoulderUpperLimitSw && gPWM.shoulder>0)
		gPWM.shoulder=0;
	if (mShoulderLowerLimitSw && gPWM.shoulder<0)
		gPWM.shoulder=0;

	if (mForeArmUpperLimitSw && gPWM.forearm>0)
		gPWM.forearm=0;
	if (mForeArmLowerLimitSw && gPWM.forearm<0)
		gPWM.forearm=0;
#endif //_04robot

//	//software limits
#define UseLimits
#ifdef UseLimits
	if (gLoop.onSecond2)
		printf("SOFTWARE LIMITS IN USE\n");

	if (gArm.forearm.curPosition > kForearmHigh && gPWM.forearm > 0)
		gPWM.forearm=0;
	if (gArm.forearm.curPosition < kForearmLow && gPWM.forearm < 0)
		gPWM.forearm=0;
	if (gArm.shoulder.curPosition > kShoulderHigh && gPWM.shoulder > 0)
		gPWM.shoulder=0;
	if (gArm.shoulder.curPosition < kShoulderLow && gPWM.shoulder < 0)
		gPWM.shoulder=0;


	//more restrictive test when  using  absoluteArm
	if ( 0== gArm.useJoyStick && gArm.controlConnectedToOI)
	{	
//		enum armLimits { kShoulderLow=340, kShoulderHigh=600, kForearmLow=75, kForearmHigh=960 };
		enum armLimits { kShoulderLow=400, kShoulderHigh=550, kForearmLow=450, kForearmHigh=550 };
		if (gArm.forearm.curPosition > kForearmHigh && gPWM.forearm > 0)
			gPWM.forearm=0;
		if (gArm.forearm.curPosition < kForearmLow && gPWM.forearm < 0)
			gPWM.forearm=0;
		if (gArm.shoulder.curPosition > kShoulderHigh && gPWM.shoulder > 0)
			gPWM.shoulder=0;
		if (gArm.shoulder.curPosition < kShoulderLow && gPWM.shoulder < 0)
			gPWM.shoulder=0;
	}
#endif


	if (mBashPlateOverride)
		gArm.overrideBashPlate=1;	//won't be set to zero until a program reset

	if (gArm.overrideBashPlate)
	{
		if  (gLoop.onSecond2) printf("Bashplate overridden\n"); //do nothing
	}
	else if (mBashPlateLimit)
	{
		if (gLoop.onSecond) printf("bash Plate\n");
		if (gPWM.forearm < 0) gPWM.forearm=0;
		if (gPWM.shoulder < 0) gPWM.shoulder=0;
	}
}



/* works on data on gPid -- need to copy data in and out. */

void armPID(int error)
{
	extern pidData gpid;	
	overlay int differential;
	overlay struct {
		long short proportional;	//'long short' is 24 bits
		long integral;
		long short differential;
	} pwm;	//these are the 3 components that make up the PID ouput.
	

//	1+a+a^2+a^3... = 1/(1-a)
//	1 + b/256 + (b/256)^2 + (b/256)^3... = 256/(256-b)
//	so max we can climb is 256* error.

	gpid.integral = gpid.integral*255;	//decay constant (e.g. 255/256) 
	gpid.integral = mDivideBy256(gpid.integral) + error;

	pwm.proportional = gpid.KP * error;
	pwm.proportional = mDivideBy64(pwm.proportional);

	// typical integral values may climb to 256 * 512, or 2^17 and as high as 2^18
	// if we keep KI on {0,256} then valuy
	// to keep pwm on {0,127} (2^7)
	pwm.integral = gpid.KI * gpid.integral;
	pwm.integral = mDivideByPowerOf2(pwm.integral,11);	//divide by 2048

//	pwm.differential = error - gpid.error_prior;
//	gpid.error_prior = error;	//save error for next loop
//	pwm.differential = gpid.KD * pwm.differential;
//	pwm.differential = mDivideBy256;

	gpid.pwm = pwm.proportional;
	gpid.pwm += pwm.integral;
//	gpid.pwm += pwm.differential;

	mLimitRange(gpid.pwm,-127,127);
}


void OperateArms(void)
{	
	gpid = gShoulderPID; //copy data structure
	armPID(gArm.shoulder.curPosition-gArm.shoulder.setpoint);
	gPWM.shoulder = gpid.pwm;
	gShoulderPID = gpid; //save updated structure
	
	gpid = gForearmPID; //copy data structure
	armPID(gArm.forearm.curPosition-gArm.forearm.setpoint);
	gPWM.forearm = gpid.pwm;
	gForearmPID = gpid;	//save updated structure
}





int MoveArm(int distance, char direction)
{
	overlay int pwm=0;
	
	if ((distance > 0) != (direction==kUp))
		return 0;	//already past our objective

	if (distance < 0) distance = -distance;		//abs()

	if (distance > mDegrees2BitsRobot(20))
		pwm = 127;	//full power
	else if (distance > mDegrees2BitsRobot(1))
		pwm = 32;	//reduced power
	else 
		pwm = 0;	//close enough
	
	if (direction==kDown)
		pwm = -pwm;
	return pwm;
}


/********************************************************************************
* FUNCTION: mapForearmInput() mapLowerArmInput();
*
* DESCRIPTION: maps OI input to correspond to sensor pot on forearm
* maps 255 -> -1
* maps {a,b} to {w,x}
* maps {0,a} to w
* maps {b,254} to x
********************************************************************/
int mapForearmInput(unsigned char inputOI)
{
	enum in {a=22, b=211};	//must be on range {0,255}, b>a
	enum out {w=400,x=600};	//must be on range {0,1023}, no other restriction
	
	overlay int setPoint = -1;

	if (255==inputOI)
		;	//error -  not a possible input. leave setpoint = -1.
	else if (inputOI >= b)
		setPoint=x;
	else if (inputOI > a)
	{
		//setPoint = ((inputOI-a)*(short long)x + (b-inputOI)*(short long)w) / (b-a);
		setPoint = ((inputOI-a)*((short long)x - w)) / (b-a) + w;
	}
	else
		setPoint=w;

	return setPoint;
}
/**************************************************************/
int mapShoulderInput(unsigned char inputOI)
{
	enum in {a=88, b=209};
	enum out {w=400,x=600};
	
	overlay int setPoint = -1;

	if (255==inputOI)
		;	//error; control disconnected; leave setpoint = -1.
	else if (inputOI >= b)
		setPoint=x;
	else if (inputOI > a)
//		setPoint = ((inputOI-a)*(int)x + (b-inputOI)*(int)w) / (int) (b-a);
		setPoint = ((long short)(inputOI-a)*((int)x - (int)w)) / (int) (b-a) + w;
	else
		setPoint=w;

	return setPoint;
}



void armPresets(void)
{
	//measure angles relative to known reference angles to make calibration easier.

	overlay int forearmPos, shoulderPos;
	overlay char buttonPressed=0;

	if (gArm.commandInProgress)
		return;	


	if (mBtnArmGroundLevel)	//load  tetra
	{
		buttonPressed=1;
		forearmPos = kForeArmLevel + mDegrees2BitsRobot(-10);
		shoulderPos = kShoulderVertical + mDegrees2BitsRobot(-45);
	}
	if (mBtnArmLoadTetra)	//carry
	{
		buttonPressed=1;
		forearmPos = kForeArmLevel + mDegrees2BitsRobot(30);
		shoulderPos = kShoulderVertical + mDegrees2BitsRobot(-45);
	}
	if (mBtnArmCarryTetra)	//low goal
	{
		buttonPressed=1;
//		forearmPos = kForeArmLevel + mDegrees2BitsRobot(60);
//		shoulderPos = kShoulderVertical + mDegrees2BitsRobot(-15);
		forearmPos = kForeArmLevel + mDegrees2BitsRobot(45);
		shoulderPos = kShoulderVertical + mDegrees2BitsRobot(-45);
	}
	if (mBtnArmRaiseTetra)
	{
		buttonPressed=1;
		forearmPos = kForeArmLevel + mDegrees2BitsRobot(80);
		shoulderPos = kShoulderVertical + mDegrees2BitsRobot(0);
	}
	if (buttonPressed)
	{
		printf("Arm move to %d / %d (forearm/shoulder)\n", (int) forearmPos, (int)shoulderPos);
		ArmMoveInitialize(forearmPos, shoulderPos);
	}
}

void ArmMoveInitialize(int forearmPosition, int shoulderPosition)
{
	if (gArm.commandInProgress) return;

	gArm.commandInProgress=1;
	gArm.timer = 4*38;

	//limit to possible range - otherwise will result in timeouts.
	//Need to also  look  for hard  switch limits
	gArm.forearm.setpoint = LimitRange(forearmPosition, kForearmLow, kForearmHigh);
	gArm.shoulder.setpoint = LimitRange(shoulderPosition, kShoulderLow,kShoulderHigh);
	
	//input error check
	if (forearmPosition != gArm.forearm.setpoint) printf("Forearm pos limited\n");
	if (shoulderPosition != gArm.shoulder.setpoint) printf("Shoulder pos limited\n");

	gArm.forearm.inPosition = gArm.shoulder.inPosition = 0;

	gArm.forearm.direction = kDown;
	if (gArm.forearm.curPosition < gArm.forearm.setpoint) gArm.forearm.direction = kUp;

	gArm.shoulder.direction = kDown;
	if (gArm.shoulder.curPosition < gArm.shoulder.setpoint) gArm.shoulder.direction = kUp;

	lockUserArmControls();	
}
/********************************************************************************
* FUNCTION: ArmMoveRun()
*
* DESCRIPTION: Uses MoveArm() (Not PID)
* Moves arms until the cross destination.
********************************************************************************/

char ArmMoveRun(void)
{
	if (!gArm.commandInProgress)
		return kResultNotRunning;
	if (gArm.timer == 0)
	{
		PrintArmPosition();
		printf("ArmMoveRun Timeout. In Pos? F/S %d, %d\n",
			(int)gArm.forearm.inPosition, (int)gArm.shoulder.inPosition);
		return kResultTimeOut;
	}

	PrintTime();

	gArm.timer--;
	if (!gArm.forearm.inPosition)
	{
		gArm.forearm.distance = gArm.forearm.setpoint-gArm.forearm.curPosition;
//		printf("distance: %d\n", (int) gArm.forearm.distance);
		gPWM.forearm = MoveArm(	gArm.forearm.distance, gArm.forearm.direction);
//		printf("gPWM.forearm = %d\n", (int)gPWM.forearm);
		if (gPWM.forearm == 0)
			gArm.forearm.inPosition=1;	
	}
	
	if (!gArm.shoulder.inPosition)
	{
		gArm.shoulder.distance = gArm.shoulder.setpoint-gArm.shoulder.curPosition;
		gPWM.shoulder = MoveArm( gArm.shoulder.distance, gArm.shoulder.direction);
		if (gPWM.shoulder == 0)
			gArm.shoulder.inPosition=1;	
	}

	//hard limit check (so we don't sit and timeout)
	//this code should be collected into a subroutine so as not to duplicate code
	//in applyLimitswitches.
	//gPWMs will be  set to zero in ApplyLimitSwitches
	if (mShoulderUpperLimitSw && gPWM.shoulder>0)
		gArm.shoulder.inPosition=1;
	if (mShoulderLowerLimitSw && gPWM.shoulder<0)
		gArm.shoulder.inPosition=1;

	if (mForeArmUpperLimitSw && gPWM.forearm>0)
		gArm.forearm.inPosition=1;
	if (mForeArmLowerLimitSw && gPWM.forearm<0)	//this switch not installed.
		gArm.forearm.inPosition=1;


	if (gArm.forearm.inPosition && gArm.shoulder.inPosition)
	{
		PrintArmPosition();
		printf("ArmMoveRun Complete\n");
		return kResultSuccess;
	}
	else
		return kResultRunning;
}

	

void MoveForearmTimedStart(char ticks, char power127)
{
	extern timedArmTask pTimedForearmTask;
	pTimedForearmTask.cyclesRemaining = ticks;
	pTimedForearmTask.power127 = power127;
}

char MoveForearmTimedRun(void)
{
	if (0==pTimedForearmTask.cyclesRemaining)
		return kResultNotRunning;

	pTimedForearmTask.cyclesRemaining--;
	gPWM.forearm  = pTimedForearmTask.power127;
	return kResultRunning;
}

/*
 * checkIfOIArmMoved()
 * looks to see if the OI arm moved.
 * call once per loop  at beginning.
  * gArms.eitherMoved will only stay true for one cycle.
 */
void checkIfOIArmMoved(void)
{
	gArm.forearm.armOImoved = gArm.shoulder.armOImoved = 0;
	
	if (gArm.eitherMoved)
	{		//clear, and save current arm position
		gArm.eitherMoved = 0;
		gArm.forearm.lastOIcmd =  mForearmAbs;
		gArm.shoulder.lastOIcmd = mShoulderAbs;
	}
	else
	{		
		if (!isWithinRange(mForearmAbs,  gArm.forearm.lastOIcmd, 3))
			gArm.eitherMoved = gArm.forearm.armOImoved = 1;
		if (!isWithinRange(mShoulderAbs, gArm.shoulder.lastOIcmd, 3))
			gArm.eitherMoved = gArm.shoulder.armOImoved = 1;
	}
}
