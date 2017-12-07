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
#include "interruptHandlers.h"
#include "MotorDrive.h"
#include "connections.h"
#include "controls.h"
#include "utilities.h"
#include "eeprom.h"
#include "LED.h"

#include "simulator.h"
#include <limits.h>

//gControl gCtrl;
CPULoadData gCPULoad={0,UINT_MAX,0,0};
encoder gEncoderRight={'R',0,0,ULONG_MAX/2}, gEncoderLeft={'L',0,0,ULONG_MAX/2};

options gOption={
	0,			//all printing
	1,1,1,1,	//A-D
	0	//stats
};
char gCameraPrinting=0;	//turn camera tracking messages on/off
union OI_LEDs gLED;

static void doCamera(void);
static void	DiagnosticSystemLoad(void);
static void ReadTerminalForOptions(void);
static void doPrintOptions(void);
static void ControlPrinting(void);
static void ReadPrintOptionsFromEPROM(void);
void printRelays(void);


void CheckForSerialPortErrors(void);
#ifdef Robot2005
	void ReconfigureFor2005Outputs(void);
#endif //Robot2005



//***********************************************************************************************
void User_Initialization (void)
{
  
#ifdef _SIMULATOR
		extern void testArea(void);
		unsigned long now;
		testArea();
#endif	

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
	
	Initialize_Interrupts();
	
	Initialize_timer();

#ifdef _SIMULATOR
	now = GetTime();
#endif 

#ifdef TERMINAL_SERIAL_PORT_1    
	stdout_serial_port = SERIAL_PORT_ONE;
#endif

#ifdef TERMINAL_SERIAL_PORT_2    
	stdout_serial_port = SERIAL_PORT_TWO;
#endif


#ifndef __18F8722
	printf("\r\r\r\r\r\r\r\r\r\rWARNING:  Not Compiled for PIC18F8722\r\r");
#endif

	ReadUserOptionsFromEPROM();	//drive train options
	ReadPrintOptionsFromEPROM();
	ReadAutonomousDistanceEPROM();

	Putdata(&txdata);            /* DO NOT CHANGE! */

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

	//clear LEDs; UserByte will alter these, since they are union'ed data.
	ClearLEDs();
	gUserByteOI=0;	
	
	UpdateTimers();
	ControlPrinting();	//after UpdateTimers, call ASAP

	GetDriveEncoderCounts(&gEncoderLeft);	//do this early for consistent readings
	GetDriveEncoderCounts(&gEncoderRight);	
	gRobotTurnSpeed = GetTurnRate();
	
	
	_asm
		nop
		nop
	_endasm

	ReadTerminalForOptions(); //Get Print options

	
	if (gLoop.onSecondA)	//new loop divider on output
		printf("\r\r*	*	*	*	*	*	*	*	*	*	*	*\r");	

	//printf("getPanOffsetFromOI: %d\r", (int) getPanOffsetFromOI());
	//getPanOffsetFromOI();
	
	AllStop();	//set all pwm's to neutral (except camera - dg)
	ClearMotorPWMs();	//sets the global motor pwm's to zero

	if (gLoop.onSecond)	//check pressure once/second (eliminates farts)
		mPump = !mSwPressure;	

	DiagnosticSystemLoad();
	CheckForSerialPortErrors();
	if (gLoop.onSecondC)
		PrintDigitalInputs();

	doCamera();


	Process_Data_Shell();
	
	gLED.BallInNo2Pos = mBallin2nd;
//	gLED.BallInNo2Pos = 1;

	DriveMotors();
	SetGearshiftRelays();	//may override motor pwms; should be after DriveMotors

#ifdef Robot2005
	ReconfigureFor2005Outputs();
#elif !defined(Robot2006)
#error unknownRobot.
#endif
	if (gLoop.onSecondA) printRelays();

	DisplayUserOptions();	//via the user_display_mode byte on the OI

	Putdata(&txdata);
}

