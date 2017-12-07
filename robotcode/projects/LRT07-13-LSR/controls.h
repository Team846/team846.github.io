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

void DisplayUserOptions(void);
void SetUserOptions(void);

void SetAutonomousOptions(void);
void ReadAutonomousEPROM(void);

void ReadUserOptionsFromEPROM(void);

extern struct userOption {
	unsigned char DriveMethod;
	char oldDriveMethodBtn;
	
	unsigned char RawDrive;
	char oldRawDriveBtn;
	
	unsigned char AutonomousRawDrive;
	
	unsigned char ServiceMode;
} gUserOption;

#endif //controls_h_	NO CODE BELOW THIS LINE
