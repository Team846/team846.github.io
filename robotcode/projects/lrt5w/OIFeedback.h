/********************************************************************************
* FILE NAME: 
*
* DESCRIPTION: 
* Routines to control LED's and User_Mode_byte on interface

* the User_Mode_byte is shared with LED_byte1
* The LED is displayed if (user_display_mode == 0) -- User Mode is Off 
* otherwise the User_Mode_Byte is displayed.
* The User_Mode_byte is displayed if 
*
* Becareful to include this file so that the 
* LED macros in ifi_aliaes.h are replaced with the macros listed here!
********************************************************************************/


#ifndef __OIFeedback_h
#define __OIFeedback_h

#include "ifi_default.h"
#include "ifi_aliases.h"

//from ifi_aliases.h: #define User_Mode_byte txdata.LED_byte1.data

typedef struct {
	union							//copy of feedback data in txdata record
	{ 
		 bitid bitselect;          /*txdata.LED_byte1.bitselect.bit0*/
		 unsigned char data;       /*txdata.LED_byte1.data*/
	} LED_byte1,LED_byte2;
	unsigned char user_byte;

	unsigned char cycles;	//set to nonzero if there is a blinking error [dg]
} OI_LED_User_bytes;

extern OI_LED_User_bytes gOIFeedback;

void UpdateOIFeedbackData(void);
void ClearOIFeedbackData(void);
void OIShowPhaseWithLEDs(char phase);
void signalErrorOnOI(void);
void OISignalDriveLimit(void);



//Undo OI aliases in ifi_aliases.h
#undef Pwm1_green
#undef Pwm1_red
#undef Pwm2_green
#undef Pwm2_red
#undef Relay1_red
#undef Relay1_green
#undef Relay2_red 
#undef Relay2_green

#undef Switch1_LED
#undef Switch2_LED
#undef Switch3_LED
#undef User_Mode_byte

//Redo the OI aliases
#define Pwm1_greenLED      gOIFeedback.LED_byte1.bitselect.bit0
#define Pwm1_redLED        gOIFeedback.LED_byte1.bitselect.bit1 
#define Pwm2_greenLED      gOIFeedback.LED_byte1.bitselect.bit2
#define Pwm2_redLED        gOIFeedback.LED_byte1.bitselect.bit3 
#define Relay1_redLED      gOIFeedback.LED_byte1.bitselect.bit4 
#define Relay1_greenLED    gOIFeedback.LED_byte1.bitselect.bit5
#define Relay2_redLED      gOIFeedback.LED_byte1.bitselect.bit6 
#define Relay2_greenLED    gOIFeedback.LED_byte1.bitselect.bit7

#define Switch1_LED     gOIFeedback.LED_byte2.bitselect.bit0
#define Switch2_LED     gOIFeedback.LED_byte2.bitselect.bit1
#define Switch3_LED     gOIFeedback.LED_byte2.bitselect.bit2

#define OIUser_Mode_byte	gOIFeedback.LED_byte1.data
#endif	//__OIFeedback_h
