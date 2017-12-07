#include "common.h"

#define AUTONOMOUS_DURATION		15	// seconds

/***********************************************************************************/

struct autonomous {
	char returnValue;
	char taskPhase;
	char stallCount;	//number of times robot has stalled in routine
	int gameTime;

	unsigned alreadyRan:1;
	
	unsigned char mode;		// loaded from EEPROM
	unsigned char dist_side;		// loaded from EEPROM
	unsigned char dist_ctr;			// loaded from EEPROM
		
} pAutonomous = {kResultNotRunning,0,0,0,0};

/*******************************************************************/

//called to allow multiple autonomous runs
void AutonomousReset(void) {
	pAutonomous.alreadyRan = 0;
	pAutonomous.returnValue=kResultNotRunning;
}

char AutonomousStatus(void) {
	return pAutonomous.returnValue;	
}

void AutonomousAbort(void) {
	pAutonomous.returnValue=kResultNotRunning;	
	pAutonomous.alreadyRan = 1;	
}


/*******************************************************************/

void AutonomousInitialize(void) {
	if (pAutonomous.returnValue==kResultRunning)
		return;	//do nothing
	if (pAutonomous.alreadyRan)	//must be cleared before running again.
	{
		pAutonomous.returnValue = kResultError;
		return;
	}
	
	pAutonomous.returnValue=kResultRunning;	//prevent running twice
	pAutonomous.stallCount=0;
	pAutonomous.taskPhase=0;
	pAutonomous.gameTime = AUTONOMOUS_DURATION * 1000 / 26.2;
	
//	pAutonomous.mode = 2;
//	pAutonomous.dist_ctr = AUTON_DIST;
//	pAutonomous.dist_side = AUTON_DIST;
}

/***********************************************************************************/

/***********************************************************************************/


char autonomous2(unsigned char* wait);
char autonomousScanOnly(unsigned char* wait);
char look(char* state2, unsigned char* wait);
char aimOrScan(char* state2, unsigned char* wait);

char autonomousPrintDist(unsigned char* wait) {
	unsigned long encLeftInitial, encRightInitial;
	char advancePhase = 0;
	
	switch (pAutonomous.taskPhase) {
		case 0:
			encLeftInitial = gEncoderLeft.posNow;
			encRightInitial = gEncoderRight.posNow;
			advancePhase = 1;
			break;
		
		case 1:
			if (gLoop.onSecond) {
				long posL = gEncoderLeft.posNow - encLeftInitial;
				long posR = gEncoderRight.posNow - encRightInitial;
				long posSum = posL + posR;
				long realposL = posL*10 / kEncoderTicksPerFt;
				long realposR = posR*10 / kEncoderTicksPerFt;
				long realposSum = posSum*10 / (kEncoderTicksPerFt*2);
				printf("EncL: %ld; EncR: %ld; Sum: %ld\r\n", posL, posR, posSum);
				printf("L: %ld / 10 ft; R: %ld / 10 ft; Average: %ld / 10 ft\r\n", posL, posR, posSum);
			}
			break;
	}
	
	return advancePhase;
}

char AutonomousRun(void)
{
	enum { kHaltBeforePhase = 100 };	//for debugging; stop at phase #
	overlay char advancePhase;			//when set true, state machine will advance to next state
	overlay char result;
//	overlay int timer;					// ! shouldn't this be static?
	static unsigned char wait = 0;		//wait this number of cycles; max 255, or about 6 sec


	if (gLoop.onSecond)
		printf("Autonomous phase: %d\r", (int)pAutonomous.taskPhase);


	do {
		result = advancePhase=0;	//clear advancePhase command and result

		if (wait)
		{
			wait--;
			printf("waiting...\r");
			continue;	//skip loop	(advancePhase is 0, so will come back next cycle)	// why not use break instead of continue?[dcl]
		}
		switch (pAutonomous.mode) {
			case 0:
				advancePhase = autonomousPrintDist(&wait);
				break;
			
			case 1:
			case 2:
			case 3:
			case 4:
				advancePhase = autonomous2(&wait);
				break;
				
			case 5:
				advancePhase = autonomousScanOnly(&wait);
				break;
		}

		if (result==kResultTimeOut)
		{	
			pAutonomous.returnValue = kResultTimeOut;
			printf("Timeout\r");
		}
		if (advancePhase)	//diagnostics
		{
			printf("\rEnd of AutoPhase=%d\r",(int)pAutonomous.taskPhase);
			PrintTime();
		}
		if (advancePhase)
		{
			if (pAutonomous.taskPhase >= kHaltBeforePhase)	//For testing
			{
				pAutonomous.returnValue = kResultNotRunning;
				break;	//exit the main loop
			}

			pAutonomous.taskPhase++;
		}
	} while (advancePhase);
	
	pAutonomous.gameTime--;
	
	if (kResultRunning != pAutonomous.returnValue)
		pAutonomous.alreadyRan = 1;

	return pAutonomous.returnValue;
}

