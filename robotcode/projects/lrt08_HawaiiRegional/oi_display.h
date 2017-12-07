/********************************************************************************
 * FILE NAME: LED.h <FRC VERSION>
 *
 * DESCRIPTION: 
 *  This file ...
 *
 ********************************************************************************/
#ifndef __OI_DISPLAY_h_
#define __OI_DISPLAY_h_
#include "_ifi_default.h"

/***** LED connections (See OI Layout below) ************************************
 -- shared by 'User_Byte' data --
 Pwm1_red    Pwm1_green, 
 Pwm2_red    Pwm2_green,  
 Relay1_red  Relay1_green
 Relay2_red, Relay2_green
 
 -- independent of 'User_Byte'
 Switch1_LED
 Switch2_LED
 Switch3_LED
 **** End of LED OI Layout *******************************************************/

/***** LED Layout on OI *******************
 1 0		Pwm_Red/Grn
 3 2		Pwm_Red/Grn
 4 5		Relay1 Red/Grn
 6 7		Relay1 Red/Grn
 0		Switch1
 1		Switch2
 2		Switch3
 ******************************************/

union OI_LEDs {
	struct {
		unsigned char byte1;
		unsigned char byte2;
	};
	struct {
		unsigned pwm1_green :1; // is order correct?? [dcl]
		unsigned pwm1_red :1;
		unsigned pwm2_green :1;
		unsigned pwm2_red :1;
		unsigned relay1_red :1;
		unsigned relay1_green :1;
		unsigned relay2_red :1;
		unsigned relay2_green :1;
		//-- second byte
		unsigned switch1 :1;
		unsigned switch2 :1;
		unsigned switch3 :1;
		unsigned null :5; // unused
	};
};
extern union OI_LEDs gOI_LED;

//#define mOILEDIR1 gOI_LED.pwm1_green
//#define mOILEDIR2 gOI_LED.pwm2_green
//#define mOILEDIR3 gOI_LED.relay1_green
//#define mOILEDIR4 gOI_LED.relay2_green
#define mOILEDIR1 gOI_LED.null
#define mOILEDIR2 gOI_LED.null
#define mOILEDIR3 gOI_LED.null
#define mOILEDIR4 gOI_LED.null

#define mOILEDHaveBall1 gOI_LED.null
#define mOILEDHaveBall2 gOI_LED.null
#define mOILEDHaveBall3 gOI_LED.relay1_red
#define mOILEDHaveBall4 gOI_LED.relay2_red

#define mOILEDHat1Red gOI_LED.pwm1_red
#define mOILEDHat1Green gOI_LED.pwm1_green
#define mOILEDHat2Red gOI_LED.pwm2_red
#define mOILEDHat2Green gOI_LED.pwm2_green

#define mOILEDHighGear gOI_LED.null
#define mOILEDLowGear gOI_LED.null
//#define mOILEDHighGear gOI_LED.switch2
//#define mOILEDLowGear gOI_LED.switch3

#define mOILEDBrakeL gOI_LED.switch1
#define mOILEDBrakeR gOI_LED.switch2

#define mOILEDMotorsReversed  gOI_LED.switch1
#define mOILEDTurnLimited     gOI_LED.null
//#define mOILEDThatNoLongerHasALight gOI_LED.null
// etc.

struct UserByte {
	unsigned char data;
	unsigned set :1;
};

extern char gUserByteOI;

void DoOIDisplay(void);
void SendLEDs(void);
void ClearLEDs(void);

#endif //__OI_DISPLAY_h_
