#include "common.h"
#include "lift.h"

typedef char (*precondition)(void);

typedef struct {
	enum ForkStates upForkSt;
	enum ForkStates downForkSt;
	precondition upForkCond;
	precondition downForkCond;
} LiftForkState;

LiftForkState states[LIFTST_NUMSTATES][FORKST_NUMSTATES];

void Lift_ChangeState(char up, LiftState *sLift, ForkState *sFork) {
#if 0
	// Commented because we can rely on main safety checks in Lift_Do
	if (sFork->state != FORKST_HOLD) {
		printf("Can't change lift state: wrong fork pos.\r");
		return;
	}
	if (!Fork_IsClear()) {
		printf("Can't change lift state: fork still clearing...\r");
		return;
	}
#endif
	
	if (sFork->state != FORKST_HOLD) {
		sFork->state = FORKST_HOLD;	// FIXME: should check precondition?
		Fork_UpdateTarget();
	}
	
	if (up) {
		if (sLift->state < LIFTST_TOP) {
			sLift->state++;
			Lift_UpdateTarget();
		}
	} else {
		if (sLift->state > LIFTST_BOTTOM) {
			sLift->state--;
			Lift_UpdateTarget();
		}
	}
}

void Fork_ChangeState(char up, LiftState *sLift, ForkState *sFork) {
	LiftForkState *curState = &states[sLift->state][sFork->state];
	printf("Changing from Lift st %d ; Fork st %d\r", sLift->state, sFork->state);
	if (up) {
		enum ForkStates upState = curState->upForkSt;
		if (upState == -1 || curState->upForkCond == NULL) {
			printf("No more states UP\r");
			return;
		}
		if ( (curState->upForkCond)() ) {
			sFork->state = upState;
			Fork_UpdateTarget();
		} else {
			printf("Can't change fork state %d to %d: precondition failed\r",
			    sLift->state, upState);
		}
	} else {
		enum ForkStates downState = curState->downForkSt;
		if (downState == -1 || curState->downForkCond == NULL) {
			printf("No more states DOWN\r");
			return;
		}
		if ( (curState->downForkCond)() ) {
			sFork->state = downState;
			Fork_UpdateTarget();
		} else {
			printf("Can't change fork state %d to %d: precondition failed\r",
			    sLift->state, downState);
		}
	}
}

char NoPrecondition(void) {
	return 1;
}

static void setupState(LiftForkState *what, char upForkState,
    precondition upCond, char downForkState, precondition downCond) {
	what->upForkSt = upForkState;
	what->upForkCond = upCond;
	what->downForkSt = downForkState;
	what->downForkCond = downCond;
}

void Lift_InitStates(void) {
	// Lift at Bottom
	setupState( &states[LIFTST_BOTTOM][FORKST_HORIZ],
		FORKST_HOLD, NoPrecondition,
		-1, NULL
	);
	setupState( &states[LIFTST_BOTTOM][FORKST_HOLD],
		FORKST_STOW, Lift_CanStow,
	    FORKST_HORIZ, NoPrecondition
	);

	setupState( &states[LIFTST_BOTTOM][FORKST_STOW],
		-1, NoPrecondition,
	    FORKST_HOLD, Lift_CanStow
	);
	
//	setupState( &states[LIFTST_BOTTOM][FORKST_HOLD],
//		FORKST_CLAMP, NoPrecondition,
//	    FORKST_HORIZ, NoPrecondition
//	);
//	setupState( &states[LIFTST_BOTTOM][FORKST_CLAMP],
//		FORKST_STOW, Lift_CanStow,
//	    FORKST_HOLD, NoPrecondition
//	);
	
//	setupState( &states[LIFTST_BOTTOM][FORKST_STOW],
//		-1, NoPrecondition,
//	    FORKST_HOLD, Lift_CanStow
//	);

	setupState( &states[LIFTST_INTERMEDIATE][FORKST_HOLD],
		-1, NULL,
	    -1, NULL
	);
	
	setupState( &states[LIFTST_SUCK][FORKST_HOLD],
		-1, NULL,
		FORKST_REMOVEBALL, NoPrecondition
	);
	setupState( &states[LIFTST_SUCK][FORKST_REMOVEBALL],
		FORKST_HOLD, NoPrecondition,
	    FORKST_HORIZ, NoPrecondition
	);
	setupState( &states[LIFTST_SUCK][FORKST_HORIZ],
		FORKST_REMOVEBALL, NoPrecondition,
	    -1, NULL
	);

	// Lift at Max Height
	setupState( &states[LIFTST_TOP][FORKST_HOLD],
		FORKST_FLIP, Lift_CanFlip,
	    -1, NULL
	);
	setupState( &states[LIFTST_TOP][FORKST_FLIP],
		-1, NULL,
		FORKST_HOLD, Lift_CanFlip
	);
}