//#define FLAT_AUTON
char autonomous2(unsigned char* wait) {
	static int timer;
	static char state2;		// secondary state variable gets reset to 0 whenever we advancePhase
	static struct {
		unsigned long encSumInitial;
		int distToTravel;
	} move;
	char advancePhase = 0;
	
	enum { ST_INIT=0, ST_FWD, ST_BRAKE, //ST_BACKUP,
			ST_SCAN, ST_CAPIT, ST_RETREAT };
		// can't think of a better, less error prone way to do this
	
	switch (pAutonomous.taskPhase) {
		case ST_INIT:
			#ifdef ROBOT_2007
				#ifdef FLAT_AUTON
					setArmPos(G_DOWN);
					setFingerPos(G_CLOSED);
				//	moveLiftToLevelRingerFlat(1)
					moveLiftToTarget(455);
				#else
					setArmPos(G_UP);
					setFingerPos(G_CLOSED);
					moveLiftToLevelRingerUp(2);
				#endif
			#endif
			advancePhase = 1;
			break;
			
		case ST_FWD:		// 5s
			if (state2 == 0) {
				// Start moving
				timer = (5000)/26.2;
	
				move.encSumInitial = gEncoderLeft.posNow + gEncoderRight.posNow;
				if (pAutonomous.mode == 1 || pAutonomous.mode == 4)
					move.distToTravel = pAutonomous.dist_side;
				else
					move.distToTravel = pAutonomous.dist_ctr;
				move.distToTravel = (move.distToTravel / 10.0) * kEncoderTicksPerFt * 2.0;
				
				printf("Starting move for %d ticks\r", (int)timer);
				state2++;
			}
			
			gUserOption.AutonomousRawDrive = 1;	// Servo Drive
			{
				char speed=(0.80*127);
				
				int ticksTillDone = move.distToTravel - (gEncoderLeft.posNow + gEncoderRight.posNow - move.encSumInitial);
				if (ticksTillDone < 3 * kEncoderTicksPerFt * 2) {
					speed = 0.60 * 127;
				}
				advancePhase = MoveRobot(move.encSumInitial, move.distToTravel, speed);
				
				//if (ticksTillDone < 3 * kEncoderTicksPerFt * 2)
				//	aim(speed);		// aim, at low speed
				
				//#define RACK_BUMPER_THRESH	200		// fixme set this properly
				//if (gProx.bumper > RACK_BUMPER_THRESH)
				//	gMotorSpeed.cimL = gMotorSpeed.cimR = 0;	// stop!	// may stay stopped in this stage up till timer expires
			}
	
			if (--timer <= 0) {
				advancePhase = 1;
				printf("Allotted time for moving expired, skipping to next stage.\r");
			}
	
			if (advancePhase)
				timer = 500/26.2;
			break;
			
		case ST_BRAKE:		// 0.5s		+2.5s
			mCoast = 0;
			advancePhase = (--timer <= 0);
//			if (advancePhase)
//				*wait = 2500/26.2;
			break;
		
		/*case ST_BACKUP:		// 2s+0.5s
			if (state2 == 0) {
				timer = (2000)/26.2;
		
				move.encSumInitial = gEncoderLeft.posNow + gEncoderRight.posNow;
				move.distToTravel = -0.5 * kEncoderTicksPerFt * 2;
				
				printf("Starting move for %d ticks\r", (int)timer);
				state2++;
			}
			
			gUserOption.AutonomousRawDrive = 1;	// Servo Drive
			advancePhase = MoveRobot(move.encSumInitial, move.distToTravel, 0.75 * 127);

			if (--timer <= 0) {
				advancePhase=1;
				printf("Allotted time for moving expired, skipping to next stage.\r");
			}
			
			if (advancePhase) {
				*wait = 500/26.2;		// after stopping, wait for robot to coast.
				state2 = 0;
			}
			
			break;*/
		
		case ST_SCAN:
			gUserOption.AutonomousRawDrive = 2;		// Raw Drive
			//advancePhase = look(&state2, wait);		// too long, broken out
			advancePhase = aimOrScan(&state2, wait);		// too long, broken out
			
			#ifdef ROBOT_2007
				if (pAutonomous.gameTime < (5000/26.2)) {	// At 5 seconds remaining,
					#ifdef FLAT_AUTON
						moveLiftToLevelRingerFlat(1);							// re-establish lift height
					#else
						moveLiftToLevelRingerUp(2);								// re-establish lift height
					#endif
				}
			#endif //ROBOT_2007
			
			if (pAutonomous.gameTime < (3000/26.2))	{	// At 3 seconds remaining, start scoring.
				advancePhase = 1;
				printf("3 seconds left in autonomous, starting scoring phase...\r");
			}
			
			break;
			
/*		case ST_AIM:
			if (gLoop.onSecond)
				printf("Aiming...\r");
			aim(0);	// Keep trying to aim, without moving forward.
			
			// Re-establish height at 5s remaining
			
			if (pAutonomous.gameTime < (3000/26.2))	{	// At 3 seconds remaining, start scoring.
				advancePhase = 1;
				printf("3 seconds left in autonomous, starting scoring phase...\r");
			}
			break;*/
			
		case ST_CAPIT:
			#ifdef ROBOT_2007
				setAction(A_SCORE);
			#endif
			*wait = 2000/26.2;	// This gives 1 second to retreat.
			advancePhase = 1;
			break;
			
		case ST_RETREAT:
			if (state2 == 0) {
				timer = (1000)/26.2;
			
				move.encSumInitial = gEncoderLeft.posNow + gEncoderRight.posNow;
				move.distToTravel = -2.0 * kEncoderTicksPerFt * 2;
				
				printf("Starting move back for %d ticks\r", (int)timer);
				state2++;
			}
			advancePhase = MoveRobot(move.encSumInitial, move.distToTravel, 0.80*127);
			gUserOption.AutonomousRawDrive = 1;	// Servo Drive
	
			if (--timer <= 0) {
				advancePhase=1;
				printf("Allotted time for moving expired, skipping to next stage.\r");
			}
	
			if (advancePhase)
				*wait = 500/26.2;		// after stopping, wait for robot to coast.
				
		default:
			if (pAutonomous.returnValue == kResultRunning)
				pAutonomous.returnValue = kResultNotRunning;
			break;	//quit;
	}
	
	if (advancePhase)
		state2 = 0;
	return advancePhase;
}


