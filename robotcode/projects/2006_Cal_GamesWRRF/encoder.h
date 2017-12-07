/*******************************************************************************
*
*	TITLE:		encoder.h 
*
*	VERSION:	0.5 (Beta)                           
*
*	DATE:		17-Dec-2005
*
*	AUTHOR:		R. Kevin Watson
*				kevinw@jpl.nasa.gov
*
*	COMMENTS:	You are free to use this source code for any non-commercial
*				use. Please do not make copies of this source code, modified
*				or un-modified, publicly available on the internet or
*				elsewhere without permission. Thanks.
*
*				Copyright ©2003-2006 R. Kevin Watson. All rights are reserved.
*
********************************************************************************
*
*	CHANGE LOG:
*
*	DATE         REV  DESCRIPTION
*	-----------  ---  ----------------------------------------------------------
*	20-Dec-2003  0.1  RKW - Original code.
*	18-Feb-2004  0.2  RKW - Reassigned the encoder digital inputs to run
*	                  on the FRC robot controller too.
*	01-Jan-2005  0.3  RKW - Get_Left_Encoder_Count(), Get_Right_Encoder_Count(),
*	                  Reset_Left_Encoder_Count() and Reset_Right_Encoder_Count()
*	                  functions added.
*	01-Jan-2005  0.3  RKW - Renamed Int_1_Handler() and Int_2_Handler() to
*	                  Left_Encoder_Int_Handler() and Right_Encoder_Int_Handler
*	                  respectively.
*	01-Jan-2005  0.3  RKW - Altered the interrupt service routines to easily
*	                  flip the direction the encoders count by altering the
*	                  RIGHT_ENCODER_TICK_DELTA and LEFT_ENCODER_TICK_DELTA
*	                  #defines found in encoder.h
*	06-Jan-2005  0.4  RKW - Rebuilt with C18 version 2.40.
*	17-Dec-2005  0.5  RKW - Added code to accomodate four more encoder on
*	                  interrupts 3 through 6. These additional encoder inputs
*	                  are optimized for position control.
*	13-Jan-2006  0.5  RKW - Verified code works properly on new PIC18F8722-
*	                  based robot controllers.
*
*******************************************************************************/

#ifndef _encoder_h
#define _encoder_h


#include <timers.h>

void Initialize_timer(void);
void Timer_0_Int_Handler(void);
long get_encoder_1_width (void);


//#define ENABLE_CIM_RIGHT_ENCODER
//#define ENABLE_CIM_LEFT_ENCODER
#define ENABLE_ENCODER_3
#define ENABLE_ENCODER_4
#define ENABLE_ENCODER_5
#define ENABLE_ENCODER_6

// Digital input pin(s) assigned to the encoder's phase-B output. Make sure 
// this pin is configured as an input in user_routines.c/User_Initialization().
#define CIM_RIGHT_ENCODER_DIR	rc_dig_in11
#define CIM_LEFT_ENCODER_DIR	rc_dig_in13
#define ENCODER_3_PHASE_B_PIN	rc_dig_in13
#define ENCODER_4_PHASE_B_PIN	rc_dig_in14
#define ENCODER_5_PHASE_B_PIN	rc_dig_in15
#define ENCODER_6_PHASE_B_PIN	rc_dig_in16

// Change the sign of these if you need	to flip the way the encoders count. 
// For instance, if a given encoder count increases in the positive direction 
// when rotating counter-clockwise, but you want it to count in the negative 
// direction when rotating in the counter-clockwise direction, flip the sign 
// (i.e., change 1 to -1) and it'll work the way you need it to.
#define ENCODER_1_TICK_DELTA	1
#define ENCODER_2_TICK_DELTA	1
#define ENCODER_3_TICK_DELTA	1
#define ENCODER_4_TICK_DELTA	1
#define ENCODER_5_TICK_DELTA	1
#define ENCODER_6_TICK_DELTA	1


// #define ENABLE_ENCODER_3_6 if encoder 3, 4, 5 or 6 is enabled
#ifdef ENABLE_ENCODER_3
#ifndef ENABLE_ENCODER_3_6
#define ENABLE_ENCODER_3_6
#endif
#endif
#ifdef ENABLE_ENCODER_4
#ifndef ENABLE_ENCODER_3_6
#define ENABLE_ENCODER_3_6
#endif
#endif
#ifdef ENABLE_ENCODER_5
#ifndef ENABLE_ENCODER_3_6
#define ENABLE_ENCODER_3_6
#endif
#endif
#ifdef ENABLE_ENCODER_6
#ifndef ENABLE_ENCODER_3_6
#define ENABLE_ENCODER_3_6
#endif
#endif

extern unsigned char Old_Port_B;

// function prototypes
void Initialize_Encoders(void);


#ifdef ENABLE_CIM_RIGHT_ENCODER
long Get_Encoder_1_Count(void);
void Reset_Encoder_1_Count(void);
void Encoder_1_Int_Handler(void);
#endif

#ifdef ENABLE_CIM_LEFT_ENCODER
long Get_Encoder_2_Count(void);
void Reset_Encoder_2_Count(void);
void Encoder_2_Int_Handler(void);
#endif

#ifdef ENABLE_ENCODER_3
long Get_Encoder_3_Count(void);
void Reset_Encoder_3_Count(void);
void Encoder_3_Int_Handler(unsigned char);
#endif

#ifdef ENABLE_ENCODER_4
long Get_Encoder_4_Count(void);
void Reset_Encoder_4_Count(void);
void Encoder_4_Int_Handler(unsigned char);
#endif

#ifdef ENABLE_ENCODER_5
long Get_Encoder_5_Count(void);
void Reset_Encoder_5_Count(void);
void Encoder_5_Int_Handler(unsigned char);
#endif

#ifdef ENABLE_ENCODER_6
long Get_Encoder_6_Count(void);
void Reset_Encoder_6_Count(void);
void Encoder_6_Int_Handler(unsigned char);
#endif


#endif
