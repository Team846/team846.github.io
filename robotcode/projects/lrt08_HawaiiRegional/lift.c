#include "common.h"
#include "lift.h"
#include "adc.h"
#include "lcd.h"

#define LIFT_OIUP_SPEED 127 //((int)OI_USERPOT2/2)
#define LIFT_OIDOWN_SPEED 127 //((int)OI_USERPOT2/2)
// 36
//#define FORK_OIUP_SPEED ((int)OI_USERPOT2/2)
//#define FORK_OIDOWN_SPEED ((int)OI_USERPOT2/2)

//#define LIFT_MINHEIGHT_FOR_DUMP 100
//#define FORK_INSIDELIMIT 100
//#define FORK_MAX 100

static LiftState sLift;

static ForkState sFork;

// Hybrid height: (750 - 94)

#define LIFT_BURN_TIME			200 / 26.2
#define LIFT_COOL_TIME			3000 / 26.2
#define LIFT_SHORT_COOL_TIME	500 / 26.2
#define LIFT_STALL_THRESHOLD	(int)(0.25 * LIFT_TICKS_PER_INCH)


static char Lift_ControlsCalibration(void);

// STATE CHANGE PRECONDITIONS ===========================================================

char Fork_IsClear(void) {
	return (sFork.relPos < FORK_CLEAR_POS);
}
char Lift_CanStow(void) {
	return sLift.relPos < LIFT_CANSTOW_POS;
}
char Lift_CanFlip(void) {
	return sLift.relPos > LIFT_CANFLIP_POS;
}

void Lift_SetTargetPos(int to) {
	sLift.target = to;
}
int Lift_GetPos(void) {
	return sLift.relPos;
}
int Fork_GetPos(void) {
	return sFork.relPos;
}
static void Lift_SetBasePos(char forcePrint) {
	sLift.basePos = sLift.absPos;
	if (forcePrint || gLoop.f.printA)
		printf("Lift: Recalibrated to %d\r", sLift.basePos);
}
static void Fork_SetBasePos(char forcePrint) {
	sFork.basePos = sFork.absPos;
	if (forcePrint || gLoop.f.printA)
		printf("Fork: Recalibrated to %d\r", sFork.basePos);
}

void Lift_UpdateTarget(void) {
	switch (sLift.state) {
		case LIFTST_BOTTOM:
			sLift.target = LIFT_POS_BOTTOM;
			break;
		case LIFTST_INTERMEDIATE:
			sLift.target = LIFT_POS_INTERMEDIATE;
			break;
		case LIFTST_SUCK:
			sLift.target = LIFT_POS_SUCK;
			break;
		case LIFTST_TOP:
			sLift.target = LIFT_POS_TOP;
			break;
	}
	sLift.timeout = (3000/26.2);
	printf("LIFT GOING TO %d / Target %d\r", sLift.state, sLift.target);
}

void Fork_UpdateTarget(void) {
	switch (sFork.state) {
		// No ball
		case FORKST_HORIZ:
			sFork.target = FORK_POS_HORIZ;
			break;
		case FORKST_STOW:
			sFork.target = FORK_POS_STOW;
			break;
			
		// Ball
		case FORKST_HOLD:
			sFork.target = FORK_POS_HOLD;
			break;
		case FORKST_FLIP:
			sFork.target = FORK_POS_FLIP;
			break;
//		case FORKST_CLAMP:
//			sFork.target = FORK_POS_CLAMP;
//			break;
	}
	sFork.enabled = 1;
	sFork.integralError = 0;
	printf("FORK GOING TO %d / Target %d\r", sFork.state, sFork.target);
}

void Lift_ReadEEPROMSettings(void) {
	sLift.basePos = EEPROM_ReadInt(EEPROM_LIFT_BASEPOS);	// First, load lift base
	sFork.basePos = EEPROM_ReadInt(EEPROM_FORK_BASEPOS);	// Next, load fork position
	printf("READ: Lift basePos %d ; Fork basePos %d\r", sLift.basePos,
	    sFork.basePos);
}

