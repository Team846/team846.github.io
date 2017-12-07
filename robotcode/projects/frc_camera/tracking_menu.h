/*******************************************************************************
*
*	TITLE:		tracking_menu.h 
*
*	VERSION:	0.1 (Beta)                           
*
*	DATE:		18-Nov-2005
*
*	AUTHOR:		R. Kevin Watson
*				kevinw@jpl.nasa.gov
*
*	COMMENTS:	This code will only work with the "bells and whistles" version
*				of tracking.c and tracking.h.
*
*				You are free to use this source code for any non-commercial
*				use. Please do not make copies of this source code, modified
*				or un-modified, publicly available on the internet or elsewhere
*				without permission. Thanks.
*
*				Copyright ©2005-2006 R. Kevin Watson. All rights are reserved.
*
********************************************************************************
*
*	CHANGE LOG:
*
*	DATE         REV  DESCRIPTION
*	-----------  ---  ----------------------------------------------------------
*	18-Nov-2005  0.1  RKW - Original code.
*
*******************************************************************************/
#ifndef _TRACKING_MENU_H
#define _TRACKING_MENU_H

// character that will bring up the tracking menu
#define TM_SETUP_KEY 't'

// tracking menu states
#define TM_INITIALIZE 0
#define TM_MENU_PRINT 1
#define TM_MENU_HANDLER 2
#define TM_USER_PROMPT 3
#define TM_GETTING_USER_INPUT 4
#define TM_USER_INPUT_ERROR 5
#define TM_PWM_MENU_HANDLER 6
#define TM_GET_DEFAULT_VALUES 7
#define TM_SAVE_TO_EEPROM 8
#define TM_EEPROM_WRITE_DELAY 9

// function prototypes
unsigned char Tracking_Menu(void);
void Tracking_Menu_Print(void);
void PWM_Menu_Print(void);
void Save_Tracking_Configuration(unsigned int);

#endif _TRACKING_MENU_H
