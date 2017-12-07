/********************************************************************************
* FILE NAME: controls.h <FRC VERSION>
*
* DESCRIPTION: 
*  This file ...
*
********************************************************************************/
#ifndef controls_h_
#define controls_h_

void controls(void);
void allStop(void);
void SetGearshiftRelays(void);
void readBallLauncherSpeed(void);
void ControlBallLauncherSpeed(void);
char threeWaySwitch(void);






enum { kLowGear,kHighGear};
extern char gDriveGear;	//set this variable to one of above to shift gears

enum {  kBallLauncherSlow=-1, kBallLauncherOff=0 };
extern char gBallLauncherSpeed;	//set this variable to control speed


enum { kFrontLiftActive , kRearLiftActive };
typedef struct {
		char rear;
		char front;
		char launch;
		char current;	//applied power level
		unsigned char direction; // 1 > rear; 0 > front
} lift;

extern lift gLift;

typedef struct {
		int left;
		int right;
		int turn;
		int fwd;
} drive;

extern drive gDrive;


typedef struct {
		char PassedRear2;
		char PassedRear1;
		char PassedFront1;
		char PassedLaunch;
} sensors;

extern sensors gSensors;

void updateSensors(void);
void controls(void);
void ballLaunchOrQueue(void);
void doLifts(void);
//
//void ballQueue(void);
//void checkDiverter(void);
//void getDirection (void);
//void driveLifts (void);
//void ballShoot(void);
//void unjamRear (void);
//void unjamFront (void);
//void checkKills (void);
//

#endif //controls_h_	NO CODE BELOW THIS LINE
