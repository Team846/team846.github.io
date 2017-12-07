/*******************************************************************************
*
*	TITLE:		interrupts.c 
*
*	VERSION:	0.2 (Beta)                           
*
*	DATE:		25-Feb-2004
*
*	AUTHOR:		R. Kevin Watson
*				kevinw@jpl.nasa.gov
*
*	COMMENTS:	This file contains template interrupt and timer
*				initialization/handling code for the IFI EDU and
*				FRC robot controllers.
*
*				Numbers within brackets refer to the PIC18F8520
*				data sheet page number where more information can
*				be found.
*
*               This file best viewed with tabs set to four.
*
********************************************************************************
*
*	Change log:
*
*	DATE         REV  DESCRIPTION
*	-----------  ---  ----------------------------------------------------------
*	22-Dec-2003  0.1  RKW Original
*	25-Feb-2004  0.2  RKW - Added the ability to clear the interrupt flag before
*	                  enabling the interrupt.
*
*******************************************************************************/

#include "ifi_picdefs.h"
#include "interrupts.h"
#include "lrtUtilities.h"	//for clock variables
#include "lrtConnections.h"	//for robot interrup connections

/*******************************************************************************
*
*	FUNCTION:		Initialize_Interrupts()
*
*	PURPOSE:		Initializes the interrupt hardware.
*
*	CALLED FROM:	user_routines.c/User_Initialization()
*
*	PARAMETERS:		None
*
*	RETURNS:		Nothing
*
*	COMMENTS:		Place "#include "interrupts.h" in the includes section
*					of user_routines.c then call Initialize_Interrupts() in
*					user_routines.c/User_Initialization().
*
*******************************************************************************/

void Initialize_Interrupts(void)  
{
	// initialize external interrupt 1 (INT2 on user 18F8520)
	TRISBbits.TRISB2 = 1;		// make sure the RB2/INT2 pin is configured as an input [108]
								//
	INTCON3bits.INT2IP = 0;		// 0: interrupt 1 is low priority (leave at 0 for IFI controllers) [91]
								// 1: interrupt 1 is high priority
								//
	INTCON2bits.INTEDG2 = 0;	// 0: trigger on the falling-edge [90]
								// 1: trigger on the rising-edge
								//
	INTCON3bits.INT2IF = 0;		// 0: external interrupt 1 hasn't happened (set to 0 before enabling the interrupt) [91]
								// 1: external interrupt 1 has happened
								//
	INTCON3bits.INT2IE = 1;		// 0: disable interrupt	1 [91]
								// 1: enable interrupt 1
	
	// initialize external interrupt 2 (INT3 on user 18F8520)
	TRISBbits.TRISB3 = 1;		// make sure the RB3/CCP2/INT3 pin is configured as an input [108]
								//
	INTCON2bits.INT3IP = 0;		// 0: interrupt 2 is low priority (leave at 0 for IFI controllers) [90]
								// 1: interrupt 2 is high priority
								//
	INTCON2bits.INTEDG3 = 0;	// 0: trigger on the falling-edge [90]
								// 1: trigger on the rising-edge
								//
	INTCON3bits.INT3IF = 0;		// 0: external interrupt 2 hasn't happened (set to 0 before enabling the interrupt) [91]
								// 1: external interrupt 2 has happened
								//
	INTCON3bits.INT3IE = 1;		// 0: disable interrupt	2 [91]
								// 1: enable interrupt 2

	// initialize external interrupts 3-6 (KBI0 - KBI3 on user 18F8520)
	TRISBbits.TRISB4 = 1;		// make sure the RB4/HBI0 pin is configured as an input [108]
	TRISBbits.TRISB5 = 1;		// make sure the RB5/KBI1/PGM pin is configured as an input [108]
	TRISBbits.TRISB6 = 1;		// make sure the RB6/KBI2/PGC pin is configured as an input [108]
	TRISBbits.TRISB7 = 1;		// make sure the RB7/KBI3/PGD pin is configured as an input	[108]
								//
  	INTCON2bits.RBIP = 0;		// 0: interrupts 3-6 are low priority (leave at 0 for IFI controllers) [90]
								// 1: interrupts 3-6 are high priority
								//
	Old_Port_B = PORTB;			// initialize the Old_Port_B variable (in user_routines_fast.c)
								//
	INTCONbits.RBIF = 0;		// 0: none of the interrupt 3-6 pins has changed state (set to 0 before enabling the interrupts) [89]
								// 1: at least one of the interrupt 3-6 pins has changed state
								//
	INTCONbits.RBIE = 0;		// 0: disable interrupts 3-6 [89]
								// 1: enable interrupts 3-6
}						  

