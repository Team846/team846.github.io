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
#include "simulator.h"
#include "LED.h"

#include "connections.h"
void testArea(void);
	unsigned char SpeedFromElevation(int elevation);


void testArea(void)
{

	char speed;
	long rpm1024;
	long constantLow,constantHigh;
	unsigned long now;
	int data;
	union {
		unsigned data;
		struct {
			unsigned char one;
			unsigned char two;
		};
	} xxx;
	char *p = &xxx.one;
	char *q = &xxx.two;
	char *s = &xxx.data;
	
#define 	kRPM 5400.0
#define kMaxTicksInHighGear (long)(1024*60.0/kRPM/400E-9/128*72/7)
#define kMaxTicksInLowGear  (long)(1024*60.0/kRPM/400E-9/128*1800/49)

static rom const long TicksInOneRPM = (1/400E-9 * 60/128);
	constantHigh = kMaxTicksInHighGear;
	constantLow = kMaxTicksInLowGear;
	
	
	xxx.data=0x0102;
	xxx.one=4;
	xxx.two=5;
	
	gLED.BallInNo2Pos=1;
	gLED.RightJoyX=1;
	SendLEDs();	
	

	rpm1024 = constantLow/7972;
	rpm1024 = constantHigh/2231;
	rpm1024=kMaxTicksInLowGear/2231;

	speed = SpeedFromElevation(-127);
	speed = SpeedFromElevation(0);
	speed = SpeedFromElevation(127);
	speed = SpeedFromElevation(-25);
	speed = SpeedFromElevation(-21);
	speed = SpeedFromElevation(-12);
	speed = SpeedFromElevation(-2);
	speed = SpeedFromElevation(2);
}
