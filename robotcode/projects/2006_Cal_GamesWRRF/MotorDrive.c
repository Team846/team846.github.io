#include "utilities.h"
#include "ifi_aliases.h"
#include "ifi_default.h"
#include "ifi_utilities.h"

#include "controls.h"
#include "MotorDrive.h"
#include "connections.h"
#include "interruptHandlers.h"
#include <stdio.h>

motorPWMs gPWM;
motorSpeed gMotorSpeed;
int gRobotTurnSpeed;


//gearBox gGearBox, gGearBoxLeft, gGearBox.right;
gearBoxes gGearBox={kHighGear};
//gearBoxes gGearBox={kLowGear};
char gDriveLimitedFlag=0;


/*****************************************************************/
/* we have 128*2 pulses per revolution
	need time per tick at max rpm
 */
#define kTicksAtMaxSpeed (300/60*128)	//rpm/sec * pulses/rev
#define kPeriodAtMaxSpeed (1/(kTicksAtMaxSpeed * 400E-9))

//********** LOCAL PROTOTYPES *********************
unsigned long GetDriveWheelPeriod(encoder *e);
char DriveOneMotor(encoder *e, char speed);
//********** END OF LOCAL PROTOTYPES *********************



/*****************************************************************
	Current Limiting Theory
EMF - linear function of motor speed from 0 to 12V (or 0 to -12V)
PWM% - duty cycle from +/-{0,127}
Iavg - average current
12V - applied battery voltage (may be somewhat higher)

Iavg = PWM% * (12V* sign(PWM%)-EMF) / Rcircuit

solve for pwm to acheive Iavg:
PWM% = (Iavg*Rcircuit) / (12V - sign(PWM)* EMF)
*****************************************************************/




