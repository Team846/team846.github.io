#ifndef __DRIVE_h_
#define __DRIVE_h_
#include "common.h"

//2006,2007 robots used OakGrigsby / Electro-NC encoder part #90Q128 (128/rev)
//	(ElectroNC90Q128 http://www.electro-nc.com/oak/p0104.pdf
//#define kEncoderTicksPerRev 128

#define DRIVE_TICKS_PER_REV 100L
//#define DRIVE_TICKS_PER_REV 128L //FIXME: 2007 setting

/* AndyMark Super Shifter - Default Gearing
 * http://andymark.biz/am-0114.html
 * Low Gear: 24.0:1
 * High Gear: 9.4:1
 */
#define DRIVE_WHEEL_DIAMETER   6.0
#define DRIVE_INCHES_PER_REV  (3.14159 * DRIVE_WHEEL_DIAMETER)
#define DRIVE_TICKS_PER_FOOT  (DRIVE_TICKS_PER_REV * 12 / DRIVE_INCHES_PER_REV)
#define DRIVE_TICKS_PER_INCH  (DRIVE_TICKS_PER_FOOT/12)

#define DRIVE_GEAR_RATIO_HIGH 24.0
#define DRIVE_GEAR_RATIO_LOW   9.4
//#define kToHighGearCIM 137
//#define kToLowGearCIM 489


#define kMaxRPM 5400.0	//6000.0
#define kTicksAtMaxRPM 60.0/kMaxRPM/400E-9/100L*DRIVE_GEAR_RATIO_LOW
//There are 400ns in one clock tick  (1/400ns = 2.5MHz)

#define DRIVE_WHEEL_BASE 25.0 //inches - distance between left & right wheels

// Pi's cancel out.  Ticks is 2x because both left and right wheel rotates.
// Recommend overriding with measured value.
//2007 robot is about 1045 ticks/robot revolution
//#define DRIVE_TICKS_PER_ROBOT_REV ((int) (2*DRIVE_TICKS_PER_REV * DRIVE_WHEEL_BASE/DRIVE_WHEEL_DIAMETER))
#ifdef R2007
#define DRIVE_TICKS_PER_ROBOT_REV (1065+6+6)
#else
#define DRIVE_TICKS_PER_ROBOT_REV (1153)
#endif

#define DRIVE_TICKS_PER_90 (DRIVE_TICKS_PER_ROBOT_REV/4)



typedef struct {
	int left;
	int right;
	int turn;
	int fwd;
	unsigned brakeLeft:1;
	unsigned brakeRight:1;
} Drive;

enum { FORWARD, REVERSE };
enum DriveGear { DRIVE_HIGHGEAR, DRIVE_LOWGEAR, DRIVE_NEUTRALGEAR };

extern Drive gMotorSpeed;
extern int gRobotTurnSpeed;

void ClearMotorPWMs(void);
void GetDriveEncoderCounts(encoder *e);
void Drive_SetSpeed(int left, int right);
void Drive_Do(void);
int GetTurnRate(void);
void LimitDrive127(Drive *d);
void PrintDrive(Drive *d);

void Drive_Shift(enum DriveGear to);
void Drive_DoShifting(void);
enum DriveGear Drive_GetGear(void);
void Drive_ShiftControls(void);

void HighSpeedTurn(Drive *drive);
void BrakedDrive(Drive *drive, int maxBraking);
int computeTurn(long bearingTicks);
//int computeTurnGyro(long bearingDegrees10);

#endif	//__DRIVE_h_
