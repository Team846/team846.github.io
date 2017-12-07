#include "ifi_default.h"
#include "ifi_aliases.h"
#include "ifi_utilities.h"
#include "utilities.h"
#include "controls.h"
#include "connections.h"
#include "tracking.h"
#include <stdio.h>
#include <limits.h>
#include "MotorDrive.h"
#include "user_routines.h"
#include "interruptHandlers.h"
#include "LED.h"

#define kMaxSpeed256 200
#define kMinSpeed256 20


enum { kNonNegative, kNegative};


//need a constant to convert time to max speed 
// at 3000rpm, 

#define kMaxWheelRPM (17000.0/5)
#define kTicksInOneRPM (60.0/400E-9)
#define kTicksAtMaxRPM (kTicksInOneRPM/kMaxWheelRPM)

//#define kLoad 0.07	//load on motor, typically about 5-25%
//#define kMinPWM (kLoad*127)

static char kMinPWM=1;

char gLauncherBelowSpeed;
//************ Local Prototypes ************************************************* 

char ComputePWMforLaunchWheel(unsigned long wheelTicks, char inputSpeed, char LeftOrRight);
unsigned long GetWheelPeriod(volatile wheelSensor *shooter);
unsigned char SpeedFromElevation(int elevation);
unsigned char SpeedFromVerticalError(int elevation);
//************************************************************************************



unsigned long GetWheelPeriod(volatile wheelSensor *shooter)
{
	unsigned long now;
	unsigned long d0 = shooter->t0.ts - shooter->t1.ts;
	unsigned long dNow;	// delta time since last pulse

	//Check if pulse happened in last  26.2ms loop.
	//Use either old data, or current time minus last timestamp,  whichever is greater
	
	//printf("GetWheelPeriod\r");

	if (shooter->newdata)
	{
		shooter->newdata=0;	//reset flag
		//printf("new data\r");
	}
	else	//no new data since last loop.
	{
//		if (d0 > (unsigned long) (10*kTicksAtMaxRPM))
		if (1)
		{
			now = GetTime();
	
			dNow = now - shooter->t0.ts;
			if (dNow > d0)
				d0  = dNow;	//use best available estimate (motor must  be slowing)
			
			//printf("Estimating data\r");
		}
	}
	//return d0; if it is zero, return max signed long
	return d0==0? ULONG_MAX/2: d0;		//return d0, but make sure it is non-zero
}

//************************************************************************************
void ControlLaunchWheelSpeeds(char inputSpeed)
{
	char pwmSpeed;
	long wheelPeriodLeft, wheelPeriodRight;

	gLauncherBelowSpeed=0;
	wheelPeriodLeft = GetWheelPeriod(&gShooterLeft);
	pwmSpeed = ComputePWMforLaunchWheel(wheelPeriodLeft, inputSpeed,'L');
	mPWMLeftShooter = 127u+pwmSpeed;

	wheelPeriodRight = GetWheelPeriod(&gShooterRight);
	pwmSpeed = ComputePWMforLaunchWheel(wheelPeriodRight, inputSpeed,'R');
	mPWMRightShooter = 127u+pwmSpeed;

	if (!disabled_mode)	//don't show error if disabled
		gLED.LauncherBelowSpeed = gLauncherBelowSpeed;

	if (inputSpeed==0)	//warn that launcher is 'off' at all times.
		gLED.LauncherBelowSpeed = gLoop.count>>2;	//fast blink
}

//************************************************************************************
/*
 * ControlLaunchWheelSpeed()
 * is a closed loop control routine for the ball launcher wheels.
 * the applied pwm has two components:
	1) the pwm needed to acheive the input speed based on calculations and empirical data
 *     ( computed in motorPWMSpeedXferFunction() )
 *  2) an error correction based on proportionally on the error
 *     error is the desired speed - measured speed
 *	   loop gain is adjusted to account for the non-linear ESC-motor drive characteristic
 *
 *  D.Giandomenico 4/2006
 */
