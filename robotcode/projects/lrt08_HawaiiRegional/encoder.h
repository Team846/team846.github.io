#ifndef __ENCODERS_h
#define __ENCODERS_h

#include "common.h"

typedef struct {
	//const char ID;	//either 'R' or 'L' for left or right
	volatile long posNow;	//instantaneous position updated via interrupt
	volatile timestamp t0;	//most recent
	volatile timestamp t1;	//one step earlier
	volatile unsigned newdata:1;
	volatile unsigned direction:1;	//either kFwd or kRev
	timestamp d0;	//  (t0-t1)

	long position;			//copy of positon at beginning of loop
	long oldPosition;
	long projectedPos;	//where we expect to be at next tick given current Vel.

	int velocity;
	int oldVelocity;
	
	long positionOneSecondAgo;	//updated once per second (diagnostic count)
	long ticksInOneSecond;		// ditto

	char impulseReductionTimer;
	unsigned cumulativeDirectionErrors;	//incremented when encoders don't run in same diretion as applied power.
		//Errors slowly decay, and is cleared on when going from disabled to enabled.
} encoder;

enum { LEFT, RIGHT };

extern encoder gEncoders[];		// gEncoders[LEFT], gEncoders[RIGHT]

long encoder_diff_absolute(void);
long encoder_diff_corrected(void);
long encoder_sum(void);
#define encoder_diff() encoder_diff_corrected()

void checkEncodersWorking(void);
void LCD_DisplayAnyEncoderError(void);
#endif	// __ENCODERS_h
