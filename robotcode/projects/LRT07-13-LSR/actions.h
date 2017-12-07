#ifndef __ACTIONS_h
#define __ACTIONS_h

#include "common.h"

enum {
	A_NONE=0,
	A_AIM,
	A_DISTANCE,
	A_SCORE
	// A_UNSPOIL,
	// A_READYPICKUP
};

void doAction(void);
void doActionBtns(void);
char setAction(char to);

#endif	// __ACTIONS_h
