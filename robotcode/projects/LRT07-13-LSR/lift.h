#ifndef __LIFT_h
#define __LIFT_h

#ifdef ROBOT_2007

#include "common.h"

void doLift(void);
void doLiftBtns(void);
void ReadLiftPresetsFromEPROM(void);
int moveLiftToTarget(int to);
int getLiftPos(void);
void calibrateLift(void);
void moveLiftToLevelRingerUp(char level);
void moveLiftToLevelRingerFlat(char level);
void moveLift(int x);


#define LIFT_POT_TURNS			(int) 10
//#define LIFT_SPROCKET_TEETH		26
#define LIFT_SPROCKET_TEETH		((int) 23)		// [dg]

#define LIFT_UNITS_PER_INCH		(2048.0 / (LIFT_POT_TURNS * (int) LIFT_SPROCKET_TEETH))		// = 8.903
// derived from: 1024 / ( 2 * pot turns * teeth * 0.25" chain )
//factor of 2 because lift carriage goes at twice as fast as second stage
//matches empirical data well. [dg, dcl]


#define LIFT_L0						((int) (2.0 * LIFT_UNITS_PER_INCH))

// Going in ringer UP (vertical)
#define LIFT_L1_INCHES				(13.5)
#define LIFT_L2_INCHES				(LIFT_L1_INCHES+34.0 +2)
#define LIFT_L3_INCHES				(LIFT_L2_INCHES+34.0 +2)
#define LIFT_L1						((int) (LIFT_L1_INCHES * LIFT_UNITS_PER_INCH))
#define LIFT_L2						((int) (LIFT_L2_INCHES * LIFT_UNITS_PER_INCH))
#define LIFT_L3						((int) (LIFT_L3_INCHES * LIFT_UNITS_PER_INCH))

// difference between vertical and horizontal scoring positions
#define LIFT_FLAT_SCORING_OFFSET	((int) ((9.125 + 3.0) * LIFT_UNITS_PER_INCH))

//#define moveLiftInches(x)	moveLift((x) * LIFT_UNITS_PER_INCH)

#endif	// ROBOT_2007

#endif	// __LIFT_h
