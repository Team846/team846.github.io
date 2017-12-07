
#include "common.h"
#include "lift.h"

#ifdef ROBOT_2007

static struct {
	int PWM;
	int pos;
	int basePos;		// the pot value at the lower limit switch
	int target;
	int timeout;
	
	// Stall protection variables
	int stallPos;
	int stallTime;
	int coolTime;
//	char locked;		// Stall protection activated, and someone is still trying to apply power. Will not unlock until power is removed.
} sLift = {0, 0, 66, -1, 0, 0, 0, 0};

#define MANUAL_LIFT_SPEED_UP	127
#define MANUAL_LIFT_SPEED_DOWN	45

#define LIFT_TIMEOUT			2500 / 26.2

// Experimentally, 106 / 32 worked well for the gain. [dcl]
#define LIFT_GAIN	60
	//mDivideBy32(106)
	//mDivideBy64(Get_Analog_Value(mUserPot1Port))		// max=32

#define LIFT_BURN_TIME			200 / 26.2
#define LIFT_COOL_TIME			3000 / 26.2
#define LIFT_SHORT_COOL_TIME	500 / 26.2
#define LIFT_STALL_THRESHOLD	(int)(0.25 * LIFT_UNITS_PER_INCH)

static void doClosedLoopLift(void);
static void liftPresettingBtns(void);

int positions[4] = {LIFT_L0, LIFT_L1, LIFT_L2, LIFT_L3};

// ************************************************************************************************

#define SMART_GOAL_LEVELS

void doLiftBtns(void)
{
	sLift.PWM = 0;
	
	if (mBtnSetLiftPreset) {
		liftPresettingBtns();
	} else {
		// Handle auto lift buttons first so that manual buttons can override
		if (mBtnLiftL0) {		// only in manual mode. Ready Pickup uses same button in actions.c
			moveLiftToLevelRingerUp(0);
			setFingerPos(G_OPEN);
			setArmPos(G_DOWN);
			//setGripperTrap(G_ON);
			setGripperTrap(G_OFF);
		}
		if (mBtnLiftL1) {		// all modes.
			#ifdef SMART_GOAL_LEVELS
				if (getArmPos() == G_DOWN)
					moveLiftToLevelRingerFlat(1);
				else
					moveLiftToLevelRingerUp(1);
			#else
				setArmPos(G_UP);
				moveLiftToLevelRingerUp(1);
			#endif
			setFingerPos(G_CLOSED);		// shouldn't be open when raising lift anyways
			setGripperTrap(G_OFF);
		}
		if (mBtnLiftL2) {		// all modes.
			#ifdef SMART_GOAL_LEVELS
				if (getArmPos() == G_DOWN)
					moveLiftToLevelRingerFlat(2);
				else
					moveLiftToLevelRingerUp(2);
			#else
				setArmPos(G_UP);
				moveLiftToLevelRingerUp(2);
			#endif
			setFingerPos(G_CLOSED);		// shouldn't be open when raising lift anyways
			setGripperTrap(G_OFF);
		}
		if (mBtnLiftL3) {		// all modes.
			#ifdef SMART_GOAL_LEVELS
				if (getArmPos() == G_DOWN)
					moveLiftToLevelRingerFlat(3);
				else
					moveLiftToLevelRingerUp(3);
			#else
				setArmPos(G_UP);
				moveLiftToLevelRingerUp(3);
			#endif
			setFingerPos(G_CLOSED);		// shouldn't be open when raising lift anyways
			setGripperTrap(G_OFF);
		}
	}
	
	// Handle manual lift buttons
	if (mBtnLiftUp)
		sLift.PWM = MANUAL_LIFT_SPEED_UP;
	if (mBtnLiftDown)
		sLift.PWM = -MANUAL_LIFT_SPEED_DOWN;
	if (mBtnLiftUp || mBtnLiftDown) {
		sLift.timeout = 0;		// disable closed loop control until user requests it again
	}
	
	if (mBtnLiftPlus12)
		moveLift(12 * LIFT_UNITS_PER_INCH);
	
	if (mBtnPotLift) {
		int potVal = ((int) Get_Analog_Value(mUserPot1Port)) - 512;
		sLift.PWM = mDivideBy4(potVal);
		printf("Pot Lift: %d\r\n", sLift.PWM);
	}
}
//********************************************************************************

