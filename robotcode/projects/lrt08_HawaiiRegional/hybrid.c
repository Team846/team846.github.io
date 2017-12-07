#include "hybrid.h"
#include "utilities.h"
#include "lcd.h"

enum HybridAction {
	HY_STOP,
	HY_FORWARD,
	HY_RIGHT,
	HY_LEFT90,
	HY_AUTOFWD,
	HY_FORWARDOVERPASS,
	HY_LEFTARC
};

static struct {
	char inited;
	enum HybridAction curAction;
} sHybridState =
	{ 0, -1 };

void HybridInit(void) {
	sHybridState.inited = 1;
	sHybridState.curAction = HY_STOP;
}
//***********************************************************************
char HybridDoTurnEncoders(int deltaTicks, char start, Drive *drive);
char HybridDoTurnGyro(int deltaTicks, char start, Drive *drive);
char driveHeadingDistance(int deltaTicks, int fwd, int feet10, char start,
    Drive *drive);
char HeadingDistanceRun(Drive *drive, long targetAbsDistanceLRSum,  long bearingTicks, char power);
char HeadingDistanceRunGyro(Drive *drive, long targetAbsDistanceLRSum,  long bearingTicks, char power);
void HybridMotorAction(char *action);
//***********************************************************************
//char getTurnCorrection(int angle10)
//{
//	const int gain256 = OI_USERPOT3; //110; // was 90
//	long error;
//	int turn;
//
//	error = gain256 * (angle10 - Get_Gyro_Angle());
//
//	turn = Limit127(mDivideBy128(error));
//	//mLimitRange(turn,-50,50);
//	// was +/- 40
//
//	printf("GetTurnC, gain=%d, error=%d, turn=%d\r",
//		gain256, (int)(angle10 - Get_Gyro_Angle()), (int)turn);
//
//	return turn;
//}
////***********************************************************************
//char doTurnGyro(int angle10, char start, Drive *drive) 
//{
//	static int timer = 0;
//	
//	if (start) 
//	{
//		timer = 4000/26.2;
//		Reset_Gyro_Angle();
//	}
//	drive->turn = getTurnCorrection(angle10);
//	mLimitRange(drive->turn,-50,50);
//	// was +/- 40
//
////	printf("DoTurn, gain=%d, out1024=%d, error=%d, turnout=%d\r",
////		gain256, (int)error, (int)(angle10 - Get_Gyro_Angle()), (int)mDivideBy256(error));
//
//	if (timer > 0)
//		timer--;
//	return (timer == 0);
//}
//***********************************************************************
char HybridDoTurnEncoders(int deltaTicks, char start, Drive *drive)
{
	static int timer;
	static long targetBearing;
	
	printfLCD(LCD_temp,"deltaticks %d",deltaTicks);
	if (start) {
		timer = 4000/26.2;
		targetBearing = encoder_diff() + deltaTicks;
	}

	drive->turn = computeTurn(targetBearing);
//	mLimitRange(drive->turn,-50,50);
	
	if (timer > 0)
		timer--;
	return (timer == 0);
}

