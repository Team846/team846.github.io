#include "common.h"
#include "pwm.h"

Drive	gMotorSpeed;
int		gRobotTurnSpeed;

/*****************************************************************/
/* we have 128*2 pulses per revolution
	need time per tick at max rpm
 */
#define kTicksAtMaxSpeed (300/60*128)	//rpm/sec * pulses/rev
#define kPeriodAtMaxSpeed (1/(kTicksAtMaxSpeed * 400E-9))

//********** LOCAL PROTOTYPES *********************
static unsigned long GetDriveWheelPeriod(encoder *e);
#if 0
static char ServoDriveMotor(encoder *e, char speed);
static char motorPWMSpeedXferFunction(char speedIn, unsigned char minPWM);
#endif
//********** END OF LOCAL PROTOTYPES *********************


//****************************************************************************

void ClearMotorPWMs(void)
{
	gMotorSpeed.left = gMotorSpeed.right = 0;
	gMotorSpeed.brakeLeft = gMotorSpeed.brakeRight = 0;
}
//****************************************************************************
void LimitDrive127(Drive *d)
{
	d->left  = Limit127(d->left);
	d->right = Limit127(d->right);
	d->fwd   = Limit127(d->fwd);
	d->turn  = Limit127(d->turn);	
}
//****************************************************************************

void GetDriveEncoderCounts(encoder *e)
{
	e->oldPosition = e->position; 
	e->position = e->posNow;	//posNow may change; get a copy	

	//collect and print diagnostic data (not used elsewhere)
	if (gLoop.f.printC)	//collect one second data
	{
		int speed=-1;
		e->ticksInOneSecond= e->position - e->positionOneSecondAgo;
		e->positionOneSecondAgo=e->position;

			//double ticks, since we count pulses on leading and trailing edge
			//but timing only on leading edge
		speed = e->ticksInOneSecond * 60/(2*DRIVE_TICKS_PER_REV);
		printf("Encoder %6d=pos %6d=dpos  %3d=rpm (1sec sum)\r",
			(int)  e->position, (int)e->ticksInOneSecond, (int)speed);
	}
}

//****************************************************************************

void Drive_Do(void)
{
	mLimitRange(gMotorSpeed.left,  -127,127);
	mLimitRange(gMotorSpeed.right, -127,127);

//	if (gUserOption.ServoDrive == 0)
//	{
		PWM_CIM_L = removeESCDeadband(-gMotorSpeed.left);
		PWM_CIM_R = removeESCDeadband(-gMotorSpeed.right);
		if (mRolling()) printf("RAW DRIVE\r\n");
//	}
//	else
//	{
//		int DrivePWMLeft = ServoDriveMotor(&gEncoders[LEFT], gMotorSpeed.left);
//		int DrivePWMRight = ServoDriveMotor(&gEncoders[RIGHT], gMotorSpeed.right);
//		
//		PWM_CIM_L = removeESCDeadband(DrivePWMLeft);
//		PWM_CIM_R = removeESCDeadband(DrivePWMRight);
//		if (mRolling()) printf("SERVO DRIVE - gain pot = %d\r\n", OI_SERVODRIVE_GAIN_POT);
//	}

	DIGOUT_COASTL = !gMotorSpeed.brakeLeft;
	DIGOUT_COASTR = !gMotorSpeed.brakeRight;
	mOILEDBrakeL = !DIGOUT_COASTL;
	mOILEDBrakeR = !DIGOUT_COASTR;
//	printf("Coasting %d : L%d R%d\r", gLoop.count&7, DIGOUT_COASTL, DIGOUT_COASTR);
}

//****************************************************************************

int GetTurnRate(void)
{
	struct {
		int right,left;
	} s;
	int turnSpeed=0;

	s.right = kTicksAtMaxRPM*1024 / GetDriveWheelPeriod(&gEncoders[RIGHT]);
	if (gEncoders[RIGHT].direction)
		s.right = -s.right;
	
	s.left = kTicksAtMaxRPM*1024 / GetDriveWheelPeriod(&gEncoders[LEFT]);
	if (gEncoders[LEFT].direction)
		s.left = -s.left;

	turnSpeed=s.left-s.right;
	if (gLoop.f.printB)
		printf("turn=%d, tL=%d, tR=%d\r",turnSpeed,s.left,s.right);
		
	return turnSpeed;
}

