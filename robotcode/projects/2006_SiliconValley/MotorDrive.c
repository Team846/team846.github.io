#include "utilities.h"
#include "ifi_aliases.h"
#include "MotorDrive.h"
#include "connections.h"

motorPWMs gPWM;
motorSpeed gMotorSpeed;

//gearBox gGearBox, gGearBoxLeft, gGearBox.right;
gearBoxes gGearBox;
char gDriveLimitedFlag=0;

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
* DESCRIPTION: obvious
********************************************************************************/
void ClearMotorPWMs(void)
{
	gPWM.cimL=0;	
	gPWM.cimR=0;
}


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
	gMotorSpeed.cimL =		motorSpeedCIM( EncoderLeft.velocity,			gGearBox.left.cmdGearSpeedToMatch);
	gMotorSpeed.cimR =		motorSpeedCIM( EncoderRight.velocity,			gGearBox.right.cmdGearSpeedToMatch);
	
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
int motorSpeedCIMT(unsigned long timestamp, char signDirection, char gear )
{
	overlay int result;
	
	if (timestamp > 0x0000FFFFL)
		return 0;	//really long time between ticks
	
	if (0==timestamp) return 0;	//error, avoid zero divide.

	
	if (gear==kHighGear)
		result = ((unsigned short long)127 * kToHighGearCIM)/ (unsigned int)timestamp;
	else	//in LowGear
		result = ((unsigned short long)127 * kToLowGearCIM)/ (unsigned int)timestamp;


	if (result > 126) result = 126;	//limit in case of overspeed
		//keep below 127, to avoid potential zero divide later.

	if (signDirection) result=-result;
	return result;
}
/*****************************************************************/
