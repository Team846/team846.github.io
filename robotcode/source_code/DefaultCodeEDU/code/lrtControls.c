/********************************************************************************
* FILE NAME: lrtControls.c
*
* DESCRIPTION: 
*
********************************************************************************/
#include "ifi_aliases.h"
#include "ifi_default.h"
#include "lrtConnections.h"
#include "printf_lib.h"
#include "lrtUtilities.h"
#include "lrtMotorDrive.h"
#include "arm.h"
#include "lrtResultCodes.h"
#include "hook.h"
#include "xferProfiles.h"
#define TORQUE_DRIVE 1		//for user control only; see lrtRobotMove.c too

enum activeRoutines { kNoRoutine, kRoutineAutonomous, kRoutineClimb, kRoutineDescend };
// enum activeRoutines { kNoRoutine, kRoutineAutonomous, kHookTetra, kReleaseTetra };

static struct {
	char returnValue;
	char activeRoutine;
} automationState = { kResultNotRunning, kNoRoutine };

char gHookPosition;	//in hook.h

#ifdef _04Robot
char _NullPWM;		//used to test on prior year's robot
#endif //_04Robot

/*******************************************************************************
* FUNCTION NAME: LRTConsoleMapping
* PURPOSE:       Performs the default mappings of inputs to outputs for the
*                Robot Controller.
* CALLED FROM:   this file, Process_Data_From_Master_uP routine
* ARGUMENTS:     none
* RETURNS:       void
*******************************************************************************/
void lrtJoystickDrive(void);
static void LRTAutomationRoutines(void);

void LRTConsoleMapping(void)
{
	static struct {
		unsigned autonomousLastCycle:1;
	} modeFlag={0};

	if (1 || gLoop.onSecond) queryWhichArmInterface();	//see if joysticks or control arm is connected
		//check every second in case they are changed on the fly.
		//sets gArm.useJoyStick bit.

	
	//catch transition from Autonomous to UserMode for any initialization	
//check if UserMode started (immediately following autonomousMode)
	if (!autonomous_mode && modeFlag.autonomousLastCycle)
	{
//		TimerHookExtend(1);	//freeze control
//		TimerBoomElevate(1);
//		TimerBoomRotate(1);
		//abort autonomous?
		//any other initialization?
	}
	modeFlag.autonomousLastCycle = autonomous_mode;


	if (gLoop.onSecond) printf("Slow26msLoop\n");

	//for testing & setup purposes only.  Send raw input to ouput.
	if (0)mPWMTestOut = mLeftDriveJoyStick; //pwm10 & p4_y; see lrtConnections.h


	LRTAutomationRoutines();

	//return value in global 'automationState'
	if (automationState.returnValue != kResultRunning)	//lockout joystick and legs if automation
	{
		if (mShiftLow)	ShiftLowGear();
		if (mShiftHigh) ShiftHighGear();

		if (mBtnHook) gHookPosition=kHookUp;
		if (mBtnHookRelease) gHookPosition=kHookDown;


		//print control arm diagnostic
		if (gLoop.onSecond4)
		{
			if (!gArm.controlConnectedToOI)
				printf("No arm control connected\n");
			else if (gArm.useJoyStick)
				printf("Joystick arm control\n");
			else
				printf("Custom arm control\n");
		}


		if (gArm.controlConnectedToOI)
		{
			if (mAbortArm)
			{
				printf("Abort Arm\n");
				gArm.commandInProgress=0;	//clear any commands
				//unlock arms?
			}
			armPresets();	//look for any presets pressed.
	
			if (gArm.useJoyStick)
				UserControlArmsJoystick();
			else
				UserControlArmsAbsolute();
		}

		if (0 && gLoop.onSecond)
			printf("GearShift shifting = %d\n", (int) (0!=gGearBox.shifting));

		if (gGearBox.shifting)
			DoShift();
		else
			lrtJoystickDrive();		//drive controls locked out while shifting

//		CheckLockOnLegControls();
//		LRTFrontRearLegMapping();
	}
	else
	{
//		gLegs.front.locked = gLegs.rear.locked = 1;
//		Automated_OperateLegs();
	}
	if (gArm.commandInProgress)
		if (kResultRunning != ArmMoveRun())
			gArm.commandInProgress = 0;


}
/*******************************************************************************/

