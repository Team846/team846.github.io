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

void SetUserOptions(void);
void Controls_Init(void);
void ReadUserOptionsFromEPROM(void);
void Brake_Test(void);

extern struct UserOption {
	unsigned char DriveMethod;
	unsigned char ServoDrive;
	unsigned char ServiceMode;
} gUserOption;

#endif //controls_h_	NO CODE BELOW THIS LINE
