#include "common.h"
#include "adc.h"
#include "LCD.h"
#include "hybrid.h"
#include "Compass.h"
#include "pwm.h"
#include "_ifi_aliases.h"
#include "lift.h"
#include <string.h>
#include "ultrasonic.h"

static void DiagnosticSystemLoad(void);
static void CheckZeroIdleCycles(void);
static void saveSelectOIValuesForAutonomous(void);
void LCD_Print_Gyro_Angle(char lcd_line_no);

#ifndef __LARGE__
#error Not Compiled with -ml Large (>64K) data option
#endif

CPULoadData gCPULoad =
	{ -1, 0, UINT_MAX, 0, 0 };

//***********************************************************************************************

/*******************************************************************************
 * FUNCTION NAME: Process_Data_From_Master_uP
 * PURPOSE:       Executes every 26.2ms when it gets new data from the master 
 *                microprocessor.
 * CALLED FROM:   main.c
 * ARGUMENTS:     none
 * RETURNS:       void
 *******************************************************************************/
void Process_Data_From_Master_uP(void) {
	void Process_Shell_0(void);
static long elapsedTime;
	gLoop.startTime = GetTime();	//record for possible print suppression features
	Getdata(&rxdata);
	Process_Shell_0();
	Putdata(&txdata);
elapsedTime = GetTime()-gLoop.startTime;
}

//**********************************************************************************
/* Process_Shell_0() contains time sensitive routines
 * Most customized code will be in Process_Shell_1()
 */
void Process_Shell_0(void) {	
	GetDriveEncoderCounts(&gEncoders[LEFT]); //do this early for consistent readings
	GetDriveEncoderCounts(&gEncoders[RIGHT]);
	//No Condition statements before this point to minimize timing errors
	saveSelectOIValuesForAutonomous();

	CheckZeroIdleCycles();	//call before UpdateTimers()
	UpdateTimers();
	
	
	ControlPrinting();
	ReadTerminalForOptions();

	ClearLEDs();
	gUserByteOI=0;

	if (gPrintOptions.printStats) DiagnosticSystemLoad();


	//PrintDigitalInputs();
	checkEncodersWorking();	//call before clearing gMotorSpeed.
	AllStop(); // set all pwm's to stopped
	ClearMotorPWMs();

//	Gyro_Update();
#ifdef R2007
	if (mRolling()) puts("WARNING: Compiled for 2007 Robot\r");
#endif
	//Recommend enabling promotion of chars to ints in calculations. Can't check at compile time.
	//if ((128 != 127+1) && mRolling()) puts("char->int promotion disabled\r");

	if (gLoop.f.printLCD)
	{
		//Any error messages generated before this block will be lost!
		LCD_Blink(LCD_Error, 0);	//clear any blinking on the error line.
		if (autonomous_mode)	//NB: if disabled is false, then autnomous is false
		{
			printfLCD(LCD_Error, "    autonomous");
			LCD_Blink(LCD_Error, 1);
		}
		else if (disabled_mode)
			printfLCD(LCD_Error, "     disabled");
		else
			printfLCD(LCD_Error, "     ENABLED");
		
		if (-1 != gCPULoad.busyCycle)	//Warnings for No Free CPU Cycles
		{
			printfLCD(LCD_Error, "CPU 0%% idle #%d", (int)gCPULoad.busyCycle);
			LCD_Blink(LCD_Error,1);
		}
		
		LCD_DisplayAnyEncoderError();
		printfLCD(LCD_Encoder,"%+6.6ld %+6.6ld",
			gEncoders[LEFT].position,gEncoders[RIGHT].position);
		printfLCD(LCD_Encoder_Corrected,"Bear=%+6ld/%+6ldA",
			encoder_diff_corrected(), encoder_diff_absolute());
		
		printfLCD(LCD_OI_Pots,"pots %3d %3d %3d", (int)OI_USERPOT1, (int)OI_USERPOT2, (int)OI_USERPOT3);
		
//		LCD_Print_Gyro_Angle(LCD_Gyro);
		PrintDigitalInputsLCD(LCD_Digital_IO_on_FRC);
		printfLCD(LCD_End,"     * * * * *");
	}

	doUltraSonic();

	Process_Data_Shell();
	
	Drive_Do();	//processes global motor commands and sets pwm outputs.
	Drive_DoShifting();
	Lift_Do();
	
	//Display the Motors after all motor processing is completed.
	if (gLoop.f.printLCD)
	{
		//NB: %c requires an int.
		char n=printfLCD(LCD_Joy_And_Motors,"%+4.4d %+4.4d %+4.4d%c%+4.4d%c",
				(int)OI_JOYFWD-127,(int)OI_JOYTURN-127,
				gMotorSpeed.left, (int)(gMotorSpeed.brakeLeft  ? 'B':'_'),
				gMotorSpeed.right,(int)(gMotorSpeed.brakeRight ? 'B':'_'));
	}
	
	updateLCD();
	DoOIDisplay();
}