// Allow user to set height presets by pressing mBtnSetLiftPreset + mBtnLiftL0/L1/L2/L3
static void liftPresettingBtns(void) {
	static char liftBtnDebouncer = 0;
	int liftpos = getLiftPos();
	
	if (!mBtnSetLiftPreset) return;
	
//	printf("Lift Preset Combo pressed!\r\n");
	
	if (mBtnLiftL0 && !liftBtnDebouncer) {
		EEPROM_write(kEPROMAdr_LiftL0, ((char*) &liftpos)[0]);		//save across resets
		EEPROM_write(kEPROMAdr_LiftL0+1, ((char*) &liftpos)[1]);	//save across resets
		positions[0] = liftpos;
		printf("Reset Level 0 to %d\r\n", liftpos);
	}
	if (mBtnLiftL1 && !liftBtnDebouncer) {
		EEPROM_write(kEPROMAdr_LiftL1, ((char*) &liftpos)[0]);		//save across resets
		EEPROM_write(kEPROMAdr_LiftL1+1, ((char*) &liftpos)[1]);	//save across resets
		positions[1] = liftpos;
		printf("Reset Level 1 to %d\r\n", liftpos);
	}
	if (mBtnLiftL2 && !liftBtnDebouncer) {
		EEPROM_write(kEPROMAdr_LiftL2, ((char*) &liftpos)[0]);		//save across resets
		EEPROM_write(kEPROMAdr_LiftL2+1, ((char*) &liftpos)[1]);	//save across resets
		positions[2] = liftpos;
		printf("Reset Level 2 to %d\r\n", liftpos);
	}
	if (mBtnLiftL3 && !liftBtnDebouncer) {
		EEPROM_write(kEPROMAdr_LiftL3, ((char*) &liftpos)[0]);		//save across resets
		EEPROM_write(kEPROMAdr_LiftL3+1, ((char*) &liftpos)[1]);	//save across resets
		positions[3] = liftpos;
		printf("Reset Level 3 to %d\r\n", liftpos);
	}
	
	if (mBtnLiftL0 || mBtnLiftL1 || mBtnLiftL2 || mBtnLiftL3)
		liftBtnDebouncer = 1;
	else
		liftBtnDebouncer = 0;
	
	if (user_display_mode)
		gUserByteOI = liftpos;
}
//********************************************************************************

void ReadLiftPresetsFromEPROM(void) {
	( (char*) &(positions[0]) )[0] = EEPROM_read(kEPROMAdr_LiftL0);
	( (char*) &(positions[0]) )[1] = EEPROM_read(kEPROMAdr_LiftL0+1);
	( (char*) &(positions[1]) )[0] = EEPROM_read(kEPROMAdr_LiftL1);
	( (char*) &(positions[1]) )[1] = EEPROM_read(kEPROMAdr_LiftL1+1);
	( (char*) &(positions[2]) )[0] = EEPROM_read(kEPROMAdr_LiftL2);
	( (char*) &(positions[2]) )[1] = EEPROM_read(kEPROMAdr_LiftL2+1);
	( (char*) &(positions[3]) )[0] = EEPROM_read(kEPROMAdr_LiftL3);
	( (char*) &(positions[3]) )[1] = EEPROM_read(kEPROMAdr_LiftL3+1);
}

//********************************************************************************

static void printPresets(void) {
	static char printed = 0;
	if (printed) return;
	printf("READ: Levels %d, %d, %d, %d\r\n", positions[0], positions[1], positions[2], positions[3]);
	printed = 1;
}

//********************************************************************************

