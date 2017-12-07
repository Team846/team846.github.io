#ifndef __MOVEROBOT2_h
#define __MOVEROBOT2_h

#include "common.h"

char MoveRobot(unsigned long encoderSumInitial, long distance, char power);
void aim(char fwd);
void autodistance(void);

#endif	// __MOVEROBOT2_h
