#ifndef _interruptHandlers_h
#define _interruptHandlers_h

#include <timers.h>

typedef union
{
	unsigned long ts;
	struct
	{
		unsigned int tsL;
		unsigned int tsH;
	};
} timestamp;

void Initialize_timer(void);
void Timer_0_Int_Handler(void);
extern timestamp gInterruptTime;
unsigned long GetTime(void);

#define ENABLE_INTERRUPT_1		// Left encoder
#define ENABLE_INTERRUPT_2		// Right encoder
#define ENABLE_INTERRUPT_6		// Ultrasonic Sensor

//#define ENABLE_INTERRUPT_5		// Compass Module


// Digital input pin(s) assigned to the encoder's phase-B output. Make sure 
// this pin is configured as an input in user_routines.c/User_Initialization().


// Change the sign of these if you need	to flip the way the encoders count. 
// For instance, if a given encoder count increases in the positive direction 
// when rotating counter-clockwise, but you want it to count in the negative 
// direction when rotating in the counter-clockwise direction, flip the sign 
// (i.e., change 1 to -1) and it'll work the way you need it to.


// #define ENABLE_INTERRUPT_3_6 if encoder 3, 4, 5 or 6 is enabled
#ifdef ENABLE_INTERRUPT_3
#ifndef ENABLE_INTERRUPT_3_6
#define ENABLE_INTERRUPT_3_6
#endif
#endif
#ifdef ENABLE_INTERRUPT_4
#ifndef ENABLE_INTERRUPT_3_6
#define ENABLE_INTERRUPT_3_6
#endif
#endif
#ifdef ENABLE_INTERRUPT_5
#ifndef ENABLE_INTERRUPT_3_6
#define ENABLE_INTERRUPT_3_6
#endif
#endif
#ifdef ENABLE_INTERRUPT_6
#ifndef ENABLE_INTERRUPT_3_6
#define ENABLE_INTERRUPT_3_6
#endif
#endif

extern unsigned char Old_Port_B;

// function prototypes
void Initialize_Interrupts(void);


#ifdef ENABLE_INTERRUPT_1
long Get_Interrupt_1_Count(void);
void Reset_Interrupt_1_Count(void);
void Interrupt_1_Int_Handler(void);
#endif

#ifdef ENABLE_INTERRUPT_2
long Get_Interrupt_2_Count(void);
void Reset_Interrupt_2_Count(void);
void Interrupt_2_Int_Handler(void);
#endif

#ifdef ENABLE_INTERRUPT_3
long Get_Interrupt_3_Count(void);
void Reset_Interrupt_3_Count(void);
void Interrupt_3_Int_Handler(unsigned char);
#endif

#ifdef ENABLE_INTERRUPT_4
long Get_Interrupt_4_Count(void);
void Reset_Interrupt_4_Count(void);
void Interrupt_4_Int_Handler(unsigned char);
#endif

#ifdef ENABLE_INTERRUPT_5
long Get_Interrupt_5_Count(void);
void Reset_Interrupt_5_Count(void);
unsigned int Get_Compass(void);
void Interrupt_5_Int_Handler(unsigned char);
#endif

#ifdef ENABLE_INTERRUPT_6
long Get_Interrupt_6_Count(void);
void Reset_Interrupt_6_Count(void);
void Interrupt_6_Int_Handler(unsigned char);
#endif

#endif	//_interruptHandlers_h