//**********************************************************************************

void Process_Data_Shell(void) {

	static char wasDisabled=0;
//	if (wasDisabled && !disabled_mode) {
//		//LRT_Set_Gyro_Bias();
//		//Tilt_Set_Level_State();
//	}
	wasDisabled = disabled_mode;

	if (disabled_mode) {
		//LRT_Accumulate_Bias();
		//Tilt_Accumulate_Level_State();
		Controls_Init();	//resets target bearing to current bearing.
		if (mRolling())
			printf("Disabled\r\r");

		if (gLoop.disabledCount < 5000/26.2)
			gMotorSpeed.brakeLeft = gMotorSpeed.brakeRight = 1;	//brakes will be set in doDrive()
		SetUserOptions();
		
		// hat test
		mOILEDHat1Green = mOILEDHat2Green = !!(gLoop.count & 0x4);
		mOILEDHat1Red = mOILEDHat2Red     =  !(gLoop.count & 0x4);
		///////

		Brake_Test();

		HybridInit();
	} else {
		if (autonomous_mode)
		{
//			if (!HybridRun()) {
				autonomousRun();
				if (mRolling())
					printf("Auton!\r");
//			} else {
//				if (mRolling())
//					printf("HYBRID OVERRIDE!\r");
//			}
		}
		else
		{
			if (mRolling())
				printf("Controls!\r");
			controls();
		}
	}
}

//**********************************************************************************

void Process_Data_From_Local_IO(void) {
	// Empty.
}

//***********************************************************************************************

