/*******************************************************************************
* FILE NAME: wrrf.c
* Author: D.Giandomenico

* DESCRIPTION:
*  This file contains a series of training routines examples
*   wrrf_0() through  wrrf_C()
*
*  Also includes a testArea() routine to demonstrate MPLab SIM 
*
*******************************************************************************/

#include "wrrf.h"
#include "ifi_aliases.h"
#include "ifi_utilities.h"
#include "ifi_default.h"
#include "printf_lib.h"

char gLoopCount= -1;	//set to -1 so gLoopCount is zero on 1st test



// Local prototypes:
#ifdef _SIMULATOR	//
static void testArea(void);
#endif //_SIMULATOR

/*******************************************************************************/
void wrrf(void)
{
//	if (++gLoopCount >= (1000/17)) gLoopCount=0;	

	allStop();
	wrrf_0();


#ifdef _SIMULATOR
	testArea();
	printf("SIM Mode\n"); //try to warn user we are in simulation mode
#endif
}


/*******************************************************************************/
// use to limit values to the range {-127,+127}	
int limit127(int input)
{
	if (input > 127) return 127;
	if (input < -127) return -127;
	return input;
}

/************* Stop all motors ********************************/
void allStop(void)
{
	pwm01=pwm02=pwm03=pwm04=pwm05=pwm06=pwm07=pwm08=127;
}




/*****How to  make the motors turn ********************************************/
void wrrf_0(void)
{
	//Variable declarations must appear before a variable is used
	
	//We can use pwm0x's without because they declared (as global variables)
	//in "ifi_aliases.h" as part of a structure "txdata",
	//which in turn is declared in "ifi_default.h"
	
	pwm05 =254;	//full speed	
	//pwm05 = 0;	//full reverse

	//pwm05 = 127 + 64;	//half speed
}



/*******   Simple time based program to automate motors  ***********************************/
void wrrf_1(void)
{
	static int timer=-1;

	timer = timer+1;
	if (timer < (3000/17))		//go fwd until 3 secons
		pwm05 = 254;
	else if (timer < 4000/17)	//stop until 4 seconds
		pwm05 = 127;
	else if (timer < 7000/17)	//go rev until 7 seconds
		pwm05 = 0;
	else if (timer < 15000/17)	//stop until 15 secs
		pwm05 = 127;
	else
		timer = -1;	//start over

}	
/*********** Read from sensor; print data  ***********************************/
void wrrf_2(void)
{
	int input;
	//use  MPLab menu item "Project:Find in Project Files..."
	// to locate declaration for Get_Analog_Value and rc_ana_in	

	input = Get_Analog_Value(rc_ana_in01);	//returns 10-bit number on {0,1023}

	printf("input = %d\n", (int) input);
}







//******************* Printing Data at at controlled rate **************
void wrrf_4(void)
{
	int input;
	input = Get_Analog_Value(rc_ana_in01);	//returns 10-bit number on {0,1023}
	input = input/4;	//input is now on {0,254}

	gLoopCount = gLoopCount+1;
	if (gLoopCount==(500/17))	//reset every 1/2 second
		gLoopCount = 0;

	if (gLoopCount==0) printf("input = %d\n", (int) input);
}
/*******************************************************************************/





/******************* Mapping Input to {-127,+128} ************************/
void wrrf_5(void)
{
	//SKIP THIS ONE
	int input;
	input = Get_Analog_Value(rc_ana_in01);	//returns 10-bit number on {0,1023}

	input = (input-511)/4;	// range: {-127,+128}
	
	//make sure counter is enabled in wrrf()
	if (gLoopCount==0) printf("input = %d\n", (int) input);
}






/********************Digital Inputs **********************/
void wrrf_6(void)
{
	char input1, input2;

	//Inputs & Outputs declared in "ifi_aliases.h"
	input1= rc_dig_in12;//will be either 0 or 1
	input2= rc_dig_in14;//will be either 0 or 1

	if (gLoopCount==0) printf("inputs 1&2 = %d / %d\n", (int) input1, (int)input2);
}



/********************* Start and Stop ******************************************/
void wrrf_7(void)
{
	char start, stop;
	
	static char running=0;

	//Inputs & Outputs declared in "ifi_aliases.h"
	start= rc_dig_in12;//will be either 0 or 1
	stop= rc_dig_in14;//will be either 0 or 1

	if (gLoopCount==0) printf("start=%d / Stop = %d\n", (int) start, (int)stop);


	if (!running && start)
		running=1;	//non zero is true in 'C'
	else if (running && stop)
		running=0;	//zero is false in 'C'

	if (running) pwm05 += 127;		//same as pwm05 = pwm05+127;
}




/********************* Analog input to motor output **************************/
void wrrf_8(void)
{	int forward;
	forward = Get_Analog_Value(rc_ana_in01);	//returns 10-bit number on {0,1023}

	forward = (forward-511)/4;	// range: {-127,+128}
	
	if (gLoopCount==0) printf("forward = %d\n", (int) forward);

	pwm05 = pwm06 = pwm07 = pwm08 = 127 + forward;
}

