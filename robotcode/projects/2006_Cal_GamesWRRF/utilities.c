/*******************************************************************************
*	TITLE:		utilities.c 
******************************************************************************/

#include "ifi_aliases.h"
#include "ifi_default.h"
#include "ifi_utilities.h"
#include "connections.h"
#include "utilities.h"
#include <stdio.h>

volatile struct Clock gClock={0,0,0};

int LimitRange(long a, int low, int high)
{
	if (a<low) return low;
	if (a>high) return high;	
	return a;
}
/***********************************************************************************************/
char Limit127(int a)
{
	if (a>127) return 127;
	if (a<-127) return -127;
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
removeESCDeadband() extends the useful input range of pwm values
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




unsigned char removeESCDeadband(int pwm)
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
//	enum in {a=0,b=127-10L, c=127+10L,d=254};
	enum in {a=0,b=127-15L, c=127+15L,d=254};
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



void AllStop(void)
{
	pwm01 = pwm02 = pwm03 = pwm04 = pwm05 = pwm06 = pwm07 = pwm08 = 127u;

#ifdef _FRC_BOARD	//some of these not defined in EDU board
//	pwm09 = pwm10 = pwm11 = pwm12 = pwm13 = pwm14 = pwm15 = pwm16 = 127u;
	pwm11 = pwm12 = pwm13 = pwm14 = pwm15 = pwm16 = 127u;
//	relay1_fwd = relay1_rev = 0;	//not using any others, but should do all
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

SlowLoopTiming gLoop={-1,-1,-1,0};

void UpdateTimers(void)
{
	++gLoop.count;	
	++gLoop.count38;

	if (gLoop.count38>=38)
		gLoop.count38=0;		//reset; keep on range {0,37}

	if (0==gLoop.count38)	//1st time through, count won't be 38
	{
		++gLoop.secondz;
		gLoop.onSecond = 1;		//loop is a new second
	}
	else
		gLoop.onSecond=0;

	gLoop.onSecondA = gLoop.onSecondB = gLoop.onSecondC =
		gLoop.onSecondD = gLoop.onSecondLAST =0;

	switch(gLoop.count38)
	{
		case 0:  gLoop.onSecond =1;		break;
		case 1:  gLoop.onSecondA=1;		break;
		case 2:  gLoop.onSecondB=1;		break;
		case 3:  gLoop.onSecondC=1;		break;
		case 4:  gLoop.onSecondD=1;		break;
		case 5:  gLoop.onSecondLAST=1;	break;
	}
}
/***********************************************************************************************/
void PrintTime(void)
{
	printf("Time: %ds %3d/38\n", (int) gLoop.secondz, (int) gLoop.count38);	
}

/***********************************************************************************************/

char isWithinRange(int a, int b, int range)
{
	overlay int diff = a-b;
	if (diff<0) diff = -diff;
	return diff<=range;
}


void PrintPWMs(void)
{
	printf("pwm01=%d, pwm02=%d\n", (int) pwm01, (int)pwm02);	
}
/***********************************************************************************************/
void PrintDigitalInputs(void)
{
	//Get 0's and 1's or each input, and then add character '0' to each char.
	// also, add a space every five values, and append "\r\0"
	char digIO[]={
		rc_dig_in01,rc_dig_in02,rc_dig_in03,rc_dig_in04,rc_dig_in05,' '-'0',
		rc_dig_in06,rc_dig_in07,rc_dig_in08,rc_dig_in09,rc_dig_in10,' '-'0',
		rc_dig_in11,rc_dig_in12,rc_dig_in13,rc_dig_in14,rc_dig_in15,' '-'0',
		rc_dig_in16,rc_dig_in17,rc_dig_in18,-1};	//use -1 to terminate string
	char *p;
	for (p=digIO; *p!=-1; p++)
		*p += '0';
	*p=0;	//add line termination
	printf("%s\r", digIO);	//NB: can't use puts(), 'cause str pointer must be in rom
}