/*******************************************************************************
*
*	FUNCTION:		Initialize_Timer_0()
*
*	PURPOSE:		Initializes the timer 0 hardware.
*
*	CALLED FROM:	user_routines.c/User_Initialization()
*
*	PARAMETERS:		None
*
*	RETURNS:		Nothing
*
*	COMMENTS:		Place "#include "interrupts.h" in the includes section
*					of user_routines.c then call Initialize_Timer_0() in
*					user_routines.c/User_Initialization().
*
*					Timer 0 documentation starts on page 131 of the data sheet.
*
*					If you must use timer 0, make sure to substitute the
*					library ifi_alltimers.lib for ifi_library.lib
*
*******************************************************************************/
void Initialize_Timer_0(void)  
{
	TMR0L = 0x00;			// least significant 8-bits of the timer 0 register (this is readable and writable)
	TMR0H = 0x00;			// most significant 8-bits of the timer 0 register (this is readable and writable)
							//
	T0CONbits.T0PS0 = 0;	// T0PS2 TOPS1 T0PS0
	T0CONbits.T0PS1 = 0;	//   0     0     0		1:2 prescaler (clock=5MHz/each tick=200ns)
	T0CONbits.T0PS2 = 0;	//   0     0     1		1:4 prescaler (clock=2.5MHz/each tick=400ns)
							//   0     1     0		1:8 prescaler (clock=1.25MHz/each tick=800ns)
							//   0     1     1		1:16 prescaler (clock=625KHz/each tick=1.6us)
							//   1     0     0		1:32 prescaler (clock=312.5KHz/each tick=3.2us)
							//   1     0     1		1:64 prescaler (clock=156.25KHz/each tick=6.4us)
							//   1     1     0		1:128 prescaler (clock=78.125KHz/each tick=12.8us)
							//   1     1     1		1:256 prescaler (clock=39.0625 KHz/each tick=25.6us)
							//
	T0CONbits.PSA = 1;		// 0: use the prescaler to derive the timer clock
							// 1: don't use the prescaler (clock=10MHz/each tick=100ns)
							//
	T0CONbits.T0SE = 0;		// 0: when using an external clock, timer increments on the rising-edge
							// 1: when using an external clock, timer increments on the falling-edge
							//
	T0CONbits.T0CS = 0;		// 0: use the internal clock (leave at 0)
							// 1: use an external clock on RA4/T0CKI (don't use - not available on IFI controllers)
							//
	T0CONbits.T08BIT = 0;	// 0: timer 0 is configured as a 16-bit timer/counter
							// 1: timer 0 is configured as an 8-bit timer/counter
							//
	INTCON2bits.TMR0IP = 0;	// 0: timer 0 overflow interrupt is low priority (leave at 0 for IFI controllers)
							// 1: timer 0 overflow interrupt is high priority
							//
	INTCONbits.TMR0IF = 0;	// 0: timer 0 overflow hasn't happened (set to 0 before enabling the interrupt)
							// 1: timer 0 overflow has happened
							//
	INTCONbits.TMR0IE = 0;	// 0: disable timer 0 interrupt on overflow (i.e., a transition from FFFF->0 or FF->0)
							// 1: enable timer 0 interrupt on overflow (i.e., a transition from FFFF->0 or FF->0)
							//	
	T0CONbits.TMR0ON = 0;	// 0: timer 0 is disabled
							// 1: timer 0 is enabled (running)
}

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
void Initialize_Timer_1(void)  
{
	TMR1L = 0x00;			// least significant 8-bits of the timer 1 register (this is readable and writable)
	TMR1H = 0x00;			// most significant 8-bits of the timer 1 register (this is readable and writable)
							//
	T1CONbits.T1CKPS0 = 0;	// T1CSP1 T1CSP0
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
	PIE1bits.TMR1IE = 1;	// 0: disable timer 1 interrupt on overflow (i.e., a transition from FFFF->0)
							// 1: enable timer 1 interrupt on overflow (i.e., a transition from FFFF->0)
							//	
	T1CONbits.TMR1ON = 1;	// 0: timer 1 is disabled
							// 1: timer 1 is enabled (running)
}