char autonomousScanOnly(unsigned char* wait) {
	static char state2;		// secondary state variable gets reset to 0 whenever we advancePhase
	char advancePhase = 0;
	
	switch (pAutonomous.taskPhase) {
		case 0:
			advancePhase = 1;		// resets state2
			break;
			
		case 1:
			advancePhase = look(&state2, wait);		// too long, broken out
			break;
	}
	
	if (advancePhase)
		state2 = 0;
	return advancePhase;
}


#define SCAN_PULSE_ON		4
#define SCAN_PULSE_OFF		8
#define SCAN_PULSES_LEFT	5	//how many pulses to scan left/right
#define SCAN_PULSES_RIGHT	5
#define SCAN_TURN_POWER		(1 * 127)

char look(char* state2, unsigned char* wait) {
	static int timer;
	static struct {
		int bestSum;
		long bestBearing;		// in difference of encoder L - R
		int readingSum;
		
		int pulsesToDo;
	} scan;
	char advancePhase = 0;
	
	enum { 
		SCAN_INIT=0,
		SCAN_GOLEFT, SCAN_READLEFT,			// the order is extremely important!
		SCAN_GORIGHT, SCAN_READRIGHT,		// the order is extremely important!
		SCAN_AIMBEST
	};
	
	switch(*state2) {
		case SCAN_INIT:
			timer = SCAN_PULSE_ON;
			(*state2)++;
			scan.bestSum = scan.bestBearing = scan.pulsesToDo = 0;
			scan.pulsesToDo = SCAN_PULSES_LEFT;
			break;
			
		case SCAN_GOLEFT:		// turn left
		case SCAN_GORIGHT:		// turn right
			// pan and scan
			{
				int turn = SCAN_TURN_POWER;
				if (*state2 == SCAN_GORIGHT) turn = -turn;
				gMotorSpeed.cimL = -turn;
				gMotorSpeed.cimR = turn;
				
				if (--timer <= 0) {
					printf("turn was %d\r\n", turn);
					timer = SCAN_PULSE_OFF;
					(*state2)++;
				}
			}
			break;
			
		case SCAN_READLEFT:		// take reading
		case SCAN_READRIGHT:	// take reading
			scan.readingSum += mDivideBy2(gProx.left + gProx.right);
			
			if (--timer <= 0) {
				printf("Read prox: %d @ %ld", scan.readingSum, gEncoderLeft.posNow - gEncoderRight.posNow);
				if (scan.readingSum > scan.bestSum) {
					scan.bestSum = scan.readingSum;
					scan.bestBearing = gEncoderLeft.posNow - gEncoderRight.posNow;
					printf(" * * * Amazing!!! @ %ld", scan.bestBearing);
				}
				printf("\r\n");
				// we can directly compare sums since they are always taken over
				// the same number of readings (SCAN_PULSE_OFF cycles).
				
				scan.readingSum = 0;
				
				printf("Pulses To Do: %d\r\n", scan.pulsesToDo);
				timer = SCAN_PULSE_ON;
				if (--scan.pulsesToDo <= 0) {
					(*state2)++;
					if (*state2 == SCAN_GORIGHT) {
						*wait = 500/26.2;
						scan.pulsesToDo = SCAN_PULSES_LEFT + SCAN_PULSES_RIGHT;
					}
					//else if (state2 == SCAN_AIMBEST)
				} else {
					(*state2)--;		// return to pulse state
				}
			}
			break;
		
		case SCAN_AIMBEST:
			{
				// lets hope this will never overflow (unsigned long -> long)
				long currentBearing = (long)(gEncoderLeft.posNow - gEncoderRight.posNow);
				long bearingError = currentBearing - scan.bestBearing; // ~940 = 360° IIRC
				
				// figuring out signs and left/right:
				// a positive bearing indicates a RIGHT turn
				// if at 0, the error is negative
				// the correction turn is positive, which should turn RIGHT
				
				#define SCAN_TURNGAIN	128//mUserOIPot2
				#define SCAN_TURNLIMIT	(0.80 * 127)
				#define SCAN_TURNNONE	6
				#define SCAN_TURNMIN	15
				
				int turnPwr = Limit127(mDivideBy512( mAbsolute(bearingError) * SCAN_TURNGAIN ));
				if (turnPwr > SCAN_TURNLIMIT)
					turnPwr = SCAN_TURNLIMIT;
				if (turnPwr < SCAN_TURNMIN)
					if (turnPwr > SCAN_TURNNONE)
						turnPwr = SCAN_TURNMIN;
					else
						turnPwr = 0;
				
				if (-bearingError < 0)
					turnPwr = -turnPwr;
				
				if (gLoop.onSecond)
					printf("Target: %ld; Current: %ld; Error: %ld; Gain: %d; Output: %d\r\n",
						scan.bestBearing, currentBearing, bearingError, SCAN_TURNGAIN, turnPwr);
				
				if (mAbsolute(gRobotTurnSpeed) > 600) {
					printf("Turn Limited!\r\n");
					turnPwr = 0;
				}

				gMotorSpeed.cimL = turnPwr;	// positive turnPwr => turn right
				gMotorSpeed.cimR = -turnPwr;
			}
			//advancePhase = 1;	// finally!
			break;
	}
	
	return advancePhase;
}









