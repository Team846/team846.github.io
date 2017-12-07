/********************************************************************************
* FILE NAME: LRTUtilities.c
*
*
*****************************************************************************/

#include "lrtUtilities.h"
#include "ifi_default.h"
#include "ifi_aliases.h"
#include "printf_lib.h"
#include "arm.h"

int LimitRange(long a, int low, int high)
{
	if (a<low) return low;
	if (a>high) return high;	
	return a;
}
/***********************************************************************************************/
//A low value pot on the OI will cause output from a non-zero number
//to 254.  Remap this from 0-254

void calibrateInput(unsigned char *in, unsigned char lowValue)
{
	if (*in <= lowValue) *in=0;
	else if (254 <= *in) *in = 254;
	else
		*in = ((*in-lowValue)*(unsigned int)254)/(254-lowValue);

}
/***********************************************************************************************/

/******
removePWMDeadband() extends the useful input range of pwm values
The Victor883/884's *after*they*are*calibrated have
are full on at pwm=11 and at pwm=248
similarly, they are full off from 122 to 133.
This leaves 'deadband' in the ranges {0,10},{122,133},&{248,254}
This was determined empirically with an oscilloscope, after calibrating
both Victor883's and 884's, per InnovationFirst instructions.

By remapping the input pwm to output pwm, the effective input range
can be extended such that the pwm delivered to the Victor883/884
is limited to the operational range of the Victor883/884.
(This needs some rework for clarity)

Maps input range to output range:
	{-127,-1) to {11-121}   //127 values to 111 values (~12% more)
	0-> 127
	{1,254} to {127,248}	//127 to 115 values (~10% more)
and -127 -> 0; 127->254  //changed [dg] 2/7/05
		
D.Giandomenico
*******/




unsigned char removePWMDeadband(int pwm)
{
	enum in {a=-127,b=-1, c=1,d=127};
	enum out {w=11,x=121,y=134,z=248};
	int newPwm=127;

	if (pwm>=d)
		newPwm = 254;
	else if (pwm>=c)
		newPwm = ((pwm-c)*z + (d-pwm)*y) / (d-c);
	else if (pwm>b)
		newPwm = kNeutral;
	else if (pwm>=a)
		newPwm = ((pwm-a)*x + (b-pwm)*w) / (b-a);
	else
		newPwm = 0;
	return newPwm;
}



/***********************************************************************************************/
/******
addDeadband()	introduces a deadband range around 127+/-3
maps input range:
	{0,127-3) to {0-126}
	{127-2,127+2} -> 127
	{127+3,254} to {128,254}
*******/

char addDeadband(unsigned char pwm)
{
	enum in {a=0,b=127-10L, c=127+10L,d=254};
	enum out {w=-127,x=-1,y=1,z=127};
	enum {neutral=0};
	
	int newPwm;

	if (pwm>d)
		newPwm = neutral;   //error
	else if (pwm>=c)
		newPwm = ((pwm-c)*(int)z + (d-pwm)*(int)y) / (int) (d-c);
	else if (pwm>b)
		newPwm = neutral;
	else if (pwm>=a)
		newPwm = ((pwm-a)*(int)x + (b-pwm)*(int)w) / (int) (b-a);
	else
		newPwm = neutral;   //error
		
//	txdata.user_byte1.allbits=pwm;
//	txdata.user_byte2.allbits=newPwm;
	return newPwm;
}
/***********************************************************************************************/

//use on operator interface for front/rear legs
unsigned char addLargeDeadband(unsigned char pwm)
{
	enum in {a=0,b=127-30L, c=127+30L,d=254};
	enum out {w=0,x=126,y=128,z=254};
	enum {neutral=127};
	
	int newPwm;

	if (pwm>d)
		newPwm = neutral;   //error
	else if (pwm>=c)
		newPwm = ((pwm-c)*(int)z + (d-pwm)*(int)y) / (d-c);
	else if (pwm>b)
		newPwm = neutral;
	else if (pwm>=a)
		newPwm = ((pwm-a)*(int)x + (b-pwm)*(int)w) / (b-a);
	else
		newPwm = neutral;   //error
		
//	txdata.user_byte1.allbits=pwm;
//	txdata.user_byte2.allbits=newPwm;
	return newPwm;
}

/***********************************************************************************************/
/***********************************************************************************************/

/*******
void GetEncoderPositionNow() gets copies of the current encoder position
It computes the velocity and projected encoder position for the next tick
based on the current velocity.
Because the position is updated via interrups, encoder.positionNow may
change during execution of the slow loops.
 
Call at the beginning of the slow & autonomous loops
D.Giandomenico
**********/

void GetEncoderPositionNow(void)
{
   	//get copies of the position
   	// 'position' changes via interrupts
    //	Save previous position & velocity
   	EncoderRight.oldPosition = EncoderRight.position;
   	EncoderRight.oldVelocity = EncoderRight.velocity;
   	   
    EncoderLeft.oldPosition = EncoderLeft.position;
   	EncoderLeft.oldVelocity = EncoderLeft.velocity;
  	 	
   	EncoderRight.position = EncoderRight.posNow;
   	EncoderLeft.position = EncoderLeft.posNow;
   	
	//velocity calc include effect of acceleration
//	EncoderRight.velocity = 2*(EncoderRight.position - EncoderRight.oldPosition)
//		 - EncoderRight.oldVelocity; 	

//	EncoderLeft.velocity = 2*(EncoderLeft.position - EncoderLeft.oldPosition)
//		 - EncoderLeft.oldVelocity; 	

  	EncoderRight.velocity = EncoderRight.position - EncoderRight.oldPosition;
   	EncoderRight.projectedPos = EncoderRight.position + EncoderRight.velocity;

  	EncoderLeft.velocity = EncoderLeft.position - EncoderLeft.oldPosition;
   	EncoderLeft.projectedPos = EncoderLeft.position + EncoderLeft.velocity;
}


