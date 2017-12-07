/*******************************************************************************
* FILE NAME: user_routines.c <FRC VERSION>
*
* DESCRIPTION:
*  This file contains the default mappings of inputs  
*  (like switches, joysticks, and buttons) to outputs on the RC.  
*******************************************************************************/

#include "ifi_aliases.h"
#include "ifi_default.h"
#include "ifi_utilities.h"
#include "user_routines.h"
#include "printf_lib.h"
#include "interrupts.h"
#include "lrtUtilities.h"
#include "OIFeedback.h"
#include "lrtConnections.h"
#include "lrtMotorDrive.h"
#include "arm.h"
#include "hook.h"
#include "motorSIM.h"
#include "autonomous.h"

encoder EncoderRight={0,0,0,0}, EncoderLeft={0,0,0,0};



struct Clock volatile gClock;

static void Process_Data_Shell0(void);
static void Slow26msLoop(void);
static void ClearInputsForSimulation(void);


/*******************************************************************************
* FUNCTION NAME: Process_Data_From_Master_uP
* PURPOSE:       Executes every 26.2ms when it gets new data from the master 
*                microprocessor.
* CALLED FROM:   main.c
* ARGUMENTS:     none
* RETURNS:       void
*******************************************************************************/
//Note: 26.2ms is 2^18 * (1/10MHz) [D.Giandomenico]
//There are 38.15 cycles / second

void Process_Data_From_Master_uP(void)
{
 	Getdata(&rxdata);   /* Get fresh data from the master microprocessor. */
//printf("Begin Loop\n");

#ifdef SIM
	autonomous_mode=1;	//for simulation
#ifdef _FRC_BOARD
	disabled_mode=0;	//for simulation
#endif //_FRC_BOARD
//	ClearInputsForSimulation();
#endif	//SIM
	
	UpdateSlowLoopTimers();

#ifdef _FRC_Board
	if (disabled_mode)
		if (gLoop.onSecond2)
			printf("Outputs & OI Disabled\n");
			//outputs are disabled, but routines continue to run.
#endif	//_FRC_BOARD

//	if (autonomous_mode || gLoop.onSecond2)
//		printf("autnonomous_mode=%d\n", (int)autonomous_mode);
//
	Process_Data_Shell0();
//printf("End Loop\n");

  Putdata(&txdata);             /* DO NOT CHANGE! */
}


/******************************************************************************/
static void Process_Data_Shell0(void)
{
	GetEncoderPositionNow();
	GetMotorSpeeds();

	GetArmPositions();
#ifdef SIM
	GetSimulatedArmPositions();	//overrides values from GetArmPositions()
#endif //SIM

	ClearOIFeedbackData();	//clear copy of OI LED/User bytes
//	StallDetect();		//check if power was applied but no velocity resulted
	
	Slow26msLoop();
//printf("exit Slow26msLoop\n");

#ifdef SIM
		SIMmotorDrive();
		SimulateArmMotors();
#endif
	UpdateOIFeedbackData(); //copy OI LED/User bytes to txdata
//	StallDetect_SaveDrivePWMs();	//save info about power applied to drive
}

/******************************************************************************/
static void Slow26msLoop(void)	//called from Process_data_From_Master_uP() above
{
	AllStop();	//force all pwms to neutral;
	ClearMotorPWMs();	//clear all motor drive values.
	gDriveLimitedFlag = 0;	//clear drive current limited flag

//	Operate Compressor as needed
	mCompressorRelay = !mCompressorAtPressure;

	WhackTetraReset();	//send tetra whacker to initial position,
		// only on startup when enabled (disabled if autonomous ever active)

	//diagnostics; print every four seconds
	if(gLoop.onSecond4)
	{
		printf("\n   SlowLoop -- ");
		PrintTime();
		PrintDistanceFeet();
		PrintVelocity();
		PrintArmPosition();
#ifdef _FRC_BOARD
		//empirical formula based on voltage reported on OI.  Formula given in aliases.h is inaccurate. [dg]
		printf("battery voltage: %ddV / %ddV\n",
			(int)rxdata.rc_main_batt*163 >>8,
			(int)rxdata.rc_backup_batt*163 >>8);
#endif //_FRC_BOARD
#ifdef SIM
		printf("\n\n* * * MOTOR SIMULATION MODE * * * \n\n"); //warning!!
#endif //SIM
	}

//	if (gLoop.onSecond) printf("Slow26msLoop\n");
if (gLoop.onSecond2) {
	if (mLeftLowGearSw) printf("Left in Low Gear\n");
	if (mLeftHighGearSw) printf("Left in High Gear\n");
	if (mRightLowGearSw) printf("Right in Low Gear\n");
	if (mRightHighGearSw) printf("Right in High Gear\n");

	if (mShoulderUpperLimitSw) printf("Shoulder Upper Limit\n");
	if (mShoulderLowerLimitSw) printf("Shoulder Lower Limit\n");
	if (mForeArmUpperLimitSw) printf("Forearm Upper Limit\n");
	if (mForeArmLowerLimitSw) printf("Forearm Lower Limit\n");
	
}

	
	if (mTetraLoadServiceFlag) ToggleSignalFlag();

#ifdef _FRC_BOARD
	if (disabled_mode) gHookPosition = kHookUp;
#endif //_FRC_BOARD

	mHookRelay=gHookPosition;	//copy Hook position to relay
	LRTConsoleMapping();
//printf("exit consolemapping\n");

	SignalFlagPWM();

	//send data back to 'Dashboard'
	txdata.user_byte5=EncoderLeft.velocity;
	txdata.user_byte6=EncoderRight.velocity;


  //Generate_Pwms(pwm13,pwm14,pwm15,pwm16);
	ApplyArmLimitSwitches();
	RemoveAllPWMDeadbands();
}
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

static void ClearInputsForSimulation(void)
{
	rc_dig_in01=rc_dig_in02=rc_dig_in03=rc_dig_in04=rc_dig_in05=rc_dig_in06=
	rc_dig_in07=rc_dig_in08=rc_dig_in09=rc_dig_in10=rc_dig_in11=rc_dig_in12=
	rc_dig_in13=rc_dig_in14=rc_dig_in15=rc_dig_in16=rc_dig_in17=rc_dig_in18=1;

//set all inputs to relaxed 'high' postition for simulation only
}
