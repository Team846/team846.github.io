/********************************************************************************
* FILE NAME: wrrf.h
* Author: D.Giandomenico
*
* DESCRIPTION: 
*  This file contains...
*	declarations used in the WRRF workshop Nov 12,2005
*    
********************************************************************************/

#ifndef __wrrf_h_
#define __wrrf_h_

/****** Global Variable DECLARATIONS **********/
extern char gLoopCount;	//counter for rough timing


/****** PROTOTYPES ************/
void allStop(void);	//set all pwm's to neutral
int limit127(int input);

void wrrf(void);	//wrrf "Default_Routine()"
//pregressively more advanced training routines.
void wrrf_0(void);	//beginning level loop
void wrrf_1(void);	//to be replaced by calls to more advanced level loops
void wrrf_2(void);
void wrrf_3(void);
void wrrf_4(void);
void wrrf_5(void);
void wrrf_6(void);
void wrrf_7(void);
void wrrf_8(void);
void wrrf_9(void);
void wrrf_A(void);
void wrrf_B(void);
void wrrf_C(void);

#endif __wrrf_h_	//Nothing below this line!!!
