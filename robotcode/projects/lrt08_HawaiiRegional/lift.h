#ifndef LIFT_H_
#define LIFT_H_

enum LiftStates {
	LIFTST_BOTTOM,
	LIFTST_INTERMEDIATE,
	LIFTST_SUCK,
	LIFTST_TOP,
	LIFTST_NUMSTATES
};
enum ForkStates {
	FORKST_HORIZ,
	FORKST_REMOVEBALL,
	FORKST_HOLD,
//	FORKST_CLAMP,
	FORKST_STOW,
	FORKST_FLIP,
	FORKST_NUMSTATES
};

// 1024 ticks    1 rev     4 links   1 inch first stage
// ---------- * -------- * ------- * ---------------------  = 17.06
//   10 rev     24 links     inch    2 inches second stage
#define LIFT_TICKS_PER_INCH (1024*4/10/24)

//#define LIFT_MAX (834L-94)
#define LIFT_MAX 732L

#define LIFT_LIMIT_LOW	((int)(2.5 * LIFT_TICKS_PER_INCH))
#define LIFT_LIMIT_HIGH	((int)(LIFT_MAX - 0.5 * LIFT_TICKS_PER_INCH))

#define LIFT_POS_BOTTOM  0L
#define LIFT_POS_INTERMEDIATE (336L+2*LIFT_TICKS_PER_INCH)
#define LIFT_POS_SUCK  635L
#define LIFT_POS_TOP    ((int)(LIFT_MAX - 0.5 * LIFT_TICKS_PER_INCH)) //(30 * LIFT_TICKS_PER_INCH)
//#define LIFT_POS_TOP    400L //(30 * LIFT_TICKS_PER_INCH)

#define FORK_CLEAR_POS 405L
//#define FORK_CLEAR_POS 456L
//#define FORK_CLEAR_POS 480L
#define LIFT_CANSTOW_POS 13L
#define LIFT_CANFLIP_POS 710L

//#define FORK_POS_DROPOFF (100L-11)

//#define FORK_POS_HORIZ   (290L-11 + (290L-209L) - (360L-326))
//#define FORK_POS_HORIZ   281L
//#define FORK_POS_HORIZ   276L
#define FORK_POS_HORIZ   245L	// after fork got bent upwards
//#define FORK_POS_HORIZ   (316L-61L)

#define FORK_POS_REMOVEBALL   276L

// OLD
//#define FORK_POS_HOLD    378L
//#define FORK_POS_HOLD    384L
//#define FORK_POS_HOLD    435L
#define FORK_POS_HOLD    360L
//#define FORK_POS_HOLD    (500L-61L)
//#define FORK_POS_HOLD    (443L-11)

//#define FORK_POS_CLAMP    473L
#define FORK_POS_CLAMP    510L

//#define FORK_POS_STOW    (577L-11)
#define FORK_POS_STOW    631L

//#define FORK_POS_VACUUMOFF    (650L)
//#define FORK_POS_VACUUMOFF    (680L)
#define FORK_POS_VACUUMOFF    (611L)

// OLD VALUES
#define FORK_POS_FLIP   (705L-11)
#define FORK_LIMIT_MIN	40
#define FORK_LIMIT_MAX	FORK_POS_FLIP


typedef struct {
	int pwm;
	int absPos;

	int basePos;
	int relPos;

	enum LiftStates state;
	int target;
	int timeout;

	int stallPos;
	int stallTime;
	int coolTime;
} LiftState;

typedef struct {
	int pwm;
	int absPos;

	int basePos;
	int relPos;

	enum ForkStates state;
	int target;
	char enabled;
	
//	int pGain, iGain;
	int integralError;
	
	int vacuumPressure;
	char haveBall;
} ForkState;

void Lift_Initialize(void);
void Lift_Do(void);
void Lift_Controls(void);
void Lift_SetTargetPos(int to);
int Fork_GetPos(void);

void Lift_UpdateTarget(void);
void Fork_UpdateTarget(void);

char Fork_IsClear(void);
char Lift_CanStow(void);
char Lift_CanDropoff(void);
char Lift_CanFlip(void);

void Lift_InitStates(void);

void Lift_ChangeState(char up, LiftState *sLift, ForkState *sFork);
void Fork_ChangeState(char up, LiftState *sLift, ForkState *sFork);

#define NULL 0

#endif /*LIFT_H_*/
