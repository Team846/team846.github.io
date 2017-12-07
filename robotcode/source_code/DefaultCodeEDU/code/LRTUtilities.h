/********************************************************************************
* FILE NAME: LRTUtilities.h
*
*
********************************************************************************/

#ifndef __lrtUtilities_h_
#define __lrtUtilities_h_

#include "ifi_default.h"
//#include "lrtTypes.h"

enum { kRight, kLeft };

#define kPI	3.14159
#define kAxleLengthInch 28
#define kEncoderTicksPerFt	81.49
#define kEncoderTicksPerFt256	kEncoderTicksPerFt*256L	//use care to wrap in '()' when dividing!
//#define kEncoderTicksPerFtSmallW	207.8
//#define kEncoderTicksPerFtSmallW256	207.8*256L	//use care to wrap in '()' when dividing!

//#define kEncoderTicksPerFt	67.1
//#define kEncoderTicksPerFt256	67.1*256L	//use care to wrap in '()' when dividing!

	//about 3.1 ticks per degree -- very sensitive!
#define kTurnTicksPerDegree (long)kAxleLengthInch*128L*2/(360*6)
#define kTurnTicksPerDegree256 (long)(kAxleLengthInch*256*128L*2/(360*6)

#define kMaxMotorVel 28		//Velocity in ticks/26.2ms at max RPM
#define kFast 0				//use this to go as fast as possible

#define kNeutral 127u

void testArea(void);


//mSign(A) returns the sign +/-1 or zero of argument 'A'
#define mSign(A) ((A)>0 ? 1 : ((A)<0 ? -1:0))
#define mAbsolute(A) ((A)<0?-(A):(A))
#define mAbsDiff(A,B) ((A)>(B)?((A)-(B)):((B)-(A)))
#define mModPowerOf2(AAA,pwr2) ((AAA > 0) ? ((1L<<pwr2)-1)&(AAA) : -(((1L<<pwr2)-1)&(-(AAA))))
#define mIsPowerOf2(AAA) (0==(((AAA)-1)&(AAA)))

#define mLimitRange(number,low,high) do {\
	if ((number)>high) (number)=(high);\
	else if ((number)<low) (number)=(low); } while (0) 

#define mDivideByPowerOf2(AAA,pwr2) ((AAA)>=0 ? ((AAA)>>(pwr2)) : -(-(AAA)>>(pwr2)))
#define mDivideBy2(AAA) mDivideByPowerOf2(AAA,1)
#define mDivideBy4(AAA) mDivideByPowerOf2(AAA,2)
#define mDivideBy8(AAA) mDivideByPowerOf2(AAA,3)
#define mDivideBy16(AAA) mDivideByPowerOf2(AAA,4)
#define mDivideBy32(AAA) mDivideByPowerOf2(AAA,5)
#define mDivideBy64(AAA) mDivideByPowerOf2(AAA,6)
#define mDivideBy128(AAA) mDivideByPowerOf2(AAA,7)
#define mDivideBy256(AAA) mDivideByPowerOf2(AAA,8)

typedef struct { long left, right; } LeftRightPair;

typedef
	union {
		unsigned long ts;
		struct {
			unsigned int tsL;
			unsigned int tsH;
		};
} timestamp;

//short long's are 24 bit
typedef struct {
	volatile short long posNow;		//instantaneous position updated via interrupt
	short long position;			//copy of positon at beginning of loop
	short long oldPosition;
	short long projectedPos;	//where we expect to be at next tick given current Vel.
	int velocity;
	int oldVelocity;

	char direction;	//direction at last interrupt 0 is fwd, 1 is rev
		//in keeping with sign(velocity)

	timestamp t0;	//most recent
	timestamp t1;	//one step earlier
	timestamp t2;	//two steps earlier
	
	timestamp d0;	//  (t0-t2) /2/64
//	timestamp d1;	//  (t1-t3)	/2/64
} encoder;

extern encoder EncoderRight, EncoderLeft;
//allocated in user_routines.c

//38.1470	 0.9961


struct Clock {				//timer data
	unsigned int cycle;		//	increments at 38.1470 Hz (10MHz/2^18) rollover ~1718sec
	unsigned char cycle38;	//as above; rollover every 38
	unsigned int secondz;	//one secondz = 0.9961 seconds
} extern volatile gClock; //allocated in interrupts.c

//use to control printf's for debugging
typedef struct {			//loopcount data (similar to above)
		unsigned int count;		//total cycles; rolls over every ~1718sec
		unsigned char count38;	//rolls over every 38;
		unsigned char secondz;	//rolls over every 256 seconds (4+ minutes)
		unsigned onHalfSecond :1;
		unsigned onSecond  :1;
		unsigned onSecond2 :1;
		unsigned onSecond4 :1;
} SlowLoopTiming;
extern SlowLoopTiming gLoop;	//alloc. in LRTUtilities.c




typedef struct {
	union
	{ 
		 bitid bitselect;          /*txdata.LED_byte1.bitselect.bit0*/
		 unsigned char data;       /*txdata.LED_byte1.data*/
	} LED_byte1,LED_byte2;
	unsigned char user_byte;
} OI_LED_User_bytes;


int LimitRange(long a, int low, int high);
char isWithinRange(int a, int b, int range);

unsigned char removePWMDeadband(int pwm);
char addDeadband(unsigned char pwm);
//unsigned char addLargeDeadband(unsigned char pwm);
void calibrateInput(unsigned char *in, unsigned char lowValue);
void RemoveAllPWMDeadbands(void);

char profile(char in127, rom const char *xfer);
char piecewiselinear(char in);

void ToggleSignalFlag(void);
void SignalFlagPWM(void);


void GetEncoderPositionNow(void);

void AllStop(void);

void UpdateSlowLoopTimers(void);

//void TeeBallStartSwing(void);
//char TeeBallRun(void);
//void TeeBallReset(void);
//
//mElevate() returns #26.2ms cycles corresponding to the angle in degrees
//estimate turn of 180 degrees in 4000ms -- verify this
//note: Motor speeds are not symmetrical.
//positive is up, negative is down
//#define mElevate(_ANGLE_) _ANGLE_*4000L/(180L*26.2)
//void BoomElevateInitialize(int cycles);
//char BoomElevateRun(void);
//void BoomElevateReset(void);
//
////mRotate returns #26.2ms cycles corresponding to the angle in degrees
////estimate turn of 180 degrees in 4000ms -- verify this
////note: Motor speeds are not symmetrical.
////positive is CCW, negative is CW
//#define mRotate(_ANGLE_) _ANGLE_*4000L/(180L*26.2)
//void BoomRotateInitialize(int cycles);
//char BoomRotateRun(void);
//void BoomRotateReset(void);

void AutonomousAbort(void);
void AutonomousInitialize(void);
void AutonomousReset(void);
char AutonomousRun(void);
char AutonomousStatus(void);


void PrintDistanceFeet(void);
void PrintVelocity(void);
void PrintTime(void);
void PrintArmPosition(void);
void PrintPWMs(void);



#endif //__lrtUtilities_h_
