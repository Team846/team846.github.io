/*******************************************************************************
*
*	TITLE:		user_routines_fast.c 
*
*	VERSION:	0.4 (Beta)                           
*
*	DATE:		17-Feb-2004
*
*	AUTHOR:		R. Kevin Watson
*				kevinw@jpl.nasa.gov
*
*	COMMENTS:	This file is a drop-in replacement for innovation
*				first's user_routines_fast.c file.
*
*				If you use timer 0, you must compile your code
*				with ifi_alltimers.lib instead of ifi_library.lib.
*
*				Numbers within brackets refer to the PIC18F8520
*				data sheet page number where more information can
*				be found.
*
*				This code is specific to the EDU controller.
*
********************************************************************************
*
*	CHANGE LOG:
*
*	DATE         REV  DESCRIPTION
*	-----------  ---  ----------------------------------------------------------
*	08-Dec-2003  0.1  RKW Original
*	15-Dec-2003  0.2  RKW - Changed "interrupt" pragma on line 89 to "interruptlow"
*	22-Dec-2003  0.3  RKW - Fixed bug where timer 3/4 interrupts wound never be
*	                  handled because the wrong interrupt flags were used.
*	17-Feb-2004  0.4  RKW - Fixed bug where an interrupt handler would be called
*	                  even if that interrupt wasn't enabled.
*	                  RKW - Added additional interruptlow pragma options in
*	                  function InterruptHandlerLow()
*
*******************************************************************************/

#include "ifi_default.h"
#include "user_routines.h"
#include "printf_lib.h"
#include "gyro.h"
#include "adc.h"

/*******************************************************************************
*
*	FUNCTION:		InterruptVectorLow()
*
*	PURPOSE:		Installs the low priority interrupt code at the low
*					priority interrupt vector, which is a fixed place in
*					memory where the microcontroller will start executing
*					code when it detects an interrupt condition. Because
*					this place in memory, at address 24/0x18, is intended 
*					to contain only a very small amount of code, general
*					practice is to place a "goto" instruction here that
*					will point to the real interrupt handler somewhere else
*					in memory. More information on interrupts can be found
*					in section nine [87] of the PIC18F8520 data sheet.
* 
*	CALLED FROM:	Called in response to a hardware generated interrupt
*
*	PARAMETERS:		None
*
*	RETURNS:		Nothing
* 
*******************************************************************************/
#pragma code InterruptVectorLow = LOW_INT_VECTOR // put this code at address 0x18

void InterruptVectorLow (void)
{
	_asm
	goto InterruptHandlerLow  // jump to InterruptHandlerLow(), below
	_endasm
}

#pragma code

/*******************************************************************************
*
*	FUNCTION:		InterruptHandlerLow()
*
*	PURPOSE:		Determines which individual interrupt handler should
*					be called, clears the interrupt flag and then calls 
*					the interrupt handler.
* 
*	CALLED FROM:	InterruptVectorLow()
*
*	PARAMETERS:		None
*
*	RETURNS:		Nothing
* 
*******************************************************************************/
// Please make sure you know what you're doing before you alter this
// #pragma statement. More information can be found in section 2.9.2
// of the PIC C18 user's guide.
//
#pragma interruptlow InterruptHandlerLow save=PROD,section(".tmpdata")
// #pragma interruptlow InterruptHandlerLow save=PROD,section("MATH_DATA")
// #pragma interruptlow InterruptHandlerLow save=PROD
// #pragma interruptlow InterruptHandlerLow save=PROD,section("MATH_DATA"),section(".tmpdata")

void InterruptHandlerLow()     
{
	if (PIR1bits.ADIF && PIE1bits.ADIE) // ADC interrupt
	{
		PIR1bits.ADIF = 0; // clear the ADC interrupt flag
		ADC_Int_Handler(); // call the ADC interrupt handler (in gyro.c)
	}

	if (PIR1bits.TMR2IF && PIE1bits.TMR2IE) // timer 2 interrupt?
	{
		PIR1bits.TMR2IF = 0; // clear the timer 2 interrupt flag [92]
		Timer_2_Int_Handler(); // call the timer 2 interrupt handler (in gyro.c)
	}  
}

/*******************************************************************************
*
*	FUNCTION:		Process_Data_From_Local_IO()
*
*	PURPOSE:		Execute real-time code here. Code located here will
*					execute continuously as opposed to code located in 
*					user_routines.c/Process_Data_From_Master_uP(), which
*					only executes every 17 ms, when the master processor
*					sends a new data packet.
* 
*	CALLED FROM:	main.c/main()
*
*	PARAMETERS:		None
*
*	RETURNS:		Nothing
* 
*******************************************************************************/

void Process_Data_From_Local_IO(void)
{
	// new ADC data available?
	if(Get_ADC_Result_Count())
	{
		Process_Gyro_Data();
	
		Reset_ADC_Result_Count();
	}	
}