static void liftStallProtection(void) {
	int relpos = getLiftPos();

#ifdef SERIAL_LED
	mSLEDLiftStalled = 0;
	mSLEDLiftCooling = 0;
#endif //SERIAL_LED

#define DG
#ifdef DG

	//Look for stall condition, and if found, set cool down timer and disable motor.
 
	if (0==sLift.coolTime && 0!=sLift.PWM) {			
		if (mAbsDiff(sLift.pos, sLift.stallPos) < LIFT_STALL_THRESHOLD) { //power applied but no motion?
			if (++sLift.stallTime > LIFT_BURN_TIME) {			 		// ... for some time
				printf("Lift: Motor on fire! Motor on fire! Disabled.\r\n");
				
				
				#define VARIABLE_COOL_TIME
				#ifdef VARIABLE_COOL_TIME	// Cool down timer depends on lift position and direction: [dcl]
					if (	( relpos < (int)(2.0 * LIFT_UNITS_PER_INCH)  && sLift.PWM < 0 ) ||
							( relpos > (int)(30.0 * LIFT_UNITS_PER_INCH) && sLift.PWM > 0 )	) {
						sLift.coolTime = LIFT_COOL_TIME;		// long cool time if near limits
					} else {
						sLift.coolTime = LIFT_SHORT_COOL_TIME;	// short cool time if away from limits
					}
				#else
					sLift.coolTime = LIFT_COOL_TIME;				// Disable by setting cool down timer
				#endif //VARIABLE_COOL_TIME
				
				sLift.stallTime = 0;
			}
		}
		else {
			sLift.stallTime = 0;
			sLift.stallPos = sLift.pos;		//save the last known position
		}
	}

	if (sLift.coolTime>0) {
		--sLift.coolTime;
#ifdef SERIAL_LED
		mSLEDLiftCooling=1;	//mSLEDLiftStalled not used.
#endif //SERIAL_LED
		sLift.PWM = 0;
	}
	
#else //DCL's code

	if (sLift.PWM) {															// If the motor is being powered ...
		if (mAbsDiff(sLift.pos, sLift.stallPos) < LIFT_STALL_THRESHOLD) {		// ... and has not moved a total of STALL_THRESHOLD units ...
			if (sLift.stallTime++ > LIFT_BURN_TIME) {							// ... for BURN_TIME cycles ...
				printf("Lift: Motor on fire! Motor on fire! Disabled.\r\n");	// ... Motor Is On Fire!!!
				sLift.PWM = 0;													// Disable it, quick!
				mSLEDLiftStalled = 1;
			}
		} else {																// If the motor has moved at least STALL_THRESHOLD ...
			sLift.stallTime = 0;												// The motor is not stalled.
			sLift.stallPos = sLift.pos;											// Update the last known position.
		}
	}
	
	if (sLift.PWM == 0 && sLift.stallTime) {				// If the motor is off after being stalled...
		mSLEDLiftCooling = 1;
		if (sLift.coolTime++ > LIFT_COOL_TIME) {			// ... for COOL_TIME cycles ...
			printf("Lift: Motor cooled.\r\n");				// ... the motor is ice cold.
			sLift.stallTime = 0;		// Motor is no longer stalling; we can apply power now.
			sLift.coolTime = 0;
			mSLEDLiftStalled = 0;
			mSLEDLiftCooling = 0;
		}
	}
	if (sLift.PWM)
		sLift.coolTime = 0;			// If power is being applied, the motor is not cooling.
		
#endif	//DG
}