/*******************************************************************************
*
*	FUNCTION:		Initialize_Timer_2()
*
*	PURPOSE:		Initializes the timer 2 hardware.
*
*	CALLED FROM:	user_routines.c/User_Initialization()
*
*	PARAMETERS:		None
*
*	RETURNS:		Nothing
*
*	COMMENTS:		Place "#include "interrupts.h" in the includes section
*					of user_routines.c then call Initialize_Timer_2() in
*					user_routines.c/User_Initialization().
*
*					Timer 2 documentation starts on page 141 of the data sheet.
*
*******************************************************************************/
void Initialize_Timer_2(void)  
{
	TMR2 = 0x00;			// 8-bit timer 2 register (this is readable and writable)
							//
	PR2	= 0xFF;				// timer 2 period register - timer 2 increments to this 
							// value then resets to zero on the next clock and starts
							// all over again
							//
	T2CONbits.T2OUTPS0 = 0;	// T2OUTPS3 T2OUTPS2 T2OUTPS1 T2OUTPS0
	T2CONbits.T2OUTPS1 = 0;	//    0        0        0        0		1:1 postscaler
	T2CONbits.T2OUTPS2 = 0;	//    0        0        0        1		1:2 postscaler
	T2CONbits.T2OUTPS3 = 0;	//    0        0        1        0		1:3 postscaler
							//    0        0        1        1		1:4 postscaler
							//    0        1        0        0		1:5 postscaler
							//    0        1        0        1		1:6 postscaler
							//    0        1        1        0		1:7 postscaler
							//    0        1        1        1		1:8 postscaler
							//    1        0        0        0		1:9 postscaler
							//    1        0        0        1		1:10 postscaler
							//    1        0        1        0		1:11 postscaler
							//    1        0        1        1		1:12 postscaler
							//    1        1        0        0		1:13 postscaler
							//    1        1        0        1		1:14 postscaler
							//    1        1        1        0		1:15 postscaler
							//    1        1        1        1		1:16 postscaler
							//
	T2CONbits.T2CKPS0 = 0;	// T2CKPS1  T2CKPS0
	T2CONbits.T2CKPS1 = 0;	//    0        0	1:1 prescaler (clock = 10MHz/each tick=100ns)
							//    0        1	1:4 prescaler (clock = 2.5MHz/each tick=400ns)
							//    1        x	1:16 prescaler (clock = 625KHz/each tick=1.6us) (T2CKPS0 doesn't matter)
							//
	IPR1bits.TMR2IP = 0;	// 0: timer 2 interrupt is low priority (leave at 0 for IFI controllers)
							// 1: timer 2 interrupt is high priority
							//
	PIR1bits.TMR2IF = 0;	// 0: TMR2 to PR2 match hasn't happened (set to 0 before enabling the interrupt)
							// 1: TMR2 to PR2 match has happened
							//
	PIE1bits.TMR2IE = 0;	// 0: disable timer 2 interrupt on PR2 match
							// 1: enable timer 2 interrupt on PR2 match
							//    if the prescaler is enabled (i.e., greater than 1:1), this
							//    match will occur n times (where n is the postscaler value)
							//    before an interrupt will be generated
							//
	T2CONbits.TMR2ON = 0;	// 0: timer 2 is disabled
							// 1: timer 2 is enabled (running)
}

