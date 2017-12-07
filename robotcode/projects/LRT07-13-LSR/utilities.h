/********************************************************************************
* FILE NAME: utilities.h <FRC VERSION>

* DESCRIPTION: 
*  This file ...
*
********************************************************************************/
#ifndef utilities_h_
#define utilities_h_


enum { kRight, kLeft };
enum { kLowGear, kHighGear };

#define kNeutral 127u


#define kPI	3.14159
#define kAxleLengthInch 26
#define kEncoderTicksPerFt	128/(6.0*kPI /12)  //81.49	
#define kEncoderTicksPerFt256	kEncoderTicksPerFt*256L	//use care to wrap in '()' when dividing!

	//about 3.1 ticks per degree -- very sensitive!
#define kTurnTicksPerDegree (long)kAxleLengthInch*128L*2/(360*6)
#define kTurnTicksPerDegree256 (long)(kAxleLengthInch*256*128L*2/(360*6)


void testArea(void);

//mSign(A) returns the sign +/-1 or zero of argument 'A'
#define mSign(A) ((A)>0 ? 1 : ((A)<0 ? -1:0))
#define mAbsolute(A) ((A)<0?-(A):(A))
#define mAbsDiff(A,B) ((A)>(B)?((A)-(B)):((B)-(A)))
#define mModPowerOf2(AAA,pwr2) ((AAA > 0) ? ((1L<<pwr2)-1)&(AAA) : -(((1L<<pwr2)-1)&(-(AAA))))
#define mIsPowerOf2(AAA) (0==(((AAA)-1)&(AAA)))

#define mLimitRange(number,low,high) do {\
	if ((number)>(high)) (number)=(high);\
	else if ((number)<(low)) (number)=(low); } while (0) 

#define mDivideByPowerOf2(AAA,pwr2) ((AAA)>=0 ? ((AAA)>>(pwr2)) : -(-(AAA)>>(pwr2)))
#define mDivideBy2(AAA) mDivideByPowerOf2(AAA,1)
#define mDivideBy4(AAA) mDivideByPowerOf2(AAA,2)
#define mDivideBy8(AAA) mDivideByPowerOf2(AAA,3)
#define mDivideBy16(AAA) mDivideByPowerOf2(AAA,4)
#define mDivideBy32(AAA) mDivideByPowerOf2(AAA,5)
#define mDivideBy64(AAA) mDivideByPowerOf2(AAA,6)
#define mDivideBy128(AAA) mDivideByPowerOf2(AAA,7)
#define mDivideBy256(AAA) mDivideByPowerOf2(AAA,8)
#define mDivideBy512(AAA) mDivideByPowerOf2(AAA,9)

typedef struct { long left, right; } LeftRightPair;

typedef union
{
	unsigned long ts;
	struct
	{
		unsigned int tsL;
		unsigned int tsH;
	};
} timestamp;


typedef struct
{
	const char ID;		//either 'R' or 'L' for left or right
	volatile timestamp t0;	//most recent
	volatile timestamp t1;	//one step earlier
	volatile unsigned char count;
	unsigned newdata:1;
} wheelSensor;

extern volatile wheelSensor gShooterLeft;
extern volatile wheelSensor gShooterRight;


typedef struct {
	const char ID;	//either 'R' or 'L' for left or right
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
} encoder;

extern encoder gEncoderRight, gEncoderLeft;
//allocated in user_routines.c

//38.1470	 0.9961
extern volatile unsigned int Timer_High_Count;	//high word of timer (should track packet count)


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
		unsigned onSecond  :1;
		unsigned onSecondA  :1;
		unsigned onSecondB  :1;
		unsigned onSecondC  :1;
		unsigned onSecondD  :1;
		unsigned onSecondLAST  :1;	//don't print on this loop. Diag.Routine prints collected data here
		unsigned onSecondCamera :1;	//set in debug.c
} SlowLoopTiming;
extern SlowLoopTiming gLoop;	//alloc. in LRTUtilities.c


int LimitRange(long a, int low, int high);
char isWithinRange(int a, int b, int range);
char Limit127(int a);
int signed9Bits(int n);

unsigned char removeESCDeadband(int pwm);
char addDeadband(unsigned char pwm);
//unsigned char addLargeDeadband(unsigned char pwm);
void calibrateInput(unsigned char *in, unsigned char lowValue);
void RemoveAllPWMDeadbands(void);
void AllStop(void);
void PrintDigitalInputs(void);

void UpdateTimers(void);

void PrintTime(void);
void PrintPWMs(void);




typedef struct {
	unsigned red1 :1;
	unsigned red2 :1;
	unsigned red3 :1;
	unsigned red4 :1;
	unsigned red5 :1;
	unsigned red6 :1;
	unsigned red7 :1;
	unsigned red8 :1;
	unsigned green1 :1;
	unsigned green2 :1;
	unsigned green3 :1;
	unsigned green4 :1;
	unsigned green5 :1;
	unsigned green6 :1;
	unsigned green7 :1;
	unsigned green8 :1;
} SLEDData;

extern SLEDData gSerialLED;

void SendLEDSerial(unsigned int ledData);


#endif //utilities_h_	NO CODE BELOW THIS LINE
