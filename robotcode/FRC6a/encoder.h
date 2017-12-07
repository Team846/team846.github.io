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
*	                  Set_Left_Encoder_Count() and Set_Right_Encoder_Count()
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
*	25-Jan-2006  0.51 removed other encoders.
*******************************************************************************/

#ifndef _encoder_h
#define _encoder_h

#define ENABLE_ENCODER_1
#define ENABLE_ENCODER_2


// Digital input pin(s) assigned to the encoder's phase-B output. Make sure 
// this pin is configured as an input in user_routines.c/User_Initialization().
#define ENCODER_1_PHASE_B_PIN	rc_dig_in11
#define ENCODER_2_PHASE_B_PIN	rc_dig_in12


// Change the sign of these if you need	to flip the way the encoders count. 
// For instance, if a given encoder count increases in the positive direction 
// when rotating counter-clockwise, but you want it to count in the negative 
// direction when rotating in the counter-clockwise direction, flip the sign 
// (i.e., change 1 to -1) and it'll work the way you need it to.
#define ENCODER_1_TICK_DELTA	1
#define ENCODER_2_TICK_DELTA	1



extern unsigned char Old_Port_B;

// function prototypes
void Initialize_Encoders(void);


#ifdef ENABLE_ENCODER_1
long Get_Encoder_1_Count(void);
void Set_Encoder_1_Count(long);
void Encoder_1_Int_Handler(void);
#endif

#ifdef ENABLE_ENCODER_2
long Get_Encoder_2_Count(void);
void Set_Encoder_2_Count(long);
void Encoder_2_Int_Handler(void);
#endif


#endif