char ComputePWMforLaunchWheel(unsigned long wheelTicks, char inputSpeed, char LeftOrRight)
{
#define kErrorScale ((int)1024)
	int speed;
	long error1024;
	const int targetSpeed1024 = (int) inputSpeed*(kErrorScale/128);
	long rpm1024=0;
	int gain;					//to be determined for optimum performance
	int gainESCCompensation;	//adjustment factor for non-linearity of ESC-Motor
	long adjustedGain; 			//overall compensated gain
	long speedCorrection;
	char fractionalPWM;
	char signCorrection;
	
	int relativeAbsError;	//used to see if motors are not up to speed

	static char cnt  = 0;
	rom static const char bitreverse[4]={0,2,1,3};
//	rom static const char bitreverse[8]={0,4,2,6,1,5,3,7};

	//try a gain of ~115 to 150.  Generally well behaved, but some overshoot.
//	gain =277;	
//	gain = (int)Get_Analog_Value(rc_ana_in16);
//	if (gLoop.onSecondD) printf("wheel launcher gain=%d", (int)gain);
	gain=1020;	//shouldn't be this high.  Could be more friction in system now.
	cnt=!cnt;

//	kMinPWM = ((int)Get_Analog_Value(rc_ana_in15)>>5) + 1;	//on range {1,32}
	kMinPWM = 8;
	if (gLoop.onSecondD) printf("kMinPWM=%d\r", (int)kMinPWM);


	rpm1024 =((unsigned long)kTicksAtMaxRPM*kErrorScale);
	rpm1024 /= wheelTicks;
 
	error1024 = targetSpeed1024-rpm1024;
	mLimitRange(error1024,-255,255);	//limit max size of error to reduce overflows.
		//given range of input, this amounts to 254/1024 or about 25% max error

	//calculate to see if we are up to speed
	relativeAbsError = mAbsolute(error1024);
	if (targetSpeed1024 != 0)
		relativeAbsError = ((long) relativeAbsError<<7) / targetSpeed1024; //100% = 128;
	
	if (relativeAbsError >= 6)	//approx 6%
		gLauncherBelowSpeed=1;

	/*
	 * Compensate for non-linearity of the gain of the pulse width modulated ESC + motor
	 * this requires detailed explanation, but generally gain of ESC+motor is proportional
	 * to 1/(pwmM- pwmMin)^2 where 
	 *  pwmMin is Iload/Is * 128 (below which the motor will not turn)
	 *  and pwm in the input, pwm>pwmMin
	 * we can compensate (linearize) the gain by multipling by (input-pwmMin)^2
	 *
 	 * D.Giandomenico
	 */
	gainESCCompensation = inputSpeed-(char)kMinPWM;	//assumes inputSpeed is non-negative
	if (gainESCCompensation<=0)	//compensation must be positive, nonzero
		gainESCCompensation=1;

//	gainESCCompensation *= gainESCCompensation;	//square it; range is now {0,2^14}
	gainESCCompensation *= 128;	//square it; range is now {0,2^14}
	
	adjustedGain = (gain * (long)gainESCCompensation)>>7;	//max range is now 2^(10+7)
		//adjusted gain is >0, so we can simply bit shift to divide by power of 2

	
	speedCorrection = error1024 * adjustedGain;	//range is now 2^(8+10+7) or 2^25
		//shift by 10 bits to reduce range to  +/- 2^15
		//shift additional bits to further control gain.  E.g. 10+7 -> +/ 255
	
	signCorrection = kNonNegative;
	if (speedCorrection < 0)
	{	
		signCorrection = kNegative;
		speedCorrection = -speedCorrection;
	}
	speedCorrection = speedCorrection >>(10+7-2);	//range is now < +/- 2^8
//	speedCorrection = speedCorrection >>(10+7-1);	//range is now < +/- 2^9


	//fraction PWM values are important for controlling motor at low speed.
	//That is, there is a significant difference in speed with pwm=7 vs. pwm=8
	//So, if we want 7.25, then use pwm=7 for 3 cycles, and pwm=8 for 1 cycle.
	//Use a bit reversal to ensure 7.5 turns on every other cycle, rather than 2 consecutive cyles
	//  out of every four.

	fractionalPWM = speedCorrection&0x3;
	speedCorrection = speedCorrection>>2; //remove fractional amount
	if ((bitreverse[gLoop.count & 0x3]) < fractionalPWM)
	{
		//printf("fracPWM=%3d, gLoopCount=%3d\r", (int)fractionalPWM,(int)(gLoop.count & 0x3));
		speedCorrection++;
	}
	
#define kMaxCorrection 63
	if (speedCorrection > kMaxCorrection)
		speedCorrection = kMaxCorrection; 	//limit range
	if (signCorrection==kNegative)
		speedCorrection = -speedCorrection;


	//add the corrected open-loop pwm as_a_function_of desired speed
	speed = motorPWMSpeedXferFunction( inputSpeed, kMinPWM );
	speed += speedCorrection;	//doesn't seem right...should be opposite sign.
		//changed for 2006 robot.


	mLimitRange(speed,0,127);


//	if ((cnt && gLoop.onSecond) || (!cnt &&gLoop.onSecondB))
	if (gLoop.onSecondD)
	{
		overlay long rpm = ((unsigned long)kTicksInOneRPM) / wheelTicks;
		printf("%c: %3d=input, %5d=rpm, %3d=pwm,  %4d=in(1K), %4d=rpm(1K), %3d=error\r",
			(char) LeftOrRight,(int) inputSpeed, (int)rpm, (int)speed,
			(int) targetSpeed1024,(int)rpm1024,(int)error1024);

		printf("%3d=gain, %6ld=adjGain, %4d=speedCorrection, %ld=wheelTicks\r", 
			(int)gain,(long)adjustedGain, (int)speedCorrection, (long)wheelTicks);
		printf("%3d=relative error\r",(int)relativeAbsError);
	}
	return speed;
}


