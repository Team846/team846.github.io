#include "common.h"

char gUserByteOI;
union OI_LEDs gOI_LED;

void DoOIDisplay(void)
{
	mOILEDHighGear = (Drive_GetGear() == DRIVE_HIGHGEAR);
	mOILEDLowGear = (Drive_GetGear() == DRIVE_LOWGEAR);
	
	if (user_display_mode)
		User_Mode_byte = gUserByteOI;
	else
		SendLEDs();
}

void SendLEDs(void) {
	txdata.LED_byte1.data = gOI_LED.byte1;
	txdata.LED_byte2.data = gOI_LED.byte2;
}

void ClearLEDs(void) {
	gOI_LED.byte1 = gOI_LED.byte2 = 0;
}