#define FOOT_THRESHOLD				20
#define FOOT_SCORING_THRESHOLD		200	//190
#define BUMPER_THRESHOLD			50
#define BUMPER_SCORING_THRESHOLD	155 //145

char aimOrScan(char* state2, unsigned char* wait) {
	static int timer;
	char advancePhase = 0;
	char oldstate = *state2;
	static char scanDir = 0;
	static char sawBumper = 0;
	
	enum { 
		ST_INIT=0,
		ST_FOLLOW,
		ST_SCAN_ON, ST_SCAN_OFF
	};
	
	switch(*state2) {
		case ST_INIT:
			// init.
			timer = 0;
			scanDir = (pAutonomous.mode == 2 ? 0 : 1);
				// 2: start by scanning right (pointed left)
				// 3: start by scanning left (pointed right)
			(*state2)++;
			break;
		
		case ST_FOLLOW:	//Move forward while turning toward spider foot
			if (gProx.maxRecentBumper < BUMPER_THRESHOLD || (gProx.maxRecentLeft < FOOT_THRESHOLD && gProx.maxRecentRight < FOOT_THRESHOLD)) {
				*state2 = ST_SCAN_ON;				// if bumper or spider foot not seen, sweep to look for it
			} else {
				aim(0);
				
				if (gProx.maxRecentLeft < FOOT_SCORING_THRESHOLD && gProx.maxRecentRight < FOOT_SCORING_THRESHOLD) {
					if (gProx.maxRecentBumper < BUMPER_SCORING_THRESHOLD) {
						// if both sensors are far from foot, and we're not too close to the bumper, go forward while turning
						if (gMotorSpeed.cimL < 0)
							gMotorSpeed.cimL = 0;
						if (gMotorSpeed.cimR < 0)
							gMotorSpeed.cimR = 0;
					}
				} else {
					if (gProx.maxRecentBumper > BUMPER_SCORING_THRESHOLD) {
						// too close! go back while turning
						if (gMotorSpeed.cimL > 0)
							gMotorSpeed.cimL = 0;
						if (gMotorSpeed.cimR > 0)
							gMotorSpeed.cimR = 0;
					}
				}
			}
			break;
		
		case ST_SCAN_ON:
			if (timer == 0)
				timer = SCAN_PULSE_ON;		// init timer
			printf("Scanning\r\n");
			
			if ((gProx.maxRecentLeft > FOOT_THRESHOLD+25 || gProx.maxRecentRight > FOOT_THRESHOLD+25) && gProx.maxRecentBumper > BUMPER_THRESHOLD) {
				// if we can now see the foot, try to follow it
				*state2 = ST_FOLLOW;
				timer = 0;		// exiting early, reset timer
			} else {
				int turn = SCAN_TURN_POWER;
				
				if (scanDir)
					turn = -turn;
				
				// if we don't see the foot, sweep right
				gMotorSpeed.cimL = Limit127(turn);
				gMotorSpeed.cimR = Limit127(-turn);
				if (--timer <= 0)
					(*state2)++;
			}
			break;
		
		case ST_SCAN_OFF:
			if (timer == 0)					// are we just starting this state?
				timer = SCAN_PULSE_OFF;		// init timer
			printf("Scanning\r\n");
				
			gMotorSpeed.cimL = gMotorSpeed.cimR = 0;
			if (--timer <= 0) {
				if (gProx.maxRecentBumper < BUMPER_THRESHOLD && sawBumper) {	// if we've seen the bumper before but we just lost it
					scanDir = !scanDir;											// scan back to try and locate it
					sawBumper = 0;
				}
				(*state2)--;
			}
			break;
	}
	
	if (gProx.maxRecentBumper > BUMPER_THRESHOLD)
		sawBumper = 1;
	
	return advancePhase;
}