/********************************************************************************
* FUNCTION: ClearMotorPWMs()
********************************************************************************/
void ClearMotorPWMs(void)
{
	gMotorSpeed.cimL=0;
	gMotorSpeed.cimR=0;
	gPWM.cimL=0;
	gPWM.cimR=0;
}
//****************************************************************************
void DriveMotors(void)
{
	if (gUserOption.RawDrive == 2)
	{
		mPWMLeftCIM = removeESCDeadband(-gMotorSpeed.cimL);
		mPWMRightCIM = removeESCDeadband(gMotorSpeed.cimR);
		if (gLoop.onSecond) printf("RAW Drive\r");
	}
	else
	{
		gPWM.cimL = DriveOneMotor(&gEncoderLeft, gMotorSpeed.cimL);
		gPWM.cimR = DriveOneMotor(&gEncoderRight, gMotorSpeed.cimR);
		
		mPWMLeftCIM = removeESCDeadband(-gPWM.cimL);
		mPWMRightCIM = removeESCDeadband(gPWM.cimR);
		if (gLoop.onSecond) printf("SERVO Drive\r");
	}

}
//****************************************************************************
int GetTurnRate(void)
{
	struct {
		int right,left;
	} s;
	int turnSpeed=0;

	
	s.right = kTicksAtMaxRPMHighGear*1024 / GetDriveWheelPeriod(&gEncoderRight);
	if (gEncoderRight.direction)
		s.right = -s.right;
	
	s.left = kTicksAtMaxRPMHighGear*1024 / GetDriveWheelPeriod(&gEncoderLeft);
	if (gEncoderLeft.direction)
		s.left = -s.left;

	turnSpeed=s.left-s.right;
	if (gLoop.onSecondB)
		printf("turn=%d, tL=%d, tR=%d\r",turnSpeed,s.left,s.right);
		
	return turnSpeed;
}
//****************************************************************************
char DriveOneMotor(encoder *e, char inputSpeed)
{
	long timeBetweenPulses;
	char minPWM;
	long rpm1024;	//big enough for intermediate computation
	int error1024;
	int gain;					//to be determined for optimum performance
	int gainESCCompensation;	//adjustment factor for non-linearity of ESC-Motor
	long adjustedGain; 			//overall compensated gain
	long speedCorrection;
	char fractionalPWM;
	char signCorrection;
	rom static const char bitreverse[4]={0,2,1,3};
	int dutyCycle;
	enum {kNonNegative,kNegative};

	if (gGearBox.cmdGearObjective==kHighGear)	//should check if really in gear
	{
		minPWM = 6;	//need to set these empirically
		gain=88;

		rpm1024 = kTicksAtMaxRPMHighGear*1024;
	}
	else	//low gear
	{
		minPWM = 6;
		//minPWM = ((int)Get_Analog_Value(rc_ana_in13)>>5) + 1;	//on range {1,32}
		gain=250;
		//gain = (int)Get_Analog_Value(rc_ana_in14);
		rpm1024 = kTicksAtMaxRPMLowGear*1024;
	}

	timeBetweenPulses = GetDriveWheelPeriod(e);
	rpm1024 /= timeBetweenPulses;
	
	if (kReverse==e->direction)
		rpm1024= -rpm1024;

	//IMPULSE REDUCTION
	#define kImpulseTimer 12

	if (e->impulseReductionTimer<0)
	{
		if ((kForward == e->direction)!=(inputSpeed>=0))
			e->impulseReductionTimer = kImpulseTimer;
	}
	if (e->impulseReductionTimer>0)
	{
		e->impulseReductionTimer--;
		inputSpeed = (long)inputSpeed *
			(kImpulseTimer-e->impulseReductionTimer)*(kImpulseTimer-e->impulseReductionTimer)
			/((int)kImpulseTimer*kImpulseTimer);
	}
	if (e->impulseReductionTimer==0)
	{
		if ((kForward == e->direction)==(inputSpeed>=0))
			e->impulseReductionTimer = -1;		//reset; ready for next activation
	}



	error1024 = inputSpeed*(1024/128)- rpm1024;
	
	mLimitRange(error1024,-255,255);	//limit max size of error to reduce overflows.
		//given range of input, this amounts to 254/1024 or about 25% max error


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
	
	gainESCCompensation = mAbsolute(inputSpeed)-(char)minPWM;
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


	//fraction PWM values are important for controlling motor at low speed.
	//That is, there is a significant difference in speed with pwm=7 vs. pwm=8
	//So, if we want 7.25, then use pwm=7 for 3 cycles, and pwm=8 for 1 cycle.
	//Use a bit reversal to ensure 7.5 turns on every other cycle, rather than 2 consecutive cyles
	//  out of every four.

	fractionalPWM = speedCorrection&0x3;
	speedCorrection = speedCorrection>>2; //remove fractional amount
	if ((bitreverse[gLoop.count & 0x3]) < fractionalPWM)
		speedCorrection++;
	
#define kMaxCorrection (32-1)
	if (speedCorrection > kMaxCorrection)
		speedCorrection = kMaxCorrection; 	//limit range
	if (signCorrection==kNegative)
		speedCorrection = -speedCorrection;


	//add the corrected open-loop pwm as_a_function_of desired speed
	dutyCycle = motorPWMSpeedXferFunction( inputSpeed, minPWM );
	if (dutyCycle > 0 && speedCorrection+dutyCycle < 1)
		dutyCycle=1;
	else if (dutyCycle < 0 && speedCorrection+dutyCycle > -1)
		dutyCycle=-1;
	else
		dutyCycle += speedCorrection;	//open loop for now.

	mLimitRange(dutyCycle,-127,127);

	User_Byte3 = rpm1024/4;		//Display wheel speed on dashboard
	if (gLoop.onSecondB)
	{
		static rom const long TicksInOneRPM = (1/400E-9 * 60/128);
		long rpm = ((unsigned long)TicksInOneRPM) / timeBetweenPulses;
		printf("minPWM=%d\r",(int) minPWM);
		printf("%c: %3d=input, %5d=rpm, %3d=D.Cycle,  %4d=in(1K), %4d=rpm(1K), %3d=error\r",
			(char) e->ID,(int) inputSpeed, (int)rpm, (int)dutyCycle,
			(int) inputSpeed*(1024/128),(int)rpm1024,(int)error1024);

		printf("%3d=gain, %6ld=adjGain, %4d=speedCorrect, %ld=dTime\r", 
			(int)gain,(long)adjustedGain, (int)speedCorrection, (long)timeBetweenPulses);
	}
	return dutyCycle;
}
//****************************************************************************
void GetDriveEncoderCounts(encoder *e)
{
	e->oldPosition = e->position; 
	e->position = e->posNow;	//posNow may change; get a copy	

	//collect and print diagnostic data (not used elsewhere)
	if (gLoop.onSecondC)	//collect one second data
	{
		int speed=-1;
		e->ticksInOneSecond= e->position - e->positionOneSecondAgo;
		e->positionOneSecondAgo=e->position;

			//double ticks, since we count pulses on leading and trailing edge
			//but timing only on leading edge
		speed = e->ticksInOneSecond * 60/(2*kEncoderTicksPerRev);
		printf("%c: Encoder %6d=pos %6d=dpos  %3d=rpm (1sec sum)\r", (char)e->ID,
			(int)  e->position, (int)e->ticksInOneSecond, (int)speed);
	}
}
//****************************************************************************