/******************** Analog 2 inputs to motor output *******************************/
void wrrf_9(void)
{
	int left=0;		//initialize to 0
	int right=0;

	left = Get_Analog_Value(rc_ana_in01);	//returns 10-bit number on {0,1023}
	left = (left-511)/4;	// range: {-127,+128}
	
	right = Get_Analog_Value(rc_ana_in02);	//returns 10-bit number on {0,1023}
	right = (right-511)/4;	// range: {-127,+128}

	pwm05 = pwm06 = 127 + left;
	pwm07 = pwm08 = 127 - right;	//reverse since motors are on opposite side

	if (gLoopCount==0) printf("left/right pwmL/R = %d/%d %d/%d\n", (int) left, (int) right, (int)pwm05, (int)pwm07);
}




//********************Mix forward / turn  inputs to motor output ****************/
//******************** Analog 2 inputs to motor output **********************
void wrrf_A(void)
{
	int forward, turn;	
	int left=0;		//initialize to 0
	int right=0;

	forward = Get_Analog_Value(rc_ana_in01);	//returns 10-bit number on {0,1023}
	forward = (forward-511)/4;	// range: {-127,+128}
	
	turn = Get_Analog_Value(rc_ana_in02);	//returns 10-bit number on {0,1023}
	turn = (turn-511)/4;	// range: {-127,+128}
	
	left = forward + turn;
	right = forward - turn;

	pwm05 = pwm06 = 127 + left;
	pwm07 = pwm08 = 127 - right;	//reverse since motors are on opposite side
	
	if (gLoopCount==0)
	{
		printf("fwd/turn left/right = %4d/%4d    %4d/%4d    %4d/%4d\n",
			(int) forward, (int) turn, (int)left, (int)right, (int) pwm05, (int) pwm07);
	}
}
/*******************************************************************************/
//******************** Mix Analog Inputs 2 inputs to motor output with limiting ***************
void wrrf_B(void)
{
	int forward, turn;	
	int left=0;		//initialize to 0
	int right=0;

	forward = Get_Analog_Value(rc_ana_in01);	//returns 10-bit number on {0,1023}
	forward = (forward-511)/4;	// range: {-127,+128}
	
	turn = Get_Analog_Value(rc_ana_in02);	//returns 10-bit number on {0,1023}
	turn = (turn-511)/4;	// range: {-127,+128}
	
	left = forward + turn;
	right = forward - turn;

	left = limit127(left);		//must limit to {-127,127}
	right = limit127(right);

	pwm05 = pwm06 = 127 + left;
	pwm07 = pwm08 = 127 - right;	//reverse since motors are on opposite side
	
	if (gLoopCount==0)
	{
		printf("fwd/turn left/right = %4d/%4d    %4d/%4d    %4d/%4d\n",
			(int) forward, (int) turn, (int)left, (int)right, (int) pwm05, (int) pwm07);
	}
}
/******************** Remap inputs  for better control **************************/


void wrrf_C(void)
{
	enum { kIn1Min=49, kIn1Max=206 };	//this is the min and max for my R/C transmitter
	enum { kIn2Min=53, kIn2Max=203 };
	int forward, turn;	
	int left=0;		//initialize to 0
	int right=0;

	forward = PWM_in1;	
	turn	= PWM_in2;	

	//remap inputs  to  {0,256}
	turn = ((int)PWM_in1 - kIn1Min)* 256L / (kIn1Max - kIn1Min);
	forward = ((int)PWM_in2 - kIn2Min)* 256L / (kIn2Max - kIn2Min);
	
	forward -= 127;
	turn -=127;

	left = forward + turn;
	right = forward - turn;

	left = limit127(left);		//must limit to {-127,127}
	right = limit127(right);

	pwm05 = pwm06 = 127 + left;
	pwm07 = pwm08 = 127 - right;	//reverse since motors are on opposite side
	
	if (gLoopCount==0)
	{
		printf("raw F/T F/T L/R = %4d/%4d  %4d/%4d  %4d/%4d  %4d/%4d\n",
			(int) PWM_in2, (int) PWM_in1, (int) forward, (int) turn, (int)left, (int)right, (int) pwm05, (int) pwm07);
	}
}




/***** TEST AREA:  try out  your formulas here using the debugger  ************/
/*************** remember to #define _SIMULATOR *******************************/
void testArea(void)
{
	static char executed=0;
	if (executed) return;
	executed=1;
	
	{	
		char dragon;
		unsigned char knight;
		int mountain;

		int RESULT;
		dragon =1;
		dragon =  dragon +1;
		dragon++;

		knight = 129;		//OK
		dragon = knight;	//gotcha! 129 too  big to fit in dragon
		RESULT = dragon;	

		RESULT = dragon = 127;
		RESULT = dragon = dragon+1;	//overflow!
		RESULT = dragon = dragon+1;

		
		mountain = 128;
		mountain = 127+1;	//gotcha - math done as char, then promted to int
		mountain = 127L+1;	//OK
		mountain = (int)127+1;	//OK

		knight = 127;
		dragon = 127;
		mountain = knight  + 1;		//compare the results
		mountain = dragon  + 1;

		//speed!  Use  the stopwatch feature of MPLab IDE
		// see  how  many cycles it takes to add, mult, div
		mountain = 18;
		mountain = mountain+1;
		
		mountain = 18;
		mountain = mountain /2;
		
		mountain = 18; 
		mountain = mountain %2;
		
		mountain = 18;
		mountain = mountain * 2;
		
		dragon = 18;		//is a char divide faster?
		dragon =dragon / 2;
			
		dragon=dragon+1;	//dummy statement
	}
}