/*****************************************************************************************/

void ReadAutonomousEPROM(void)
{
	pAutonomous.mode = EEPROM_read( kEPROMAdr_AutonomousMode );
	pAutonomous.dist_side = EEPROM_read( kEPROMAdr_AutonomousDist_Side );
	pAutonomous.dist_ctr = EEPROM_read( kEPROMAdr_AutonomousDist_Ctr );
}

/*****************************************************************************************/

// Must be disabled and in user mode
void SetAutonomousOptions(void)
{
	static struct {
		unsigned mode:1;
		unsigned dist_side:1;
		unsigned dist_ctr:1;
	} dirty = {0, 0, 0};
	static struct {
		unsigned modebtn:1;
	//	unsigned distbtn:1;		// no debounce needed, uses pot
	} debounce = {0}; //{0, 0};

	if (!disabled_mode) return;	
	
	if (user_display_mode && mBtnShowAutonomousMode)
	{
		if (mBtnSetAutonomousMode && !debounce.modebtn) {
			pAutonomous.mode++;
			if (pAutonomous.mode > 5)
				pAutonomous.mode = 0;
			dirty.mode = 1;
		}
		gUserByteOI = pAutonomous.mode;
	}
	else if (user_display_mode && mBtnShowAutonomousDistSide)
	{
		if (mBtnSetAutonomousDist) {
			pAutonomous.dist_side = mAutonomousDistPot;
			dirty.dist_side = 1;
		}
		gUserByteOI = pAutonomous.dist_side;
	}
	else if (user_display_mode && mBtnShowAutonomousDistCtr)
	{
		if (mBtnSetAutonomousDist) {
			pAutonomous.dist_ctr = mAutonomousDistPot;
			dirty.dist_ctr = 1;
		}
		gUserByteOI = pAutonomous.dist_ctr;
	}
	
	debounce.modebtn = mBtnSetAutonomousMode;
	
	// Write options to EEPROM if user is done setting options.
	if (!mBtnShowAutonomousMode && !mBtnShowAutonomousDistSide && !mBtnShowAutonomousDistCtr)
	{
		if (dirty.mode) {
			EEPROM_write(kEPROMAdr_AutonomousMode, pAutonomous.mode);
			dirty.mode = 0;
		}
		if (dirty.dist_side) {
			EEPROM_write(kEPROMAdr_AutonomousDist_Side, pAutonomous.dist_side);
			dirty.dist_side = 0;
		}
		if (dirty.dist_ctr) {
			EEPROM_write(kEPROMAdr_AutonomousDist_Ctr, pAutonomous.dist_ctr);
			dirty.dist_ctr = 0;
		}
	}
}