/*********
 *
 * Velocity is measured in ticks per 2*26.2ms
 * EMF is voltage from 0-127 where 127=12.5V
 * kKvMainDrive is scaled by 512 so that 
 * EMF =  motor's maxVelocity ( in ticks/2*26.2ms) X kKvMainDrive / 512
 * 
 * Then, limit the max difference in app
 * These values are computed in an Excel Spreadsheet.
 * D.Giandomenico
 ************/ 
 
//  when multiplied by velocity (ticks per 2*26.2ms)
//
// Could we give a temporary boost?



//pwm is applied power on range of {-127,127}
//returns pwm on {0,254}


/********************************************************************************
* FUNCTION: limitDriveMotorCurrent
*
* DESCRIPTION: 

* All voltages scaled such on range {0-127}<->{0,Vbattery}

* Vr=resistive voltage on motor, giving rise to a current Vr/Ir
* EMF - motor's electromotive potential
*
*
* define 'drivePCM' as % of full scale on range {-127,127} <-> {-100%,100%}
* Va = %power * Vbattery - |%power| *EMF
*
* drivePC is drive percent = PWM-127.  {-100%,100%} <-> {-127,127}
********************************************************************************/
void GetMotorSpeeds(void)
{
	char gear;
	unsigned long deltaTime;

	deltaTime = GetDriveWheelPeriod(&gEncoderLeft);


//	gMotorSpeed.cimL =		motorSpeedCIM( gEncoderLeft.velocity,			gGearBox.left.cmdGearSpeedToMatch);
//	gMotorSpeed.cimR =		motorSpeedCIM( gEncoderRight.velocity,			gGearBox.right.cmdGearSpeedToMatch);
	
//	txdata.user_byte3  = gMotorSpeed.cimL;
//	txdata.user_byte4  = motorSpeedCIMT( EncoderLeft.t0, EncoderLeft.direction,	gGearBox.left.cmdGearSpeedToMatch);

}

//Must call GetMotorSpeeds before calling DriveLeft/RightMotors
void DriveLeftMotors(char in)
{
	gPWM.cimL = limitDriveMotorCurrent(in, gMotorSpeed.cimL, kMaxVrCIM);
}

void DriveRightMotors(char in)
{
	gPWM.cimR = limitDriveMotorCurrent(in, gMotorSpeed.cimR, kMaxVrCIM);
}





