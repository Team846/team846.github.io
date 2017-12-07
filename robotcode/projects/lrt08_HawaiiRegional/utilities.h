#ifndef utilities_h_
#define utilities_h_

#ifndef __LARGE__
#error Not Compiled with -ml Large (>64K) data option
#endif


#define NEUTRAL 127u

#define PI      3.14159

//mSign(A) returns the sign +/-1 or zero of argument 'A'
#define mSign(A)               ((A)>0 ? 1 : ((A)<0 ? -1:0))
#define mAbsolute(A)           ((A)<0?-(A):(A))
#define mAbsDiff(A,B)          ((A)>(B)?((A)-(B)):((B)-(A)))
#define mModPowerOf2(AAA,pwr2) ((AAA > 0) ? ((1L<<pwr2)-1)&(AAA) : -(((1L<<pwr2)-1)&(-(AAA))))
#define mIsPowerOf2(AAA)       (0==(((AAA)-1)&(AAA)))

#define mLimitRange(number,low,high) do {\
	if ((number)>(high)) (number)=(high);\
	else if ((number)<(low)) (number)=(low); } while (0) 

#define mDivideByPowerOf2(AAA,pwr2) ((AAA)>=0 ? ((AAA)>>(pwr2)) : -(-(AAA)>>(pwr2)))
#define mDivideBy2(AAA)   mDivideByPowerOf2(AAA,1)
#define mDivideBy4(AAA)   mDivideByPowerOf2(AAA,2)
#define mDivideBy8(AAA)   mDivideByPowerOf2(AAA,3)
#define mDivideBy16(AAA)  mDivideByPowerOf2(AAA,4)
#define mDivideBy32(AAA)  mDivideByPowerOf2(AAA,5)
#define mDivideBy64(AAA)  mDivideByPowerOf2(AAA,6)
#define mDivideBy128(AAA) mDivideByPowerOf2(AAA,7)
#define mDivideBy256(AAA) mDivideByPowerOf2(AAA,8)
#define mDivideBy512(AAA) mDivideByPowerOf2(AAA,9)
#define mDivideBy1024(AAA) mDivideByPowerOf2(AAA,10)	//NB: These large shifts are not fast, and division may be comparable

extern volatile unsigned int Timer_High_Count;	//high word of timer (should track packet count)

//use to control printf's for debugging
typedef struct {			//loopcount data (similar to above)
	unsigned int count;		//total cycles; rolls over every ~1718sec
	unsigned char count38;	//rolls over every 38;
	unsigned char secondz;	//rolls over every 256 seconds (4+ minutes)

	unsigned int disabledCount;	//count since FRC became disabled
	unsigned int autonomousCount;
	unsigned int enabledCount;
	
	unsigned long startTime;	//This is set at beginning of each loop in Process_Data_From_Master_uP()
	unsigned char rolling;		//This is the rolling count # for spreading the printing load; See below
	
	union {
		unsigned long allFlags;	//for quick clearing of flags; must be as large as structure below
		struct {
			unsigned onSecond  :1;
			unsigned onHalfSecond  :1;	//used for LCD updates
			unsigned printA  :1;
			unsigned printB  :1;
			unsigned printC  :1;
			unsigned printD  :1;
			unsigned printStats  :1;	//don't print on this loop. Diag.Routine prints collected data here
			unsigned printLCD  :1;		//Cycle to update LCD
			unsigned printLCD1  :1;		//1st cycle to update LCD
			unsigned printLCD2  :1;		//2nd cycle to update LCD
		};
	}f;
} SlowLoopTiming;
extern SlowLoopTiming gLoop;

//use this macro to distribute printing over all cycles.
//use mRolling(1) to increment; use mRolling (1) to not increment
//#define mRolling(doIncrement) (gLoop.count38 == ((doIncrement) ? ++gLoop.rolling : gLoop.rolling))
#define mRolling() (++gLoop.rolling == gLoop.count38)



typedef struct {
	char busyCycle;	//most recent loop that had no free cycles 
	unsigned int idleCycles;	//counted in main()
	
	//data handled in userRoutines
	unsigned int minIdleCycles;
	unsigned long cumulativeIdleCyles;
	unsigned char nLoops;	//time over which data is accumulated

} CPULoadData;
extern CPULoadData gCPULoad;

int LimitRange(long a, int low, int high);
char isWithinRange(int a, int b, int range);
char Limit127(int a);
int signed9Bits(int n);

unsigned char removeESCDeadband(int pwm);
char addDeadband(unsigned char pwm);
void RemoveAllPWMDeadbands(void);
void AllStop(void);
void UpdateTimers(void);
void PrintDigitalInputs(void);
void PrintDigitalInputsLCD(char lineNo);

#endif //utilities_h_	NO CODE BELOW THIS LINE