//********************************************************************************




// -END-

//#if 1	//select method 1 or 2
/*void SetAutonomousDistance(void)
{
//	static char dirty=0;

	//must be in user mode, disabled
	if (!user_display_mode || !disabled_mode) return;	
	
//	if (mAutoWrite1Btn && mAutoWrite2Btn)
	{
//		pAutonomous.distance = ~mBallLauncherspeed;
//		dirty=1;
//	}
//	if (mAutoWrite1Btn || mAutoWrite2Btn)
//		gUserByteOI = pAutonomous.distance;
//	else if (dirty)
//	{
//		EEPROM_write(kEPROMAdr_Autonomous, pAutonomous.distance);	//save across resets
//		dirty=0;	
//	}
}*/
//********************************************************************************
/*#else	//second method 
void SetAutonomousDistance(void)
{
	static unsigned char keyDownTime=0;
	static char dirty=0;
#define kAutoRepeatTime ((1000/26.2))

	if (dirty &&  ((gLoop.secondz>>1) & 0x1))	//write every other second
	{
		EEPROM_write(kEPROMAdr_Autonomous, pAutonomous.distance);	//save across resets
		dirty=0;
	}
if (gLoop.onSecond)
//	printf("U%d D%d Q%d Up%d Dwn%d  %d\r", (int) user_display_mode,(int) disabled_mode,
//		(int)mBallQueueSw,(int)mAutoTimeUpBtn, (int)mAutoTimeDownBtn, (int)keyDownTime);
		
		
	//must be in user mode, disabled with Meta button pressed to set options
	if (!user_display_mode || !disabled_mode) return;
//	if (!mBallQueueSw) return;

	
	if (mAutoTimeUpBtn)
	{
		if (++keyDownTime==1 || keyDownTime >= kAutoRepeatTime)
			pAutonomous.distance++;
			dirty=1;
	}
	else if (mAutoTimeDownBtn)
	{
		if (++keyDownTime==1 || keyDownTime >= kAutoRepeatTime)
			pAutonomous.distance--;
			dirty=1;
	}
	else
		keyDownTime=0;
	if (keyDownTime==255) keyDownTime=254;	//so's we don't wrap
	
	gUserByteOI = pAutonomous.distance;
}
#endif //end of second method
*/


//******************************************************************************************/
//void AutonomousSaveOIControls(void)
//{
//	pAutonomous.dontLaunchBalls = (kOff==threeWaySwitch());
//}



/* TEMPLATE

char autonomousTemplate(unsigned char* wait) {
	static char state2;		// secondary state variable gets reset to 0 whenever we advancePhase
	char advancePhase = 0;
	
	switch (pAutonomous.taskPhase) {
		case 0:
			advancePhase = 1;		// resets state2
			break;
			
		case 1
			// Actions go here...
			break;
	}
	
	if (advancePhase)
		state2 = 0;
	return advancePhase;
}

*/