/*******************************************************************************
*
*	FUNCTION:		Initialize_Timer_3()
*
*	PURPOSE:		Initializes the timer 3 hardware.
*
*	CALLED FROM:	user_routines.c/User_Initialization()
*
*	PARAMETERS:		None
*
*	RETURNS:		Nothing
*
*	COMMENTS:		Place "#include "interrupts.h" in the includes section
*					of user_routines.c then call Initialize_Timer_3() in
*					user_routines.c/User_Initialization().
*
*					Timer 3 documentation starts on page 143 of the data sheet.
*
*******************************************************************************/
void Initialize_Timer_3(void)  
{
	TMR3L = 0x00;			// least significant 8-bits of the timer 3 register (this is readable and writable)
	TMR3H = 0x00;			// most significant 8-bits of the timer 3 register (this is readable and writable)
							//
	T3CONbits.T3CKPS0 = 0;	// T3CKPS1 T3CKPS0
	T3CONbits.T3CKPS1 = 0;	//    0       0		1:1 prescaler (clock=10MHz/each tick=100ns)
							//    0       1		1:2 prescaler (clock=5MHz/each tick=200ns)
							//    1       0		1:4 prescaler (clock=2.5MHz/each tick=400ns)
							//    1       1		1:8 prescaler (clock=1.25MHz/each tick=800ns)
							//
	T3CONbits.TMR3CS = 0;	// 0: use the internal clock
							// 1: use an external clock on RC0/T1OSO/T13CLK (rc_dig_in14 on full-size controller)
							//
	T3CONbits.T3SYNC = 1;	// 0: do not synchronize the external clock (this can cause timing problems)
							// 1: synchronize the external clock to the PIC18F8520s internal clock, which is desirable
							//
	T3CONbits.RD16 = 1;		// 0: timer 3 register operations are done in two 8-bit accesses
							// 1: timer 3 register operations are done in one 16-bit access
							//    In this mode, reading TMR3L will latch a copy of TMR3H into a buffer
							//    mapped to the TMR3H memory address. Conversely, a write to the buffer
							//    followed by a write to the TMR3L register will update the entire 16-bit
							//    timer at once. This solves the problem where the timer may overflow
							//    between two 8-bit accesses. Here's an example of how to do a 16-bit read:
							//
							//    unsigned char Temp_Buf; // 8-bit temporary buffer
							//    unsigned int Timer_Snapshot; // 16-bit variable
							//
							//    Temp_Buf = TMR3L; // TMR3L must be read before TMR3H
							//    Timer_Snapshot = TMR3H;
		 					//    Timer_Snapshot <<= 8; // move TMR3H data to the upper half of the variable
							//    Timer_Snapshot += Temp_Buf; // we now have all sixteen bits  
							// 
	IPR2bits.TMR3IP = 0;	// 0: timer 3 overflow interrupt is low priority (leave at 0 on IFI controllers)
							// 1: timer 3 overflow interrupt is high priority
							//
	PIR2bits.TMR3IF = 0;	// 0: timer 3 overflow hasn't happened (set to 0 before enabling the interrupt)
							// 1: timer 3 overflow has happened
							//
	PIE2bits.TMR3IE = 0;	// 0: disable timer 3 interrupt on overflow (i.e., a transition from FFFF->0)
							// 1: enable timer 3 interrupt on overflow (i.e., a transition from FFFF->0)
							//	
	T3CONbits.TMR3ON = 0;	// 0: timer 3 is disabled
							// 1: timer 3 is enabled (running)
}

