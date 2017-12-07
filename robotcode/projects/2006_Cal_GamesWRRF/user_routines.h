/*******************************************************************************
* FILE NAME: user_routines.h
*
* DESCRIPTION: 
*  This is the include file which corresponds to user_routines.c and
*  user_routines_fast.c
*  It contains some aliases and function prototypes used in those files.
*
* USAGE:
*  If you add your own routines to those files, this is a good place to add
*  your custom macros (aliases), type definitions, and function prototypes.
*******************************************************************************/

#ifndef __user_program_h_
#define __user_program_h_


/*******************************************************************************
                            MACRO DECLARATIONS
*******************************************************************************/
/* Add your macros (aliases and constants) here.                              */
/* Do not edit the ones in ifi_aliases.h                                      */
/* Macros are substituted in at compile time and make your code more readable */
/* as well as making it easy to change a constant value in one place, rather  */
/* than at every place it is used in your code.                               */
/*
 EXAMPLE CONSTANTS:
#define MAXIMUM_LOOPS   5
#define THE_ANSWER      42
#define TRUE            1
#define FALSE           0
#define PI_VAL          3.1415

 EXAMPLE ALIASES:
#define LIMIT_SWITCH_1  rc_dig_int1  (Points to another macro in ifi_aliases.h)
#define MAIN_SOLENOID   solenoid1    (Points to another macro in ifi_aliases.h)
*/

/* Used in limit switch routines in user_routines.c */
#define OPEN        1     /* Limit switch is open (input is floating high). */
#define CLOSED      0     /* Limit switch is closed (input connected to ground). */


/*******************************************************************************
                            TYPEDEF DECLARATIONS
*******************************************************************************/
/* EXAMPLE DATA STRUCTURE */
/*
typedef struct
{
  unsigned int  NEW_CAPTURE_DATA:1;
  unsigned int  LAST_IN1:1;
  unsigned int  LAST_IN2:1;
  unsigned int  WHEEL_COUNTER_UP:1;
  unsigned int  :4;
  unsigned int wheel_left_counter;
  unsigned int wheel_right_counter;
} user_struct;
*/
typedef struct {
	unsigned print:1;	//if false, don't print anything
	unsigned printOnA:1;
	unsigned printOnB:1;
	unsigned printOnC:1;
	unsigned printOnD:1;
	unsigned printOnS:1;	//stats controls 'onSecondLast'

//	unsigned pausePrintingForMenu:1;
} options;
extern options gOption;

typedef struct {
	unsigned int idleCycles;	//counted in main()
	
	//data handled in userRoutines
	unsigned int minIdleCycles;
	unsigned long cumulativeIdleCyles;
	unsigned char nLoops;	//time over which data is accumulated
} CPULoadData;

extern CPULoadData gCPULoad;
extern char gCameraPrinting;
/*******************************************************************************
                           FUNCTION PROTOTYPES
*******************************************************************************/

/* These routines reside in user_routines.c */
void User_Initialization(void);
void Process_Data_From_Master_uP(void);
void Default_Routine(void);
void Process_Data_Shell(void);
void processEncoders(void);

/* These routines reside in user_routines_fast.c */
void InterruptHandlerLow (void);  /* DO NOT CHANGE! */
void User_Autonomous_Code(void);  /* Only in full-size FRC system. */
void Process_Data_From_Local_IO(void);
char AutonomousRun(void);
void AutonomousReset(void);
void AutonomousInitialize(void);
void AutonomousSaveOIControls(void);

void ShiftGear(char gear);


#endif
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
