//lrtmoveRobot.h
#ifndef __lrtRobotMove_h_
#define __lrtRobotMove_h_

#include "utilities.h"
#include "resultCodes.h"
typedef struct {
	encoder *e;
	unsigned char *pwm;
	char phase;
	char initialDirection;
	char returnValue;
	char velocity;
	char maxPWM;	//relative to neutral 127
	int cyclesRemaining;		//remaining cycles to do task
	int maxCycles; //number of 26.2ms cycles to do task
	long destination;	// in encoder ticks
	char side;	//use for access for left/right variables and routines.
} task1Wheel;
extern task1Wheel gTaskLW, gTaskRW;

typedef struct {
	char initialDirection;
	char returnValue;
	char maxPWM;	//relative to neutral 127
	char maxPWMLeft;
	char maxPWMRight;
	int cyclesRemaining;		//remaining cycles to do task
	long destination;	// in encoder ticks
} robotTask;
extern robotTask gRobotTask;	//state variables for MoveRobot & TurnRobot functions



void MoveDriveMotor(task1Wheel *t);
//char turnRobot(task1Wheel *t);
void StopDriveMotor(task1Wheel *t);
//void StopDriveMotors(void);	//quick set pwms to neutral

void MoveRobotInitialize(int distanceETicks, int maxCyclesTimeout, char maxPWMswing);
char MoveRobotForward(void);

void TurnRobotInitialize(int turnETicks, int maxCyclesTimeout, char maxPWMLeft, char maxPWMRight);
//void TurnRobotInitialize(int turnETicks, int maxCyclesTimeout, char maxPWMswing);
char TurnRobot(void);

void StopRobotInitialize(void);
char StopRobotExecute(void);
char RobotStopped(char maxVelocity);


#endif // __lrtRobotMove_h_  
