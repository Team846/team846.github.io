/********************************************************************************
* FILE NAME: motorSim.c
*
* DESCRIPTION: simulate robot functions
* given input PWM's, 
********************************************************************************/

#include "motorSim.h"
#include "lrtMotorDrive.h"
#include "lrtUtilities.h"
#include "arm.h"
#include "lrtConnections.h"
#include "printf_lib.h"

	//the change in the number of ticks per 26.2 ms for each 0.1V ('1' in pwm)
	//see excel spreadsheet 'program constants'
#define kDeltaEncoderTicks2_16 ((0.02827 * 256 * 256L))


#ifdef SIM

static int motorResistiveVoltage(char voltagePWM, int motorEMF);

/* 
what are the state variables?
 * velocity
 * position
 * we can use the encoder values
 * T = (Vin-EMF) * K - drag
 * T = I * w (akin to F=ma) so vn = M * a * deltaT + vo
 * newTicks = vn
*/


/********************************************************************************
* FUNCTION: SIMmotorDrive
*
* DESCRIPTION: 
*  updates the Encoder positions as a function of the left and right pwm values
*  uses a model to simulate the motors driving the mass of the robot
*   computes the resistive voltage on the motor as a function of it's current Vel
*   and applied voltage

* Limitations:
*  Doesn't include any drag
*  Doesn't include any cross-coupling between the left and right drives
*  Model based on a single motor on each side, although constants are
*   based on aggregate of CIM and FP combined.
* May overestimate traveled distance, since a change in vel only contributes vel/2
*  in distance.  However, since instantaneous velocity is based on 
********************************************************************************/
void SIMmotorDrive(void)
{

	static struct { int left,right; } deltaVelRemainder = {0,0};	//store fractional ticks
#define kDrag256 ((int)256*.5)	//reduction in Vel/cycle
	struct {
		long voltage;
	} left,right;

	long deltaVelocity;
	int velocity256;	//scaled by 256
	int velocityAvg;	//scaled by 256

//must call getmotorSpeeds() prior to have accurate voltage,
//  or calculate speeds here.
	left.voltage = motorResistiveVoltage(gPWM.cimL, gMotorSpeed.cimL);
	right.voltage = motorResistiveVoltage(gPWM.cimR, gMotorSpeed.cimR);

	//compute the change in velocity due to the 'resistive' voltage applied to the motor
	//perform the calc. with greater precision and save the fractional ticks for the next loop.

//left side
	deltaVelocity = left.voltage * kDeltaEncoderTicks2_16;
	deltaVelocity = mDivideBy256(deltaVelocity);
	deltaVelocity += deltaVelRemainder.left;
	
//effect of drag -rudimentary at best
	velocity256 = EncoderLeft.velocity*(int)256 + deltaVelocity;
	if (velocity256 > kDrag256) velocity256 -= kDrag256;
	else if (velocity256 < -kDrag256) velocity256 += kDrag256;
	else velocity256=0;

	deltaVelRemainder.left = mModPowerOf2(velocity256,8);	// remainder 256	
	EncoderLeft.posNow += mDivideBy256(velocity256); // divide by 256
	


//right side
	deltaVelocity = right.voltage * kDeltaEncoderTicks2_16;
	deltaVelocity = mDivideByPowerOf2(deltaVelocity,8);		//divide by 256; discard
	deltaVelocity += deltaVelRemainder.right;

//effect of drag -rudimentary at best
	velocity256 = EncoderRight.velocity*(int)256 + deltaVelocity;
	if (velocity256 > kDrag256) velocity256 -= kDrag256;
	else if (velocity256 < -kDrag256) velocity256 += kDrag256;
	else velocity256=0;

	deltaVelRemainder.right = mModPowerOf2(velocity256,8);	// remainder 256
	EncoderRight.posNow += mDivideBy256(velocity256); // divide by 256
}




/********************************************************************************
* FUNCTION: GetSimulatedLegPositions()/SimulateLegMotors()
*
* DESCRIPTION: Simulate operation of the front and rear lifting legs
*
* If the front PWM >127, increment the leg position
* If the front PWM < 127 decrement the leg position
* To improve precision, position is stored as {0,4095}
*  and must be mapped to {0-1023} which is simply done by
*  a 2 bit right shift (>>2) ...effectively same but faster
* than a divide by 4.
* improved precision allows for better control of up/down rates
* since otherwise we are contrained to an integer/26.2ms
********************************************************************************/
static char pPrintChange = 1;	//flag to print

static struct { int forearm,shoulder; } pSimPosition = { 2048, 2048 };

void GetSimulatedArmPositions(void)
{
	extern char pPrintChange;	//private flag; set in it SimulateLegMotors()

	gArm.shoulder.curPosition = pSimPosition.forearm >>2;
	gArm.forearm.curPosition = pSimPosition.shoulder >>2;

	if (pPrintChange && gLoop.onSecond)
	{
		pPrintChange=0;	//clear flag
		printf("ADC ArmPos (Shoulder/Forearm) %d / %d",
			(int) gArm.shoulder.curPosition, (int) gArm.forearm.curPosition);
		PrintTime();
	}
}


void SimulateArmMotors(void)
{
//rates are calculated as 
//  (total travel in ticks)/(seconds for full travel * 38 cycles/sec)
// for 5 seconds to travel range of 512*4, gives 11/cycle
// (Range of legs due to hard limits is from about 500 to 1023) 
//no accounting for power or inertia -- needs work.

#define kUpRate 4	//11
#define kDownRate 4	//11
	char changed;
	extern char pPrintChange;	//private flag; used to print in GetSimulatedLegPositions()
	
	changed =1;
	if (gPWM.shoulder>0 && pSimPosition.shoulder<(4096-kUpRate))
		pSimPosition.shoulder += kUpRate;
	else if (gPWM.shoulder<0 && pSimPosition.shoulder>=kUpRate)
		pSimPosition.shoulder -= kDownRate;
	else changed=0;
	pPrintChange |= changed;
	

	changed =1;
	if (gPWM.forearm>0 && pSimPosition.forearm<(4096-kUpRate))
		pSimPosition.forearm += kUpRate;
	else if (gPWM.forearm<0 && pSimPosition.forearm>=kUpRate)
		pSimPosition.forearm -= kDownRate;
	else changed=0;
	pPrintChange |= changed;
}


static int motorResistiveVoltage(char voltagePWM, int motorEMF)
{
	overlay int motorResistiveVoltage;
	overlay char signbit;

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
	if (signbit) motorResistiveVoltage = -motorResistiveVoltage;
	return motorResistiveVoltage;
}

#endif //SIM defined