void Lift_WriteEEPROMSettings(void) {
	EEPROM_WriteInt(EEPROM_LIFT_BASEPOS, sLift.basePos);	// First, write lift base
	EEPROM_WriteInt(EEPROM_FORK_BASEPOS, sFork.basePos);	// Next, write fork position
	printf("WRITE: Lift basePos %d ; Fork basePos %d\r", sLift.basePos,
	    sFork.basePos);
}

void Lift_Initialize(void) {
	Lift_ReadEEPROMSettings();
//	sLift.basePos = 94;
//	sFork.basePos = 11;
	Lift_InitStates();

	sLift.state = LIFTST_BOTTOM;
	sFork.state = FORKST_STOW;
	
	sFork.haveBall=0;
//	// FIXME FIXME dont belong here
//	sFork.pGain = 30;
//	sFork.iGain=0;
	
	Fork_UpdateTarget();
}

#if 0
static void Lift_StallProtect(void) {
	int relpos = sLift.relPos;

	//Look for stall condition, and if found, set cool down timer and disable motor.
	if (0==sLift.coolTime && 0!=sLift.pwm) {
		if (mAbsDiff(sLift.absPos, sLift.stallPos) < LIFT_STALL_THRESHOLD) { //power applied but no motion?
			if (++sLift.stallTime > LIFT_BURN_TIME) { // ... for some time
				printf("Lift: Motor on fire! Motor on fire! Disabled.\r\n");

#define VARIABLE_COOL_TIME
#ifdef VARIABLE_COOL_TIME	// Cool down timer depends on lift position and direction: [dcl]
				if ( (relpos < (int)(2.0 * LIFT_TICKS_PER_INCH) && sLift.pwm
				        < 0 ) || (relpos > (int)(30.0 * LIFT_TICKS_PER_INCH)
				        && sLift.pwm > 0 )) {
					sLift.coolTime = LIFT_COOL_TIME; // long cool time if near limits
				} else {
					sLift.coolTime = LIFT_SHORT_COOL_TIME; // short cool time if away from limits
				}
#else
				sLift.coolTime = LIFT_COOL_TIME; // Disable by setting cool down timer
#endif //VARIABLE_COOL_TIME
				sLift.stallTime = 0;
			}
		} else {
			sLift.stallTime = 0;
			sLift.stallPos = sLift.absPos; //save the last known position
		}
	}

	if (sLift.coolTime>0) {
		--sLift.coolTime;
#ifdef SERIAL_LED
		mSLEDLiftCooling=1; //mSLEDLiftStalled not used.
#endif //SERIAL_LED
		sLift.pwm = 0;
	}
}
#endif

static void Lift_DoClosedLoop(void) {
	long error;
//	int pGain = 60;
//	int pGain = 34;
//	int pGain = 68;
//	int pGain = 36; // 3/28/2008
//	int pGain = 22;
	int pGain;
	long pOut;

//	pGain = OI_USERPOT2;
	if (sFork.haveBall) {
		pGain = 55;
	} else {
		pGain = 36;
	}

	if (sLift.timeout > 0) {
//		sLift.timeout--;
		// don't time out
	} else {
		return;
	}

	error = sLift.relPos - sLift.target; // [-1023, 1023]
	
	if (mAbsolute(error) < 0.5 * LIFT_TICKS_PER_INCH) {
		if (gLoop.f.printA)
			printf("Lift: Err %d OK\r", error);
		return;
	}

	pOut = -error * pGain;
//	pOut = mDivideBy8(pOut);
	pOut = mDivideBy16(pOut);
	
	mLimitRange(pOut, -127, 127);
	sLift.pwm = (int)pOut;

	if (gLoop.f.printA) {
		printf("Lift: Pgain %d\r", pGain);
		printf("Tgt %ld, Err %ld, P_out %ld\r",
				sLift.target, error, pOut);
	}

#define LIFT_MINPWM		20	// TODO: Find value ((int)OI_USERPOT3)
	if (sLift.pwm > 0 && sLift.pwm < LIFT_MINPWM) {
		sLift.pwm = LIFT_MINPWM;
	}
	if (sLift.pwm < 0 && sLift.pwm > -LIFT_MINPWM) {
		sLift.pwm = -LIFT_MINPWM;
	}
	if (gLoop.f.printA)
		printf("MinPWM %d; Out %d\r",
			LIFT_MINPWM, sLift.pwm);
}