/*
 * motorPWMSpeedXferFunction(speed) returns the pwm needed to operate motor at speedIn
 *
 * DC = (Ia/Is) / (1-N/Ns)
 *
 * where:
 * 
 * DC: duty cycle on {0,100%}
 * IAvg/Is is the avg current (or ILoad) / the Stall current, typically 10-25%
 * Ns is the stall speed; N is the speed
 * Then
  * 
 * we need DC on {0,127}
 * if we put Ia/Is on {0,127}, scale N/Ns on {0,128}
 * and further multiply top and bottom by 128 we get:
 * pwm = 128 * (Is/Ia) / (127-speed)
 * minPWM = 128* (Is/Ia)
 */
char motorPWMSpeedXferFunction(char speedIn, unsigned char minPWM)
{
	char direction = kForward;
	int pwm;
	if (speedIn==0) return 0;
	if (speedIn<0)
	{
		direction=kReverse;
		speedIn = -speedIn;
	}	
	pwm = ((int)minPWM * 128)/(128-speedIn); //speedIn=0 -> kLoad*128


	mLimitRange(pwm,0,127);	//can't deliver more than 100%!

	if (direction==kReverse) pwm = -pwm;
	return pwm;
}



/**********************************************
camera elevation set at 26 degrees 
CameraAngle -127     -26      0      34 127
RealAngle     --       0     26      60  -- 
Speed        max     max  <-data->  min min
**********************************************/

#define kMinSpeed (char)(127*0.15)
#define kMaxMeasRPM 1024
#define kNominalSpeed (550L*127L/kMaxMeasRPM)
unsigned char SpeedFromElevation(int elevation)
{
	static struct {
		char elevation;	//vertical camera pwm when locked
		unsigned char speed; //corresponding ball launcher speed for best shot
	} table[]= {	//data for best shot, ordered low angle to high angle (far to close)
		//{-127,  600L*127L/kMaxMeasRPM},		//first entry; far distance; original
		{-127,  700L*127L/kMaxMeasRPM},		//first entry; far distance
		{ -11,  580L*127L/kMaxMeasRPM},		//560-580
		{  -2,  575L*127L/kMaxMeasRPM},		//550-600
		{ 10,	550L*127L/kMaxMeasRPM},		//500-max close shot (angle high)
		{ 127,	kMaxMeasRPM*127L/kMaxMeasRPM}//last entry; max power
		//{ 127, 550L*127L/kMaxMeasRPM}		//last entry; original
	};
	overlay char i;
	overlay int speed;
	
	for (i= sizeof(table)/sizeof(table[0])-1; i>0; i--)
	{
		overlay int deltaElevation = elevation - table[i-1].elevation;
		if (deltaElevation < 0)
			continue;
	
		//interpolate between table[i-1] and table[i]
		speed = deltaElevation * ((int)table[i].speed - (int)table[i-1].speed);
		speed /= (int)table[i].elevation - table[i-1].elevation;
		speed += table[i-1].speed;
		break;
	}
	return speed;
}

