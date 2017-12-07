/*******************************************************************************
* FILE NAME: user_routines.c <FRC VERSION>
*
* DESCRIPTION:
*  This file contains the default mappings of inputs  
*  (like switches, joysticks, and buttons) to outputs on the RC.  
*
* USAGE:
*  You can either modify this file to fit your needs, or remove it from your 
*  project and replace it with a modified copy. 
*
*******************************************************************************/

#include <stdio.h>

#include "ifi_aliases.h"
#include "ifi_default.h"
#include "ifi_utilities.h"
#include "user_routines.h"
#include "serial_ports.h"
#include "camera.h"
#include "tracking.h"
#include "terminal.h"
#include "encoder.h"

#include "connections.h"
#include "controls.h"
#include "utilities.h"

#include "simulator.h"	

//gControl gCtrl;
CPULoadData gCPULoad={0,0xFFFFFFFF,0,0};
char gLoopCount;	//used for printing - print when ==0
encoder EncoderRight={0,0,0,0}, EncoderLeft={0,0,0,0};



static void doCamera(void);
static void	DiagnosticSystemLoad(void);

/*** DEFINE USER VARIABLES AND INITIALIZE THEM HERE ***/

/*******************************************************************************
* FUNCTION NAME: User_Initialization
* PURPOSE:       This routine is called first (and only once) in the Main function.  
*                You may modify and add to this function.
* CALLED FROM:   main.c
* ARGUMENTS:     none
* RETURNS:       void
*******************************************************************************/
void User_Initialization (void)
{
  Set_Number_of_Analog_Channels(SIXTEEN_ANALOG);    /* DO NOT CHANGE! */

/* FIRST: Set up the I/O pins you want to use as digital INPUTS. */
  digital_io_01 = digital_io_02 = digital_io_03 = digital_io_04 = INPUT;
  digital_io_05 = digital_io_06 = digital_io_07 = digital_io_08 = INPUT;
  digital_io_09 = digital_io_10 = digital_io_11 = digital_io_12 = INPUT;
  digital_io_13 = digital_io_14 = digital_io_15 = digital_io_16 = INPUT;
  digital_io_17 = digital_io_18 = INPUT;  /* Used for pneumatic pressure switch. */
    /* 
     Note: digital_io_01 = digital_io_02 = ... digital_io_04 = INPUT; 
           is the same as the following:

           digital_io_01 = INPUT;
           digital_io_02 = INPUT;
           ...
           digital_io_04 = INPUT;
    */

/* FOURTH: Set your initial PWM values.  Neutral is 127. */
	AllStop();	//set all pwms to neutral

/* FIFTH: Set your PWM output types for PWM OUTPUTS 13-16.
  /*   Choose from these parameters for PWM 13-16 respectively:               */
  /*     IFI_PWM  - Standard IFI PWM output generated with Generate_Pwms(...) */
  /*     USER_CCP - User can use PWM pin as digital I/O or CCP pin.           */
  Setup_PWM_Output_Type(IFI_PWM,IFI_PWM,IFI_PWM,IFI_PWM);

  /* 
     Example: The following would generate a 40KHz PWM with a 50% duty cycle on the CCP2 pin:

         CCP2CON = 0x3C;
         PR2 = 0xF9;
         CCPR2L = 0x7F;
         T2CON = 0;
         T2CONbits.TMR2ON = 1;

         Setup_PWM_Output_Type(USER_CCP,IFI_PWM,IFI_PWM,IFI_PWM);
  */

  /* Add any other initialization code here. */
 
  Init_Serial_Port_One();
  Init_Serial_Port_Two();

  //Kevin's encoder code
 //Initialize_Encoders();
	// Jay's timer code

	Initialize_timer();

#ifdef TERMINAL_SERIAL_PORT_1    
  stdout_serial_port = SERIAL_PORT_ONE;
#endif

#ifdef TERMINAL_SERIAL_PORT_2    
  stdout_serial_port = SERIAL_PORT_TWO;
#endif

  Putdata(&txdata);            /* DO NOT CHANGE! */

//  ***  IFI Code Starts Here***
//
//  Serial_Driver_Initialize();
//
//  printf("IFI 2006 User Processor Initialized ...\r");  /* Optional - Print initialization message. */

  User_Proc_Is_Ready();         /* DO NOT CHANGE! - last line of User_Initialization */
}

