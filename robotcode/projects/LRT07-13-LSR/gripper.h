#ifndef __GRIPPER_h
#define __GRIPPER_h

#ifdef ROBOT_2007

#include "common.h"

void doGripper(void);
void doGripperBtns(void);

char getFingerPos(void);
char getArmPos(void);
char getGripperTrap(void);
char setFingerPos(char to);
char setArmPos(char to);
char setGripperTrap(char to);


#define G_OFF		0
#define G_ON		1
#define G_OPEN		0
#define G_CLOSED	1
#define G_UP		0
#define G_DOWN		1

#endif	// ROBOT_2007

#endif	// __GRIPPER_h
