/*******************************************************************************
*
*	TITLE:		camera_menu.h 
*
*	VERSION:	0.1 (Beta)                           
*
*	DATE:		04-Oct-2005
*
*	AUTHOR:		R. Kevin Watson
*				kevinw@jpl.nasa.gov
*
*	COMMENTS:	This code will only work with the "bells and whistles" version 
*				of camera.c and camera.h.
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
*	04-Oct-2005  0.1  RKW - Original code.
*
*******************************************************************************/
#ifndef _CAMERA_MENU_H
#define _CAMERA_MENU_H

// character that will bring up the camera menu
#define CM_SETUP_KEY 'c'

// camera menu states
#define CM_INITIALIZE 0
#define CM_MENU_PRINT 1
#define CM_MENU_HANDLER 2
#define CM_USER_PROMPT 3
#define CM_GETTING_USER_INPUT 4
#define CM_USER_INPUT_ERROR 5
#define CM_GET_DEFAULT_VALUES 6
#define CM_SAVE_TO_EEPROM 7
#define CM_EEPROM_WRITE_DELAY 8
#define CM_RESTART_CAMERA 9
#define CM_RESTART_CAMERA_DELAY 10

// function prototypes
unsigned char Camera_Menu(void);
void Menu_Print(void);
void Save_Camera_Configuration(unsigned int);

#endif _CAMERA_MENU_H