/*******************************************************************************/
char limitDriveMotorCurrent(char voltagePWM, int motorEMF, const char limit)
{
	int motorResistiveVoltage;
	extern char gDriveLimitedFlag;
	char pwmOut = voltagePWM;
	char signbit;

	if (0==voltagePWM) return 0;	//save some time

	signbit=0;
	if (voltagePWM < 0)
	{
		signbit=1;		
		voltagePWM = -voltagePWM;
		motorEMF = -motorEMF;	//applied voltage reversed relative to EMF
	}
	
	//limit motorEMF to safe computation values
	mLimitRange(motorEMF,-126,126);

	//average voltage applied to resistance of motor
	motorResistiveVoltage = voltagePWM * (127-motorEMF)/127;	//always positive

	
	if (motorResistiveVoltage > limit)
	{
		gDriveLimitedFlag=1;

		//limit current
		pwmOut = (limit * (int) 127) / (127 - motorEMF);
		if (signbit) pwmOut = -pwmOut;	//correct sign

	//	motorResistiveVoltage = pwmOut * (127-motorEMF)/127;	//sanity check
	}

	return pwmOut;
}
/*******************************************************************************/
/*******************************************************************************/
char torqueDrive(char torqueIn, int motorEMF, const char limit)
{
	int motorResistiveVoltage;
	extern char gDriveLimitedFlag;
	char pwmOut = 0;
	char signbit;

	if (0==torqueIn) return 0;	//save some time

	signbit=0;
	if (torqueIn < 0)
	{
		signbit=1;		
		torqueIn = -torqueIn;
		motorEMF = -motorEMF;	//applied voltage reversed relative to EMF
	}
	
	//limit motorEMF to safe computation values
	mLimitRange(motorEMF,-126,126);
	
	pwmOut = (limit * (int) torqueIn) / (127 - motorEMF); // always positive
	if (pwmOut > 127)
	{
		pwmOut = 127;
		gDriveLimitedFlag=1;
	}
	if (signbit) pwmOut = -pwmOut;	//correct sign

	//	motorResistiveVoltage = pwmOut * (127-motorEMF)/127;	//sanity check

	return pwmOut;
}
/*******************************************************************************/

//29.90		25.09		8.37		7.03

int motorSpeedCIM(char velocity,char gear)
{
	overlay int result;
	overlay char signbit=0;
	if (velocity < 0)
	{
		signbit=1;
		velocity = -velocity;
	}

	//calc speed.  If in between gears, the we are trying to match speed in this gear
	if (gear==kLowGear)
		result = (int)velocity * 243>>4;
	else	//assume in High gear
		result = (int)velocity * 17>>2;	//mult by 127/29.90



	if (result > 126) result = 126;	//limit in case of overspeed
		//keep below 127, to avoid potential zero divide later.


	if (signbit) result=-result;
	return result;
}

/*****************************************************************/
int motorSpeedCIMT(unsigned long theTime, char signDirection, char gear )
{
	overlay int result;
	
	if (theTime > 0x0000FFFFL)
		return 0;	//really long time between ticks
	
	if (0==theTime) return 0;	//error, avoid zero divide.

	
	if (gear==kHighGear)
		result = ((unsigned short long)127 * kToHighGearCIM)/ (unsigned int)theTime;
	else	//in LowGear
		result = ((unsigned short long)127 * kToLowGearCIM)/ (unsigned int)theTime;


	if (result > 126) result = 126;	//limit in case of overspeed
		//keep below 127, to avoid potential zero divide later.

	if (signDirection) result=-result;
	return result;
}
//***********************************************************************************
unsigned long GetDriveWheelPeriod(encoder *e)
{
	unsigned long now;
	unsigned long d0 = e->t0.ts - e->t1.ts;
	unsigned long dNow;	// delta time since last pulse

	long test = kPeriodAtMaxSpeed;

	//Check if pulse happened in last  26.2ms loop.
	//Use either old data, or current time minus last timestamp,  whichever is greater

	if (e->newdata)
		e->newdata=0;	//reset flag; Use the new data
	else	//no new data since last loop.
	{
		//Either use old data, or timeNow - "last known time"
		now = GetTime();
		dNow = now - e->t0.ts;
		if (dNow > d0)
			d0  = dNow;	//use best available estimate (motor must  be slowing)

//		if (d0 > (unsigned long) (10*kPeriodAtMaxSpeed))	//This may need rethinking for motordrive
//		{
//			if (dNow > d0)
//				d0  = dNow;	//use best available estimate (motor must  be slowing)
//		}
	}
	//return d0; if it is zero, return max signed long
	return d0==0? (~0L)>>1: d0;		//return d0, but make sure it is non-zero
}

