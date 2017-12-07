// debug.c: Serial debug/diagnostics

#include "common.h"
#include "debug.h"

void ReadTerminalForOptions(void);
void doPrintOptions(void);
void ControlPrinting(void);
void ReadPrintOptionsFromEPROM(void);
void printRelays(void);
void CheckForSerialPortErrors(void);
void Tracking_Info_Terminal(void);

printOptions gPrintOptions = {
	0,			//all printing
	1,1,1,1,	//A-D
	0,			//camera
	0,			//stats
};

char gFakeDisableSw=1;
char gFakeAutonomousSw=0;

//********************************************************************************
// if we detect a framing or overrun error, then blink LED3 for 6 seconds
void CheckForSerialPortErrors(void)
{
	static char ledtimer=0;
#define kLEDBlinkTime 10
	
//	gLED.RxTxError=0;
//	if (ledtimer) gLED.RxTxError=gLoop.count&0x10;	//blink on/off 16 cycles each (1/2 sec)

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



void ReadPrintOptionsFromEPROM(void)
{
	*(char *) &gPrintOptions = EEPROM_read( kEPROMAdr_PrintOptions );
	gPrintOptions.printCamera =	EEPROM_read( kEPROMAdr_CameraPrint );
	gPrintOptions.print=0;	//no printing on reset.
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
				gPrintOptions.printCamera = !gPrintOptions.printCamera;
				EEPROM_write( kEPROMAdr_CameraPrint, gPrintOptions.printCamera );
				break;

			case 'D':
				gFakeDisableSw=1;
				break;
			case 'd':
				gFakeDisableSw=0;
				break;
			case 'A':
				gFakeAutonomousSw=1;
				break;
			case 'a':
				gFakeAutonomousSw=0;
				break;
				
			case 'r':	// Reset EEPROM lift values
				#ifdef ROBOT_2007
				{
					int positions[4] = {LIFT_L0, LIFT_L1, LIFT_L2, LIFT_L3};
					
					EEPROM_write(kEPROMAdr_LiftL0, ((char*) &positions[0])[0]);		//save across resets
					EEPROM_write(kEPROMAdr_LiftL0+1, ((char*) &positions[0])[1]);	//save across resets
					printf("Reset Level 0 to %d\r\n", positions[0]);
					
					EEPROM_write(kEPROMAdr_LiftL1, ((char*) &positions[1])[0]);		//save across resets
					EEPROM_write(kEPROMAdr_LiftL1+1, ((char*) &positions[1])[1]);	//save across resets
					printf("Reset Level 1 to %d\r\n", positions[1]);
					
					EEPROM_write(kEPROMAdr_LiftL2, ((char*) &positions[2])[0]);		//save across resets
					EEPROM_write(kEPROMAdr_LiftL2+1, ((char*) &positions[2])[1]);	//save across resets
					printf("Reset Level 2 to %d\r\n", positions[2]);
					
					EEPROM_write(kEPROMAdr_LiftL3, ((char*) &positions[3])[0]);		//save across resets
					EEPROM_write(kEPROMAdr_LiftL3+1, ((char*) &positions[3])[1]);	//save across resets
					printf("Reset Level 3 to %d\r\n", positions[3]);
				}
				#endif //ROBOT_2007
				break;
				
			case '\r':
				break;
			default:
				puts(" unknown menu key\r");
		}
	}

	//Display what options are being printed.
	if (!gLoop.onSecond) return;
	if (!gPrintOptions.print)
	{
		puts("Enter 'p' to print\r");
		return;
	}
	printf("\rpa%d pb%d pc%d pd%d pStats%d Camera%d\r",
			(int)gPrintOptions.printOnA,
			(int)gPrintOptions.printOnB,
			(int)gPrintOptions.printOnC,
			(int)gPrintOptions.printOnD,
			(int)gPrintOptions.printOnS,
			(int)gPrintOptions.printCamera);
}


