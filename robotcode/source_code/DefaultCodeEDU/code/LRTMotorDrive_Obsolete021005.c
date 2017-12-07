#include "lrtUtilities.h"
#include "ifi_aliases.h"
#include "OIFeedback.h"
//#include "lrtTypes.h"
#include "lrtMotorDrive.h"

motorPWMs gPWM;
gearBox gGearBox, gGearBoxLeft, gGearBoxRight;
enum { kHighgear, kLowgear };


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



int motorSpeedCIM(char velocity);
int motorSpeedFisherPrice(char velocity);

/********************************************************************************
* FUNCTION: ClearMotorPWMs()
* DESCRIPTION: obvious
********************************************************************************/
void ClearMotorPWMs(void)
{
	gPWM.cimL=0;	
	gPWM.cimR=0;
	gPWM.fpriceL=0;
	gPWM.fpriceR=0;
	gPWM.shoulder=0;
	gPWM.forearm=0;
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


void DriveLeftMotors(char in)
{
	int motorEMF;
	gGearBox = gGearBoxLeft;	//copy flags;
	motorEMF = motorSpeedCIM(EncoderRight.velocity);
	gPWM.cimL = limitDriveMotorCurrent(in, motorEMF, kMaxVrCIM);

	motorEMF = motorSpeedFisherPrice(EncoderRight.velocity);
	gPWM.fpriceL = limitDriveMotorCurrent(in, motorEMF, kMaxVrFisherPrice);
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
	if (motorEMF > 126) motorEMF = 126;
	else if (motorEMF < -126) motorEMF = -126;

	//average voltage applied to resistance of motor
	motorResistiveVoltage = voltagePWM * (127-motorEMF)/127;	//always positive

//	motorResistiveVoltage = MotorResistiveVoltage(voltagePWM, velocity, CIMorFP);
	
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
char torqueDrive(char TorqueIn, int motorEMF, const char limit )
{
	overlay int drive;
	overlay	char signbit;	//sign of Torque in.
	extern char gDriveLimitedFlag;  //used to indicate output < desired input;
									//		cleared in main loop

	if (0==TorqueIn)
		return 0;	//save alot of work...

	signbit = 0;
	if (TorqueIn <= 0)
	{
		signbit=1;		//save sign of TorqueIn
		TorqueIn= -TorqueIn;	//abs value
		motorEMF = -motorEMF;	//Applied V reversed relative to EMF
	}

	//limit motorEMF to safe computation values
	if (motorEMF > 126) motorEMF = 126;
	else if (motorEMF < -126) motorEMF = -126;


	//TorqueIn is positive &  motorEMF<127 always ==> Drive is positive;
	// '127' is full battery voltage opposing EMF
	drive = TorqueIn * (int) limit / (127 - (int) motorEMF);
	

	//check if limit reached and signal Operator Interface
	if (drive > 127)
	{
		drive=127;	//limit output
		gDriveLimitedFlag=1;
	}	
	
	if (signbit) drive = -drive;	//restore sign
	return drive;
}

/********************************************************************************
* FUNCTION: MotorResistiveVoltage
*
* DESCRIPTION: 
*  computes applied voltage - motorEMF
*  returns value on approx range {-256,+256}, which exceeds range of char
********************************************************************************/
int MotorResistiveVoltage(char voltagePWM, char velocity, char CIMorFP)
{
	int motorEMF;
	int result;
	if (CIMorFP == kCIM)
		motorEMF = motorSpeedCIM(velocity);
	else //COMorFP == kFisherPrice
		motorEMF = motorSpeedFisherPrice(velocity);
	
	if (voltagePWM < 0)
		motorEMF = -motorEMF;	//applied voltage reversed relative to EMF

	result = voltagePWM * (127-motorEMF)/127;
	return result;
}
/********************************************************************************/

#ifdef CUT
unsigned char limitDriveMotorCurrent(char drivePC, char velocity )
{

	short long motorEMF;
	int motorResistiveVoltage;
	char quadrant1_3;

	if (drivePC>=0)
#error

	motorEMF = (long) velocity * (long) kKvMainDrive256;
	motorEMF = mDivideByPowerOf2(motorEMF,8+7);	//divide by 256	= 2^15


	motorResistiveVoltage = pwmIn - motorEMF;

	motorResistiveVoltage = LimitRange(motorResistiveVoltage, -kMaxVr, kMaxVr);
	pwmIn = motorEMF + motorResistiveVoltage;

}
#endif //CUT






//EXPERIMENTAL
void limit(int motorResistiveVoltage, char *dynamicLimit);
void limit(int motorResistiveVoltage, char *dynamicLimitArg)
{
	char limit = *dynamicLimitArg;

	if (motorResistiveVoltage<0)
		motorResistiveVoltage = -motorResistiveVoltage;

	if (motorResistiveVoltage > kMaxVr) //invoke limit when input >kMaxVr
	{
		if (motorResistiveVoltage > limit)
		{
			if (limit > kMaxVr)
				limit--; //go down to a min of kMaxVr
			motorResistiveVoltage = limit;
		}
	}
	else if (limit<127)	// relax the dynamic limit to it's max value
		limit++; //go up to a max of 127
	*dynamicLimitArg = limit;	//copy back the limit
}	
/*****************************************************************/

//29.90		25.09		8.37		7.03

int motorSpeedCIM(char velocity)

{
	overlay int result;
	overlay char signbit=0;
	if (velocity < 0)
	{
		signbit=1;
		velocity = -velocity;
	}

	if (gGearBox.inHighGear)
		result = (int)velocity * 17>>2;	//mult by 127/29.90
	else if (gGearBox.inLowGear)
		result = (int)velocity * 243>>4;
	else
		result=0;	//error. between gears.

	if (result > 126) result = 126;	//limit in case of overspeed
		//keep below 127, to avoid potential zero divide later.


	if (signbit) result=-result;
	return result;
}
/*****************************************************************/
int motorSpeedFisherPrice(char velocity)
{
	overlay int result;
	overlay char signbit=0;
	if (velocity < 0)
	{
		signbit=1;
		velocity = -velocity;
	}

	if (gGearBox.inHighGear)
		result = (int)velocity * 81 >> 4;	//mult by 127/
	else if (gGearBox.inLowGear)
		result = (int)velocity * 289 >> 4;
	else
		result=0;	//error. between gears.


	if (result > 126) result = 126;	//limit in case of overspeed
		//keep below 127, to avoid potential zero divide later.

	if (signbit) result=-result;
	return result;
}
/*****************************************************************/
/*****************************************************************/