//char HybridDoTurnGyro(int deltaTicks, char start, Drive *drive)
//{
//	static int timer;
//	static long targetBearing;
//	
//	printfLCD(LCD_temp,"deltaticks %d",deltaTicks);
//	if (start) {
//		timer = 4000/26.2;
//		targetBearing = Get_Gyro_Angle() + deltaTicks;
//	}
//
//	drive->turn = computeTurnGyro(targetBearing);
////	mLimitRange(drive->turn,-50,50);
//	
//	if (timer > 0)
//		timer--;
//	return (timer == 0);
//}
//***********************************************************************
char driveHeading(int angle10, int fwd, char start, Drive *drive) {
	//char done = doTurnGyro(angle10, start, drive);
	drive->fwd = fwd;
	return 0;
}
//***********************************************************************
char driveHeadingDistance(int deltaTicks, int fwd, int feet10, char start,
    Drive *drive)
{
	static long startSum;
	static int timer;
	static long startEncDiff;
	
	long currentSum = gEncoders[LEFT].position + gEncoders[RIGHT].position;
	long targetSum = 2 * feet10 * DRIVE_TICKS_PER_INCH * 12 / 10;

	if (start) {
		startEncDiff = encoder_diff();
		startSum = currentSum;
		timer = (10000/26.2);
	}
	if (timer > 0) --timer;
	else return 1;	//we've timed out
	
	drive->turn = computeTurn(startEncDiff+deltaTicks);

	drive->fwd = fwd;
	if (feet10 < 0) {
		return (currentSum-startSum <= targetSum);
	} else {
		return (currentSum-startSum >= targetSum);
	}
}
//***********************************************************************
char HybridRun(void) {
	static char ButtonPushedEver = 0;
	static enum HybridAction lastAction = -1;

	mOILEDIR1=mOILEDIR2=mOILEDIR3=mOILEDIR4=0;
	
	if (SENS_IR_STOP && SENS_IR_GO) {
		if (mRolling())
			printf("[!] Hybrid disconnected\r");
		return 0;
	}
	
	if (SENS_IR_STOP) {
		sHybridState.curAction = HY_STOP;
		mOILEDIR1=1;
		ButtonPushedEver = 1;
	}
	if (SENS_IR_GO) {
		sHybridState.curAction = HY_FORWARD;
		mOILEDIR2=1;
		ButtonPushedEver = 1;
	}
	if (SENS_IR_LEFT90) {
		sHybridState.curAction = HY_LEFT90;
		mOILEDIR3=1;
		ButtonPushedEver = 1;
	}
	if (SENS_IR_RIGHT) {
		sHybridState.curAction = HY_RIGHT;
		mOILEDIR4=1;
		ButtonPushedEver = 1;
	}

	if (lastAction != sHybridState.curAction) {
		printf("\rIR Button Pressed=%d\r\r", (int)sHybridState.curAction);
		lastAction = sHybridState.curAction;
	}

	HybridMotorAction(&sHybridState.curAction);
	return ButtonPushedEver;
}
//***********************************************************************
#define HYBRID_MAX_BRAKE 8
void HybridMotorAction(char *action) {
	Drive drive =
		{ 0, 0, 0, 0, 0, 0 };
	static char prevState = HY_STOP;
	char justStarted = (prevState != *action);
	char done=0;

	if (!sHybridState.inited)
		HybridInit();

	switch (*action) {
		case HY_STOP:
			drive.left=drive.right=0;
			drive.brakeLeft=drive.brakeRight=1;
			break;
			
		case HY_FORWARD:
			if (justStarted)
				Drive_Shift(DRIVE_HIGHGEAR);
			if (driveHeadingDistance(0, 30, 100, justStarted, &drive))
				*action = HY_STOP;
			BrakedDrive(&drive, HYBRID_MAX_BRAKE);
			break;
			
		case HY_RIGHT:
			done = HybridDoTurnEncoders(DRIVE_TICKS_PER_ROBOT_REV/4, justStarted, &drive);
//			done = HybridDoTurnGyro(900, justStarted, &drive);
			if (done) *action = HY_STOP;

			drive.left  =  drive.turn;
			drive.right = -drive.turn;
			break;
		
		case HY_LEFT90:	
			done = HybridDoTurnEncoders(-DRIVE_TICKS_PER_ROBOT_REV/4, justStarted, &drive);
//			done = HybridDoTurnGyro(-900, justStarted, &drive);
			if (done) *action = HY_STOP;
			drive.left  =  drive.turn;
			drive.right = -drive.turn;
			break;
			
		case HY_AUTOFWD:
			break;
	} // end switch
	prevState = *action;

	gMotorSpeed = drive;
}
//************************************************************************