/*******************************************************************************
*
*	FUNCTION:		Initialize_Timer_4()
*
*	PURPOSE:		Initializes the timer 4 hardware.
*
*	CALLED FROM:	user_routines.c/User_Initialization()
*
*	PARAMETERS:		None
*
*	RETURNS:		Nothing
*
*	COMMENTS:		Place "#include "interrupts.h" in the includes section
*					of user_routines.c then call Initialize_Timer_4() in
*					user_routines.c/User_Initialization().
*
*					Timer 4 documentation starts on page 147 of the data sheet.
*
*******************************************************************************/
void Initialize_Timer_4(void)  
{
	TMR4 = 0x00;			// 8-bit timer 4 register (this is readable and writable)
							//
	PR4	= 0xFF;				// timer 4 period register - timer 4 increments to this 
							// value then resets to zero on the next clock and starts
							// all over again
							//
	T4CONbits.T4OUTPS0 = 0;	// T4OUTPS3 T4OUTPS2 T4OUTPS1 T4OUTPS0
	T4CONbits.T4OUTPS1 = 0;	//    0        0        0        0		1:1 postscaler
	T4CONbits.T4OUTPS2 = 0;	//    0        0        0        1		1:2 postscaler
	T4CONbits.T4OUTPS3 = 0;	//    0        0        1        0		1:3 postscaler
							//    0        0        1        1		1:4 postscaler
							//    0        1        0        0		1:5 postscaler
							//    0        1        0        1		1:6 postscaler
							//    0        1        1        0		1:7 postscaler
							//    0        1        1        1		1:8 postscaler
							//    1        0        0        0		1:9 postscaler
							//    1        0        0        1		1:10 postscaler
							//    1        0        1        0		1:11 postscaler
							//    1        0        1        1		1:12 postscaler
							//    1        1        0        0		1:13 postscaler
							//    1        1        0        1		1:14 postscaler
							//    1        1        1        0		1:15 postscaler
							//    1        1        1        1		1:16 postscaler
							//
	T4CONbits.T4CKPS0 = 0;	// T4CKPS1  T4CKPS0
	T4CONbits.T4CKPS1 = 0;	//    0        0	1:1 prescaler (clock = 10MHz/each tick=100ns)
							//    0        1	1:4 prescaler (clock = 2.5MHz/each tick=400ns)
							//    1        x	1:16 prescaler (clock = 625KHz/each tick=1.6us) (T2CKPS0 doesn't matter)
							//
	IPR3bits.TMR4IP = 0;	// 0: timer 4 interrupt is low priority (leave at 0 for IFI controllers)
							// 1: timer 4 interrupt is high priority
							//
	PIR3bits.TMR4IF = 0;	// 0: TMR4 to PR4 match hasn't happened (set to 0 before enabling the interrupt)
							// 1: TMR4 to PR4 match has happened
							//
	PIE3bits.TMR4IE = 0;	// 0: disable timer 4 interrupt on PR4 match
							// 1: enable timer 4 interrupt on PR4 match
							//    if the prescaler is enabled (i.e., greater than 1:1), this
							//    match will occur n times (where n is the postscaler value)
							//    before an interrupt will be generated
							//
	T4CONbits.TMR4ON = 0;	// 0: timer 4 is disabled
							// 1: timer 4 is enabled (running)
}

/*******************************************************************************
*
*	FUNCTION:		Timer_0_Int_Handler()
*
*	PURPOSE:		If enabled, the timer 0 interrupt handler is called when
*					the TMR0 register overflows	and rolls over to zero.
*
*	CALLED FROM:	user_routines_fast.c/InterruptHandlerLow()
*
*	PARAMETERS:		None
*
*	RETURNS:		Nothing
*
*	COMMENTS:		The timer 0 module is documented in the PIC18F8520
*					data sheet starting on page 131.
*
*******************************************************************************/
void Timer_0_Int_Handler(void)
{
	// this function will be called when a timer 0 interrupt occurs
}

/*******************************************************************************
*
*	FUNCTION:		Timer_1_Int_Handler()
*
*	PURPOSE:		If enabled, the timer 1 interrupt handler is called when
*					the TMR1 register overflows	and rolls over to zero.
*
*	CALLED FROM:	user_routines_fast.c/InterruptHandlerLow()
*
*	PARAMETERS:		None
*
*	RETURNS:		Nothing
*
*	COMMENTS: 		The timer 1 module is documented in the PIC18F8520
*					data sheet starting on page 135.
*
*******************************************************************************/
//38.1470	 0.9961
//called 26.2ms ( 38.1470Hz = 10MHz/2^18)
void Timer_1_Int_Handler(void)
{
	gClock.cycle++;
	if (gClock.cycle38==37)
	{
		gClock.cycle38=0;
		gClock.secondz++;
	}
	else
		gClock.cycle38++;
}

/*******************************************************************************
*
*	FUNCTION:		Timer_2_Int_Handler()
*
*	PURPOSE:		If enabled, the timer 2 interrupt handler is called when the
*					TMR2 register matches the value stored in the PR2 register.
*
*	CALLED FROM:	user_routines_fast.c/InterruptHandlerLow()
*
*	PARAMETERS:		None
*
*	RETURNS:		Nothing
*
*	COMMENTS: 		The timer 2 module is documented in the PIC18F8520
*					data sheet starting on page 141.
*
*******************************************************************************/
void Timer_2_Int_Handler(void)
{
	// this function will be called when a timer 2 interrupt occurs
}

