#include "printf_lib.h"
/*******************************************************************************
*
*	TITLE:		user_routines_fast.c 
*
*	VERSION:	0.6 (Beta)                           
*
*	DATE:		24-Feb-2004
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
*				This code is specific to the FRC robot controller.
*
********************************************************************************
*
*	CHANGE LOG:
*
*	DATE         REV  DESCRIPTION
*	-----------  ---  ----------------------------------------------------------
*	08-Dec-2003  0.1  RKW Original
*	10-Dec-2003  0.2  RKW - Modified for use on the full-size FRC robot controller
*	15-Dec-2003  0.3  RKW - Changed "interrupt" pragma on line 90 to "interruptlow"
*	22-Dec-2003  0.4  RKW - Fixed bug where timer 3/4 interrupts wound never be
*	                  handled because the wrong interrupt flags were used.
*	17-Feb-2004  0.5  RKW - Fixed bug where an interrupt handler would be called
*	                  even if that interrupt wasn't enabled.
*	                  RKW - Added additional interruptlow pragma options in
*	                  function InterruptHandlerLow()
*	                  RKW - Added call to Generate_Pwms() in User_Autonomous_Code()
*	24-Feb-2004  0.6  RKW - Added IFI's PWM and Relay initialization bug fix to
*	                  User_Autonomous_Code()
*
*******************************************************************************/
#include "ifi_aliases.h"
#include "ifi_default.h"
#include "ifi_utilities.h"
#include "user_routines.h"
#include "interrupts.h"

unsigned char Old_Port_B = 0xFF;// state of port b the last time
								// this function was called  

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
#pragma code InterruptVectorLow = LOW_INT_VECTOR

void InterruptVectorLow (void)
{
  _asm
    goto InterruptHandlerLow  // jump to InterruptHandlerLow()
  _endasm
}

#pragma code

