#include "common.h"
#include "MoveRobot2.h"

char MoveRobot(unsigned long encoderSumInitial, long distance, char power)
{
	unsigned long encoderSum = gEncoderLeft.posNow + gEncoderRight.posNow;
	long distanceTravelled = encoderSum - encoderSumInitial;

	if (power < 0) power = -power;	// corrects dumb mistakes
	if (distance < 0) power = -power;
	
	printf("MoveRobot: Initial enc sum: %ld; Cur enc sum: %ld\r\n", encoderSumInitial, encoderSum);
	printf("MoveRobot: Travelled %ld/%ld; Power: %d\r\n", distanceTravelled, distance, power);

	if ( (distance > 0) ? (distanceTravelled <= distance) : (distanceTravelled >= distance) )
	{
		gMotorSpeed.cimL = gMotorSpeed.cimR = power;		// uses servo drive in autonomous mode
															// see MotorDrive.c
		return 0;
	}
	else
		return 1;
}

static char pulsePower(char input, char onTime, char offTime, char power) {
	static char timer;
	static char Q;	//output 'Q' as in a flip-flop
	
	if (mAbsolute(input) < 8) input = 0;
 	
//	#define kPulseHi 5
//	#define kPulseLo 3
//	#define kPower 127 //((char)(mUserOIPot2>>1))	//70
	
	if (--timer <= 0) {
		if (Q) {
			Q = 0;
			timer = offTime;
		} else {
			Q = 1;
			timer = onTime;
		}
	}

	if (Q) {
		if (input > 0) return power;
		if (input < 0) return -power;
	}
	return 0;
}

void aim(char fwd) {
	long turn;
	static long maxRecent=0;
	int sumLR;
	unsigned char automatic_gain_reduction;

	//sensor values up to 2.5V give range {0, approx 512}
	//If disconnected, voltage will float up toward 5V, leaving values ~900+.

	//Use recent inputs to scale sensitivity
	sumLR = gProx.left + gProx.right;
	if (sumLR > maxRecent)
		maxRecent = sumLR;
	else
		maxRecent = (maxRecent*63) >> 6;	//decay 

	//decay time constant for 'a^n' is 1/(1-a)
	// so, if a=63/64, t = 1/[1-63/64] = 64	cycles
	// if a=61/64, t = 64/3, or approx 21 cycles

//	automatic_gain_reduction = maxRecent >> (6+10-5-1); //2^(15-10) = 2^5
//	bargraph32(maxRecent >> (10-5));
	
	turn = gProx.left - gProx.right;

	if (maxRecent > 0)
		turn = turn * 64 / maxRecent;
	
	if (maxRecent > 220)
		fwd = 0;
	
//	turn = pulsePower(turn, 5, 3, (0.80 * 127));
	turn = pulsePower(turn, 4, 6, (0.80 * 127));
	gMotorSpeed.cimL =  Limit127((int)(fwd) - (int)(turn));
	gMotorSpeed.cimR =  Limit127((int)(fwd) + (int)(turn));

//#define kLimit 20
//
//	mLimitRange(turn,-kLimit,kLimit);
//	turn = limit127(turn);
//
//	pwm01 = turn + 127;

//	printf("left=%d, right=%d\r", prox.left,prox.right);
}



void autodistance() {
	long fwd;
	
	//sensor values up to 2.5V give range {0, approx 512}
	//If disconnected, voltage will float up toward 5V, leaving values ~900+.

#ifdef ROBOT_2007
	if (getLiftPos() < 100) {		// if lift could be covering sensor, don't autodistance
		gMotorSpeed.cimL = 0;
		gMotorSpeed.cimR = 0;
		return;
	}
#endif //ROBOT_2007

	#define TARGET ( (int)(mUserOIPot2>>1) )
	fwd = TARGET - gProx.bumper;
	printf("AutoDistance target distance is %d\r\n", TARGET);
	
/*	{	// OI LED feedback
		int absturn = (turn<0 ? -turn : turn);
		gLED.LeftPx1 = gLED.RightPx1 = (absturn > 15);
		gLED.LeftPx2 = gLED.RightPx2 = (absturn > 90);
		gLED.LeftPx3 = gLED.RightPx3 = (absturn > 180);
		gLED.LeftPx4 = gLED.RightPx4 = (absturn > 290);
		
		if (turn < 0)
			gLED.LeftPx1 = gLED.LeftPx2 = gLED.LeftPx3 = gLED.LeftPx4 = 0;
		else
			gLED.RightPx1 = gLED.RightPx2 = gLED.RightPx3 = gLED.RightPx4 = 0;
	}*/

//	if (maxRecent > 0)
//		turn = turn * 64 / maxRecent;

//	fwd = pulsePower(fwd, 5, 3, 127);
	fwd = Limit127(fwd);
	gMotorSpeed.cimL = fwd;
	gMotorSpeed.cimR = fwd;
}
