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

#include "common.h"

//gControl gCtrl;
CPULoadData gCPULoad={0,UINT_MAX,0,0};
encoder gEncoderRight={'R',0,0,ULONG_MAX/2}, gEncoderLeft={'L',0,0,ULONG_MAX/2};
SLEDData gSerialLED;

union OI_LEDs gLED;

void doCamera(void);
static void	DiagnosticSystemLoad(void);

void GetProximitySensors(void);

struct proxData gProx = {0,0,0};

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
	digital_io_09 = INPUT;
	digital_io_10 = digital_io_11 = digital_io_12 = OUTPUT;		// LED board output
	digital_io_13 = OUTPUT;							// electronic brakes
	digital_io_14 = digital_io_15 = digital_io_16 = INPUT;
	digital_io_17 = digital_io_18 = INPUT;
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

//clear the serialLEDs; make sure gSerialLED is 2 bytes	
	*(int *) &gSerialLED = 0;


#ifdef _SIMULATOR
	now = GetTime();
#endif 

#define TERMINAL_SERIAL_PORT_1	//[dcl]
#ifdef TERMINAL_SERIAL_PORT_1    
	stdout_serial_port = SERIAL_PORT_ONE;
#endif

#ifdef TERMINAL_SERIAL_PORT_2    
	stdout_serial_port = SERIAL_PORT_TWO;
#endif


#ifndef __18F8722
	printf("\r\r\r\r\r\r\r\r\r\rWARNING:  Not Compiled for PIC18F8722\r\r");
#endif

	ReadUserOptionsFromEPROM();
	ReadPrintOptionsFromEPROM();
	ReadAutonomousEPROM();
#ifdef ROBOT_2007
	ReadLiftPresetsFromEPROM();
#endif

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

	ReadTerminalForOptions(); //Get Print options

	if (gLoop.onSecondA)	//new loop divider on output
		printf("\r\r*	*	*	*	*	*	*	*	*	*	*	*\r");	

	AllStop();	//set all pwm's to neutral
	ClearMotorPWMs();	//sets the global motor pwm's to zero
	mCoast = 1;			// unset electronic brake

	if (gLoop.onSecond)	//check pressure once/second (eliminates farts)
		mPump = !mSwPressure;

	DiagnosticSystemLoad();
	CheckForSerialPortErrors();
	if (gLoop.onSecondC)
		PrintDigitalInputs();
	if (gLoop.onSecondC) {
		int left, right;
		left = Get_Analog_Value(mProximityLeftPort);
		right = Get_Analog_Value(mProximityRightPort);
		printf("ProxL: %d, ProxR: %d\r\n", left, right);
	}
		

//	doCamera();

	Process_Data_Shell();

	doAction();		// Call doAction before doLift, doGripper, and DriveMotors so we can power the relays.
#ifdef ROBOT_2007
	doLift();
	doGripper();
#endif
	DriveMotors();

	if (gLoop.onSecondA) printRelays();

#if defined(ROBOT_2007) && defined(SERIAL_LED)
	mSLEDLiftLimitLow = mSwLimitLiftLow ? 1 : 0;
	mSLEDLiftLimitHigh = mSwLimitLiftHigh ? 1 : 0;
//	mSLEDGripperTrap = mSwGripperTrap ? 1 : 0;
#endif
	
//if (gLoop.onSecondA) printf("p3_x: %d; p3_y: %d\r\n", (int) p3_x, (int) p3_y);

	DisplayUserOptions();	//via the user_display_mode byte on the OI
	
#ifdef SERIAL_LED
	SendLEDSerial(*(int *) &gSerialLED);
//	SendLEDSerial((unsigned int) -1);
	//SendLEDSerial((unsigned int) gLoop.count);
	//SendLEDSerial((unsigned int) p1_wheel);
	//SendLEDSerial((unsigned int)0x55);
#endif

	Putdata(&txdata);
}


//******************************************************************************
/*
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
*/

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
	SetAutonomousOptions();
	GetProximitySensors();

	if (disabled_mode)
	{
		if (gLoop.onSecond) printf("Disabled\r\r");
//		AutonomousSaveOIControls();
		AutonomousReset();
		AutonomousInitialize();	//this is not the place
	}
	else if (autonomous_mode)
	{
		AutonomousRun();
	}
	else
	{
		controls();
	}
}


//**********************************************************************************
void Process_Data_From_Local_IO(void)
{
	//Whatcha lookin at.  Ain't doin' nothin here.
}
//**********************************************************************************

#define mMilliVolts1024(mV) (int)(1023L*(mV)/5000)
#define kNear mMilliVolts1024(200)
#define kFar  mMilliVolts1024(1000)

//**********************************************************************************
void GetProximitySensors(void)
{
	gProx.left = Get_Analog_Value(mProximityLeftPort);
	gProx.right = Get_Analog_Value(mProximityRightPort);
	gProx.bumper = Get_Analog_Value(mProximityBumperPort);
	
#ifdef ROBOT_2007
	if (getLiftPos() < 250) {		// if lift could be covering sensor, don't give values
		gProx.left = 0;
		gProx.right = 0;
	}
#endif //ROBOT_2007

	//Use recent inputs to scale sensitivity
	{
		if (gProx.left > gProx.maxRecentLeft)
			gProx.maxRecentLeft = gProx.left;	
		else
			gProx.maxRecentLeft = (gProx.maxRecentLeft*63) >> 6;	//decay
			
		if (gProx.right > gProx.maxRecentRight)
			gProx.maxRecentRight = gProx.right;	
		else
			gProx.maxRecentRight = (gProx.maxRecentRight*63) >> 6;	//decay
			
		if (gProx.bumper > gProx.maxRecentBumper)
			gProx.maxRecentBumper = gProx.bumper;	
		else
			gProx.maxRecentBumper = (gProx.maxRecentBumper*61) >> 6;	//decay - OVER 21 cycles
	}
	
	{	// OI LED feedback
		gLED.LeftPx1 = (gProx.maxRecentLeft > 20); // sense threshold
		gLED.LeftPx2 = (gProx.maxRecentLeft > 100);
		gLED.LeftPx3 = (gProx.maxRecentLeft > 200); // score threshold
		gLED.LeftPx4 = (gProx.maxRecentLeft > 250);
		gLED.RightPx1 = (gProx.maxRecentRight > 20); // sense
		gLED.RightPx2 = (gProx.maxRecentRight > 100);
		gLED.RightPx3 = (gProx.maxRecentRight > 200); // score
		gLED.RightPx4 = (gProx.maxRecentRight > 250);
		gLED.BumperPx1 = (gProx.maxRecentBumper > 20);
		gLED.BumperPx2 = (gProx.maxRecentBumper > 50); // sense threshold
		gLED.BumperPx3 = (gProx.maxRecentBumper > 155); // score threshold
	}

#ifdef SERIAL_LED
	mSLEDProximityLeft0=mSLEDProximityLeft1=0;
	if (gProx.left > kNear) mSLEDProximityLeft0=1;
	if (gProx.left > kFar) mSLEDProximityLeft1=1;
	
	mSLEDProximityRight0=mSLEDProximityRight1=0;
	if (gProx.right > kNear) mSLEDProximityRight0=1;
	if (gProx.right > kFar) mSLEDProximityRight1=1;
#endif //SERIAL_LED

	if (gLoop.onSecondA)
		printf("Proximity: L %d; R %d; Bumper %d\r\n", gProx.left, gProx.right, gProx.bumper);
}	
