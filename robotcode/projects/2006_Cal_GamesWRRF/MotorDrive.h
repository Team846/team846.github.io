/********************************************************************************
* FILE NAME:  
*
* DESCRIPTION: 
*
********************************************************************************/


#ifndef __lrtMotorDrive_h
#define __lrtMotorDrive_h
#include "utilities.h"
//#define kKvMainDrive256 (4.76*256L)	//voltage 
//#define kKvMainDrive32 (4.76*32L)	//voltage

#define kEncoderTicksPerRev 128

#define kMaxVr  72	//max voltage across motor branch resistance
#define kMaxVrCIM  60	//max voltage across motor branch resistance

//2005 Gear Ratios Motor to Encoder
//	CIM			Fisher-Price
//H:    72/7	1872/35	
//L: 1800/49	9360/49

//			ticks/loop	29.90		25.09		8.37		7.03
//for 127 full scale: example 127/29.90 = 1087/256	
//256	1087	1296	3883	4627

#define kKvHighGearCIM256  1087
#define kKvLowGearCIM256  3883

//10MHz ticks / 64 at max motor speeds
//(from excel spreadsheet)
//	CIM		FP
//high 137.0	163.2
//low 489.2	582.9

#define kToHighGearCIM 137
#define kToLowGearCIM 489

//2006 numbers:
#define kMaxRPM 6000.0
#define kTicksAtMaxRPMHighGear 60.0/kMaxRPM/400E-9/128*72/7
#define kTicksAtMaxRPMLowGear  60.0/kMaxRPM/400E-9/128*1800/49
//End of 2006 numbers

//unsigned char limitDriveMotorCurrent(char voltagePWM, char velocity, boolean *currentLimited);
void GetDriveEncoderCounts(encoder *e);
void GetMotorSpeeds(void);	//call before DriveLeft/RightMotors
void DriveLeftMotors(char in);
void DriveRightMotors(char in);
char limitDriveMotorCurrent(char voltagePWM, int motorEMF, const char limit);
char torqueDrive(char TorqueIn, int motorEMF, const char limit );
int MotorResistiveVoltage(char voltagePWM, char velocity);
char motorPWMSpeedXferFunction(char speedIn, unsigned char minPWM);
void SpeedMatchMotors( char speed);
void DriveMotors(void);

int GetTurnRate(void);

//in GearBox.c
void DoShift(void);
void ShiftHighGear(void);
void ShiftLowGear(void);


void ClearMotorPWMs(void);

//speed routines based on count of encoder interrupts
int motorSpeedCIM(char velocity, char gear); 	//private - here for testing.


//speed routines based on timing of encoder interrupts
int motorSpeedCIMT(unsigned long theTime, char signDirection, char gear);


extern char gDriveLimitedFlag;
#define mDriveLimited() gDriveLimitedFlag

typedef struct {	//pwms on {-127,127} for easier calc.
	int cimL;		
	int cimR;
} motorPWMs;

extern motorPWMs gPWM;

extern int gRobotTurnSpeed;


typedef struct {
	unsigned inHighGear:1;		//boolean status copied from sensor
	unsigned inLowGear:1;		//boolean status copied from sensor
//	unsigned betweenGears:1;	//boolean status
	unsigned inObjectiveGear:1;	//boolean status

	unsigned cmdGearSpeedToMatch:1; //Binary value is the gear to calculate speed with
		//either  kLowGear or kHighGear
		//During shifts, it will differ from cmdGearObjective 
} gearBox;

typedef struct {
	unsigned cmdGearObjective:1;	// either  kLowGear or kHighGear
	unsigned shifting:1;					//shift in progress
	unsigned char shiftTimer;	//timer given to shift, during which time joysticks are locked out.
			//if gears shift sooner, then control resumes sooner.

	gearBox left;
	gearBox right;	
} gearBoxes;	
extern gearBoxes gGearBox;


enum { kLowGear=0, kHighGear=1 };





typedef struct {
	char cimL;
	char cimR;
} motorSpeed; 

extern motorSpeed gMotorSpeed;
enum { kForward, kReverse};

#endif	//__lrtMotorDrive_h
