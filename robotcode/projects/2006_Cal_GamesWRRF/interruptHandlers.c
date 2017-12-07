#include "ifi_default.h"
#include "ifi_aliases.h"
#include "interruptHandlers.h"
#include "utilities.h"
#include "connections.h"
#include <limits.h>
#include "MotorDrive.h"
//#include <stdio.h>

#ifdef ENABLE_INTERRUPT_1
volatile long Interrupt_1_Count = 0;
#endif

#ifdef ENABLE_INTERRUPT_2
volatile long Interrupt_2_Count = 0;
#endif

#ifdef ENABLE_INTERRUPT_3
unsigned char Interrupt_3_State;
volatile long Interrupt_3_Count = 0;
#endif

#ifdef ENABLE_INTERRUPT_4
unsigned char Interrupt_4_State;
volatile long Interrupt_4_Count = 0;
#endif

#ifdef ENABLE_INTERRUPT_5
unsigned char Interrupt_5_State;
volatile long Interrupt_5_Count = 0;
#endif

#ifdef ENABLE_INTERRUPT_6
unsigned char Interrupt_6_State;
volatile long Interrupt_6_Count = 0;
#endif
//timer related variables


volatile unsigned int Timer_High_Count = 0;
//volatile unsigned int arrayl[3] = {0,0,0};
//volatile unsigned int arrayh[3] = {0,0,0};
//volatile unsigned int intr_hit = 0;

volatile wheelSensor gShooterLeft ={'L',0,ULONG_MAX/2,0};
volatile wheelSensor gShooterRight={'R',0,ULONG_MAX/2,0};

volatile unsigned int t0_int_handler = 0;


// So that we'll know which interrupt pin changed state the next time through,
// the state of port b is saved in this variable each time the interrupt
// handler for interrupts 3 through 6 is called. This variable should be
// initialized to the current state of port b just before enabling interrupts
// 3 through 6.
unsigned char Old_Port_B = 0xFF;

/*******************************************************************************
*	FUNCTION:		Initialize_Interrupts()
*******************************************************************************/
void Initialize_Interrupts(void)  
{
	// if enabled in encoder.h, configure encoder 1's interrupt input
	#ifdef ENABLE_INTERRUPT_1
		TRISBbits.TRISB2	= 1;		// Configure interrupt 1 as an input
		INTCON3bits.INT2IP	= 0;		// interrupt 1 is low priority
		INTCON2bits.INTEDG2	= 1;	// trigger on rising edge
		INTCON3bits.INT2IF	= 0;		// clear the interrupt flag before enabling
		INTCON3bits.INT2IE	= 1;		// enable interrupt 1
	#endif

	// if enabled in encoder.h, configure encoder 2's interrupt input
	#ifdef ENABLE_INTERRUPT_2
		TRISBbits.TRISB3	= 1;// Configure interrupt 2 as an input
		INTCON2bits.INT3P	= 0;	// interrupt 2 is low priority
		INTCON2bits.INTEDG3	= 1;		// trigger on rising edge
		INTCON3bits.INT3IF	= 0;// clear the interrupt flag before enabling
		INTCON3bits.INT3IE	= 1;	// enable interrupt 2
	#endif

	// if enabled in encoder.h, configure the interrupt input for encoders 3-6
	#ifdef ENABLE_INTERRUPT_3_6
		TRISBbits.TRISB4 = 1;	//Configure interrupt 3-6 as  inputs
		TRISBbits.TRISB5 = 1;
		TRISBbits.TRISB6 = 1;
		TRISBbits.TRISB7 = 1;
	  	INTCON2bits.RBIP = 0;	// interrupts 3 through 6 are low priority
	
		// before enabling interrupts 3-6, take a snapshot of port b
		Old_Port_B = PORTB;

		INTCONbits.RBIF = 0;	// clear the interrupt flag before enabling
		INTCONbits.RBIE = 1;	// enable interrupts 3 through 6
	#endif
}
//*****************************************************************************
//*****************************************************************************






#ifdef ENABLE_INTERRUPT_1
//*****************************************************************************
long Get_Interrupt_1_Count(void)
{
	long count;
	INTCON3bits.INT2IE = 0;		// turn encoder's interrupt off.
	count = Interrupt_1_Count;	
	INTCON3bits.INT2IE = 1;		// turn encoder's interrupt back on.
	return(count);
}
//*****************************************************************************
void Reset_Interrupt_1_Count(void)
{
	INTCON3bits.INT2IE = 0;	//diable interrupt
	Interrupt_1_Count = 0;
	INTCON3bits.INT2IE = 1;	//enable interrupt
}
//*****************************************************************************
void Interrupt_1_Int_Handler(void)
{
	Interrupt_1_Count++;
}
//*****************************************************************************
#endif	//ENABLE_INTERRUPT_1



#ifdef ENABLE_INTERRUPT_2
//*****************************************************************************
long Get_Interrupt_2_Count(void)
{
	long count;
	INTCON3bits.INT3IE = 0;
	count = Interrupt_2_Count;
	INTCON3bits.INT3IE = 1;
	return(count);
}
//*****************************************************************************
void Reset_Interrupt_2_Count(void)
{
	INTCON3bits.INT3IE = 0;
	Interrupt_2_Count = 0;
	INTCON3bits.INT3IE = 1;
}
//*****************************************************************************
void Interrupt_2_Int_Handler(void)
{
	Interrupt_2_Count++;
}
//*****************************************************************************
#endif	//ENABLE_INTERRUPT_2