//******************************************************************************
#ifdef Robot2005
#error
void ReconfigureFor2005Outputs()
{
	unsigned char *p;
	unsigned char left=mPWMLeftCIM, right=mPWMRightCIM;

	for (p=&pwm01; p<=&pwm16; p++)	//clear all pwms
		*p=127u;

	//copy 2006 pwm's to 2005pwm's.
	mPWMcimRight = mPWMfpriceRight = right;
	mPWMcimLeft = mPWMfpriceLeft = left;

	//turn off all relays
	relay1_fwd = relay1_rev = relay2_fwd = relay2_rev = relay3_fwd = relay3_rev = 0;
	relay4_fwd = relay4_rev = relay5_fwd = relay5_rev = relay6_fwd = relay6_rev = 0;
	relay7_fwd = relay7_rev = relay8_fwd = relay8_rev=0;
}
#endif //Robot2005


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
	static unsigned int last8IdleCycleCounts[8] ={0,0,0,0};	
//accumulate new data
	gCPULoad.nLoops++;
	gCPULoad.cumulativeIdleCyles += gCPULoad.idleCycles;
	if (gCPULoad.idleCycles < gCPULoad.minIdleCycles)	//look for the current minimum
		gCPULoad.minIdleCycles = gCPULoad.idleCycles;

	//save prior counts to be printed on 'LAST' loop.
	last8IdleCycleCounts[0x7 & gLoop.count] = gCPULoad.idleCycles;
	if (gLoop.onSecondLAST)
	{
		int i;
		puts("LastFourIdleCnts:");
		for (i=1;i<=8;i++)
			printf(" %6d", last8IdleCycleCounts[0x7 & (i+gLoop.count)]);
		puts("\r");
	}


	//print and then reset accumulated data
	if (gLoop.onSecondLAST)
	{
		overlay int average=-1;
		if (gCPULoad.nLoops != 0)	//should never be 0, but just in case...
			average = gCPULoad.cumulativeIdleCyles/gCPULoad.nLoops;
	
		printf("CPU Idle Cycles - Avg/Min= %6d / %6d\r",
			(int) average, (int)gCPULoad.minIdleCycles);
		
		gCPULoad.cumulativeIdleCyles = gCPULoad.nLoops = 0;	//reset data
		gCPULoad.minIdleCycles = UINT_MAX;
	}
}



//******************************************************************************
/*******************************************************************************
* FUNCTION NAME: Process_Data_Shell
* CALLED FROM:   this file, Process_Data_From_Master_uP routine
*******************************************************************************/
void Process_Data_Shell(void)
{	
	SetUserOptions();
	SetAutonomousDistance();


	if (disabled_mode)
	{
		if (gLoop.onSecond) printf("Disabled\r\r");
		AutonomousSaveOIControls();
		AutonomousReset();
		AutonomousInitialize();	//this is not the place
	}
	
	
	if (autonomous_mode)
	{
		AutonomousRun();
		gBallLauncherSpeed = GetBallLauncherSpeedFromCamera();
		doLifts();

	}
	else
	{
		controls();	//also sets gBallLauncherSpeed
		doLifts();
	}


	
	ControlLaunchWheelSpeeds(gBallLauncherSpeed);

//	if (gLoop.onSecond) printf("gBallLauncherSpeed=%d\r",gBallLauncherSpeed);

}


//**********************************************************************************
void Process_Data_From_Local_IO(void)
{
	//Whatcha lookin at.  Ain't doin' nothin here.
}
//**********************************************************************************
static void ReadPrintOptionsFromEPROM(void)
{
	*(char *) &gOption = EEPROM_read( kEPROMAdr_PrintOptions );
	gCameraPrinting =	EEPROM_read( kEPROMAdr_CameraPrint );
	gOption.print=0;	//no printing on reset.
}
//**********************************************************************************