//****************************************************************************
// This "servo drive" uses closed-loop control to drive a motor
// at a particular -speed-. [dcl]

#if 0
static char ServoDriveMotor(encoder *e, char inputSpeed)
{
	long timeBetweenPulses;
	char minPWM;
	long rpm1024;	//big enough for intermediate computation
	int error1024;
	static int gain;					//to be determined for optimum performance
	int gainESCCompensation;	//adjustment factor for non-linearity of ESC-Motor
	long adjustedGain; 			//overall compensated gain
	long speedCorrection;
	char fractionalPWM;
	char signCorrection;
	rom static const char bitreverse[4]={0,2,1,3};
	int dutyCycle;
	enum {kNonNegative,kNegative};


	minPWM = 6;	//need to set these empirically
//	minPWM = ((int)Get_Analog_Value(mUserPot1Port)>>5) + 1;	//on range {1,32}
	//gain=88;
	if (!autonomous_mode) 
		gain = 4* (int)OI_SERVODRIVE_GAIN_POT; //FIXME: Need value here.
	if (mRolling())
		printf("Servo drive Gain: %d\r\n", gain);
	
	rpm1024 = kTicksAtMaxRPM*1024;

	timeBetweenPulses = GetDriveWheelPeriod(e);
	rpm1024 /= timeBetweenPulses;
	
	if (REVERSE == e->direction)
		rpm1024= -rpm1024;

	//IMPULSE REDUCTION				// Apparently: every 12 cycles, divides inputSpeed by 144 -> 0. functionality doubtful[?][dcl]
	#define kImpulseTimer 12

	if (e->impulseReductionTimer<0)
	{
		if ((FORWARD == e->direction)!=(inputSpeed>=0))
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
		if ((FORWARD == e->direction)==(inputSpeed>=0))
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
	gainESCCompensation *= 128;	//square it; range is now {0,2^14}								// What is this? [dcl]
	
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
	if (gLoop.printB)
	{
		static rom const long TicksInOneRPM = (1/400E-9 * 60/128);
		long rpm = ((unsigned long)TicksInOneRPM) / timeBetweenPulses;
		printf("minPWM=%d\r",(int) minPWM);
		printf("%3d=input, %5d=rpm, %3d=D.Cycle,  %4d=in(1K), %4d=rpm(1K), %3d=error\r",
			(int) inputSpeed, (int)rpm, (int)dutyCycle,
			(int) inputSpeed*(1024/128),(int)rpm1024,(int)error1024);

		printf("%3d=gain, %6ld=adjGain, %4d=speedCorrect, %ld=dTime\r", 
			(int)gain,(long)adjustedGain, (int)speedCorrection, (long)timeBetweenPulses);
	}
	return dutyCycle;
}
#endif //ServoDriveMotor
//****************************************************************************

static unsigned long GetDriveWheelPeriod(encoder *e)
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
#if 0
static char motorPWMSpeedXferFunction(char speedIn, unsigned char minPWM)
{
	char direction = FORWARD;
	int pwm;
	if (speedIn==0) return 0;
	if (speedIn<0)
	{
		direction=REVERSE;
		speedIn = -speedIn;
	}	
	pwm = ((int)minPWM * 128)/(128-speedIn); //speedIn=0 -> kLoad*128


	mLimitRange(pwm, 0, 127);	//can't deliver more than 100%!

	if (direction == REVERSE) pwm = -pwm;
	return pwm;
}
#endif


#define DRIVE_HIGHGEAR_POS 55
#define DRIVE_LOWGEAR_POS  -55

// kept falling out
//#define DRIVE_HIGHGEAR_POS 45
//#define DRIVE_LOWGEAR_POS  -39

// too much
//#define DRIVE_HIGHGEAR_POS 100
//#define DRIVE_LOWGEAR_POS  -100

// too much
//#define DRIVE_HIGHGEAR_POS 120
//#define DRIVE_LOWGEAR_POS  -120

#define DRIVE_NEUTRAL_POS  0
#define DRIVE_SHIFT_TIME   (1000/26.2)
#define DRIVE_SHIFT_REASSERT_TIME   (10000/26.2)

static struct {
	enum DriveGear state;
	int target;
	int timer;
	int timerUntilReassert;
} sShifting = { DRIVE_HIGHGEAR, DRIVE_HIGHGEAR_POS, 0,0 };
//*************************************************************
void Drive_Shift(enum DriveGear to) {
	switch (to) {
		case DRIVE_HIGHGEAR:
			sShifting.target = DRIVE_HIGHGEAR_POS;	//may need left/right targets.
			break;
		case DRIVE_LOWGEAR:
			sShifting.target = DRIVE_LOWGEAR_POS;
			break;
		case DRIVE_NEUTRALGEAR:
			sShifting.target = DRIVE_NEUTRAL_POS;
			break;
	}
	sShifting.timer = DRIVE_SHIFT_TIME;
}
//*************************************************************
void Drive_DoShifting(void)
{
	struct {
		unsigned char active;
		char left,right;
	} shift={0,0,0};

	const char isMoving =
		mAbsolute(gMotorSpeed.left) > 3 || mAbsolute(gMotorSpeed.right) > 3;

	//Keep reasserting the shift every few seconds, but only if the robot is moving
	if ( isMoving && sShifting.timerUntilReassert > 0)
			sShifting.timerUntilReassert--;
	
	if (0==sShifting.timerUntilReassert)
	{
		sShifting.timer = DRIVE_SHIFT_TIME; // begin the shift.
		sShifting.timerUntilReassert = DRIVE_SHIFT_REASSERT_TIME; //reset the reassert timer
	}
	
	//If not moving and ending shift, reassert as soon as robot starts moving again.
	if (!isMoving && 1==sShifting.timer)
		sShifting.timerUntilReassert=1;	//will timeout on next cycle if robot is moving
	
	//Do the shift
	if (sShifting.timer > 0)
	{
		sShifting.timer--;
		shift.active = 1;
//		shift.left  = Limit127(sShifting.target);	//L/R servo's may have different set points
//		shift.right = Limit127(sShifting.target);	//Not used here.
	}
	//This could/should be done elsewhere, but...
	//first PWM position is used for testing/setting up servo

#if 0	//Servo seup code; pot3 controls pwm14; pwm13 127+/-30;
	shift.active = 1;
	pwm13 = 127 +  ((gLoop.secondz & 0x1)? 30:-30);
	if (mRolling())
		printf("Servo Setup: pot3=pwm14=%d\r",(int)OI_USERPOT3);
	if (gLoop.f.printLCD1)
	{
		printfLCD(LCD_Error,"Servo Setup");
		LCD_Blink(LCD_Error,1);
	}
#endif //End of Servo Setup Code
	
	shift.active = 1;	// HACK: servos always on
	PWM(pwm13, OI_USERPOT3, 127+sShifting.target, 127-sShifting.target, shift.active);
}
//*************************************************************
enum DriveGear Drive_GetGear(void) {
	return sShifting.state;
}
void Drive_ShiftControls(void)
{
	static int neutralTimer = 0;
	static char debounce = 0;

	if (OI_SHIFTHI && OI_SHIFTLO)
	{
		debounce = 1;
		if (neutralTimer < 1000/26.2)
		{
			neutralTimer++;
		}
		else
		{
			Drive_Shift(DRIVE_NEUTRALGEAR);
		}
	}
	else
	{
		neutralTimer = 0;
		if (debounce)
		{
			if (!OI_SHIFTHI && !OI_SHIFTLO)
			{
				// no buttons pressed
				debounce = 0;
			}
		}
		else
		{
			if (OI_SHIFTHI)
			{
				Drive_Shift(DRIVE_HIGHGEAR);
				//				sShifting.target = OI_USERPOT2;	// FIXME testing
			}
			else if (OI_SHIFTLO)
			{
				Drive_Shift(DRIVE_LOWGEAR);
			}
		}
	}
}




void HighSpeedTurn(Drive *drive)
{
	struct {
		unsigned turn:1;
		unsigned fwd:1;
	} sign = {0,0};
	Drive temp;
	int reduction;
	
	temp = *drive;

	if (temp.fwd < 0) {
		temp.fwd = -temp.fwd;
		sign.fwd = 1;
	}
	if (temp.turn < 0) {
		temp.turn = -temp.turn;
		sign.turn = 1;
	}

	//reduce fwd by alpha * abs(normalized turn), 0<=alpha<1;  alpha defined as (kFwdReduction/128)
	//both drive.fwd and .turn are positive here.
	#define kFwdReduction 64L
	reduction = ((long)temp.fwd * temp.turn * kFwdReduction) >> 14;	//divide by 128*128
	temp.fwd -= reduction;

	if (sign.fwd) temp.fwd = -temp.fwd;
	if (sign.turn) temp.turn = -temp.turn;

	drive->left = Limit127(temp.fwd + temp.turn);
	drive->right = Limit127(temp.fwd - temp.turn);
}

#define TURN_IN_PLACE_THRESH 1
#define DITHER_RES_ORDER 3
#define DITHER_RES (1 << DITHER_RES_ORDER)	// == 2 ** DITHER_RES_ORDER
//#define MAX_BRAKING (3)
#define BRAKETURN_DEADBAND 50

//***********************************************************************
void BrakedDrive(Drive *drive, int maxBraking)
{
	// Calculate dither pattern by reversing bits of binary 0-7
	rom static const char ditherPattern[]={0,4,2,6,1,5,3,7};
	enum {CW,  CCW}   turnDir;
	enum {LEFT,RIGHT} inboardSide;
	int absTurn, brakeAmt;
	
	drive->brakeLeft = drive->brakeRight = 0;
	LimitDrive127(drive);
	
	if (mAbsolute(drive->fwd) <= TURN_IN_PLACE_THRESH)
	{	
		drive->left  = +drive->turn;
		drive->right = -drive->turn;	
	}
	else	//Use DitherBraking
	{
		drive->left  = drive->fwd + drive->turn;
		drive->right = drive->fwd - drive->turn;
		
		if (drive->turn >= 0)
		{
			turnDir = CW;
			absTurn = drive->turn;
			inboardSide = (drive->fwd >= 0 ? RIGHT : LEFT);
		}
		else
		{
			turnDir = CCW;
			absTurn = -drive->turn;
			inboardSide = (drive->fwd >= 0 ? LEFT : RIGHT);
		}

		
		// If we're turning within the deadband, then we only scale
		// down the power on one side. If we exceed the deadband,
		// then we apply successively greater braking to that side.
		
		if (absTurn < BRAKETURN_DEADBAND)
		{
			int inboardSidePower = drive->fwd - drive->fwd * absTurn / BRAKETURN_DEADBAND;
			if (inboardSide == LEFT)
				drive->left  = inboardSidePower;
			else
				drive->right = inboardSidePower;
		}
		else	//Use Dithered braking
		{
			if (inboardSide == LEFT)
				drive->left  = 0;
			else
				drive->right = 0;
			
			brakeAmt = (absTurn - BRAKETURN_DEADBAND)
				* (maxBraking+1) / (128 - BRAKETURN_DEADBAND);
			// brakeAmt on range {0, maxBraking}
			
			if ((ditherPattern[gLoop.count & (DITHER_RES-1)]) < brakeAmt) {
				if (inboardSide == LEFT) {
					drive->brakeLeft = 1;
				} else {
					drive->brakeRight = 1;
				}
			}
			
			//printf("brkAmt %d/%d => L%d R%d\r", brakeAmt, maxBraking, drive->brakeLeft, drive->brakeRight);
		}
	}
}
//***********************************************************************
void Drive_Brake(Drive *drive) {
	drive->brakeLeft = drive->brakeRight = 1;
}
//***********************************************************************
void PrintDrive(Drive *d)
{
	printf("L=%+4.4d.%c R=%+4.4d.%c F=%+4.4d T=%+4.4d\r",
		(int)d->left, d->brakeLeft  ? 'B':' ',
		(int)d->right,d->brakeRight ? 'B':' ',
			(int)d->fwd,(int)d->turn);
}



//**********************************************************
int computeTurn(long bearingTicks)
{
	const int gain = 254;
//	const int gain = 170;
//	const int gain = OI_USERPOT3;	//FIXME: Should be hardwired,or eprom.
	const long encDiff = encoder_diff();
	long error = gain * (bearingTicks - encDiff);
	error = mDivideBy128(error);
	return Limit127(error);
}
//**********************************************************


//**********************************************************
//int computeTurnGyro(long bearingDegrees10)
//{
////	const int gain = 254;
////	const int gain = 170;
//	const int gain = OI_USERPOT3;	//FIXME: Should be hardwired,or eprom.
//	const long curBearing = Get_Gyro_Angle();
//	long error = gain * (bearingDegrees10 - curBearing);
//	error = mDivideBy128(error);
//	return Limit127(error);
//}