//******************************************************************
//handle print options by supresssing 'onSecondX' after they are set by UpdateTimers()
void ControlPrinting(void)
{
	if (!gPrintOptions.print || !gPrintOptions.printOnA) gLoop.onSecondA=0;
	if (!gPrintOptions.print || !gPrintOptions.printOnB) gLoop.onSecondB=0;
	if (!gPrintOptions.print || !gPrintOptions.printOnC) gLoop.onSecondC=0;
	if (!gPrintOptions.print || !gPrintOptions.printOnD) gLoop.onSecondD=0;
	if (!gPrintOptions.print || !gPrintOptions.printOnS) gLoop.onSecondLAST=0;	//stats only

	gLoop.onSecondCamera=0;
	// Sets onSecondCamera if onSecond and camera enabled
	if (gLoop.onSecond && gPrintOptions.print && gPrintOptions.printCamera)
		gLoop.onSecondCamera=1;
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
			gPrintOptions.print=!gPrintOptions.print;
			break;
		case 'a':
			gPrintOptions.printOnA=!gPrintOptions.printOnA;
			break;
		case 'b':
			gPrintOptions.printOnB=!gPrintOptions.printOnB;
			break;
		case 'c':
			gPrintOptions.printOnC=!gPrintOptions.printOnC;
			break;
		case 'd':
			gPrintOptions.printOnD=!gPrintOptions.printOnD;
			break;
		case 's':	//stats
			gPrintOptions.printOnS=!gPrintOptions.printOnS;
			break;
		default:
			dirty = 0;
			puts("unknown option\r");
		}
	
		if (dirty)
			 EEPROM_write( kEPROMAdr_PrintOptions, 	*(char *) &gPrintOptions );

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
	
#ifdef SERIAL_LED
//	mSLEDGripperArmFwd = relay1_fwd;
//	mSLEDGripperArmRev = relay1_rev;
//	mSLEDGripperGripFwd = relay2_fwd;
//	mSLEDGripperGripRev = relay2_rev;
#endif
}


#if 0
/*******************************************************************************
*
*	FUNCTION:		Tracking_Info_Terminal()
*
*	PURPOSE:		This function is designed to send tracking information
*					to a terminal attached to your robot controller's 
*					programming port.
*
*	CALLED FROM:	user_routines.c/Process_Data_From_Master_uP()
*
*	PARAMETERS:		none
*
*	RETURNS:		nothing
*
*	COMMENTS:		This code assumes that the terminal serial port is 
*					properly set in camera.h
*
*******************************************************************************/
void Tracking_Info_Terminal(void)
{
	static unsigned char count = 0;
	static unsigned int old_camera_t_packets = 0;
	//static unsigned char pkt_cnt = 1;

	// has a new camera tracking packet arrived since we last checked?
	if(camera_t_packets != old_camera_t_packets)
	{
		count++;
		//printf("#26.2ms Loops %d\r",(int)pkt_cnt);
		//pkt_cnt=1;
		// only show data on every five t-packets
		if(count >= 5)
		{
			// reset the counter
			count = 0;
			
			#ifdef _TRACK_TILT
				if (gPrintOptions.printCamera) printf("Tracking Tilt: ");
			#else
				if (gPrintOptions.printCamera) printf("Static Tilt: ");
			#endif
			
			// does the camera have a tracking solution?
			if(T_Packet_Data.my == 0)
			{
				if (gPrintOptions.printCamera)
					printf("Searching...");
			}
			else
			{
			//	printf("\r\n");

				// pan angle = ((current pan PWM) - (pan center PWM)) * degrees/pan PWM step
				//printf(" Pan Angle (degrees) = %d\r\n", (((int)PAN_SERVO - 124) * 65)/124);

				// tilt angle = ((current tilt PWM) - (tilt center PWM)) * degrees/tilt PWM step
				//printf("Tilt Angle (degrees) = %d\r\n", (((int)TILT_SERVO - 144) * 25)/50);
//			if (gLoop.onSecondA )
//				printf(" Pan Error (Pixels)  = %d\r\n", (int)T_Packet_Data.mx - PAN_TARGET_PIXEL_DEFAULT);
//				printf("Tilt Error (Pixels)  = %d\r\n", (int)T_Packet_Data.my - TILT_TARGET_PIXEL_DEFAULT);
				//printf(" Blob Size (Pixels)  = %u\r\n", (unsigned int)T_Packet_Data.pixels);
				//printf("Confidence (Pixels)  = %u\r\n", (unsigned int)T_Packet_Data.confidence);
			}
			
			if (gPrintOptions.printCamera) printf("\r\n");
		}
	}else {
		//pkt_cnt++;
	}
}
#endif //0