/*******************************************************************************
*
*	FUNCTION:		InterruptHandlerLow()
*
*	PURPOSE:		Determines which individual interrupt handler
*					should be called, clears the interrupt flag and
*					then calls the interrupt handler.
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
// #pragma interruptlow InterruptHandlerLow save=PROD,section(".tmpdata")
// #pragma interruptlow InterruptHandlerLow save=PROD,section("MATH_DATA")
// #pragma interruptlow InterruptHandlerLow save=PROD
#pragma interruptlow InterruptHandlerLow save=PROD,section("MATH_DATA"),section(".tmpdata")

void InterruptHandlerLow()     
{                               
	unsigned char Port_B;
	unsigned char Port_B_Delta;       
	
	if (INTCONbits.TMR0IF && INTCONbits.TMR0IE) // timer 0 interrupt?
	{
		INTCONbits.TMR0IF = 0; // clear the timer 0 interrupt flag [89]
		Timer_0_Int_Handler(); // call the timer 0 interrupt handler (in interrupts.c)
	}
	else if (PIR1bits.TMR1IF && PIE1bits.TMR1IE) // timer 1 interrupt?
	{
		PIR1bits.TMR1IF = 0; // clear the timer 1 interrupt flag [92]
		Timer_1_Int_Handler(); // call the timer 1 interrupt handler (in interrupts.c)
	}  
	else if (PIR1bits.TMR2IF && PIE1bits.TMR2IE) // timer 2 interrupt?
	{
		PIR1bits.TMR2IF = 0; // clear the timer 2 interrupt flag [92]
		Timer_2_Int_Handler(); // call the timer 2 interrupt handler (in interrupts.c)
	}  
	else if (PIR2bits.TMR3IF && PIE2bits.TMR3IE) // timer 3 interrupt?
	{
		PIR2bits.TMR3IF = 0; // clear the timer 3 interrupt flag [93]
		Timer_3_Int_Handler(); // call the timer 3 interrupt handler (in interrupts.c)
	}  
	else if (PIR3bits.TMR4IF && PIE3bits.TMR4IE) // timer 4 interrupt?
	{
		PIR3bits.TMR4IF = 0; // clear the timer 4 interrupt flag [94]
		Timer_4_Int_Handler(); // call the timer 4 interrupt handler (in interrupts.c)
	}  
	else if (INTCON3bits.INT2IF && INTCON3bits.INT2IE) // external interrupt 1?
	{ 
		INTCON3bits.INT2IF = 0; // clear the interrupt flag [91]
		Int_1_Handler(); // call the interrupt 1 handler (in interrupts.c)
	}
	else if (INTCON3bits.INT3IF && INTCON3bits.INT3IE) // external interrupt 2?
	{
		INTCON3bits.INT3IF = 0; // clear the interrupt flag [91]
		Int_2_Handler(); // call the interrupt 2 handler (in interrupts.c)
	}
	else if (INTCONbits.RBIF && INTCONbits.RBIE) // external interrupts 3 through 6?
	{
		Port_B = PORTB; // remove the "mismatch condition" by reading port b            
		INTCONbits.RBIF = 0; // clear the interrupt flag [89]
		Port_B_Delta = Port_B ^ Old_Port_B; // determine which bits have changed
		Old_Port_B = Port_B; // save a copy of port b for next time around
	 
		if(Port_B_Delta & 0x10) // did external interrupt 3 change state?
		{
			Int_3_Handler(Port_B & 0x10 ? 1 : 0); // call the interrupt 3 handler (in interrupts.c)
		}
		if(Port_B_Delta & 0x20) // did external interrupt 4 change state?
		{
			Int_4_Handler(Port_B & 0x20 ? 1 : 0); // call the interrupt 4 handler (in interrupts.c)
		}
		if(Port_B_Delta & 0x40) // did external interrupt 5 change state?
		{
			Int_5_Handler(Port_B & 0x40 ? 1 : 0); // call the interrupt 5 handler (in interrupts.c)
		}
		if(Port_B_Delta & 0x80) // did external interrupt 6 change state?
		{
			Int_6_Handler(Port_B & 0x80 ? 1 : 0); // call the interrupt 6 handler (in interrupts.c)
		}
	}
}

/*******************************************************************************
*
*	FUNCTION:		User_Autonomous_Code()
*
*	PURPOSE:		Executes user's code during autonomous robot operation.
* 					You should modify this routine by adding code which you
*					wish to run in autonomous mode. This executes every 26.2ms.
*
*	CALLED FROM:	main.c/main()
*
*	PARAMETERS:		None
*
*	RETURNS:		Nothing
* 
*******************************************************************************/
void User_Autonomous_Code(void)
{
//	// Initialize all PWM and relay outputs when entering autonomous mode or else
//	// they will be initialized to the last values read from the joystick(s).
//	// Even when disabled, the operator interface is still sending user input
//	// to the robot controller.
//    pwm01 = pwm02 = pwm03 = pwm04 = pwm05 = pwm06 = pwm07 = pwm08 = 127;
//    pwm09 = pwm10 = pwm11 = pwm12 = pwm13 = pwm14 = pwm15 = pwm16 = 127;
//    relay1_fwd = relay1_rev = relay2_fwd = relay2_rev = 0;
//    relay3_fwd = relay3_rev = relay4_fwd = relay4_rev = 0;
//    relay5_fwd = relay5_rev = relay6_fwd = relay6_rev = 0;
//    relay7_fwd = relay7_rev = relay8_fwd = relay8_rev = 0;
//
//	while (autonomous_mode)
//	{
//		if (statusflag.NEW_SPI_DATA)
//		{
//			Getdata(&rxdata);   // bad things will happen if you move or delete this
//
//			// Autonomous code goes here.
//
//			Generate_Pwms(pwm13,pwm14,pwm15,pwm16);
//
//			Putdata(&txdata);   // even more bad things will happen if you mess with this
//		}
//	}
}

/*******************************************************************************
*
*	FUNCTION:		Process_Data_From_Local_IO()
*
*	PURPOSE:		Execute real-time code here. Code located here will
*					execute continuously as opposed to code located in 
*					user_routines.c/Process_Data_From_Master_uP(), which
*					only executes when the master processor	sends a new 
*					data packet.
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
#ifndef _FRC_BOARD
	static int counter=-3;
	static int subcount=0;
	subcount++;
	if (subcount >= 1000)
	{
		subcount = 0;
		counter++;
		printf("process local IO %d\n", (int) counter);
	}
#endif //_FRC_BOARD
}
