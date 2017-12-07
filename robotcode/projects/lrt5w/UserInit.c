#include "ifi_aliases.h"
#include "ifi_default.h"
#include "ifi_utilities.h"
#include "user_routines.h"
#include "printf_lib.h"
#include "interrupts.h"

/*******************************************************************************
* FUNCTION NAME: User_Initialization
* PURPOSE:       This routine is called first (and only once) in the Main function.  
*                You may modify and add to this function.
* CALLED FROM:   main.c
* ARGUMENTS:     none
* RETURNS:       void
*******************************************************************************/
void User_Initialization (void)
{
	extern void testArea(void);	
	rom const	char *strptr = "IFI User Processor Initialized ...";
	testArea();	//call this routine for debugging code snippets

#ifdef _FRC_BOARD
	Set_Number_of_Analog_Channels(SIXTEEN_ANALOG);    /* DO NOT CHANGE! */
#else
	Set_Number_of_Analog_Channels(TWO_ANALOG);    /* DO NOT CHANGE! */
	printf("EDU board\n");	//warning
#endif


	
/* FIRST: Set up the I/O pins you want to use as digital INPUTS. */
#ifdef _FRC_BOARD
  digital_io_01 = digital_io_02 = digital_io_03 = digital_io_04 = INPUT;
  digital_io_05 = digital_io_06 = digital_io_07 = digital_io_08 = INPUT;
  digital_io_09 = digital_io_10 = digital_io_11 = digital_io_12 = INPUT;
  digital_io_13 = digital_io_14 = digital_io_15 = digital_io_16 = INPUT;
  digital_io_17=digital_io_18 = INPUT;
#else	//EDU board
	IO1 = IO2 = IO3 = IO4 = IO5 = IO6 = IO7 = IO8 = INPUT;
	IO9 = IO10 = IO11 = IO12 = IO13 = IO14 = IO15 = IO16 = INPUT;
#endif	//_FRC_BOARD

/* FOURTH: Set your initial PWM values.  Neutral is 127. */
//	AllStop();	//sets all pwms 00-16 to neutral 127
//	TeeBallReset();	//set to extreme position
	
/* should learn how to setup a pwm to control a servo over its full range - DG */


/* FIFTH: Set your PWM output types for PWM OUTPUTS 13-16.
  /*   Choose from these parameters for PWM 13-16 respectively:               */
  /*     IFI_PWM  - Standard IFI PWM output generated with Generate_Pwms(...) */
  /*     USER_CCP - User can use PWM pin as digital I/O or CCP pin.           */
  Setup_PWM_Output_Type(IFI_PWM,IFI_PWM,IFI_PWM,IFI_PWM);

  /* 
     Example: The following would generate a 40KHz PWM with a 50% duty cycle on the CCP2 pin:

         CCP2CON = 0x3C;
         PR2 = 0xF9;
         CCPR2L = 0x7F;
         T2CON = 0;F
         T2CONbits.TMR2ON = 1;

         Setup_PWM_Output_Type(USER_CCP,IFI_PWM,IFI_PWM,IFI_PWM);
  */

  /* Add any other initialization code here. */

  Initialize_Serial_Comms();

  // Initialize timers and external interrupts
  // NOTE WELL:  Interrupt vector is installed in User_Routines_Fast.c
  // Interrupt handlers are located in interrupts.c
  Initialize_Interrupts();
  Initialize_Timer_0();
  Initialize_Timer_1();	//timer 1 will be used
  Initialize_Timer_2();
  Initialize_Timer_3();
  Initialize_Timer_4();

  Putdata(&txdata);             /* DO NOT CHANGE! */

  printf("%s\n", strptr);       /* Optional - Print initialization message. */

  User_Proc_Is_Ready();         /* DO NOT CHANGE! - last line of User_Initialization */
}
