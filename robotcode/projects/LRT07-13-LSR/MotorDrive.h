/********************************************************************************
* FILE NAME:  
*
* DESCRIPTION: 
*
********************************************************************************/


#ifndef __lrtMotorDrive_h
#define __lrtMotorDrive_h
#include "utilities.h"
#include "connections.h"

//2006,2007 robots used OakGrigsby / Electro-NC encoder part #90Q128 (128/rev)
//	(ElectroNC90Q128 http://www.electro-nc.com/oak/p0104.pdf
#define kEncoderTicksPerRev 128


//#define kToHighGearCIM 137
//#define kToLowGearCIM 489


//2006 numbers:
#ifdef ROBOT_2007
	#define kMaxRPM 5400.0	//6000.0
	#define kTicksAtMaxRPM 60.0 / kMaxRPM/400E-9/128L*16
	//There are 400ns in one clock tick  (1/400ns = 2.5MHz)

#elif defined ROBOT_2006
	#define kMaxRPM 6000.0

	// Assuming always driven in low gear [dcl]
	#define kTicksAtMaxRPM 60.0 / kMaxRPM/400E-9/128*72/7
	//#define kTicksAtMaxRPMHighGear 60.0/kMaxRPM/400E-9/128*72/7
	//#define kTicksAtMaxRPMLowGear  60.0/kMaxRPM/400E-9/128*1800/49
#endif //ROBOT 2006-7





//unsigned char limitDriveMotorCurrent(char voltagePWM, char velocity, boolean *currentLimited);
void GetDriveEncoderCounts(encoder *e);
void GetMotorSpeeds(void);	//call before DriveLeft/RightMotors
//void DriveLeftMotors(char in);
//void DriveRightMotors(char in);
char limitDriveMotorCurrent(char voltagePWM, int motorEMF, const char limit);
char torqueDrive(char TorqueIn, int motorEMF, const char limit );
//int MotorResistiveVoltage(char voltagePWM, char velocity);
char motorPWMSpeedXferFunction(char speedIn, unsigned char minPWM);
void SpeedMatchMotors( char speed);
void DriveMotors(void);

int GetTurnRate(void);

void ClearMotorPWMs(void);



extern char gDriveLimitedFlag;
#define mDriveLimited() gDriveLimitedFlag

extern int gRobotTurnSpeed;

typedef struct {
	char cimL;
	char cimR;
} motorSpeed; 

extern motorSpeed gMotorSpeed;
enum { kForward, kReverse};

#endif	//__lrtMotorDrive_h