void AllStop(void)
{
	pwm01 = pwm02 = pwm03 = pwm04 = pwm05 = pwm06 = pwm07 = pwm08 = 127u;

#ifdef _FRC_BOARD	//some of these not defined in EDU board
	pwm09 = pwm10 = pwm11 = pwm12 = pwm13 = pwm14 = pwm15 = pwm16 = 127u;
	relay1_fwd = relay1_rev = 0;	//not using any others, but should do all
#endif // _FRC_BOARD	
}


/***********************************************************************************************/
//UpdateSlowLoopTimers(void) updates the counts & seconds in the slow loops
// for controling timing in debug printf statements
//call once at beginning of the slow 26.2ms loops in User_routines and Autonomous
//use times as
// if (gLoop.onSecond) printf("message\n);
// if (gLoop.onHalfSecond) printf("message\n);
// if (gLoop.onSecond && (0==(gLoop.count & 0x3)) printf("message\n);	//print every 4 seconds

SlowLoopTiming gLoop={-1,-1,-1};

void UpdateSlowLoopTimers(void)
{
	++gLoop.count;	
	++gLoop.count38;

	if (gLoop.count38>=38)
		gLoop.count38=0;		//reset; keep on range {0,37}

	if (0==gLoop.count38)	//1st time through, count won't be 38
	{
		++gLoop.secondz;
		gLoop.onHalfSecond = 1;	//loop is a new halfSecond
		gLoop.onSecond = 1;		//loop is a new second
		if ((gLoop.secondz& 0x1) ==0)
		{
			gLoop.onSecond2 = 1;	//loop is an even second
			if ((gLoop.secondz & 0x2)== 0)
				gLoop.onSecond4=1;	//loop is a 4th second
		}
	}
	else if (gLoop.count38==38/2)
		 gLoop.onHalfSecond = 1;
	else
	{
		gLoop.onHalfSecond=0;
		gLoop.onSecond=0;
		gLoop.onSecond2=0;
		gLoop.onSecond4=0;
	}
}


char isWithinRange(int a, int b, int range)
{
	overlay int diff = a-b;
	if (diff<0) diff = -diff;
	return diff<=range;
}


void PrintArmPosition(void)
{
	printf("Shoulder=%d;  forearm=%d\n", (int) gArm.shoulder.curPosition, (int)gArm.forearm.curPosition);
}
void PrintDistanceFeet(void)
{
	struct {
			int left10x,right10x,angleCCW;
	} traversed;
	
	traversed.left10x = EncoderLeft.position*10/(kEncoderTicksPerFt);	//kEncoderTicksPerFt;
	traversed.right10x = EncoderRight.position*10/(kEncoderTicksPerFt);
	traversed.angleCCW =(EncoderRight.position-EncoderLeft.position)*10/(kTurnTicksPerDegree);
	
	printf("Position 10x L=%d,R=%d, angle=%d\n",(int)traversed.left10x,(int)traversed.right10x,(int)traversed.angleCCW);
}
void PrintVelocity(void)
{
	printf("Velocity L=%d,R=%d\n",(int) EncoderLeft.velocity,(int)EncoderRight.velocity);
}
void PrintTime(void)
{
	printf("Time: %ds %3d/38\n", (int) gClock.secondz, (int) gClock.cycle38);	
}

void PrintPWMs(void)
{
	printf("pwm01=%d, pwm02=%d\n", (int) pwm01, (int)pwm02);	
}
/***********************************************************************************************/

//=IF(ABS(input)<=$H$1,input/$I$1,SIGN(input)*($H$1/$I$1+(128-$H$1/$I$1) * (ABS(input)-$H$1)/(128-$H$1)))



char piecewiselinear(char in)
{
	enum { kBreakpoint=95u, kDivisorPower2=3 };
#define kValueAtBreakPoint (kBreakpoint>>kDivisorPower2)

	overlay char sign=0;
	overlay char result;
	overlay char input = in;	//work with faster variable
	
	if (0x80==input) input=0; //special case  since 0x80 == -0x80
	if (input<0)
	{
		sign = 1;
		input = -input;
	}
	
	//at this point, input is positive definite
	if (input<=kBreakpoint)
		result = input >> kDivisorPower2;
	else
		result = kValueAtBreakPoint + 
			(127u-kValueAtBreakPoint) * (unsigned int)(input-kBreakpoint)/(127u-kBreakpoint);

	if (sign)
		result = -result;

	return result;
}


char profile(char in127, rom const char *xfer)
{
	overlay char sign=0;
	overlay char input=in127;
	overlay char result;
	if (input == -128) input = 0;	//special case.  since 0x80 == -0x80
	if (input<0)
	{
		sign=1;
		input = -input;
	}
	
	result= xfer[input];
	if (sign) result  = -result;
	return result;
}
