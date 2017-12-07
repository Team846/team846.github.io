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
char readBallLauncherSpeedFromOI(void);
void ControlBallLauncherSpeed(void);
void ControlLaunchWheelSpeeds(char speed);	//in wheelsensors.c

void DisplayUserOptions(void);
void SetUserOptions(void);


void SetAutonomousDistance(void);
void ReadAutonomousDistanceEPROM(void);

extern unsigned char gUser_Byte_on_OI;

char threeWaySwitch(void);


char GetBallLauncherSpeedFromCamera(void);

extern char gLauncherBelowSpeed;


enum { kLowGear,kHighGear};


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


extern struct userOption {
	unsigned char DriveMethod;
	char oldDriveMethodBtn;
	
	unsigned char RawDrive;
	char oldRawDriveBtn;
} gUserOption;


typedef struct {
		int left;
		int right;
		int turn;
		int fwd;
} drive;

extern drive gDrive;

typedef struct {
	char index;
	int sum;
	char pulse[16];		//Duration of pulse in cycles...must be power of 2
} pulse_response;

extern pulse_response gTurnPulse;


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



extern char gDriveGear;	//gear cmd (may not be in gear)


void ReadUserOptionsFromEPROM(void);
enum eeprom_addresses {
	kEPROMAdr_drive_type=0,
	kEPROMAdr_drive_method,
	kEPROMAdr_PrintOptions,
	kEPROMAdr_CameraPrint,
	kEPROMAdr_Autonomous,
	
	//more values here

	kEPROM_N
};
#endif //controls_h_	NO CODE BELOW THIS LINE
