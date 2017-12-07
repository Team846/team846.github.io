/********************************************************************************
* FILE NAME: LED.h <FRC VERSION>
*
* DESCRIPTION: 
*  This file ...
*
********************************************************************************/
#ifndef LED_h_
#define LED_h_
#include "ifi_default.h"




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

union  OI_LEDs{
	struct {
		unsigned char byte1;
		unsigned char byte2;
	};		
	struct {
		unsigned MotorsReversed:1;		//0 Pwm1_green;
		unsigned turnRateLimited	:1;	//1 Pwm1_red;
		unsigned :1;	//2 Pwm1_green;
		unsigned :1;	//3 Pwm2_red;
		unsigned RxTxError			:1;		//4 Relay1_red
		unsigned CameraStatus		:1;		//5 Relay1_green
		unsigned LauncherBelowSpeed	:1;		//6 Relay2_red
		unsigned BallInNo2Pos		:1;		//7 Relay2_green
		//-- second byte
		unsigned LeftJoyY			:1;		//0 Switch1_LED
		unsigned RightJoyY			:1;		//1 Switch2_LED
		unsigned RightJoyX			:1;		//2 Switch3_LED
		unsigned :5;	//unused.
	};
};
extern union OI_LEDs gLED;

struct UserByte {
	unsigned char data;
	unsigned set:1;
};

extern char gUserByteOI;


#define SendLEDs() do {\
	txdata.LED_byte1.data = gLED.byte1;\
	txdata.LED_byte2.data = gLED.byte2;\
	 } while (0)

#define ClearLEDs() do {\
	gLED.byte1 = gLED.byte2 = 0;\
	 } while (0)

#endif //LED_h_	NO CODE BELOW THIS LINE/***** LED connections (See OI Layout below) ************************************
