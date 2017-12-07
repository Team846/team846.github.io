#ifdef _SIMULATOR
#define doTest
#endif //_SIMULATOR

#ifndef doTest
void testArea(void) {}
#else //doTest


static void testtimer1(void);
static void motorDrives(void);
static void testDivision(void);
#include "interrupts.h"
#include "ifi_picdefs.h"
#include "lrtUtilities.h"
#include "lrtMotorDrive.h"
#include "delays.h"
#include "lrtConnections.h"
#include "trig.h"
#include "arm.h"
#include "xferProfiles.h"

static void testTimeStamp()
{
	extern void Int_2_Handler(void);
	int speed;

	Initialize_Timer_1();

while (1) {
	Int_2_Handler();

	speed = motorSpeedCIMT(EncoderRight.d0.ts, 0, kHighGear );

	speed = motorSpeedCIMT(EncoderRight.d0.ts, 0, kLowGear );

	speed = motorSpeedFisherPriceT(EncoderRight.d0.ts, 0, kHighGear );

	speed = motorSpeedFisherPriceT(EncoderRight.d0.ts, 0, kLowGear );
//	Delay1KTCYx(38);
	Delay10TCY();
	Delay1KTCYx(38);
}

} 
static void testDivision(void)
{
	char xxx;
	int iii;
	long lll;
	int anInt;

	lll = 0x5a5a5a5a;
	iii = lll/128;

	anInt = 0x5a5a;
	iii = anInt/128;

	lll = 0x5a5a5a5a;
	iii = lll/2;
//--

	lll = 1023;
	iii = lll/256;

	lll = 1023;
	iii = lll/128;

	lll = 1023;
	iii = lll/2;

	lll = -1025;
	xxx = mDivideBy128(lll);
	lll = 1023;
	xxx = lll/xxx;

	lll = 2 * 85;
	lll = 2 * (int) 85;
	lll = 85;
	lll = lll * 2;


testtimer1();

}

static void motorDrives(void)
{
	
//char limitDriveMotorCurrent(char voltagePWM, char velocity,char CIMorFP);
//char torqueDrive(char TorqueIn, char velocity, char CIMorFP );
//int MotorResistiveVoltage(char voltagePWM, char velocity, char CIMorFP);

	int voltage;
	char velocity;
	int pwmIn;
	int motorEMF;
//	void DriveLeftMotors(char in);

	*(char*)&gGearBox=0;	//clear flags

//	gGearBox.inHighGear=1;
//	EncoderRight.velocity = 10;
//	pwmIn = 127;
//	motorEMF = motorSpeedCIM(EncoderRight.velocity);
//	gPWM.cimL = limitDriveMotorCurrent(pwmIn, motorEMF, kMaxVrCIM);
//
//	EncoderRight.velocity = -10;
//	pwmIn = 127;
//	motorEMF = motorSpeedCIM(EncoderRight.velocity);
//	gPWM.cimL = limitDriveMotorCurrent(pwmIn, motorEMF, kMaxVrCIM);
//	
//	EncoderRight.velocity = -10;
//	pwmIn = -127;
//	motorEMF = motorSpeedCIM(EncoderRight.velocity);
//	gPWM.cimL = limitDriveMotorCurrent(pwmIn, motorEMF, kMaxVrCIM);
//	
//	EncoderRight.velocity = +10;
//	pwmIn = -127;
//	motorEMF = motorSpeedCIM(EncoderRight.velocity);
//	gPWM.cimL = limitDriveMotorCurrent(pwmIn, motorEMF, kMaxVrCIM);


//			ticks/loop	29.90		25.09		8.37		7.03
	
	EncoderRight.velocity = -29-1;
	motorEMF = motorSpeedCIM(EncoderRight.velocity, kHighGear);

	EncoderRight.velocity = -20-1;
	motorEMF = motorSpeedFisherPrice(EncoderRight.velocity, kHighGear);

	EncoderRight.velocity = -8-1;
	motorEMF = motorSpeedCIM(EncoderRight.velocity, kLowGear);

	EncoderRight.velocity = -5-1;
	motorEMF = motorSpeedFisherPrice(EncoderRight.velocity, kLowGear);

}