char autonomousRun(void)
{
	static struct {
		char phase;
		char alreadyRan;
		int gameTimer;
		
		long startDistanceSum;
		long targetSum;	//used for next destination;
		long startBearing;
		long targetBearing;
		
		char power;
	} a={0,0};
	
	char advancePhase=0;
	Drive drive = { 0, 0, 0, 0, 0, 0 };
	
	if (a.alreadyRan)
		return 1;
	
//	if (OI_USERPOT2 > 127) {
//		static int startDelay = (1000/26.2);
//		if (startDelay > 0) {
//			startDelay--;
//			return;
//		}
//	}
		
	
//	printf("Autonomous %d\r", gLoop.count);

#define AUTON_STRAIGHT_DIST ((long)(33.0*2*DRIVE_TICKS_PER_FOOT))

//#define AUTON_STRAIGHT_DIST ((long)(32.5*2*DRIVE_TICKS_PER_FOOT))
//#define AUTON_STRAIGHT_DIST ((long)(32.0*2*DRIVE_TICKS_PER_FOOT))
//#define AUTON_STRAIGHT_DIST ((long)(31.0*2*DRIVE_TICKS_PER_FOOT))
#define AUTON_INITIAL_DIST  ((long)( 3.0*2*DRIVE_TICKS_PER_FOOT))
//#define AUTON_LATERAL_DIST  ((long)(10.0*2*DRIVE_TICKS_PER_FOOT))	// crashed barrier twice (3/28/2008)
//#define AUTON_LATERAL_DIST  ((long)(15.0*2*DRIVE_TICKS_PER_FOOT))	// 4 lines, grazed wall
//#define AUTON_LATERAL_DIST  ((long)(13.0*2*DRIVE_TICKS_PER_FOOT))	// 4 lines, grazed wall

#define AUTON_LATERAL_FIRST_DIST  ((long)(10.0*2*DRIVE_TICKS_PER_FOOT))
#define AUTON_LATERAL_DIST  	  ((long)(10.0*2*DRIVE_TICKS_PER_FOOT))

#define AUTON_TURN90_ADDITION  (57L * 3.2 / 2)
//#define AUTON_TURN90_ADDITION  (65L * 3.2 / 2) // Hawaii-without trim
// about 20 degrees off after two turns. Every degree ~ 3 ticks
//#define AUTON_TURN90_ADDITION  (20L * 3.2 / 2) // At SVR

#define AUTON_GYRO_90	900L
//#define GYRO_AUTON

	/* Made 4 laps but crashed into wall
#define AUTON_STRAIGHT_DIST ((long)(26.0*2*DRIVE_TICKS_PER_FOOT))
#define AUTON_INITIAL_DIST  ((long)( 8.0*2*DRIVE_TICKS_PER_FOOT))
#define AUTON_LATERAL_DIST  ((long)(10.0*2*DRIVE_TICKS_PER_FOOT))
// about 20 degrees off after two turns. Every degree ~ 3 ticks
#define AUTON_TURN90_ADDITION  (20L * 3 / 2)
*/
/*
 * Autonomous observations:
	- Went too far in lateral
	- Angle too low
	- Crashed into wall early in return straightaway
#define AUTON_STRAIGHT_DIST ((long)(26.0*2*DRIVE_TICKS_PER_FOOT))
#define AUTON_INITIAL_DIST  ((long)( 8.0*2*DRIVE_TICKS_PER_FOOT))
#define AUTON_LATERAL_DIST  ((long)(15.0*2*DRIVE_TICKS_PER_FOOT))
*/
//#define AUTON_LATERAL_DIST  ((long)(10.0*2*DRIVE_TICKS_PER_FOOT))
	
	switch(a.phase)
	{
		case 0:	//initialize
//			a.gameTimer = (5000/26.2);
			a.gameTimer = (15000/26.2);
			a.startDistanceSum = a.targetSum = encoder_sum();
#ifdef GYRO_AUTON
			Reset_Gyro_Angle();
			a.startBearing = a.targetBearing = 0;
#else
			a.startBearing = a.targetBearing = encoder_diff();
#endif
//			a.power = 90;
			a.power = 100;
			
			Drive_Shift(DRIVE_HIGHGEAR);
			advancePhase=1;
			break;
			
		case 1: // initial dist + straightaway
			printf("Setting Initial Dist %ld\r", AUTON_INITIAL_DIST+AUTON_STRAIGHT_DIST);
			a.targetSum += AUTON_INITIAL_DIST+AUTON_STRAIGHT_DIST;
#ifdef GYRO_AUTON
			a.targetBearing -= 0;
#else
			a.targetBearing -= 0;
#endif
			advancePhase=1;
			break;
		case 2:
#ifdef GYRO_AUTON
			if (HeadingDistanceRunGyro(&drive, a.targetSum, a.targetBearing, a.power))
				advancePhase=1;
#else
			if (HeadingDistanceRun(&drive, a.targetSum, a.targetBearing, a.power))
				advancePhase=1;
#endif
			break;
			
			
		// - START LOOP -
			
			
		case 3: // turn left 90, go lateral
		{
//			static char firstLeg = 1;
			
			int turnTicks = DRIVE_TICKS_PER_ROBOT_REV/4 + AUTON_TURN90_ADDITION;
//			if (firstLeg) {
//				a.targetSum += AUTON_LATERAL_FIRST_DIST;
//				firstLeg = 0;
//			} else {
				a.targetSum += AUTON_LATERAL_DIST;
//			}
			a.targetSum += turnTicks;
#ifdef GYRO_AUTON
			a.targetBearing -= AUTON_GYRO_90;
#else
			a.targetBearing -= turnTicks;
#endif
			advancePhase=1;
			break;
		}
		case 4:
#ifdef GYRO_AUTON
			if (HeadingDistanceRunGyro(&drive, a.targetSum, a.targetBearing, a.power))
				advancePhase=1;
#else
			if (HeadingDistanceRun(&drive, a.targetSum, a.targetBearing, a.power))
				advancePhase=1;
#endif
			break;
			
		case 5: // turn left 90, do straightaway
		{
			int turnTicks = DRIVE_TICKS_PER_ROBOT_REV/4 + AUTON_TURN90_ADDITION;
			a.targetSum += AUTON_STRAIGHT_DIST;
			a.targetSum += turnTicks;
#ifdef GYRO_AUTON
			a.targetBearing -= AUTON_GYRO_90;
#else
			a.targetBearing -= turnTicks;
#endif
			advancePhase=1;
			break;
		}
		case 6:
#ifdef GYRO_AUTON
			if (HeadingDistanceRunGyro(&drive, a.targetSum, a.targetBearing, a.power))
				a.phase =    3;
#else
			if (HeadingDistanceRun(&drive, a.targetSum, a.targetBearing, a.power))
				a.phase =    3;
#endif
			break;
			
			
		// ^ REPEAT LOOP ^
			
			
		default:	// never happens
			gMotorSpeed.brakeLeft=gMotorSpeed.brakeRight=1;
			return 1;	//we are done!
			
	}  //End of Switch
		
	if (advancePhase)
	{
		printf("Finished Phase %d; t=%d\r", (int)a.phase, gLoop.count);
		a.phase++;
	}
	
	BrakedDrive(&drive, HYBRID_MAX_BRAKE);
	
//	printf("StartBear: %ld, StartSum: %ld\rTgtBear: %ld, TgtSum: %ld\r", a.startBearing,a.startDistanceSum,a.targetBearing,a.targetSum);
	gMotorSpeed = drive;
//	PrintDrive(&gMotorSpeed);

	
	if (a.gameTimer > 0)
		a.gameTimer--;
//	if (a.gameTimer < (500/26.2)) {
//		gMotorSpeed.left=gMotorSpeed.right=0;
//		gMotorSpeed.brakeLeft=gMotorSpeed.brakeRight=1;
//	}
	if (a.gameTimer <= 0) {
		a.alreadyRan=1;
		return 1;
	}
	
	return 0;
}

// Determine motor values
// Report when objective distance has been exceeded.
// Input: targetAbsDistanceLRSum - the SUM of the left at right encoders at their destination.
//		targetBearing - the difference in encoder ticks on the final bearing.
//		power (indicating direction as well)

// Output: Drive *drive

char HeadingDistanceRun(Drive *drive, long targetAbsDistanceLRSum,  long bearingTicks, char power)
{
	long currentSum = encoder_sum();
	   
	drive->turn = computeTurn(bearingTicks);
	drive->fwd = power;
	if (power >= 0) {
		return (currentSum >= targetAbsDistanceLRSum);
	} else {
		return (currentSum <= targetAbsDistanceLRSum);
	}
}

//char HeadingDistanceRunGyro(Drive *drive, long targetAbsDistanceLRSum,  long bearingTicks, char power)
//{
//	long currentSum = encoder_sum();
//	   
//	drive->turn = computeTurnGyro(bearingTicks);
//	drive->fwd = power;
//	if (power >= 0) {
//		return (currentSum >= targetAbsDistanceLRSum);
//	} else {
//		return (currentSum <= targetAbsDistanceLRSum);
//	}
//}

void Hybrid_ReadEEPROM(void){;}