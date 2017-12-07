/********************************************************************************
* FILE NAME: OIFeedback.c 
*
* DESCRIPTION: 
* Routines to set LED's and user data working on copy of data
********************************************************************************/

#include "ifi_aliases.h"
#include "ifi_default.h"
#include "OIFeedback.h"
#include "lrtMotorDrive.h"

//from ifi_aliases.h: #define User_Mode_byte txdata.LED_byte1.data



OI_LED_User_bytes gOIFeedback ={0,0,0};
/********************************************************************************/
void UpdateOIFeedbackData(void)
{
	extern OI_LED_User_bytes gOIFeedback;

	if (user_display_mode == 0) /* User Mode is Off */
	{
		txdata.LED_byte1.data = gOIFeedback.LED_byte1.data;
		txdata.LED_byte2.data = gOIFeedback.LED_byte2.data;
	}
	else	//the LED data will be displayed as the 'user byte'
	{
		//from ifi_aliases.h: #define User_Mode_byte txdata.LED_byte1.data
		txdata.LED_byte1.data = gOIFeedback.user_byte;
		txdata.LED_byte2.data =0;	//maybe could do something here.
	}
}
/********************************************************************************/
void ClearOIFeedbackData(void)
{
	gOIFeedback.LED_byte1.data = gOIFeedback.LED_byte2.data = gOIFeedback.user_byte=0;
}

/***********
* OIShowPhaseWithLEDs(char phase)
 *  displays value of 'phase' from 0-7 with a display of 0-7 Green LEDs
 * on the Operator Interface.
 * Does not affect red LEDs
 ************/
void OIShowPhaseWithLEDs(char phase)
{
	//use the pwm green LED's as a phase indicator
	const static unsigned char LEDpwmRelay[8] = {
		0b00000000,
		0b00000001, //1 green led
		0b00000101, //2 green leds, skipping red leds
		0b00100101,
		0b10100101,	// 4 green leds
		0b10100101,
		0b10100101,
		0b10100101};
	const static unsigned char LEDswitch[8] = {
		0,0,0,0,0b000,0b001,0b011,0b111}; //0-3 remaining green leds
	
	if (0!=user_display_mode) return;	//don't waste time; won't be displayed
	if (phase < 0 || phase >= 8) return;	//error - do nothing
	
	//preserve red LED's with some bitwise operations
	gOIFeedback.LED_byte1.data &= ~0b10100101;	//clear bits to zero
	gOIFeedback.LED_byte1.data |= LEDpwmRelay[phase]; //set bits
	
	gOIFeedback.LED_byte2.data &= ~0b111;	//clear bits to zero
	gOIFeedback.LED_byte2.data |= LEDswitch[phase]; //set bits
}

void signalErrorOnOI(void)
{
	//should flash some lights for a few seconds!!


}
void OISignalDriveLimit(void)
{
	if (gDriveLimitedFlag)
	{
		Pwm1_redLED=1;
		Pwm2_redLED=1;
	}
}
