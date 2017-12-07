/*******************************************************************************
*
*	TITLE:		pwm.h 
*
*	VERSION:	0.1 (Beta)                           
*
*	DATE:		29-Dec-2006
*
*	AUTHOR:		R. Kevin Watson
*				kevinw@jpl.nasa.gov
*
*	COMMENTS:	You are free to use this source code for any non-commercial
*				use. Please do not make copies of this source code, modified
*				or un-modified, publicly available on the internet or elsewhere
*				without permission. Thanks.
*
*				Copyright ©2006-2007 R. Kevin Watson. All rights are reserved.
*
********************************************************************************
*
*	CHANGE LOG:
*
*	DATE         REV  DESCRIPTION
*	-----------  ---  ----------------------------------------------------------
*	29-Dec-2006  0.1  RKW - Original code.
*
*******************************************************************************/
#ifndef _PWM_H
#define _PWM_H

// These values define how much each respective PWM output 
// pulse will increase or decrease relative to the center/
// neutral pulse width (defined below) for each PWM step.
// The default value of fifty provides for a pulse width
// range of 1.28 ms (256*0.000005=0.00128), which is the
// same provided by Generate_Pwms(). If you're using
// servos, you should consider decreasing the gain to
// 40 or less. 
#define PWM_13_GAIN   50	// 5.0 microseconds per step
#define PWM_14_GAIN   50	// 5.0 microseconds per step
#define PWM_15_GAIN   50	// 5.0 microseconds per step
#define PWM_16_GAIN   50	// 5.0 microseconds per step

// These values define how wide each respective center/
// neutral pulse is with a PWM input value of 127. The 
// default value of 15000 provides for a pulse width of 
// 1.5 ms, which is the same provided by Generate_Pwms().
#define PWM_13_CENTER 15000 // 1.5 milliseconds
#define PWM_14_CENTER 15000 // 1.5 milliseconds
#define PWM_15_CENTER 15000 // 1.5 milliseconds
#define PWM_16_CENTER 15000 // 1.5 milliseconds

#define HIBYTE(value) ((unsigned char)(((unsigned int)(value)>>8)&0xFF))
#define LOBYTE(value) ((unsigned char)(value))

// function prototypes
void Initialize_PWM(void);
void PWM(unsigned char,unsigned char,unsigned char,unsigned char,unsigned char);

#endif
