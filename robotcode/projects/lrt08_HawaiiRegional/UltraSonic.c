// UltraSonic.c
// Runs Parallax "PING)))" Ultrasonic Sensor
// RadioShack Catalog #276-031

enum { kObjectDistance=60	//inches
	};

#include "common.h"

#include "_interruptHandlers.h"

#include "UltraSonic.h"
#include "delays.h"

//unsigned int gUltraSonicDist;
ultrasonic gUltraSonic;
void UltraSonicDetector(void);
void Ping(void);

//Ping_Start***************************************
//	This routine starts the ping cycle and in the 
//	end gets a distance result from the time the
// 	sound takes to travel. Remember: sound travels
//	at a speed of 1130 ft/sec.
//*************************************************
void Ping(void)
{
	gUltraSonic.ticks = UINT_MAX;	//default value; Greater than max time of the specific ultrasonic unit.
	gUltraSonic.valid=0;

	DIGIO_ULTRASONIC_PORT = OUTPUT;

	DIGIO_ULTRASONIC = 1;		//start 5uSec key pulse
	Delay10TCYx(5);	//5 uSec (5 x 10 x 0.1 uSec)	
	DIGIO_ULTRASONIC = 0;		//end key pulse
	
	//prepare to capture the pulse start
	DIGIO_ULTRASONIC_PORT = INPUT;
}


#define kUltraTicksPerInch ((int)(2/(12*1127.0 * 400E-9)))	//about 370
	//speed of sound=1127ft/sec;  2x for round trip; 1 clock tick is 400nSec

//*******************************************************************************
void doUltraSonic(void)
{
	gUltraSonic.inches = gUltraSonic.ticks /kUltraTicksPerInch;	//takes about 20uSec
	
	Ping();	//The ping on our ultrasonic lasts less than 1 cycle, so we can ping every cycle.
	UltraSonicDetector();
	
	if (gLoop.f.onHalfSecond)
	{
		//printf("%d inches\r", gUltraSonic.inches );
		printfLCD(LCD_UltraSonic,"%d inches %c", gUltraSonic.inches,
			(int)(gUltraSonic.objectDetected? 'X':'.'));
	}
}
//*******************************************************************************
void UltraSonicDetector(void)
{
	//What to do with the ultrasonic data?
	//How do we discriminate true and false readings, filtering gaps and ignoring singular events?
	
	//On each cycle, look for nearby objects.
	//Introduce a 'discriminator', whose value we use as a measure of confidence.
	//On each cycle we decay the discriminator by 1, toward zero.
	//If we find an object, we add 3 (net 2), max 10
	//We 'detect' an object if discriminator > N, currently 6;
	//Finally, we relax the detection threshold if we have previously detected an object.
	
	if (gUltraSonic.inches < kObjectDistance)
	{
		gUltraSonic.discriminator +=3;
		if (gUltraSonic.discriminator > 10) gUltraSonic.discriminator=10;
	}
	
	if (gUltraSonic.discriminator>0)
		gUltraSonic.discriminator--;
	
	//Finally, detection is based on discriminator rising above a threshold.
	if (gUltraSonic.objectDetected)	//provide hysteresis; Once detected, relax detection
		gUltraSonic.objectDetected = (gUltraSonic.discriminator >= 6-2);
	else
		gUltraSonic.objectDetected = (gUltraSonic.discriminator >= 6);
	
	ULTRASONIC_LED = gUltraSonic.objectDetected;
}