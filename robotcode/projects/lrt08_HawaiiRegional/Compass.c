// Compass.c
// Get Compass direction from Acroname R117-Compass
// website:  http://www.acroname.com


#include "_ifi_default.h"
#include "_ifi_aliases.h"
#include "user_routines.h"

#include "_interruptHandlers.h"


/*******************************************************************************
*
*	FUNCTION:		Initialize_Timer_1()
*
*	PURPOSE:		Initializes the timer 1 hardware.
*
*	CALLED FROM:	user_routines.c/User_Initialization()
*
*	PARAMETERS:		None
*
*	RETURNS:		Nothing
*
*	COMMENTS:		Place "#include "interrupts.h" in the includes section
*					of user_routines.c then call Initialize_Timer_1() in
*					user_routines.c/User_Initialization().
*
*					Timer 1 documentation starts on page 135 of the data sheet.
*
*******************************************************************************/
static void Initialize_Timer_1(void)  
{
	TMR1L = 0x00;			// least significant 8-bits of the timer 1 register (this is readable and writable)
	TMR1H = 0x00;			// most significant 8-bits of the timer 1 register (this is readable and writable)
							//
	T1CONbits.T1CKPS0 = 1;	// T1CSP1 T1CSP0
	T1CONbits.T1CKPS1 = 1;	//   0      0		1:1 prescaler (clock=10MHz/each tick=100ns)
							//   0      1		1:2 prescaler (clock=5MHz/each tick=200ns)
							//   1      0		1:4 prescaler (clock=2.5MHz/each tick=400ns)
							//   1      1		1:8 prescaler (clock=1.25MHz/each tick=800ns)
							//
	T1CONbits.T1OSCEN = 0;	// 0: timer 1 oscillator disabled (leave at 0 to allow the use of an external clock)
							// 1: timer 1 oscillator enabled (can't be used because of hardware constraints)
							//
	T1CONbits.TMR1CS = 0;	// 0: use the internal clock
							// 1: use an external clock on RC0/T1OSO/T13CLK (rc_dig_in14 on full-size controller)
							//
	T1CONbits.RD16 = 1;		// 0: timer 1 register operations are done in two 8-bit accesses
							// 1: timer 1 register operations are done in one 16-bit access
							//    In this mode, reading TMR1L will latch a copy of TMR1H into a buffer
							//    mapped to the TMR1H memory address. Conversely, a write to the buffer
							//    followed by a write to the TMR1L register will update the entire 16-bit
							//    timer at once. This solves the problem where the timer may overflow
							//    between two 8-bit accesses. Here's an example of how to do a 16-bit read:
							//
							//    unsigned char Temp_Buf; // 8-bit temporary buffer
							//    unsigned int Timer_Snapshot; // 16-bit variable
							//
							//    Temp_Buf = TMR1L; // TMR1L must be read before TMR1H
							//    Timer_Snapshot = TMR1H;
		 					//    Timer_Snapshot <<= 8; // move TMR1H data to the upper half of the variable
							//    Timer_Snapshot += Temp_Buf; // we now have all sixteen bits  
							// 
	IPR1bits.TMR1IP = 0;	// 0: timer 1 overflow interrupt is low priority (leave at 0 on IFI controllers)
							// 1: timer 1 overflow interrupt is high priority
							//
	PIR1bits.TMR1IF = 0;	// 0: timer 1 overflow hasn't happened (set to 0 before enabling the interrupt)
							// 1: timer 1 overflow has happened
							//
	PIE1bits.TMR1IE = 0;	// 0: disable timer 1 interrupt on overflow (i.e., a transition from FFFF->0)
							// 1: enable timer 1 interrupt on overflow (i.e., a transition from FFFF->0)
							//	
	T1CONbits.TMR1ON = 0;	// 0: timer 1 is disabled
							// 1: timer 1 is enabled (running)
}



unsigned int gCompassDirection;

//Initialize_Compass****************************
//	initializes the compass
//*************************************************
void Initialize_Compass(void)
{
	
	
	INTCONbits.RBIE = 0;	// make sure interrupt is disabled
	digital_io_05 = OUTPUT;	

/*	
	//debug
	rc_dig_out01 = 0;
	
	*/
	
	Initialize_Timer_1();
	
	digital_io_05 = INPUT;
	INTCONbits.RBIF = 0;	// make sure flag is zero
	INTCONbits.RBIE = 1;	// enable interrupt
	
	
}//end Initialize_Compass



void GetCompassHeading(void)
{
	INTCONbits.RBIE = 0;	// make sure interrupt is disabled
//	T1CONbits.TMR1ON = 0;	// ensure timer 1 is disabled
//	T1CONbits.T1CKPS0 = 1;	// T1CSP1 T1CSP0
//	T1CONbits.T1CKPS1 = 1;	//   0      0		1:1 prescaler (clock=10MHz/each tick=100ns)
//							//   0      1		1:2 prescaler (clock=5MHz/each tick=200ns)
//							//   1      0		1:4 prescaler (clock=2.5MHz/each tick=400ns)
//							//   1      1		1:8 prescaler (clock=1.25MHz/each tick=800ns)
//							//
//	TMR1H = 0x00;
//	TMR1L = 0x00;
	
	Initialize_Timer_1();
	
	digital_io_05 = INPUT;
	INTCONbits.RBIF = 0;	// make sure flag is zero
	INTCONbits.RBIE = 1;	// enable interrupt
	
}//end GetCompassHeading