static void Fork_DoClosedLoop(void) {
	long error;
//	int Pgain= ((int)OI_USERPOT3>>4) - 2;
//	int Igain= ((int)OI_USERPOT2>>4) - 2;
	int pGain, iGain;
	long pOut, iOut;
//	pGain = sFork.pGain;
//	iGain = sFork.iGain;
	
	if (sFork.haveBall) {
//		pGain = 40;
		pGain = 105;
		iGain = 0;
	} else {
//		pGain = 30;
		pGain = 50;
		iGain = 0;
	}
//	pGain = OI_USERPOT2;
//	iGain = 0;
	
	if (pGain<0) pGain = 0;
	if (iGain<0) iGain = 0;
	
//	int deadband = ((int)OI_USERPOT2);

	if (!sFork.enabled)
		return;

	error = sFork.relPos - sFork.target; // [-1023, 1023]
//	if (mAbsolute(error) < 8*1024L/300) error = 0;
//	if (mAbsolute(error) < deadband) error = 0;
	

	pOut = -error * pGain;
//	pOut = mDivideBy64(pOut);
	pOut = mDivideBy128(pOut);
	
	sFork.integralError += error;
	mLimitRange(sFork.integralError, -512, 512);
	iOut = -sFork.integralError * iGain;
//	iOut = mDivideBy512(iOut);
//	iOut = mDivideBy64(iOut);
	iOut=0;
	
	sFork.pwm = pOut + iOut;

	if (gLoop.f.printA) {
		printf("Fork_DoClosedLoop: Pgain %d; Igain %d\r", pGain, iGain);
		printf("Error %d, Ierror %d, P_out %d, I_out %d, FinalOut %d\r",
			error, sFork.integralError, pOut, iOut, sFork.pwm);
	}

//#define FORK_MINPWM_UP		36
//	if (sFork.pwm > 0 && sFork.pwm < FORK_MINPWM_UP)
//		sFork.pwm = FORK_MINPWM_UP;
	
#define FORK_MAXPWM_DOWN	36
#define FORK_MAXPWM_UP		127 // no limit
	mLimitRange(sFork.pwm, -FORK_MAXPWM_DOWN, FORK_MAXPWM_UP);
//	if (sFork.pwm < -FORK_MAXPWM_DOWN)
//		sFork.pwm = -FORK_MAXPWM_DOWN;

	
	// EXPERIMENTAL CODE
	// If the fork is in the way and we're trying to go to HOLD,
	// apply a pulse.
//#define FORK_MINDOWN	20
//	if (sFork.state == FORKST_HOLD && sFork.relPos < FORK_CLEAR_POS) {
//		sFork.pwm = -FORK_MINDOWN;
//	}
	// END EXPERIMENTAL
}

void Fork_HaveBall_Buttons(void) {
	if (OI_HAVEBALL)
		sFork.haveBall=1;
	else if (OI_DONTHAVEBALL)
		sFork.haveBall=0;
}
void Fork_HaveBall_PressureSensor(void) {
	int latchTimer = 0;
	if (sFork.vacuumPressure < 30) {
		latchTimer = (250/26.2);
//		printf("Y");
	} else {
//		printf("N");
	}
	if (latchTimer > 0) {
		latchTimer--;
		sFork.haveBall=1;
	} else {
		sFork.haveBall=0;
	}
}
void Hat_HaveBall_Indicator(void) {
	if (sFork.haveBall) {
		mOILEDHaveBall1 = mOILEDHaveBall2 = mOILEDHaveBall3 = mOILEDHaveBall4 = 
			1;//(gLoop.count & 0x2);
		mOILEDHat1Green=mOILEDHat2Green= !!(gLoop.count & 0x02);
	} else {
		mOILEDHaveBall1 = mOILEDHaveBall2 = mOILEDHaveBall3 = mOILEDHaveBall4 = 0;
		mOILEDHat1Green=mOILEDHat2Green= 0;//!!(gLoop.count & 0xFE);
		// green light is on by default == "Hat Connected" indicator
	}
}

