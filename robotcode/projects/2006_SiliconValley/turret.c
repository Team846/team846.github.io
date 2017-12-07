/*******************************************************************************
*	TITLE:		turret.c 
******************************************************************************/

#include "ifi_aliases.h"
#include "ifi_default.h"
#include "ifi_utilities.h"
#include "connections.h"
#include "controls.h"
#include <stdio.h>

#include "utilities.h"
#include "turret.h"
#include "tracking.h"
#include "camera.h"
//********************************************************************************
#define mTurretSpeed  47
//************ Prototypes ****************************************************
static void MoveTurretPulseMode(int TurretError);
static void MoveTurretServoMode(int TurretError);
static void DriveTurretLimitESC(char turretSpeed);


//************* End of prototypes ***************************************


//********************************************************************************
// DriveTurret:
// applies limit switches
// takes turret speed on {-127,127} and maps to {0,255}
// and sets turret pwm

static void DriveTurretLimitESC(char turretSpeed)
{
	//extra: turn on turret limit LED here
	
	if(gLoop.onSecond)
		printf("mTurretCWLimit: %d; mTurretCCWLimit: %d\r", (int)mTurretCWLimit, (int)mTurretCCWLimit);
	
	if (mTurretCWLimit && turretSpeed>0)
		return;	//turret already at limit

	if (mTurretCCWLimit && turretSpeed<0)
		return;	//turret already at limit

	//extra: turn off turret limit LED here
	
	mPWMTurretMotor = 127u + turretSpeed;
	//mPWMTurretMotor = removeESCDeadband(turretSpeed);
		if(gLoop.onSecond)
		printf("turretSpeed=%d, pwmTurretmotor=%d\r", (int) turretSpeed,(int) mPWMTurretMotor);

}
//********************************************************************************



void MoveTurretManual(void)
{
	overlay char turret=0;
	overlay char turretSpeed = mTurretSpeed;	//{0,255}->{0,127}
	static unsigned char BtnDownCount;
	int TurretError;
	
	//diagnostic to help evaluate user control w/ push buttons	
	if (mPanLeftSw || mPanRightSw)
		BtnDownCount++;
	else if (BtnDownCount)
	{
		printf("\rButton down for %d ticks\r",(int)BtnDownCount);
		BtnDownCount=0;
	}

	if (mPanLeftSw == mPanRightSw)	//both buttons the same
	{
		TurretError = get_pan_error();	
		MoveTurretAuto(TurretError);
		return;
	}
	else if (mPanLeftSw)
		turret = turretSpeed;
	else if (mPanRightSw)
		turret = -turretSpeed;

	DriveTurretLimitESC(turret);
}
//***********************************************************************


void MoveTurretAuto(int TurretError)
{
	enum { kPulseMode, kServoMode };
	int absTurretError=mAbsolute(TurretError);


	if (gLoop.onSecond)
	printf("in MoveTurretAuto, error=%d\r", (int)TurretError);

	if (absTurretError<2)
		return;
	if (absTurretError<14)
		MoveTurretPulseMode(TurretError);
	else
		MoveTurretServoMode(TurretError);
}
//***********************************************************************
void MoveTurretPulseMode(int TurretError)
{
	enum { kOnTime=2, kOffTime=10};

	static unsigned char timer=0;
	static unsigned char on=0;
#define kPulsePower 22
//	unsigned char kPulsePower = Get_Analog_Value(rc_ana_in15)>>4;
	if (TurretError==0) return;

	if (timer==0)
	{
		if (on)
		{
			on=0;
			timer = kOffTime;
		}
		else
		{
			on=1;
			timer = kOnTime;
		}
	}
	else timer--;

	if (gLoop.onSecond)
		printf("kPulsePower=%d\r",(int)kPulsePower);
	if (on)
		mPWMTurretMotor = TurretError> 0 ? 127u+kPulsePower:127u-kPulsePower;
}


