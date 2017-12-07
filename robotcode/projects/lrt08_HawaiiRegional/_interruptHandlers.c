#include "common.h"
#include "Compass.h"
#include "UltraSonic.h"

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
	  	INTCON2bits.RBPU = 1; //Disable port B weak pullup **IMPORTANT** for ultrasonic [DG/DW]
		TRISBbits.TRISB4 = 1;	//Configure interrupt 3-6 as  inputs
		TRISBbits.TRISB5 = 1;
		TRISBbits.TRISB6 = 1;
		TRISBbits.TRISB7 = 1;
		// before enabling interrupts 3-6, take a snapshot of port b
		Old_Port_B = PORTB;
	
		INTCON2bits.RBIP = 0;	// interrupts 3 through 6 are low priority
		INTCONbits.RBIF = 0;	// clear the interrupt flag before enabling
		INTCONbits.RBIE = 1;	// enable interrupts 3 through 6
	#endif
}
//*****************************************************************************
//*****************************************************************************






#ifdef ENABLE_INTERRUPT_1		// Left encoder
//*****************************************************************************
void Interrupt_1_Int_Handler(void)
{
#ifdef R2007
	gEncoders[LEFT].direction = (SENS_LEFTENC_BCOUNT ? FORWARD: REVERSE);
#else
	gEncoders[LEFT].direction = (SENS_LEFTENC_BCOUNT ? REVERSE : FORWARD);
#endif
	if (gEncoders[LEFT].direction==FORWARD)
		gEncoders[LEFT].posNow++;
	else
		gEncoders[LEFT].posNow--;

	gEncoders[LEFT].newdata=1;
	gEncoders[LEFT].t1.ts = gEncoders[LEFT].t0.ts;	//save last timestamp
	gEncoders[LEFT].t0.ts = gInterruptTime.ts;
}
//*****************************************************************************
#endif	//ENABLE_INTERRUPT_1



#ifdef ENABLE_INTERRUPT_2		// Right encoder
//*****************************************************************************
void Interrupt_2_Int_Handler(void)
{
#ifdef R2007
	gEncoders[RIGHT].direction = (SENS_RIGHTENC_BCOUNT ? REVERSE:FORWARD);
#else
	gEncoders[RIGHT].direction = (SENS_RIGHTENC_BCOUNT ? FORWARD : REVERSE);
#endif

	if (gEncoders[RIGHT].direction==FORWARD)
		gEncoders[RIGHT].posNow++;
	else
		gEncoders[RIGHT].posNow--;

	gEncoders[RIGHT].newdata=1;
	gEncoders[RIGHT].t1.ts = gEncoders[RIGHT].t0.ts;	//save last timestamp
	gEncoders[RIGHT].t0.ts = gInterruptTime.ts;
}
//*****************************************************************************
#endif	//ENABLE_INTERRUPT_2



#ifdef ENABLE_INTERRUPT_3
//*****************************************************************************
//*****************************************************************************
void Interrupt_3_Int_Handler(unsigned char state)
{
	//
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
	//
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

#define SAMPLES 3
unsigned int Readings[SAMPLES];
unsigned char p=0;
unsigned char Temp_Buf;
unsigned int Timer_Snapshot;
unsigned int a;

unsigned int Get_Compass(void)
{
	int i;
	unsigned long x = 0;
	
	//printf("Timer_Snapshot = %x,a = %x, Temp_Buf=%x\r",Timer_Snapshot,a,(int)Temp_Buf);
	for(i=0;i<SAMPLES;i++)
		x+=Readings[i];
	return (unsigned int)(x/SAMPLES);
}



void Interrupt_5_Int_Handler(unsigned char state)
{

//	if(state)
//	{	
//		Temp_Buf = TMR1L;
//		Timer_Snapshot = TMR1H;
//		if(Temp_Buf == 0xFF)
//			Timer_Snapshot--;
//		Timer_Snapshot <<=8;
//		Timer_Snapshot += Temp_Buf;
//		TMR1L = 0;
//		TMR1H = 0;
//		Readings[p++] = ((Timer_Snapshot - 0x4000) / 125) % 360;
//		p %= SAMPLES;
//	}
		
	
	
	static unsigned char latch = 0;

	//unsigned char a,b;
	
	if(state && !latch) // rising edge
	{
		// Need to set TMR1H before TMR1L
		TMR1H = 0x00;
		TMR1L = 0x00;
		
		
		T1CONbits.TMR1ON = 1;	// Start Timer 1
		//debug
		//rc_dig_out01 = 1;
		latch = 1;
		
	}
	else if(!state && latch)
	{
		T1CONbits.TMR1ON = 0;	// stop timer 1
		//INTCONbits.RBIE = 0; // disable interrupt
		//digital_io_05 = OUTPUT;	// set I/O 5 to OUTPUT
		// need to read TMR1L before TMR1H 
		Temp_Buf = TMR1L;
		Timer_Snapshot = TMR1H;
		Timer_Snapshot <<= 8;
		a = Timer_Snapshot;
		Timer_Snapshot += Temp_Buf;
		//Readings[p++] = ((Timer_Snapshot - 0x4000) / 125) % 360;
		Readings[p++] = Timer_Snapshot;
		p %= SAMPLES;
		//printf("Timer_Snapshot = %u, a = %u, b = %u\r",Timer_Snapshot,(int)a,(int)b);
		//gCompassDirection = Timer_Snapshot;
		//debug
		//rc_dig_out01 = 0;
		//printf("Compass Stop timestamp = %ul, %u\r",gInterruptTime.ts,Timer_Snapshot);
		
		latch = 0;
		
	}
	
	
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
	static unsigned long timestamp;
	if (state) // start on rising edge of interrupt
	{
		timestamp = gInterruptTime.ts;
//		DIAGNOSTIC_LED=1;
	}
	else // falling edge
	{
		gUltraSonic.ticks = gInterruptTime.ts-timestamp;
		gUltraSonic.valid=1;
//		INTCONbits.RBIE = 0; // disable interrupt; Reenable in PingStart()
//		DIAGNOSTIC_LED=0;
	}
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
#ifdef _SIMULATOR
	statusflag.NEW_SPI_DATA = 1;	//timer0 set to 26.2ms
#endif
}
//*****************************************************************************
unsigned long GetTime(void)
{
	timestamp now;

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