void lrtJoystickDrive(void)
{
	//joystick output at extremes: stick back=0; fwd=254; stick left=254; right=0
	overlay int Left;
	overlay int Right;	
	overlay int Forward;
	overlay int Turn;

	Left = addDeadband(mLeftDriveJoyStick);
	Right = addDeadband(mRightDriveJoyStick);
	Turn = addDeadband(mTurnJoyStick);
		
//	Left = profile(Left, xferPower4_5);	//a smooth curve
//	Right = profile(Right, xferPower4_5);
//
	if (gGearBox.left.inHighGear && gGearBox.right.inHighGear)
	{
		Left = profile(Left, xfer96_4);	//a piecewiselinear curve
		Right = profile(Right, xfer96_4);
	}

//	Left = piecewiselinear(Left); //a piecewise linear function
//	Right = piecewiselinear(Right);

	if (mRightJoyStickOnly)	//need to mix inputs
	{
		//reduce gain on 'turn'
		//'turn' input is on right joystick
		Left =  (int)Right - (int)Turn;
		Right = (int)Right + (int)Turn;
	}
	else {

		Forward =	Left + Right;	//this area for rescaling turn and fwd
		Turn =		Left - Right;
		
		
//		printf("inHighGear %d %d L/R\n", (unsigned  int) gGearBox.left.inHighGear,
//			(unsigned  int) gGearBox.right.inHighGear);
//
		if (gGearBox.left.inHighGear && gGearBox.right.inHighGear)
		{
			Turn = mDivideBy2(Turn);
//			printf("Divide by 16\n");
		}
		else
		{
//			printf("Turn %d", (int) Turn);
			Turn = mDivideBy2(Turn);
//			printf("Turn %d\n", (int)Turn); 
//			printf("Divide by 8\n");
		}

		Left =	Forward + Turn;
		Right = Forward - Turn;

		Left = mDivideBy2(Left);
		Right =	mDivideBy2(Right);
	}		

	if (mLowRate)
	{
//		Forward =	Left + Right;	//this area for rescaling turn and fwd
//		Turn =		Left - Right;
//		Left =	Forward + Turn;
//		Right = Forward - Turn;
		Left = mDivideBy2(Left);
		Right = mDivideBy2(Right);
	}

//User_Byte1=LeftInput;
//User_Byte2=RightInput;

	mLimitRange(Left,-127,127);
	mLimitRange(Right,-127,127);
	
	DriveLeftMotors(Left);
	DriveRightMotors(Right);
}

/******************************************************************************
Remove deadband from Victor 884 ESC's
at the same time, map {-127,127} to {0,255}
******************************************************************************/
void RemoveAllPWMDeadbands(void)
{
	//inputs are signed; outputs are unsigned chars
#ifdef _04Robot	//connected opposite on 04Robot
	gPWM.forearm = -gPWM.forearm;
//	gPWM.shoulder = -gPWM.shoulder;
#endif
	if (gPWM.forearm < 0)	//moved to removePWMDeadband()
		gPWM.forearm = mDivideBy4(gPWM.forearm);
	if (gPWM.shoulder < 0) 	//moved to removePWMDeadband()
		gPWM.shoulder = mDivideBy4(gPWM.shoulder);
	else
		gPWM.shoulder = mDivideBy2(gPWM.shoulder);


	mPWMcimLeft=		removePWMDeadband(-gPWM.cimL);
	mPWMcimRight=		removePWMDeadband(gPWM.cimR);
	mPWMfpriceLeft=		removePWMDeadband(-gPWM.fpriceL);
	mPWMfpriceRight=	removePWMDeadband(gPWM.fpriceR);
	mPWMshoulder=		removePWMDeadband(-gPWM.shoulder);
	mPWMforearm=		removePWMDeadband(gPWM.forearm);
	//don't remove deadbands from servos
}	
/******************************************************************************/

static void LRTAutomationRoutines(void)
{
	//Abort automation routines
//#ifndef _SIMULATOR
//#endif	//_SIMULATOR
	if (mDriverAbort || disabled_mode)
	{
		switch (automationState.activeRoutine)
		{
			case kRoutineAutonomous:
				printf("AUTO Abort\n");
				AutonomousAbort();
				if (disabled_mode)
					AutonomousReset();	//allow another run after disable released.
				break;
//			case kRoutineClimb:
////				ClimbStepAbort();
//				break;
//			case kRoutineDescend:
////				DescendStepAbort();
//				break;
//			default:
				break;
		}
		automationState.activeRoutine = kNoRoutine;
		automationState.returnValue = kResultNotRunning;
		return;
	}

	//Initialize (start) automation routines
	if (automationState.returnValue != kResultRunning)
	{
		if (!autonomous_mode)
			AutonomousReset();
		
		if (autonomous_mode)
		{
//			if (mSwAutonomous) AutonomousReset();	//allow more than one run
			AutonomousInitialize();
			if (kResultRunning == AutonomousStatus())	//sucessful init? Then proceed.
			{
				automationState.activeRoutine = kRoutineAutonomous;
				automationState.returnValue = kResultRunning;
			}
		}
//		else if (mClimbStep)
//		{
////			ClimbStepInitialize();
//			automationState.activeRoutine = kRoutineClimb;
//			automationState.returnValue = kResultRunning;
//		}
//		else if (mDescendStep)
//		{
////			DescendStepInitialize();
//			automationState.activeRoutine = kRoutineDescend;
//			automationState.returnValue = kResultRunning;
//		}
	}	//End of Initialize  automation routines


	//Run automation routines
	if (automationState.returnValue==kResultRunning)
	{

		switch (automationState.activeRoutine)
		{
			case kRoutineAutonomous:
				automationState.returnValue = AutonomousRun();
				break;
			case kRoutineClimb:
//				automationState.returnValue = ClimbStepRun();
				break;
			case kRoutineDescend:
//				automationState.returnValue = DescendStepRun();
				break;
			default:
				break;
		}
	}
}

/*******************************************************************************/