void Lift_Do(void) {
	sLift.absPos = 1024 - Get_Analog_Value(SENS_LIFT_POS); // wired backwards
	sFork.absPos = Get_Analog_Value(SENS_FORK_POS);
	sFork.vacuumPressure = Get_Analog_Value(SENS_VACUUM_PRESSURE);
	
//	sLift.absPos = 1024 - Get_ADC_Result(SENS_LIFT_POS); // wired backwards
//	sFork.absPos = Get_ADC_Result(SENS_FORK_POS);
//	sFork.vacuumPressure = Get_ADC_Result(SENS_VACUUM_PRESSURE);

	if (SENS_LIFT_LOLIMIT) {
		Lift_SetBasePos(0);
	}
	// No fork limit sensors
	
	Lift_ControlsCalibration();	// run in disabled as well as enabled mode

	sLift.relPos = sLift.absPos - sLift.basePos;
	sFork.relPos = sFork.absPos - sFork.basePos;

	if (gLoop.f.printLCD1)
	{
		printfLCD(LCD_Lift, "LFT%4dR+%4dB",
				sLift.relPos, sLift.basePos);
		printfLCD(LCD_Fork, "FRK%4dR+%4dB/V%4d",
				sFork.relPos, sFork.basePos, sFork.vacuumPressure);
	}

	if (gLoop.f.printA) {
		printf("Lift Pos: %d (%d - %d)\r", sLift.relPos, sLift.absPos,
		    sLift.basePos);
		printf("Fork Pos: %d (%d - %d)\r", sFork.relPos, sFork.absPos,
		    sFork.basePos);
	}

//	Lift_StallProtect();

	Lift_DoClosedLoop();
	Fork_DoClosedLoop();
	
//	{
//		char *str;
//		str = Fork_IsClear() ? "   " : "Clr";
//		printfLCD(LCD_AutomationState, "%s", str);
//	}
//	
	if (sLift.pwm) {
		if (!Fork_IsClear()) {
			sLift.pwm = 0;
			printf("Fork not clear, not running lift\r");
		}
	}
	
//	if ( (SENS_LIFT_LOLIMIT || sLift.relPos < 3 * LIFT_TICKS_PER_INCH) && sLift.pwm < 0)
//	if ( (SENS_LIFT_LOLIMIT || sLift.relPos < 2 * LIFT_TICKS_PER_INCH) && sLift.pwm < 0)
	if ( sLift.relPos < LIFT_LIMIT_LOW  && sLift.pwm < 0)
		sLift.pwm = 0;
	if ( sLift.relPos > LIFT_LIMIT_HIGH && sLift.pwm > 0)
		sLift.pwm = 0;
//	if (sFork.relPos > (450L-11 - (524L-450L) - (457L-412))
//			&& sFork.pwm > 0)
//		sFork.pwm = 0;
//	if ( sFork.relPos < FORK_TARGET_HORIZ && sFork.pwm < 0)
	if (sFork.relPos > FORK_LIMIT_MAX && sFork.pwm > 0)
		sFork.pwm = 0;
	if ( sFork.relPos < FORK_LIMIT_MIN && sFork.pwm < 0)
		sFork.pwm = 0;
	
	if (gLoop.f.printA) {
		printf("L/F %d/%d  Lb %d/%d  Fb %d/%d\r", sLift.pwm, sFork.pwm,
			OI_LIFT_UP_MANUAL, OI_LIFT_DOWN_MANUAL,
			OI_FORK_UP_MANUAL, OI_FORK_DOWN_MANUAL
			);
	}

	PWM_LIFT = 127- Limit127(sLift.pwm);
	PWM_FORKL=PWM_FORKR = 127+ Limit127(sFork.pwm);
}

static char Lift_ControlsCalibration(void) {
	static int lbtnCount = 0;
	static int fbtnCount = 0;
	
	if (OI_LIFT_SETBASE) {
		if (++lbtnCount > 1500/26.2) {
			lbtnCount = 0;
			printf("LiftSetBase!\r");
			Lift_SetBasePos(1);
			Lift_WriteEEPROMSettings();
		}
	} else {
		lbtnCount=0;
	}
	if (OI_FORK_SETBASE) {
		if (++fbtnCount > 1500/26.2) {
			fbtnCount = 0;
			printf("ForkSetBase!\r");
			Fork_SetBasePos(1);
			Lift_WriteEEPROMSettings();
		}
	} else {
		fbtnCount=0;
	}

	return OI_META;
}