void doLift(void)
{
	int relpos;
	sLift.pos = Get_Analog_Value(mLiftPotPort);		// 0-1023
	relpos = getLiftPos();
	
	printPresets();

//	if (sLift.basePos == -1)
//		calibrateLift();

	if (gLoop.onSecondA) {
		printf("Raw Lift Position: %d\r\n", sLift.pos);
	}

	if (mSwLimitLiftLow) {
		sLift.basePos = sLift.pos;		// record base position when lower limit switch
		if (gLoop.onSecondA)
			printf("Lift: Recalibrated, base pos: %d\r\n", sLift.pos);
	}
	
	doClosedLoopLift();



	//	Limit speed on descent if nearing bottom.
	#define kSlowDecent -30		// found using pot lift control, can't go slower
	
	if(gLoop.onSecondA) {
		printf("Pos: %d, Base %d, Diff %d\r\n", sLift.pos, sLift.basePos, relpos);
		//printf("8in = %d ticks in above base\r\n", (int) (8.0 * LIFT_UNITS_PER_INCH));
		//printf("sliftpwm=%d; kslow=%d\r\n", sLift.PWM, kSlowDecent);
	}
	

//#define PAUSE1		((int)(500/26.2))
#ifdef PAUSE1
	{
		static int pause1 = 0;
		if (relpos < (int)(8.0 * LIFT_UNITS_PER_INCH)) {
			if (sLift.PWM < 0) {				// trying to go down..
				if (pause1 < PAUSE1) {		// have we not stopped yet?
					printf("Pause 1\r\n");
					sLift.PWM = 0;				// stop for STOP1_TIME
				} else {
					printf("Slow 1\r\n");
					sLift.PWM = kSlowDecent;	//go slow
				}
			}
			
			if (sLift.PWM == 0 && pause1 < PAUSE1)
				pause1++;
		} else {
			pause1 = 0;
		}
	}
#endif //PAUSE1

//#define PAUSE2		((int)(500/26.2))
#ifdef PAUSE2
/*	{
		static int pause2 = 0;
		if (relpos > (int)(30.0 * LIFT_UNITS_PER_INCH)) {
			if (sLift.PWM > 0) {				// trying to go up..
				if (pause2 < PAUSE2) {		// have we not stopped yet?
					printf("Pause 2\r\n");
					sLift.PWM = 0;				// stop for STOP1_TIME
				} else {
					printf("Slow 2\r\n");
					sLift.PWM = kSlowAscent;	//go slow
				}
			}
			
			if (sLift.PWM == 0 && pause2 < PAUSE2)
				pause2++;
		} else {
			pause2 = 0;
		}
	}*/
#endif //PAUSE2

	if (sLift.PWM < 0 && relpos < (int)(2.0 * LIFT_UNITS_PER_INCH)) {
		printf("Stopped - BOTTOM (pot)\r\n");
		sLift.PWM = 0;	//Stop; Will need to be adjusted. Should light an LED.
	}

//#define LIFT_TENSIONING
#ifdef LIFT_TENSIONING
#define TENSIONUP_TIME	((int)4)
#define TENSIONUP_POWER	((int)(0.5*127))
	{
		static char tensioned = 0;
		static char tensionTimer = -1;
		if (sLift.PWM > 0) {
			if (!tensioned) {
				if (tensionTimer == -1) {
					printf("Start tensioning\r\n");
					tensionTimer = TENSIONUP_TIME;
				}
				
				if (--tensionTimer > 0) {
					printf("Tensioning lift chain!\r\n");
					sLift.PWM = TENSIONUP_POWER;
				} else {
					printf("Tensioning done\r\n");
					tensioned = 1;
				}
			}
		} else {
			tensioned = 0;
			tensionTimer = -1;
		}
	}
#endif // LIFT_TENSIONING

	//Limit Switches
	if (mSwLimitLiftHigh && sLift.PWM > 0) sLift.PWM = 0;
	if (mSwLimitLiftLow && sLift.PWM < 0) sLift.PWM = 0;
	
	if (sLift.PWM < 0 && relpos < (int)(2.0 * LIFT_UNITS_PER_INCH)) {
		printf("POT MISALIGNED!\r\n");
		sLift.PWM = 0;	//Stop; Will need to be adjusted. Should light an LED.
	}
	
	if (mSwLimitLiftLow && sLift.PWM < 0) sLift.PWM = 0;
	
	liftStallProtection();		// needs to go after everything that can change PWM output
	
	mPWMLift = removeESCDeadband( sLift.PWM );
	if (mPWMLift != 127)
		printf("Applying %d to lift (%d)\r\n", (int)mPWMLift - 127, sLift.PWM);
}

// ************************************************************************************************

int moveLiftToTarget(int to) {
	printf("Setting Target Lift Position To %d\r\n", to);
	sLift.target = to + sLift.basePos;
	sLift.timeout = LIFT_TIMEOUT;
}

// ************************************************************************************************

int getLiftPos(void) {
	return sLift.pos - sLift.basePos;
}

// ************************************************************************************************

void moveLift(int x) {
	moveLiftToTarget(getLiftPos() + x);
}

// ************************************************************************************************

void calibrateLift(void) {
	sLift.target = 0;
	sLift.timeout = LIFT_TIMEOUT;
}

// ************************************************************************************************

void moveLiftToLevelRingerUp(char level) {
	moveLiftToTarget(positions[level]);
}

// ************************************************************************************************

void moveLiftToLevelRingerFlat(char level) {
	moveLiftToTarget(positions[level] + LIFT_FLAT_SCORING_OFFSET);
}

// ************************************************************************************************

static void doClosedLoopLift(void)
{
	int error;

	if (sLift.timeout > 0)
		sLift.timeout--;
	else
		return;
	
	error = sLift.pos - sLift.target;		// [-1023, 1023]
	error = mDivideBy32(error);

	sLift.PWM = -error * LIFT_GAIN;

	if (gLoop.onSecondA)
		printf("doClosedLoopLift: Gain %d; Error: %d, Output: %d\r\n", LIFT_GAIN, error, sLift.PWM);

#define LIFT_MINPWM_UP		80
	if (sLift.PWM > 0 && sLift.PWM < LIFT_MINPWM_UP)
		sLift.PWM = LIFT_MINPWM_UP;
	// No min down

	/*if (mAbsolute(error) < LIFT_MAX_ERROR) {
		sLift.target = -1;
	}*/
}

#endif // ROBOT_2007