static void testtimer1(void)
{
	static unsigned int theTime, theTime2;
int apple=0;
	unsigned char Temp_Buf = 0;
	int Timer_Snapshot = 0;

	Initialize_Timer_1();
	apple = apple+1;

//	Temp_Buf = TMR1L; // TMR1L must be read before TMR1H
//	Timer_Snapshot = TMR1H;
//	Timer_Snapshot <<= 8; // move TMR1H data to the upper half of the variable
//	Timer_Snapshot += Temp_Buf;		  
//	

	theTime = 0x01FF - 7;	
	TMR1L = ((char *) &theTime)[0];	//must read low byte first
	TMR1H = ((char *) &theTime)[1];	//read in high byte
//T1CONbits.TMR1ON=0;
//	TMR1 = theTime;
//T1CONbits.TMR1ON=1;

#ifdef CUT
	theTime = TMR1;
	theTime = TMR1;
	((char *) &theTime)[0] = TMR1L;	//must read low byte first
	((char *) &theTime)[1] = TMR1H;	//read in high byte

Delay10TCY();	//1 usec delay
	((char *) &theTime2)[0] = TMR1L;	//must read low byte first
	((char *) &theTime2)[1] = TMR1H;	//read in high byte
#endif
}








//static void testRemovePWM(void)
//{
//	extern unsigned char removePWMDeadband(int pwm);
//
//	int pwmin;
//	unsigned char pwmout;
//	pwmout = removePWMDeadband(0);
//	pwmout = removePWMDeadband(-130);
//	pwmout = removePWMDeadband(-127);
//	pwmout = removePWMDeadband(-63);
//	pwmout = removePWMDeadband(-1);
//	pwmout = removePWMDeadband(32);
//	pwmout = removePWMDeadband(126);
//	pwmout = removePWMDeadband(127);
//	pwmout = removePWMDeadband(128);
//
//	
//}
//

#if CUT
char softstart(char pwmIn, char pwmCurrent)
{
	overlay char signIn;
	char pwmOut;

	signIn=0;
	if (pwmIn < 0)
	{
		signIn = 1;
		pwmIn = -pwmIn;
	}
	
	if (pwmCurrent < 0)
	{
		if (signIn = 0)	//signs opposite
			pwmOut = 2;
		else pwmCurrent = -pwmCurrent
	}
	else
	{


	}
}
#endif

void testInput(void)
{
	p4_y = 10+127L;
	p3_y = -10+127L;
	p3_x = 0+127L;
	lrtJoystickDrive();



}

extern int  sine128(char angleArg);
extern int  cosine128(char angleArg);
void testTrig(void)
{
	int sine;
	sine = sine128(0);
	sine = sine128(64);

	sine = sine128(16);
	sine = sine128(-16);
	sine = sine128(256+16);
	sine = sine128(512+16);
	sine = sine128(256-16);

	sine = cosine128(256+16);
	sine = cosine128(256-16);
	sine = cosine128(0);
	sine = cosine128(128);
	sine = cosine128(128+32);

}

#define mdivbyp2(xxx,pwr2) do {\
	if ((xxx)>=0) (xxx)>>=(pwr2); else (xxx) = -(-(xxx)>>(pwr2)); } while (0)
//place at end so don't have to have prototypes

void testArmMapping(void)
{
	extern int mapForearmInput(unsigned char inputOI);
	extern int mapShoulderInput(unsigned char inputOI);
	int sensor;

	sensor = mapForearmInput(0);
	sensor = mapForearmInput(255);
	sensor = mapForearmInput(64);
	sensor = mapForearmInput(10);
	sensor = mapForearmInput(180);

	sensor = mapForearmInput(190);
	sensor = mapShoulderInput((unsigned char)190);
	sensor = mapShoulderInput(190);

	sensor = mapForearmInput(240);

}
void testProfiles(void)
{
	char output;
	output = piecewiselinear(0);
	output = piecewiselinear(64);
	output = piecewiselinear(-64);
	output = piecewiselinear(127);
	output = piecewiselinear(-127);
	output = piecewiselinear(128);
	output = piecewiselinear(-128);

	output = profile(0, xferPower4_5);
	output = profile(127, xferPower4_5);
	output = profile(64, xferPower4_5);
	output = profile(-64, xferPower4_5);
	output = profile(-127, xferPower4_5);
	output = profile(-128, xferPower4_5);
}

void testArea(void)
{
	testArmMapping();
}

#endif //doTest