/*******************************************************************************
*
*	FUNCTION:		Timer_3_Int_Handler()
*
*	PURPOSE:		If enabled, the timer 3 interrupt handler is called when
*					the TMR3 register overflows	and rolls over to zero.
*
*	CALLED FROM:	user_routines_fast.c/InterruptHandlerLow()
*
*	PARAMETERS:		None
*
*	RETURNS:		Nothing
*
*	COMMENTS: 		The timer 3 module is documented in the PIC18F8520
*					data sheet starting on page 143.
*
*******************************************************************************/
void Timer_3_Int_Handler(void)
{
	// this function will be called when a timer 3 interrupt occurs
}

/*******************************************************************************
*
*	FUNCTION:		Timer_4_Int_Handler()
*
*	PURPOSE:		If enabled, the timer 4 interrupt handler is called when the
*					TMR4 register matches the value stored in the PR4 register.
*
*	CALLED FROM:	user_routines_fast.c/InterruptHandlerLow()
*
*	PARAMETERS:		None
*
*	RETURNS:		Nothing
*
*	COMMENTS: 		The timer 4 module is documented in the PIC18F8520
*					data sheet starting on page 147.
*
*******************************************************************************/
void Timer_4_Int_Handler(void)
{
	// this function will be called when a timer 4 interrupt occurs
}

/*******************************************************************************
*
*	FUNCTION:		Int_1_Handler()
*
*	PURPOSE:		If enabled, the interrupt 1 handler is called when the
*					interrupt 1/digital input 1 pin changes logic level. The 
*					edge that the interrupt 1 pin reacts to is programmable 
*					(see comments in the Initialize_Interrupts() function, 
*					above).
*
*	CALLED FROM:	user_routines_fast.c/InterruptHandlerLow()
*
*	PARAMETERS:		None
*
*	RETURNS:		Nothing
*
*	COMMENTS:		The PIC18F8520's RB2/INT2 pin on port b is mapped to
*					interrupt 1 on the EDU-RC and digital input 1 on the
*					FRC-RC [108].
*
*******************************************************************************/
void Int_1_Handler(void)
{
	//encoder's 'B' output determines direction of motion
	EncoderLeft.direction=0;
	if (!mEncoderLeftBin)
	{
		EncoderLeft.direction=0;	//fwd
		EncoderLeft.posNow++;
	}
	else {
		EncoderLeft.direction=1;	//reverse
		EncoderLeft.posNow--;
	}

	EncoderLeft.t2.ts = EncoderLeft.t1.ts; 
	EncoderLeft.t1.ts = EncoderLeft.t0.ts;

	//copy the current time. Byte copy to preserve byte write order
	((char *) &EncoderLeft.t0.tsL)[0] = TMR1L;	//must read low byte first
	((char *) &EncoderLeft.t0.tsL)[1] = TMR1H;	//read in high byte
//	EncoderLeft.t0.tsL = TMR1;	//should watch for rollovers, but we don't
	EncoderLeft.t0.tsH = gClock.cycle;	//should watch for rollovers

	//save the last difference
//	EncoderLeft.d1.ts = EncoderLeft.d0.ts;

	//subtract over two ticks, average and div by 32 ( /2; then /16)
	EncoderLeft.d0.ts = (EncoderLeft.t0.ts - EncoderLeft.t2.ts)>>5;
	

}