void User_Initialization(void) {
#ifdef _SIMULATOR
	extern void testArea(void);
	testArea();
#endif

	Set_Number_of_Analog_Channels(SIXTEEN_ANALOG);    // Taking our own control of ADC using Kevin's code (D. Wachenschwanz 2/10/08)

	digital_io_01 = digital_io_02 = digital_io_03 = digital_io_04 = INPUT;
	digital_io_05 = digital_io_06 = digital_io_07 = digital_io_08 = INPUT;
	digital_io_09 = digital_io_10 = digital_io_11 = digital_io_12 = INPUT;
	digital_io_13 = digital_io_14 = digital_io_15 = digital_io_16 = INPUT;
	digital_io_17 = digital_io_18 = INPUT;

//	ULTRASONIC_LED_PORT = DIAGNOSTIC_LED_PORT = OUTPUT;
	ULTRASONIC_LED_PORT = OUTPUT;
	DIGOUT_COASTL_PORT = DIGOUT_COASTR_PORT = OUTPUT;
	DIGOUT_PRGM_PORT = OUTPUT;
//	DIGOUT_TRIGGER_PORT = OUTPUT;
	DIGOUT_PRGM = 1;

	AllStop(); // set all pwms to neutral

	/* FIFTH: Set your PWM output types for PWM OUTPUTS 13-16.
	 /*   Choose from these parameters for PWM 13-16 respectively:               */
	/*     IFI_PWM  - Standard IFI PWM output generated with Generate_Pwms(...) */
	/*     USER_CCP - User can use PWM pin as digital I/O or CCP pin.           */
	//Setup_PWM_Output_Type(IFI_PWM,IFI_PWM,IFI_PWM,IFI_PWM);
	Setup_PWM_Output_Type(USER_CCP,USER_CCP,USER_CCP,USER_CCP);

	/* Add any other initialization code here. */

	Init_Serial_Port_One();
	stdout_serial_port = NULL;	//Suppress printing

	Init_Serial_Port_Two();

	//Initialize_ADC();

	Initialize_Interrupts();
	Initialize_timer();
	//Initialize_Gyro();
	
	InitLCD();
#ifdef ENABLE_INTERRUPT_5
	Initialize_Compass();
#endif

	Initialize_PWM();
	Lift_Initialize();

	ReadUserOptionsFromEPROM();
	ReadPrintOptionsFromEPROM();
	Hybrid_ReadEEPROM();

	stdout_serial_port = SERIAL_PORT_ONE;	//Enable Printing
	puts("ABCDEF Running");
#ifndef __18F8722
	printf("WARNING:  Not Compiled for PIC18F8722\r\r");
#endif
	Putdata(&txdata); /* DO NOT CHANGE! */
	User_Proc_Is_Ready(); /* DO NOT CHANGE! - last line of User_Initialization */
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
static void DiagnosticSystemLoad(void) {
	static unsigned int last8IdleCycleCounts[8] =
		{ 0, 0, 0, 0, 0, 0, 0, 0 };
	
	if (!gPrintOptions.printStats)	//don't collect stats if we don't print them
			return;
		
		//accumulate new data
	gCPULoad.nLoops++;
	gCPULoad.cumulativeIdleCyles += gCPULoad.idleCycles;
	if (gCPULoad.idleCycles < gCPULoad.minIdleCycles) //look for the current minimum
		gCPULoad.minIdleCycles = gCPULoad.idleCycles;

	//save prior counts to be printed on 'LAST' loop.
	last8IdleCycleCounts[0x7 & gLoop.count] = gCPULoad.idleCycles;
	if (gLoop.f.printStats) {
		int i;
		int average = -1;

		puts("RecentIdleCnts:");
		for (i=1; i<=8; i++)
			printf(" %6d", last8IdleCycleCounts[0x7 & (i+gLoop.count)]);
		puts("\r");

		if (gCPULoad.nLoops != 0) //should never be 0, but just in case...
			average = gCPULoad.cumulativeIdleCyles/gCPULoad.nLoops;

		printf("CPU Idle Cycles - Avg/Min= %6d / %6d\r", (int) average,
		    (int)gCPULoad.minIdleCycles);

		gCPULoad.cumulativeIdleCyles = gCPULoad.nLoops = 0; //reset data
		gCPULoad.minIdleCycles = UINT_MAX;

//		PrintDigitalInputs();
//		printRelays();
	}
}
//************************************************************************
static void CheckZeroIdleCycles(void)
{
	if (gCPULoad.busyCycle == gLoop.count38)
		gCPULoad.busyCycle = -1;	//clear prior secondz report
	
	if (0==gCPULoad.idleCycles)	//record any loops without free cycles
		gCPULoad.busyCycle = gLoop.count38;
	
	if (0==gCPULoad.idleCycles)
		printf("WARNING: CPU 0%% idle on #%d\r", (int)gCPULoad.busyCycle);
}

//************************************************************************
//During autonomous, data from the OI is 'zeroed' or neutral
//So, save select values when !autonomous and restore them when in autonomous.
static void saveSelectOIValuesForAutonomous(void)
{
	static unsigned char p1,p2,p3, btnsA, btnsB;
	
	if (!autonomous_mode)
	{
		p1 = OI_USERPOT1;
		p2 = OI_USERPOT2;
		p3 = OI_USERPOT3;
		
		btnsA = rxdata.oi_swA_byte.allbits;	//save buttons on OI ports 1 & 2
		btnsB = rxdata.oi_swB_byte.allbits;	//save buttons on OI ports 3 & 4
	}
	else //restore select OI pots and buttons
	{
		OI_USERPOT1=p1;
		OI_USERPOT2=p2;
		OI_USERPOT3=p3;
		
		rxdata.oi_swA_byte.allbits = btnsA;	//restore buttons on OI ports 1 & 2
		rxdata.oi_swB_byte.allbits = btnsB;	//restore buttons on OI ports 3 & 4
	}
}
//**********************************************************************
//void LCD_Print_Gyro_Angle(char lcdLineNo)
//{
//	//The reported angle is 10x; This routine inserts a decimal in the display; I.e. 123 -> 12.3	
//	char str[12];
//	char cnt = sprintf(str,"%02d", (int)Get_Gyro_Angle());	//keep a leading zero for small numbers
//	str[cnt+1] = NULL;
//	str[cnt] = str[cnt-1];
//	str[cnt-1] = '.';
//	printfLCD(lcdLineNo,"Gyro " THETA ": %s" DEGREES,str);
//}