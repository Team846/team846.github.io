/*******************************************************************************
*
*	TITLE:		encoder.c 
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
*	17-Dec-2005  0.5  RKW - Added code to accommodate four more encoders on
*	                  interrupts 3 through 6. These additional encoder inputs
*	                  are optimized for position control.
*	25-Jan-2006  0.51  remvoed other encoders
*******************************************************************************/

//#include <p18f8722.h>		// use this header file for PIC18F8722
#include <p18f8520.h>		// use this header file for PIC18F8520
#include "ifi_aliases.h"
#include "encoder.h"

// These variables are used to keep track of the encoder position over time.
// Though these are global variables, they shouldn't be modified directly. 
// Functions to modify these variables are included below.
#ifdef ENABLE_ENCODER_1
volatile long Encoder_1_Count = 0;
#endif

#ifdef ENABLE_ENCODER_2
volatile long Encoder_2_Count = 0;
#endif



// So that we'll know which interrupt pin changed state the next time through,
// the state of port b is saved in this variable each time the interrupt
// handler for interrupts 3 through 6 is called. This variable should be
// initialized to the current state of port b just before enabling interrupts
// 3 through 6.
unsigned char Old_Port_B = 0xFF;

/*******************************************************************************
*
*	FUNCTION:		Initialize_Encoders()
*
*	PURPOSE:		Initializes the encoder software.
*
*	CALLED FROM:	user_routines.c/User_Initialization()
*
*	PARAMETERS:		None
*
*	RETURNS:		Nothing
*
*	COMMENTS:
*
*******************************************************************************/
void Initialize_Encoders(void)  
{
	// if enabled in encoder.h, configure encoder 1's interrupt input
	#ifdef ENABLE_ENCODER_1

	// make sure interrupt 1 is configured as an input
	TRISBbits.TRISB2 = 1;

	// interrupt 1 is low priority
	INTCON3bits.INT2IP = 0;

	// trigger on rising edge
	INTCON2bits.INTEDG2 = 1;

	// make sure the interrupt flag is reset before enabling
	INTCON3bits.INT2IF = 0;

	// enable interrupt 1
	INTCON3bits.INT2IE = 1;
	#endif

	// if enabled in encoder.h, configure encoder 2's interrupt input
	#ifdef ENABLE_ENCODER_2

	// make sure interrupt 2 is configured as an input
	TRISBbits.TRISB3 = 1;

	// interrupt 2 is low priority
	INTCON2bits.INT3IP = 0;

	// trigger on rising edge
	INTCON2bits.INTEDG3 = 1;

	// make sure the interrupt flag is reset before enabling
	INTCON3bits.INT3IF = 0;

	// enable interrupt 2
	INTCON3bits.INT3IE = 1;
	#endif


}


#ifdef ENABLE_ENCODER_1
/*******************************************************************************
*
*	FUNCTION:		Get_Encoder_1_Count()
*
*	PURPOSE:		Gets the current number of encoder 1 counts.		
*
*	CALLED FROM:
*
*	PARAMETERS:		None.
*
*	RETURNS:		Number of encoder 1 counts since the last reset.
*
*	COMMENTS:
*
*******************************************************************************/
long Get_Encoder_1_Count(void)
{
	long count;

	// Since we're about to access the Encoder_1_Count variable,
	// which can also be modified in the interrupt service routine,
	// we'll briefly disable the encoder's interrupt to make sure
	// that the Encoder_1_Count variable doesn't get altered while
	// we're using it.
	INTCON3bits.INT2IE = 0;

	// Now we can get a local copy of the encoder count without fear
	// that we'll get corrupted data.
	count = Encoder_1_Count;

	// Okay, we have a local copy of the encoder count, so turn the 
	// encoder's interrupt back on.
	INTCON3bits.INT2IE = 1;

	// Return the encoder count to the caller.
	return(count);
}

/*******************************************************************************
*
*	FUNCTION:		Reset_Encoder_1_Count()
*
*	PURPOSE:		Resets the encoder 1 count to zero	
*
*	CALLED FROM:
*
*	PARAMETERS:		None.
*
*	RETURNS:		Nothing.
*
*	COMMENTS:
*
*******************************************************************************/
void Reset_Encoder_1_Count(void)
{
	// Since we're about to access the Encoder_1_Count variable,
	// which can also be modified in the interrupt service routine,
	// we'll briefly disable the encoder's interrupt to make sure
	// that the Encoder_1_Count variable doesn't get altered while
	// we're using it.
	INTCON3bits.INT2IE = 0;

	// Now we can set the value of the encoder count without fear
	// that we'll write corrupted data.
	Encoder_1_Count = 0;

	// Okay, we're done updating the encoder count, so turn the 
	// left encoder's interrupt back on.
	INTCON3bits.INT2IE = 1;
}