void Lift_Controls(void) {
	static char debounceL = 0;
	static char debounceF = 0;

	sLift.pwm = sFork.pwm = 0;
	
	if (OI_META)	//doing calibration   Lift_ControlsCalibration())
		return;
	
	// AUTOMATIC
	
	if (OI_PANEL_A)
		sFork.integralError = 0;

	if (!debounceL) {
		if (OI_LIFT_UP_AUTO) {
			Lift_ChangeState(1, &sLift, &sFork);
		} else if (OI_LIFT_DOWN_AUTO) {
			Lift_ChangeState(0, &sLift, &sFork);
		}
	}
	if (!debounceF) {
		if (OI_FORK_UP_AUTO) {
			Fork_ChangeState(1, &sLift, &sFork);
		} else if (OI_FORK_DOWN_AUTO) {
			Fork_ChangeState(0, &sLift, &sFork);
		}
	}
	debounceL = (OI_LIFT_UP_AUTO || OI_LIFT_DOWN_AUTO);
	debounceF = (OI_FORK_UP_AUTO || OI_FORK_DOWN_AUTO);
	
//	Fork_HaveBall_Buttons();
	Fork_HaveBall_PressureSensor();
	Hat_HaveBall_Indicator();
//	if (OI_AUTOMODE && ) {
//		//
//	}
	
//	if (OI_HAVEBALL) {
//		sFork.pGain = 40;
//		sFork.iGain=0;
//	
////		sFork.pGain = 7;
////		sFork.iGain = 2;
//	}
//	if (OI_DONTHAVEBALL) {
//		sFork.pGain = 30;
//		sFork.iGain=0;
//	
////		sFork.pGain = 8;
////		sFork.iGain = 0;
//	}
//	sFork.pGain = OI_USERPOT2;
//	sFork.iGain = OI_USERPOT3;
	
//	if (sFork.haveBall)
	
	
	
	
	

	// MANUAL

	if (!OI_AUTOMODE) {
		if (gLoop.f.printA) {
			printf("L: %d %d / F: %d %d\r", OI_LIFT_UP_MANUAL, OI_LIFT_DOWN_MANUAL,
				OI_FORK_UP_MANUAL, OI_FORK_DOWN_MANUAL);
	//		printf("%d\r", FORK_OIUP_SPEED);
		}
		
		if (OI_LIFT_UP_MANUAL) {
			sLift.pwm = LIFT_OIUP_SPEED;
		} else if (OI_LIFT_DOWN_MANUAL) {
			sLift.pwm = -LIFT_OIDOWN_SPEED;
		}
		if (OI_LIFT_UP_MANUAL || OI_LIFT_DOWN_MANUAL) {
			// Override auto actions if any manual buttons pressed
			sLift.timeout = 0;
			sFork.enabled = 0;
		}
		if (OI_FORK_UP_MANUAL) {
			// Fork up button is used to set lift target to Pot2
			sLift.timeout=3000;
			sLift.target = LIFT_POS_BOTTOM + mDivideBy256(
				(int)OI_USERPOT2 * LIFT_POS_TOP
			);
		}

		// Manual fork from UserPot
		sFork.target = 80 + mDivideBy256((int)OI_USERPOT1 * (FORK_POS_FLIP-80));
//		sFork.pGain = 30;
//		sFork.pGain = 11;
//		sFork.iGain = 0;
		if (mRolling())
			printf("Fork Target: %d\r", sFork.target);
		sFork.enabled = 1;
	}


//	if (OI_FORK_UP_MANUAL) {
//		sFork.pwm = FORK_OIUP_SPEED;
//	} else if (OI_FORK_DOWN_MANUAL) {
//		sFork.pwm = -FORK_OIDOWN_SPEED;
//	}
//	if (OI_FORK_UP_MANUAL || OI_FORK_DOWN_MANUAL) {
//		sLift.timeout = 0;
//		sFork.enabled = 0;
//	}
}

void Lift_TestArea(void) {
	sLift.target=400;
	sLift.relPos=6;
	sLift.state = LIFTST_TOP;
	OI_USERPOT2=254;
	Lift_UpdateTarget();
	Lift_DoClosedLoop();
}