/*******************************************************************************
* FUNCTION NAME: Process_Data_From_Master_uP
* PURPOSE:       Executes every 26.2ms when it gets new data from the master 
*                microprocessor.
* CALLED FROM:   main.c
* ARGUMENTS:     none
* RETURNS:       void
*******************************************************************************/
void Process_Data_From_Master_uP(void)
{
	Getdata(&rxdata);

	if (++gLoopCount>(int)(1000/26.2))
		gLoopCount=0;
	
	//printf("getPanOffsetFromOI: %d\r", (int) getPanOffsetFromOI());
	//getPanOffsetFromOI();
	
	AllStop();	//set all pwm's to neutral (except camera - dg)
//	mPump = !mSwPressure;	//pump removed
	UpdateTimers();
	DiagnosticSystemLoad();
	doCamera();

	readBallLauncherSpeed();	//read speed here; might override in Process_Data_Shell()

	Process_Data_Shell();
	
	ControlBallLauncherSpeed();	

	mLEDBallInNo2Pos = mBallin2nd;
//	mLEDBallInNo2Pos = 1;


	SetGearshiftRelays();
	Putdata(&txdata);

}
//******************************************************************************
void doCamera(void)
{	
	// send diagnostic information to the terminal
	Tracking_Info_Terminal();

	// This function is responsable for camera initialization 
	// and camera serial data interpretation. Once the camera
	// is initialized and starts sending tracking data, this 
	// function will continuously update the global T_Packet_Data 
	// structure with the received tracking information.
	Camera_Handler();

	// This function reads data placed in the T_Packet_Data
	// structure by the Camera_Handler() function and if new
	// tracking data is available, attempts to keep the center
	// of the tracked object in the center of the camera's
	// image using two servos that drive a pan/tilt platform.
	// If the camera doesn't have the object within it's field 
	// of view, this function will execute a search algorithm 
	// in an attempt to find the object.
	Servo_Track();
}

//******************************************************************************
// SystemLoad monitors free cyles where no new data is available.
//	If free cycles drop to zero, we are overburdening the CPU
//for reference:
//typedef struct {
//	unsigned int idleCycles;	//counted in main()
//	
//	//data handled in userRoutines
//	unsigned int minIdleCycles;
//	unsigned long cumulativeIdleCyles;
//	unsigned char nLoops;	//time over which data is accumulated
//} CPULoadData;

void DiagnosticSystemLoad(void)
{
	//accumulate new data
	gCPULoad.nLoops++;
	gCPULoad.cumulativeIdleCyles += gCPULoad.idleCycles;
	if (gCPULoad.idleCycles < gCPULoad.minIdleCycles)	//look for the current minimum
		gCPULoad.minIdleCycles = gCPULoad.idleCycles;

	//print and then reset accumulated data
	if (gLoop.onSecond)
	{
		overlay int average=-1;
		if (gCPULoad.nLoops != 0)	//should never be 0, but just in case...
			average = gCPULoad.cumulativeIdleCyles/gCPULoad.nLoops;
			
		printf("CPU Idle Cycles - Avg/Min= %6d / %6d\r",
			(int) gCPULoad.idleCycles, (int) average);
		
		gCPULoad.cumulativeIdleCyles = gCPULoad.nLoops = 0;	//reset data
		gCPULoad.minIdleCycles = 0xFFFFFFFF;	//max unsigned int
	}
}



//******************************************************************************
/*******************************************************************************
* FUNCTION NAME: Process_Data_Shell
* CALLED FROM:   this file, Process_Data_From_Master_uP routine
* ARGUMENTS:     none
* RETURNS:       void
*******************************************************************************/
void Process_Data_Shell(void) //Dennis is better than IFI
{
	processEncoders();
	
	if (disabled_mode)
	{
		if (gLoop.onSecond) printf("Disabled\r\r");
		AutonomousReset();
		AutonomousInitialize();	//this is not the place
	}
	if (autonomous_mode)
		AutonomousRun();
	else
		controls();
}
//******************************************************************************

void processEncoders (void)
{
	//int encoder1Count;
	long encoder_width;
	
	encoder_width = get_encoder_1_width();	
	if (gLoop.onSecond)
	{
		//encoder1Count = (int) Get_Encoder_1_Count();
		//printf("Encoder 1 %d\r", encoder1Count);
		
	}

	
}




//static void AutomationRoutines(void)
//{
//	//Abort automation routines
//	if (!disabled_mode)
//	{
//		switch (automationState.activeRoutine)
//		{
//			case kRoutineAutonomous:
//				printf("AUTO Abort\n");
//				AutonomousAbort();
//				if (disabled_mode)	//competition mode is 'disable' switch
//					AutonomousReset();	//allow another run after disable released.
//				break;
//			//other automated routines here
//			default:
//				break;
//		}
//		automationState.activeRoutine = kNoRoutine;
//		automationState.returnValue = kResultNotRunning;
//		return;
//	}
//}
//
