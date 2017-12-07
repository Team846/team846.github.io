
#include "common.h"
#include "gripper.h"

#ifdef ROBOT_2007

#define RELAY_TIMEOUT	500 / 26.2

#define ARM_RAISABLE_BELOW		378		//177

struct {
	unsigned finger :1;
	unsigned arm :1;
	unsigned trapping :1;
	int relayTimer;
} sGripper = {G_OPEN, G_DOWN, G_OFF, 0};

static void doGripperTrap(void);


void doGripperBtns(void) {
	// Handle manual buttons
	if (mBtnArmUp)
		setArmPos(G_UP);
	if (mBtnArmDown)
		setArmPos(G_DOWN);
	if (mBtnFingerOpen)
		setFingerPos(G_OPEN);
	if (mBtnFingerClose)
		setFingerPos(G_CLOSED);
		
	/*static char debounce1, debounce2;	// not really a debouncer
	{ // TEMPORARY: TOGGLE-MODE BUTTONS
		//printf("D1 %d ; D2 %d\r\n", debounce1, debounce2);
		if (mBtnArmUp && !debounce1)
			setArmPos(!getArmPos());
		if (mBtnFingerOpen && !debounce2)
			setFingerPos(!getFingerPos());
		debounce1 = mBtnArmUp ? 1 : 0;
		debounce2 = mBtnFingerOpen ? 1 : 0;
	}*/
}

void doGripper(void) {
	doGripperTrap();

	if (sGripper.relayTimer > 0) {
		mRelayFingerClose = (sGripper.finger == G_CLOSED);
		mRelayFingerOpen = (sGripper.finger == G_OPEN);
		mRelayArmUp = (sGripper.arm == G_UP);
		mRelayArmDown = (sGripper.arm == G_DOWN);
		sGripper.relayTimer--;
	}
}

char getFingerPos(void) {
	return sGripper.finger;
}

char getArmPos(void) {
	return sGripper.arm;
}

char getGripperTrap(void) {
	return sGripper.trapping;
}

char setFingerPos(char to) {
	sGripper.finger = to;
	sGripper.relayTimer = RELAY_TIMEOUT;
//	printf("Setting Finger Position %d\r\n", to);
	return sGripper.finger;
}

char setArmPos(char to) {
	if (to == G_UP && getLiftPos() > ARM_RAISABLE_BELOW) {
		printf("I'm sorry, but I can't do that, DAve.\r\n");
		return sGripper.arm;
	}
	if (to == G_UP)
		setFingerPos(G_CLOSED);
	
	sGripper.arm = to;
	sGripper.relayTimer = RELAY_TIMEOUT;
//	printf("Setting Arm Position %d\r\n", to);
	return sGripper.arm;
}

char setGripperTrap(char to) {
	sGripper.trapping = to;
	return sGripper.trapping;
}

static void doGripperTrap(void) {
	if (sGripper.trapping == G_ON) {
		if (gLoop.onSecondA)
			printf("Gripper Trapping Armed.\r\n");
		if (mSwGripperTrap) {
			printf("Gripper Trap CLOSING!\r\n");
			setFingerPos(G_CLOSED);
			setGripperTrap(G_OFF);
		}
	}
}

#endif // ROBOT_2007