#ifdef ENABLE_INTERRUPT_3
//*****************************************************************************
//*****************************************************************************
long Get_Interrupt_3_Count(void)
{
	long count;
	INTCONbits.RBIE = 0;
	count = Interrupt_3_Count;
	INTCONbits.RBIE = 1;
	return(count);
}
//*****************************************************************************
void Reset_Interrupt_3_Count(void)
{
	INTCONbits.RBIE = 0;
	Interrupt_3_Count = 0;
	INTCONbits.RBIE = 1;
}
//*****************************************************************************
void Interrupt_3_Int_Handler(unsigned char state)
{
	//change '==' to '!=' to correct rotation sense
	gEncoderLeft.direction = (mLeftEncoderBCount != state ? kForward:kReverse);

	if (gEncoderLeft.direction==kForward)
		gEncoderLeft.posNow++;
	else
		gEncoderLeft.posNow--;

	if (state)	//only get time on rising edge (encoder white/black striping may not be uniform)
	{
		gEncoderLeft.newdata=1;
		gEncoderLeft.t1.ts = gEncoderLeft.t0.ts;	//save last timestamp
		gEncoderLeft.t0.ts = gInterruptTime.ts;
	}
}
//*****************************************************************************
//*****************************************************************************
#endif	//ENABLE_INTERRUPT_3



#ifdef ENABLE_INTERRUPT_4
//*****************************************************************************
//*****************************************************************************
long Get_Interrupt_4_Count(void)
{
	long count;
	INTCONbits.RBIE = 0;
	count = Interrupt_4_Count;
	INTCONbits.RBIE = 1;
	return(count);
}
//*****************************************************************************
void Reset_Interrupt_4_Count(void)
{
	INTCONbits.RBIE = 0;
	Interrupt_4_Count = 0;
	INTCONbits.RBIE = 1;
}
//*****************************************************************************
void Interrupt_4_Int_Handler(unsigned char state)
{
	//change '==' to '!=' to correct rotation sense
	gEncoderRight.direction = (mRightEncoderBCount == state ? kForward:kReverse);

	if (gEncoderRight.direction==kForward)
		gEncoderRight.posNow++;
	else
		gEncoderRight.posNow--;

	if (state)	//only get time on rising edge (encoder white/black striping may not be uniform)
	{
		gEncoderRight.newdata=1;
		gEncoderRight.t1.ts = gEncoderRight.t0.ts;	//save last timestamp
		gEncoderRight.t0.ts = gInterruptTime.ts;
	}
}
//*****************************************************************************
//*****************************************************************************
#endif //ENABLE_INTERRUPT_4



#ifdef ENABLE_INTERRUPT_5
//*****************************************************************************
//*****************************************************************************
long Get_Interrupt_5_Count(void)
{
	long count;
	INTCONbits.RBIE = 0;
	count = Interrupt_5_Count;
	INTCONbits.RBIE = 1;
	return(count);
}
//*****************************************************************************
void Reset_Interrupt_5_Count(void)
{
	INTCONbits.RBIE = 0;
	Interrupt_5_Count = 0;
	INTCONbits.RBIE = 1;
}
//*****************************************************************************
void Interrupt_5_Int_Handler(unsigned char state)
{
//	static char FiveToOneCount=0;
	if(state == 0)
		return;	//ignore falling edges
	
//	if (--FiveToOneCount>0)
//		return; //only execute every five turns to simulate 5:1 gearbox	
//	FiveToOneCount=5;
	gShooterLeft.newdata=1;	//cleared in main routines
	
	gShooterLeft.t1.ts = gShooterLeft.t0.ts;	//save last timestamp
	gShooterLeft.t0.ts = gInterruptTime.ts;
	gShooterLeft.count++;
}
//*****************************************************************************
//*****************************************************************************
#endif	//ENABLE_INTERRUPT_5



#ifdef ENABLE_INTERRUPT_6
//*****************************************************************************
//*****************************************************************************
long Get_Interrupt_6_Count(void)
{
	long count;
	INTCONbits.RBIE = 0;
	count = Interrupt_6_Count;
	INTCONbits.RBIE = 1;
	return(count);
}
//*****************************************************************************
void Reset_Interrupt_6_Count(void)
{
	INTCONbits.RBIE = 0;
	Interrupt_6_Count = 0;
	INTCONbits.RBIE = 1;
}
//*****************************************************************************
void Interrupt_6_Int_Handler(unsigned char state)
{
//	static char FiveToOneCount=0;
	if(state == 0)
		return;	//ignore falling edges

//	if (--FiveToOneCount>0)
//		return; //only execute every five turns to simulate 5:1 gearbox	
//	FiveToOneCount=5;

	gShooterRight.newdata=1;	//cleared in main routines
	gShooterRight.t1.ts = gShooterRight.t0.ts;	//save last timestamp
	gShooterRight.t0.ts = gInterruptTime.ts;
	gShooterRight.count++;
}
//*****************************************************************************
//*****************************************************************************
#endif	//ENABLE_INTERRUPT_6



//*****************************************************************************
void Initialize_timer(void)
{
	OpenTimer0(TIMER_INT_ON & T0_16BIT  & T0_SOURCE_INT & T0_PS_1_4);
}
//*****************************************************************************
void Timer_0_Int_Handler(void)
{
	Timer_High_Count++;
}
//*****************************************************************************
unsigned long GetTime(void)
{
	overlay timestamp now;	//overlay or static will result in more compact code than auto here.

	INTCONbits.GIE=0;	//turn off all interrupts
		((char *) &now.tsL)[0] = TMR0L;	//must read low byte first
		((char *) &now.tsL)[1] = TMR0H;	//read buffered high byte
		now.tsH = Timer_High_Count;
	INTCONbits.GIE=1;	//restore interrupts
	
	if (now.tsL==0)	//correct for rollover if TMR0 was 0xFFFF->0x0000
		now.tsH++;
	return now.ts;
}
//*****************************************************************************