/*******************************************************************************
*
*	FUNCTION:		Int_2_Handler()
*
*	PURPOSE:		If enabled, the interrupt 2 handler is called when the
*					interrupt 2/digital input 2 pin changes logic level. The 
*					edge that the interrupt 2 pin reacts to is programmable 
*					(see comments in the Initialize_Interrupts() function, 
*					above).
*
*	CALLED FROM:	user_routines_fast.c/InterruptHandlerLow()
*
*	PARAMETERS:		None
*
*	RETURNS:		Nothing
*
*	COMMENTS:		The PIC18F8520's RB3/CCP2/INT3 pin on port b is mapped
*					to interrupt 2 on the EDU-RC and digital input 2 on the
*					FRC-RC [108].
*
*******************************************************************************/
void Int_2_Handler(void)
{	
	//encoder's 'B' output determines direction of motion
	EncoderRight.direction=0;
	if (mEncoderRightBin)
	{
		EncoderRight.direction=0;	//fwd
		EncoderRight.posNow++;
	}
	else {
		EncoderRight.direction=1;	//reverse
		EncoderRight.posNow--;
	}

	EncoderRight.t2.ts = EncoderRight.t1.ts; 
	EncoderRight.t1.ts = EncoderRight.t0.ts;

	//copy the current time. Byte copy to preserve byte write order
	((char *) &EncoderRight.t0.tsL)[0] = TMR1L;	//must read low byte first
	((char *) &EncoderRight.t0.tsL)[1] = TMR1H;	//read in high byte
//	EncoderRight.t0.tsL = TMR1;	//should watch for rollovers, but we don't
	EncoderRight.t0.tsH = gClock.cycle;	//should watch for rollovers

	//save the last difference
//	EncoderRight.d1.ts = EncoderRight.d0.ts;

	//subtract over two ticks, average and div by 32 ( /2; then /16)
	EncoderRight.d0.ts = (EncoderRight.t0.ts - EncoderRight.t2.ts)>>5;
	
}

/*******************************************************************************
*
*	FUNCTION:		Int_3_Handler()
*
*	PURPOSE:		If enabled, the interrupt 3 handler is called when the
*					interrupt 3/digital input 3 pin changes logic level. 
*
*	CALLED FROM:	user_routines_fast.c/InterruptHandlerLow()
*
*	PARAMETERS:		RB4_State is the current logic level of the
*					interrupt 3 pin.
*
*	RETURNS:		Nothing
*
*	COMMENTS:		The PIC18F8520's RB4/KBI0 pin on port b is mapped to
*					interrupt 3 on the EDU-RC and digital input 3 on the
*					FRC-RC [108].
*
*******************************************************************************/
void Int_3_Handler(unsigned char RB4_State)
{
	// this function will be called when an interrupt 3 occurs
}

/*******************************************************************************
*
*	FUNCTION:		Int_4_Handler()
*
*	PURPOSE:		If enabled, the interrupt 4 handler is called when the
*					interrupt 4/digital input 4 pin changes logic level. 
*
*	CALLED FROM:	user_routines_fast.c/InterruptHandlerLow()
*
*	PARAMETERS:		RB5_State is the current logic level of the
*					interrupt 4 pin.
*
*	RETURNS:		Nothing
*
*	COMMENTS:		The PIC18F8520's RB5/KBI1/PGM pin on port b is mapped
*					to interrupt 4 on the EDU-RC and digital input 4 on the
*					FRC-RC [108].
*
*******************************************************************************/
void Int_4_Handler(unsigned char RB5_State)
{
	// this function will be called when an interrupt 4 occurs
}

/*******************************************************************************
*
*	FUNCTION:		Int_5_Handler()
*
*	PURPOSE:		If enabled, the interrupt 5 handler is called when the
*					interrupt 5/digital input 5 pin changes logic level. 
*
*	CALLED FROM:	user_routines_fast.c/InterruptHandlerLow()
*
*	PARAMETERS:		RB6_State is the current logic level of the
*					interrupt 5 pin.
*
*	RETURNS:		Nothing
*
*	COMMENTS:		The PIC18F8520's RB6/KBI2/PGC pin on port b is mapped
*					to interrupt 5 on the EDU-RC and digital input 5 on the
*					FRC-RC [108].
*
*******************************************************************************/
void Int_5_Handler(unsigned char RB6_State)
{
	// this function will be called when an interrupt 5 occurs
}

/*******************************************************************************
*
*	FUNCTION:		Int_6_Handler()
*
*	PURPOSE:		If enabled, the interrupt 6 handler is called when the
*					interrupt 6/digital input 6 pin changes logic level. 
*
*	CALLED FROM:	user_routines_fast.c/InterruptHandlerLow()
*
*	PARAMETERS:		RB7_State is the current logic level of the
*					interrupt 6 pin.
*
*	RETURNS:		Nothing
*
*	COMMENTS:		The PIC18F8520's RB7/KBI3/PGD pin on port b is mapped
*					to interrupt 6 on the EDU-RC and digital input 6 on the
*					FRC-RC [108].
*
*******************************************************************************/
void Int_6_Handler(unsigned char RB7_State)
{
	// this function will be called when an interrupt 6 occurs
}