//***********************************************************************
void MoveTurretServoMode(int TurretError)
{

	int TurretPWM=0;
		//get the max turret speed from the OI	
//	char maxTurretSpeed = mTurretSpeed >>1;	//{0,255}->{0,127}
#define kMaxTurretSpeed  127	//In Auto, go full speed
	static struct {
		int lastTurretError;
		unsigned char timer;	//max 255/38 = ~6 seconds
	} motorStewing = {0,0};

#define adjustTurretGain
#ifdef adjustTurretGain
//	char CameraPropGain = (int)(Get_Analog_Value(rc_ana_in16))>>4;
	char CameraPropGain = 18;
	if (gLoop.onSecond)
	{
		printf("CameraPropGain=%d\r",(int)CameraPropGain);
		printf("TurretError=%d\r",(int)TurretError);
	}
#else //adjustTurretGain
	#define CameraPropGain 23
#endif //adjustTurretGain

//	//Limit cycles that power is applied if error doesn't change.
//	//Protects against motor stewing when error is small
//	//and against extended moves where limit switch has failed.
//	//check against diff of '1' since error has 'jitter'
//	if (mAbsDiff(motorStewing.lastTurretError,TurretError)>1)
//	{
//		motorStewing.timer = 1000/26.2;	//reset timer to one second
//		motorStewing.lastTurretError = TurretError;
//	}
//	else if (motorStewing.timer != 0)
//		motorStewing.timer--;
//	else
//		TurretError = 0;	//set error to zero. We're not moving anyway


	TurretPWM = TurretError * CameraPropGain;
	TurretPWM = mDivideBy32(TurretPWM);


	TurretPWM = LimitRange(TurretPWM, -kMaxTurretSpeed, kMaxTurretSpeed);

	if (gLoop.onSecond)
	{
		printf("TurretPWM=%d\r",(int)TurretPWM);
	}
	
	DriveTurretLimitESC(TurretPWM);
}
//***********************************************************************




//********************************************************************************
char get_pan_error(void)
{
	char result;
	enum led_act action;
	if(T_Packet_Data.my == 0)
	{
		result = 0;
		action = OFF;
	}
	else
	{	
		char absError;
//		char trim = getPanOffsetFromOI();

		action = SLOWBLINK;
		//Relay1_green = 0;
		//Relay1_red = 0;

//		result = trim + (char) (T_Packet_Data.mx - PAN_TARGET_PIXEL_DEFAULT);

		result = getAdjustedPanTrackingError();

		absError=mAbsolute(result);
		
		if (absError <= 1)
			action = ON;
		else if ( absError <=12 ) 
			action = FASTBLINK;  

		//Doesn't take into account trim when printing.  Should it?
		if (gLoop.onSecond)
		{
			printf("Unadjusted Camera Error : %d,%d\r",
				(int)(char) (T_Packet_Data.mx - PAN_TARGET_PIXEL_DEFAULT),
				(int) (char)(T_Packet_Data.my - TILT_TARGET_PIXEL_DEFAULT));
			
			printf("ShootingSpeed: %d\r", (int)gBallLauncherSpeed);
			
			printf(" Pan Angle (degrees) = %d\r", (((int)PAN_SERVO - 124) * 65)/124);
			printf("Tilt Angle (degrees) = %d\r", (((int)TILT_SERVO - 144) * 25)/50);
		}
		//result = (char) (T_Packet_Data.mx - PAN_TARGET_PIXEL_DEFAULT);
		
	}
		led_activity(action);
		return result;
}

//********************************************************************************

void led_activity (enum led_act action) {
	static char cnt = 0;

	cnt++;
	
//	switch (action) 
//	{
//		case ON         : Relay1_green = 1; 
//							break;
//		//case 'OFF'        : Relay1_green = 0; 
//		//					break;
//		case FASTBLINK  : Relay1_green = cnt>>2;
//							break;
//		case SLOWBLINK  : Relay1_green = cnt>>4; 
//							break;
//		default  		  : Relay1_green = 0;
//	}

	switch (action) 
	{
		case ON         : mLEDCameraStatus = 1; 
							break;
		case FASTBLINK  : mLEDCameraStatus = cnt>>2;
							break;
		case SLOWBLINK  : mLEDCameraStatus = cnt>>4; 
							break;
		default  		  : mLEDCameraStatus = 0;
	}
}

