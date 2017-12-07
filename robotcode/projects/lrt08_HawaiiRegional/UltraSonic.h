// UltraSonic.h

#ifndef _ultrasonic_h
#define _ultrasonic_h

//extern unsigned int gUltraSonicDist;	

typedef struct {
	unsigned objectDetected:1;	//The end value to the user
	volatile unsigned valid:1;		//clear this after reading; set to 1 when new data read.
	volatile unsigned int ticks;	//changes at interrupt time
	unsigned int inches;
	unsigned char discriminator;	//internal; Keeps a sum of recent detections; Used to filter readings
} ultrasonic;

extern ultrasonic gUltraSonic;

void doUltraSonic(void);
void Ping_Start(void);

#endif