/*******************************************************************************
*
*	FUNCTION:		Encoder_1_Int_Handler()
*
*	PURPOSE:		If enabled, the encoder 1 interrupt handler is called when
*					the interrupt 1 pin changes from a logic 0 to a logic 1.
*
*	CALLED FROM:	user_routines_fast.c/InterruptHandlerLow()
*
*	PARAMETERS:		None.
*
*	RETURNS:		Nothing.
*
*	COMMENTS:
*
*******************************************************************************/
void Encoder_1_Int_Handler(void)
{
	// Encoder 1's phase a signal just transitioned from zero to one, causing 
	// this interrupt service routine to be called. We know that the encoder 
	// just rotated one count or "tick" so now check the logical state of the 
	// phase b signal to determine which way the the encoder shaft rotated.
	if(ENCODER_1_PHASE_B_PIN == 0)
	{
		Encoder_1_Count -= ENCODER_1_TICK_DELTA;
	}
	else
	{
		Encoder_1_Count += ENCODER_1_TICK_DELTA;
	}
}
#endif

#ifdef ENABLE_ENCODER_2
/*******************************************************************************
*
*	FUNCTION:		Get_Encoder_2_Count()
*
*	PURPOSE:		Gets the current number of encoder 2 counts.		
*
*	CALLED FROM:
*
*	PARAMETERS:		None.
*
*	RETURNS:		Number of encoder 2 counts since the last reset.
*
*	COMMENTS:
*
*******************************************************************************/
long Get_Encoder_2_Count(void)
{
	long count;

	// Since we're about to access the Encoder_2_Count variable,
	// which can also be modified in the interrupt service routine,
	// we'll briefly disable the encoder's interrupt to make sure
	// that the Encoder_2_Count variable doesn't get altered while
	// we're using it.
	INTCON3bits.INT3IE = 0;

	// Now we can get a local copy of the encoder count without fear
	// that we'll get corrupted data.
	count = Encoder_2_Count;

	// Okay, we have a local copy of the encoder count, so turn the 
	// encoder's interrupt back on.
	INTCON3bits.INT3IE = 1;

	// Return the encoder count to the caller.
	return(count);
}

/*******************************************************************************
*
*	FUNCTION:		Reset_Encoder_2_Count()
*
*	PURPOSE:		Resets the encoder 2 count to zero	
*
*	CALLED FROM:
*
*	PARAMETERS:		None.
*
*	RETURNS:		Nothing.
*
*	COMMENTS:
*
*******************************************************************************/
void Reset_Encoder_2_Count(void)
{
	// Since we're about to access the Encoder_2_Count variable,
	// which can also be modified in the interrupt service routine,
	// we'll briefly disable the encoder's interrupt to make sure
	// that the Encoder_2_Count variable doesn't get altered while
	// we're using it.
	INTCON3bits.INT3IE = 0;

	// Now we can set the value of the encoder count without fear
	// that we'll write corrupted data.
	Encoder_2_Count = 0;

	// Okay, we're done updating the encoder count, so turn the 
	// left encoder's interrupt back on.
	INTCON3bits.INT3IE = 1;
}

/*******************************************************************************
*
*	FUNCTION:		Encoder_2_Int_Handler()
*
*	PURPOSE:		If enabled, the encoder 2 interrupt handler is called when
*					the interrupt 2 pin changes from a logic 0 to a logic 1.
*
*	CALLED FROM:	user_routines_fast.c/InterruptHandlerLow()
*
*	PARAMETERS:		None.
*
*	RETURNS:		Nothing.
*
*	COMMENTS:
*
*******************************************************************************/
void Encoder_2_Int_Handler(void)
{
	// Encoder 2's phase a signal just transitioned from zero to one, causing 
	// this interrupt service routine to be called. We know that the encoder 
	// just rotated one count or "tick" so now check the logical state of the 
	// phase b signal to determine which way the the encoder shaft rotated.
	if(ENCODER_2_PHASE_B_PIN == 0)
	{
		Encoder_2_Count -= ENCODER_2_TICK_DELTA;
	}
	else
	{
		Encoder_2_Count += ENCODER_2_TICK_DELTA;
	}
}
#endif
