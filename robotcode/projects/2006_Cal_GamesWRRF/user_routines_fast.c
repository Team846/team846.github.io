/*******************************************************************************
* FILE NAME: user_routines_fast.c <FRC VERSION>
*
* DESCRIPTION:
*  This file is where the user can add their custom code within the framework
*  of the routines below. 
*
* USAGE:
*  You can either modify this file to fit your needs, or remove it from your 
*  project and replace it with a modified copy. 
*
* OPTIONS:  Interrupts are disabled and not used by default.
*
*******************************************************************************/

#include "ifi_aliases.h"
#include "ifi_default.h"
#include "ifi_utilities.h"
#include "user_routines.h"
#include "serial_ports.h"
#include "interruptHandlers.h"

#include "utilities.h"

timestamp gInterruptTime={0};

/*** DEFINE USER VARIABLES AND INITIALIZE THEM HERE ***/


/*******************************************************************************
* FUNCTION NAME: InterruptVectorLow
* PURPOSE:       Low priority interrupt vector
* CALLED FROM:   nowhere by default
* ARGUMENTS:     none
* RETURNS:       void
* DO NOT MODIFY OR DELETE THIS FUNCTION 
*******************************************************************************/
#pragma code InterruptVectorLow = LOW_INT_VECTOR
void InterruptVectorLow (void)
{
  _asm
    goto InterruptHandlerLow  /*jump to interrupt routine*/
  _endasm
}


/*******************************************************************************
* FUNCTION NAME: InterruptHandlerLow
* PURPOSE:       Low priority interrupt handler
* If you want to use these external low priority interrupts or any of the
* peripheral interrupts then you must enable them in your initialization
* routine.  Innovation First, Inc. will not provide support for using these
* interrupts, so be careful.  There is great potential for glitchy code if good
* interrupt programming practices are not followed.  Especially read p. 28 of
* the "MPLAB(R) C18 C Compiler User's Guide" for information on context saving.
* CALLED FROM:   this file, InterruptVectorLow routine
* ARGUMENTS:     none
* RETURNS:       void
*******************************************************************************/
//If we have local variables in functions *called* by InterruptHandleLow,
//  then we must use the .section(".tmpdata")
//Timing info
//  without saving .tmpdata, overhead is 29+23=52 instructions (entry+exit)
//	saving .tmpdata, overhead is 107+101=208 instructions
//	5.2usec vs 20.8 usec at 40MHz
//  entry delay can result in wrong encoder B-channel reading
//	D.Giandomenico

#pragma code
//#pragma interruptlow InterruptHandlerLow save=PROD,section(".tmpdata")
#pragma interruptlow InterruptHandlerLow save=PROD
void InterruptHandlerLow ()     
{
	unsigned char Port_B;
	unsigned char Port_B_Delta;


		//Read Timer
	((char *) &gInterruptTime.tsL)[0] = TMR0L;	//must read low byte first
	((char *) &gInterruptTime.tsL)[1] = TMR0H;	//read in high byte

	//copy the high counter; NB: during interupts, timer overflow may be pending...
	if (INTCONbits.TMR0IF)
		gInterruptTime.tsH = Timer_High_Count+1;	//catch pending timer rollovers & correct
	else
		gInterruptTime.tsH = Timer_High_Count;




	if (PIR1bits.RC1IF && PIE1bits.RC1IE) // rx1 interrupt?
	{
		#ifdef ENABLE_SERIAL_PORT_ONE_RX
		Rx_1_Int_Handler(); // call the rx1 interrupt handler (in serial_ports.c)
		#endif
	}                              
	else if (PIR3bits.RC2IF && PIE3bits.RC2IE) // rx2 interrupt?
	{
		#ifdef ENABLE_SERIAL_PORT_TWO_RX
		Rx_2_Int_Handler(); // call the rx2 interrupt handler (in serial_ports.c)
		#endif
	} 
	else if (PIR1bits.TX1IF && PIE1bits.TX1IE) // tx1 interrupt?
	{
		#ifdef ENABLE_SERIAL_PORT_ONE_TX
		Tx_1_Int_Handler(); // call the tx1 interrupt handler (in serial_ports.c)
		#endif
	}                              
	else if (PIR3bits.TX2IF && PIE3bits.TX2IE) // tx2 interrupt?
	{
		#ifdef ENABLE_SERIAL_PORT_TWO_TX
		Tx_2_Int_Handler(); // call the tx2 interrupt handler (in serial_ports.c)
		#endif
	}
	else if (INTCON3bits.INT2IF && INTCON3bits.INT2IE) // encoder 1 interrupt?
	{ 
		INTCON3bits.INT2IF = 0; // clear the interrupt flag
		#ifdef ENABLE_CIM_RIGHT_ENCODER
		Interrupt_1_Int_Handler(); // call the left encoder interrupt handler (in encoder.c)
		#endif
	}
	else if (INTCON3bits.INT3IF && INTCON3bits.INT3IE) // encoder 2 interrupt?
	{
		INTCON3bits.INT3IF = 0; // clear the interrupt flag
		#ifdef ENABLE_CIM_LEFT_ENCODER
		Interrupt_2_Int_Handler(); // call right encoder interrupt handler (in encoder.c)
		#endif
	}
	else if (INTCONbits.RBIF && INTCONbits.RBIE) // encoder 3-6 interrupt?
	{
		Port_B = PORTB; // remove the "mismatch condition" by reading port b            
		INTCONbits.RBIF = 0; // clear the interrupt flag
		Port_B_Delta = Port_B ^ Old_Port_B; // determine which bits have changed
		Old_Port_B = Port_B; // save a copy of port b for next time around
	 
		if(Port_B_Delta & 0x10) // did external interrupt 3 change state?
		{
			#ifdef ENABLE_ENCODER_3
			Interrupt_3_Int_Handler(Port_B & 0x10 ? 1 : 0); // call the encoder 3 interrupt handler
			#endif
		}
		if(Port_B_Delta & 0x20) // did external interrupt 4 change state?
		{
			#ifdef ENABLE_ENCODER_4
			Interrupt_4_Int_Handler(Port_B & 0x20 ? 1 : 0); // call the encoder 4 interrupt handler
			#endif
		}
		if(Port_B_Delta & 0x40) // did external interrupt 5 change state?
		{
			#ifdef ENABLE_ENCODER_5
			Interrupt_5_Int_Handler(Port_B & 0x40 ? 1 : 0); // call the encoder 5 interrupt handler
			#endif
		}
		if(Port_B_Delta & 0x80) // did external interrupt 6 change state?
		{
			#ifdef ENABLE_ENCODER_6
			Interrupt_6_Int_Handler(Port_B & 0x80 ? 1 : 0); // call the encoder 6 interrupt handler
			#endif
		}
	}
	else if (INTCONbits.TMR0IF && INTCONbits.TMR0IE) // timer 0 interrupt?
	{
		INTCONbits.TMR0IF = 0; // clear the timer 0 interrupt flag [89]
		Timer_0_Int_Handler(); // call the timer 0 interrupt handler (in interrupts.c)
	}
}