/**********************************************
camera elevation set at 26 degrees 
CameraAngle -127     -26      0      34 127
RealAngle     --       0     26      60  -- 
Speed        max     max  <-data->  min min
**********************************************/
unsigned char SpeedFromVerticalError(int elevation)
{
	static struct {
		char elevation;	//vertical camera pwm when locked
		unsigned char speed; //corresponding ball launcher speed for best shot
	} table[]= {	//data for best shot, ordered low position to high position (close to far)
		{-127,  900L*127L/kMaxMeasRPM},		 //0	first entry; max power
		{ -94,  800L*127L/kMaxMeasRPM},		 //1 max close shot (angle high)
		{ -86,  784L*127L/kMaxMeasRPM},		 //2
		{ -22,  600L*127L/kMaxMeasRPM},		 //3
		{   7,  600L*127L/kMaxMeasRPM},		 //2
		{  28,  576L*127L/kMaxMeasRPM},		 //3
		{  53,  608L*127L/kMaxMeasRPM},		 //2
		{  75,  624L*127L/kMaxMeasRPM},      //3
		{  87,  648L*127L/kMaxMeasRPM},      //3
		{  90,  700L*127L/kMaxMeasRPM},		 //2
//		{ 110, 1000L*127L/kMaxMeasRPM},      //1 far distance
		{ 110,  930L*127L/kMaxMeasRPM},      //1 far distance	new max measured of 938 dg 10/24/06
		{ 127, kMaxMeasRPM*127L/kMaxMeasRPM} //0 last entry; max power
	};
	overlay char i;
	overlay int speed;
	
	for (i= sizeof(table)/sizeof(table[0])-1; i>0; i--)
	{
		overlay int deltaElevation = elevation - table[i-1].elevation;
		if (deltaElevation < 0)
			continue;
	
		//interpolate between table[i-1] and table[i]
		speed = deltaElevation * ((int)table[i].speed - (int)table[i-1].speed);
		speed /= (int)table[i].elevation - table[i-1].elevation;
		speed += table[i-1].speed;
		break;
	}
	return speed;
}


#define kSpeedLatchTime (int)(6000/26.2)
#define kSlewRateTicks 4		//slews 1 every kSlewRateTicks
// GetBallLauncherSpeedFromCamera()
//  Gets speed from camera when target locked
//    when camera looses lock, speed stays constant for kSpeedLatchTime
//	  After this time, speed drops (slews) to kNominalBallLauncherSpeed at
//	  rate of kSlewRateTicks (e.g. 4 -> 4*127/38 == 13sec for 0->127 change in speed)
char GetBallLauncherSpeedFromCamera(void)
{
	static int HoldSpeedTimer=0;	//timer to remain at last known speed(elevation)
	static char SlewRateTimer=0;
	static char currentSpeed=kNominalSpeed;

	if (SlewRateTimer <=0)			//use to control rate of pwm slewing
		SlewRateTimer=kSlewRateTicks;	//reduce pwm 1 per kTicksPerPWM cycles
	SlewRateTimer--;

	if (GetRawTrackingState() == TARGET_IN_VIEW)
	{
#ifdef _TRACK_TILT
		int doubleTiltAngle = get2xTiltAngle();
		currentSpeed = SpeedFromElevation(doubleTiltAngle);
		if (gLoop.onSecond && gCameraPrinting) printf("DoubleTiltAngle: %d\r", (int)doubleTiltAngle);
#else
		int tiltError = getTiltError();
		currentSpeed = SpeedFromVerticalError(tiltError);
#endif
		
		HoldSpeedTimer = kSpeedLatchTime;		//reset hold time
	}
	else
	{
		if (gLoop.onSecond && gCameraPrinting) printf("Lost target\r");
	}

	if (HoldSpeedTimer>0)
		HoldSpeedTimer--;
	else if (0==SlewRateTimer)	//slew toward kNominalBallLauncherSpeed at 
	{
		int diff = (int) currentSpeed-kNominalSpeed;
		if (diff >0)
			currentSpeed--;
		else if (diff<0)
			currentSpeed++;
	}
	return currentSpeed;
}
