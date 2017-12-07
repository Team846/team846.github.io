/*******************************************************************************
*
*	TITLE:		interrupts.h 
*
*	VERSION:	0.2 (Beta)                           
*
*	DATE:		25-Feb-2004
*
*	AUTHOR:		R. Kevin Watson
*				kevinw@jpl.nasa.gov
*
*	COMMENTS:
*
********************************************************************************
*
*	CHANGE LOG:
*
*	DATE         REV  DESCRIPTION
*	-----------  ---  ----------------------------------------------------------
*	22-Dec-2003  0.1  RKW Original
*	25-Feb-2004  0.2  RKW - Added the ability to clear the interrupt flag before
*	                  enabling the interrupt.
*
*******************************************************************************/

#ifndef _interrupts_h
#define _interrupts_h

// function prototypes
void Initialize_Interrupts(void);	// initializes the interrupt hardware
void Initialize_Timer_0(void);		// initializes the timer 0 hardware
void Initialize_Timer_1(void);		// initializes the timer 1 hardware
void Initialize_Timer_2(void);		// initializes the timer 2 hardware
void Initialize_Timer_3(void);		// initializes the timer 3 hardware
void Initialize_Timer_4(void);		// initializes the timer 4 hardware
void Timer_0_Int_Handler(void);		// timer 0 interrupt handler
void Timer_1_Int_Handler(void);		// timer 1 interrupt handler
void Timer_2_Int_Handler(void);		// timer 2 interrupt handler
void Timer_3_Int_Handler(void);		// timer 3 interrupt handler
void Timer_4_Int_Handler(void);		// timer 4 interrupt handler
void Int_1_Handler(void);			// external interrupt 1 handler
void Int_2_Handler(void);			// external interrupt 2 handler
void Int_3_Handler(unsigned char);	// external interrupt 3 handler
void Int_4_Handler(unsigned char);	// external interrupt 4 handler
void Int_5_Handler(unsigned char);	// external interrupt 5 handler
void Int_6_Handler(unsigned char);	// external interrupt 6 handler

extern unsigned char Old_Port_B;

#endif // _interrupts_h
