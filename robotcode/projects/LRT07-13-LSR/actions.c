#include "common.h"
#include "actions.h"

static void actionScore(void);
//static void actionUnspoil(void);
//static void actionAutoAim(void);
//static void actionReadyPickup(void);

struct {
	char currentAction;
	int timer;
	char state;
} sAuto = { A_NONE, 0, 0 };

//***********************************************************************************
void doActionBtns(void) {
	if (mBtnScore)
		setAction(A_SCORE);

	if (mBtnAutoAim)
		setAction(A_AIM);
	else if (sAuto.currentAction == A_AIM)		// only aim while button pressed
		setAction(A_NONE);
		
	if (mBtnAutoDistance)
		setAction(A_DISTANCE);
	else if (sAuto.currentAction == A_DISTANCE)	// only distance while button pressed
		setAction(A_NONE);
	
	if (mBtnAbortAction)
		setAction(A_NONE);
		
//	if (mBtnAutoAim)
//		setAction(A_AIM);
//	if (mBtnUnspoil)
//		setAction(A_UNSPOIL);
//	if (mBtnReadyPickup)
//		setAction(A_READYPICKUP);
}
//***********************************************************************************
void doAction(void) {
	switch (sAuto.currentAction) {
		case A_SCORE:
			if (gLoop.onSecond)
				printf("Action: Scoring!\r\n");
			actionScore();
			break;
		
		case A_AIM:
			if (gLoop.onSecond)
				printf("Action: Auto-aiming!\r\n");
			aim(0);
			break;
		case A_DISTANCE:
			if (gLoop.onSecond)
				printf("Action: Auto-distancing!\r\n");
			autodistance();
			break;
		
		/*case A_UNSPOIL:
			if (gLoop.onSecond)
				printf("Action: Unspoiling!\r\n");
			actionUnspoil();
			break;
		case A_READYPICKUP:
			if (gLoop.onSecond)
				printf("Action: Ready Pickup!\r\n");
			actionReadyPickup();
			break;*/
	}
}
//***********************************************************************************
char setAction(char to) {
	//printf("Current Action set to %d", to);
	sAuto.currentAction = to;
	sAuto.timer = -1;	// reset timer
	sAuto.state = 0;
}
//***********************************************************************************

//***********************************************************************************
#define SCORE_FLAT_DELAY		20	// (Get_Analog_Value(mUserPot1Port)>>5)
#define SCORE_FLAT_LOWER_INCHES	6	// (Get_Analog_Value(mUserPot1Port)>>5)
#define SCORE_UP_DELAY			14	// (Get_Analog_Value(mUserPot1Port)>>5)
static void actionScore(void) {
#ifdef ROBOT_2007

	static enum { RINGER_FLAT, RINGER_UP } scoringMode;
	
	if (sAuto.timer == -1) {		// Just started
		if (getArmPos() == G_DOWN) {
			scoringMode = RINGER_FLAT;
			moveLift(-SCORE_FLAT_LOWER_INCHES);
			sAuto.timer = SCORE_FLAT_DELAY;
		} else {
			scoringMode = RINGER_UP;
			setArmPos(G_DOWN);
			sAuto.timer = SCORE_UP_DELAY;
		}
		printf("Autoscore! Using timer: %d\r\n", sAuto.timer);
	}
	if (sAuto.timer-- <= 0) {
		setFingerPos(G_OPEN);
		setAction(A_NONE);
	}
	
#endif // ROBOT_2007
}






#ifdef OLD_STUFF
/*
#define FOOT_PRESENCE_THRESHOLD		100
#define AIM_TURNTIME				200 / 26.2
#define AIM_TURNSPEED				127
#define AIM_WAITTIME				200 / 26.2

void actionAutoAim() {
	static char state = 0;
	int left, right;
	char leftDetected, rightDetected;
	
	leftDetected = (left > FOOT_PRESENCE_THRESHOLD) ? 1 : 0;
	rightDetected = (right > FOOT_PRESENCE_THRESHOLD) ? 1 : 0;
	
	switch (sAuto.state) {
		case 0:
			if (leftDetected && rightDetected) {
				setAction(A_NONE);
			} else if (leftDetected) {
				sAuto.timer = AIM_TURNTIME;
				sAuto.state = 1;
			} else if (rightDetected) {
				sAuto.timer = AIM_TURNTIME;
				sAuto.state = 2;
			} else {
				// Can't autoaim
				setAction(A_NONE);
			}
			break;
		case 1:			// turns left
			if (sAuto.timer-- > 0) {
				gMotorSpeed.cimL = -AIM_TURNSPEED;
				gMotorSpeed.cimR = AIM_TURNSPEED;
			} else {
				sAuto.timer = AIM_WAITTIME;
				sAuto.state = 3;
			}
			break;
		case 2:			// turns right
			if (sAuto.timer-- > 0) {
				gMotorSpeed.cimL = AIM_TURNSPEED;
				gMotorSpeed.cimR = -AIM_TURNSPEED;
			} else {
				sAuto.timer = AIM_WAITTIME;
				sAuto.state = 3;
			}
			break;
		case 3:			// waits
			if (sAuto.timer-- > 0) {
				// wait.
			} else {
				sAuto.state = 0;
			}
			break;
	}
}
//***********************************************************************************
#define UNSPOIL_UPHEIGHT		50
#define UNSPOIL_DELAY			1500 / 26.2
#define UNSPOIL_BACKUPSPEED		100
#define UNSPOIL_BACKUPTIME		500 / 26.2
static void actionUnspoil(void) {
	switch (sAuto.state) {
		case 0:
			setArmPos(G_DOWN);
			setFingerPos(G_OPEN);
			setGripperTrap(G_ON);
			sAuto.state++;
			break;
		case 1:
			if (getFingerPos() == G_CLOSED) {	// wait for gripper trap to be activated
				moveLiftToTarget(getLiftPos() + UNSPOIL_UPHEIGHT);
				sAuto.timer = UNSPOIL_DELAY;	// set timer for lift to go up
				sAuto.state++;
			}
			break;
		case 2:
			if (sAuto.timer-- <= 0)	// wait for lift to go up
				sAuto.state++;
			break;
		case 3:
			sAuto.timer = UNSPOIL_BACKUPTIME;	// set timer for backing up
			sAuto.state++;
			break;
		case 4:
			if (sAuto.timer-- > 0)											// For the duration of the timer,
				gMotorSpeed.cimL = gMotorSpeed.cimR = -UNSPOIL_BACKUPSPEED;	// back up the robot.
			else
				setAction(A_NONE);	// We're Done!
			break;
	}
}
//***********************************************************************************
static void actionReadyPickup(void) {
	switch (sAuto.state) {
		case 0:
			moveLiftToLevelRingerUp(0);
			setFingerPos(G_OPEN);
			setArmPos(G_DOWN);
			setGripperTrap(G_ON);
			break;
		case 1:
			if (getFingerPos() == G_CLOSED) {
				setArmPos(G_UP);
				moveLiftToLevelRingerUp(1);
			}
			break;
	}
}
//***********************************************************************************
*/
#endif // OLD_STUFF
