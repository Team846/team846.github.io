#include "ifi_picdefs.h"
#include "ifi_aliases.h"
#include "lrtConnections.h"
#include "lrtMotorDrive.h"
#include "lrtUtilities.h"
#include "printf_lib.h"

#define kTimerCount (4*38)	//about 1 seconds  - max value=255 (8-bit)
#define kRelayPulse 38	// about 1/2 sec

#if kTimerCount-kRelayPulse <= 0
#error //make sure we don't inadvertantly set the relay to too short a pulse (below)
#endif


/* ShiftHighGear() initiates a shift into high gear.
 * The work is done in DoShift
 */

void ShiftHighGear(void)
{
	if (gGearBox.shifting && gGearBox.cmdGearObjective==kHighGear)
		return;	//already asked to shift to this gear; don't reset attempt

	gGearBox.shifting=1;	
	gGearBox.cmdGearObjective=kHighGear;
//	gGearBox.shiftTimer = kTimerCount;	//one second to shift
	gGearBox.shiftTimer = kRelayPulse;	
}

/*****************************************************************/
void ShiftLowGear(void)
{
	if (gGearBox.shifting && gGearBox.cmdGearObjective==kLowGear)
		return;	//already asked to shift to this gear; don't reset attempt

	gGearBox.shifting=1;	
	gGearBox.cmdGearObjective=kLowGear;
//	gGearBox.shiftTimer = kTimerCount;	//one second to shift
	gGearBox.shiftTimer = kRelayPulse;	
}


//#define FancyShift
#ifndef FancyShift
/*****************************************************************/
void DoShift(void)
{
	mGearShiftHighRelay = mGearShiftLowRelay = 0;	//turn off both solenoids
	if (gGearBox.shiftTimer > 0) gGearBox.shiftTimer--;
	else gGearBox.shifting=0;	
	
	if (gGearBox.shifting==0)
		return;

	mGearShiftHighRelay = (gGearBox.cmdGearObjective==kHighGear);	//set relay values
	mGearShiftLowRelay = (gGearBox.cmdGearObjective==kLowGear);

printf("%d: mGearShiftHighRelay=%d, mGearShiftLowRelay=%d\n", (int)gGearBox.shiftTimer,
	(int)mGearShiftHighRelay, (int)mGearShiftLowRelay);


	if (gGearBox.cmdGearObjective==kLowGear && mLeftLowGearSw && mRightLowGearSw)
		gGearBox.shiftTimer=0;

	else if (gGearBox.cmdGearObjective==kHighGear && mLeftHighGearSw && mRightHighGearSw)
		gGearBox.shiftTimer=0;
}








#else //FancyShift
void DoShift(void)	//call GetMotorSpeeds() prior to calling DoShift
{
#define _HaveGearBoxSensors

	mGearShiftHighRelay = mGearShiftLowRelay = 0;	//turn off both solenoids

#ifdef _HaveGearBoxSensors
	//do this, even if not shifting so we can display status on LED's
	gGearBox.left.inLowGear = mLeftLowGearSw;	//copy the gearbox sensor positions
	gGearBox.left.inHighGear = mLeftHighGearSw;	//for easier access to variables
	gGearBox.right.inLowGear = mRightLowGearSw;
	gGearBox.right.inHighGear = mRightHighGearSw;
#endif //_HaveGearBoxSensors

	if (gGearBox.shifting==0) return;



	
if (gGearBox.shiftTimer !=0) printf("shiftTimer=%d\n",(int)gGearBox.shiftTimer);


	//only power relay solenoid for first 1/2 sec.
	if (gGearBox.shiftTimer > kTimerCount-kRelayPulse)
	{
		mGearShiftHighRelay = gGearBox.cmdGearObjective;	//set relay values
		mGearShiftLowRelay = !gGearBox.cmdGearObjective;
	}
#ifndef _HaveGearBoxSensors
	else gGearBox.shiftTimer=0;	//end shift at pulse if no sensors
#endif
	
	//evaluate gearbox objective

#ifdef _HaveGearBoxSensors
	if (gGearBox.cmdGearObjective==kLowGear)
	{
		gGearBox.left.inObjectiveGear = gGearBox.left.inLowGear;
		gGearBox.right.inObjectiveGear = gGearBox.right.inLowGear;
	}
	else  //then  (gGearBox.cmdGearObjective==kHighGear)
	{
		gGearBox.left.inObjectiveGear = gGearBox.left.inHighGear;
		gGearBox.right.inObjectiveGear = gGearBox.right.inHighGear;
	}

	if (gGearBox.left.inObjectiveGear && gGearBox.right.inObjectiveGear)
	{
		gGearBox.shifting=0;	//early success on shift; leave timer to see how many cycles remained.
		return;
	}
#endif //_HaveGearBoxSensors


	if (gGearBox.shiftTimer==0)
	{
		gGearBox.shifting=0;
		//failure -- timeout
		return;
	}
	
	gGearBox.shiftTimer--;

	//If we are still in old gear, match speed to old gear
	//otherwise, match to new gear (the objective)

	

#ifdef HaveGearBoxSensors	//if sensors on shifters, be intelligent...
	if (gGearBox.cmdGearObjective==kHighGear)
	{
		gGearBox.right.cmdGearSpeedToMatch = (gGearBox.right.inLowGear ? kLowGear:kHighGear);
		gGearBox.left.cmdGearSpeedToMatch = (gGearBox.left.inLowGear ? kLowGear:kHighGear);
	}
	else	//trying to shift to low gear
	{
		gGearBox.right.cmdGearSpeedToMatch = (gGearBox.right.inHighGear ? kHighGear:kLowGear);
		gGearBox.left.cmdGearSpeedToMatch = (gGearBox.left.inHighGear ? kHighGear:kLowGear);
	}
#else 	//NoGearBoxSensors;
	gGearBox.right.cmdGearSpeedToMatch = gGearBox.cmdGearObjective;
	gGearBox.left.cmdGearSpeedToMatch = gGearBox.cmdGearObjective;
#endif 	//NoGearBoxSensors

	


	GetMotorSpeeds();	//calculate the motor speeds to match.  Results in gMotorSpeed

	//calculate pwm's with reduced current limits.
	gPWM.cimL = limitDriveMotorCurrent(gMotorSpeed.cimL, gMotorSpeed.cimL, kMaxVrCIM/2);
	gPWM.fpriceL = limitDriveMotorCurrent(gMotorSpeed.fpriceL, gMotorSpeed.fpriceL, kMaxVrFisherPrice/2);
	gPWM.cimR = limitDriveMotorCurrent(gMotorSpeed.cimR, gMotorSpeed.cimR, kMaxVrCIM/2);
	gPWM.fpriceR = limitDriveMotorCurrent(gMotorSpeed.fpriceR, gMotorSpeed.fpriceR, kMaxVrFisherPrice/2);
}


#endif //fancyShift