void ReadTerminalForOptions(void)
{
	char input_chr, input2;
	

	
	while(0!=(input_chr=Read_Serial_Port_One()))
	{
		putc(input_chr,stdout);
		switch (input_chr)
		{
			case 'p':
				doPrintOptions();
				break;
			case 'c':
				gCameraPrinting=!gCameraPrinting;
				EEPROM_write( kEPROMAdr_CameraPrint, gCameraPrinting );
				break;
			case 't':
				gPanAsTilt = !gPanAsTilt;
				if (gPanAsTilt)
					puts("Pan offset = Tilt");
				else
					puts("Pan offset = Normal");
				break;
			case '\r':
				break;
			default:
				puts(" unknown menu key\r");
		}
	}

	//Display what options are being printed.
	if (!gLoop.onSecond) return;
	if (!gOption.print)
	{
		puts("Enter 'p' to print\r");
		return;
	}
	printf("\rpa%d pb%d pc%d pd%d pStats%d Camera%d\r",
			(int)gOption.printOnA,
			(int)gOption.printOnB,
			(int)gOption.printOnC,
			(int)gOption.printOnD,
			(int)gOption.printOnS,
			(int)gCameraPrinting);
}


//******************************************************************
//handle print options by supresssing 'onSecondX' after they are set by UpdateTimers()
void ControlPrinting(void)
{
	if (!gOption.print || !gOption.printOnA) gLoop.onSecondA=0;
	if (!gOption.print || !gOption.printOnB) gLoop.onSecondB=0;
	if (!gOption.print || !gOption.printOnC) gLoop.onSecondC=0;
	if (!gOption.print || !gOption.printOnD) gLoop.onSecondD=0;
	if (!gOption.print || !gOption.printOnS) gLoop.onSecondLAST=0;	//stats only
}
//******************************************************************
void doPrintOptions(void)
{
	char input_chr = input_chr=Read_Serial_Port_One();
	char dirty =1;	//assume option changed
	switch (input_chr)
	{
		case 0:
		case '\r':
			gOption.print=!gOption.print;
			break;
		case 'a':
			gOption.printOnA=!gOption.printOnA;
			break;
		case 'b':
			gOption.printOnB=!gOption.printOnB;
			break;
		case 'c':
			gOption.printOnC=!gOption.printOnC;
			break;
		case 'd':
			gOption.printOnD=!gOption.printOnD;
			break;
		case 's':	//stats
			gOption.printOnS=!gOption.printOnS;
			break;
		default:
			dirty = 0;
			puts("unknown option\r");
		}
	
		if (dirty)
			 EEPROM_write( kEPROMAdr_PrintOptions, 	*(char *) &gOption );

}
//******************************************************************


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
//********************************************************************************
// if we detect a framing or overrun error, then blink LED3 for 6 seconds
void CheckForSerialPortErrors(void)
{
	static char ledtimer=0;
#define kLEDBlinkTime 10
	
	gLED.RxTxError=0;
	if (ledtimer) gLED.RxTxError=gLoop.count&0x10;	//blink on/off 16 cycles each (1/2 sec)

	if (gLoop.count!=0)	//only execute once per second
		return;

	//executing once/second from here on down
	if (ledtimer) ledtimer--;
	
	if (RX_1_Framing_Errors)
	{
		printf("%d Rx1 Framing_Errors");
		RX_1_Framing_Errors=0;
		ledtimer=kLEDBlinkTime;
	}
	if (RX_2_Framing_Errors)
	{
		printf("%d Rx2 Framing_Errors");
		RX_2_Framing_Errors=0;
		ledtimer=kLEDBlinkTime;
	}
	if (RX_1_Overrun_Errors)
	{
		printf("%d Rx1 Overrun Errors");
		RX_1_Overrun_Errors=0;
		ledtimer=kLEDBlinkTime;
	}
	if (RX_2_Overrun_Errors)
	{
		printf("%d Rx2 Overrun Errors");
		RX_2_Overrun_Errors=0;
		ledtimer=kLEDBlinkTime;
	}
}
//********************************************************************************
void printRelays(void)
{
	printf("relay %d%d %d%d %d%d\r",
		(int)relay1_fwd,
		(int)relay1_rev,
		(int)relay2_fwd,
		(int)relay2_rev,
		(int)relay3_fwd,
		(int)relay3_rev
	);
}
