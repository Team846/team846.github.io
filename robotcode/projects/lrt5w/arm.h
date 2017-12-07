/********************************************************************************
* FILE NAME: Arm.h
*
* DESCRIPTION: arm variables and routines
*
********************************************************************************/


#ifndef __Arm_h
#define __Arm_h


#define _ArmJoyStick 1	// true if using joystick to control arm
#define _ArmAbsolute !_ArmJoyStick


//enum { kForeArmLevel=541, kShoulderVertical=565 };	//these need to be measured
//enum { kForeArmLevel=530, kShoulderVertical=565 };	//these need to be measured
//enum { kForeArmLevel=520, kShoulderVertical=565 };	//these need to be measured
enum { kForeArmLevel=500, kShoulderVertical=565 };	//these need to be measured

/*
 * Pots have full range rotation of 295 degrees
 * geared at 72T/44T so that ~180 degrees arm rotation -> 295 degrees pot rotation
 * A2D is 10 bit with range 2^10=1024
 * So 1 degree arm rotation = 5.68    =(72/44) * 1024/295
 */


/* on OI:
 * shoulder vertical: 127 (CALIBRATION PT)
 * back 45 deg: `215-220
 * fwd ~5 deg: 100
 * 
 *
 * forearm: level:127  (CALIBRATION PT)
 * max down: ~30
 * max up: 235
 * vertical: ~250
 */

//pot rotation  angle  is rated  at 295 degrees
//on robot:
#define mDegrees2BitsRobot(_degree) ((int)((_degree)*72L*1024/(44*295)))

//on OI  (Operator  Interface)
#define mDegrees2BitsOI(_degree) ((int)(_degree)*72L*256/(44*295))

enum { kUp, kDown };

#define kOIArmMovedThreshold 3

typedef struct
{
//	int position1024;	//current ten bit reading
	int curPosition;	//current ten bit reading
	int oldPosition;	//last ten bit reading

	int curInput;		//ten bit input from operator (after mapping)
	int setpoint;		//ten bit location to go to.
//	int setpointOld;	//prior
	int distance;

//	unsigned char cmd;	//the position sent from the OI

	unsigned char  lastOIcmd; //position sent from OI; used to detect if arm  input changed;
	unsigned direction:1;
	unsigned inPosition: 1;
	unsigned locked:	1;	//don't allow user control until input differs
						//from lastUserValue
	unsigned armOImoved:1;	//if arm on OI  moved since last OI  cmd
} arm;

typedef struct
{
	arm shoulder;	//e.g. gArm.upper.curPosition
	arm forearm;	//e.g. gArm.fore.curPosition

	unsigned char timer;	//max 255 ccyles, or 6.7 secs; for automated arm motion
	unsigned commandInProgress: 1;
	unsigned useJoyStick : 1;
	unsigned controlConnectedToOI : 1;
	unsigned overrideBashPlate:1;	//in case of bashPlate failure, user can override
	unsigned eitherMoved:1;	//if  either arm  moved
} arms;

extern arms gArm;


typedef struct {
	unsigned char KP;
	unsigned char KI;
	unsigned char KD;

	long integral;
	int error_prior;
	int pwm;
} pidData;

/*******************************************************************************/
void ApplyArmLimitSwitches(void);
void GetArmPositions(void);
void checkArmLocks(void);
void lockUserArmControls(void);
//void moveArms(void);
void OperateArms(void);
void UserControlArmsJoystick(void);	//joystick control
void UserControlArmsAbsolute(void);	//control arm
void queryWhichArmInterface(void);

void ArmMoveInitialize(int forearmPosition, int shoulderPosition);


void MoveForearmTimedStart(char ticks, char power127);
char MoveForearmTimedRun(void);

char ArmMoveRun(void);
void armPresets(void);
void checkIfOIArmMoved(void);
#endif	//__Arm_h



