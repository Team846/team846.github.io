/*******************************************************************************
* FILE NAME: user_routines.c <EDU VERSION>
*
* DESCRIPTION:
*  This file contains the default mappings of inputs  
*  (like switches, joysticks, and buttons) to outputs on the EDU RC.  
*
* USAGE:
*  You can either modify this file to fit your needs, or remove it from your 
*  project and replace it with a modified copy. 
*
*******************************************************************************/

#include "ifi_aliases.h"
#include "ifi_default.h"
#include "ifi_utilities.h"
#include "user_routines.h"
#include "printf_lib.h"


/*** DEFINE USER VARIABLES AND INITIALIZE THEM HERE ***/
/* EXAMPLES: (see MPLAB C18 User's Guide, p.9 for all types)
unsigned char wheel_revolutions = 0; (can vary from 0 to 255)
unsigned int  delay_count = 7;       (can vary from 0 to 65,535)
int           angle_deviation = 142; (can vary from -32,768 to 32,767)
unsigned long very_big_counter = 0;  (can vary from 0 to 4,294,967,295)
*/

/*******************************************************************************
* FUNCTION NAME: Limit_Switch_Max
* PURPOSE:       Sets a PWM value to neutral (127) if it exceeds 127 and the
*                limit switch is on.
* CALLED FROM:   this file
* ARGUMENTS:     
*     Argument       Type             IO   Description
*     --------       -------------    --   -----------
*     switch_state   unsigned char    I    limit switch state
*     *input_value   pointer           O   points to PWM byte value to be limited
* RETURNS:       void
*******************************************************************************/
void Limit_Switch_Max(unsigned char switch_state, unsigned char *input_value)
{
  if (switch_state == CLOSED)
  { 
    if(*input_value > 127)
      *input_value = 127;
  }
}


/*******************************************************************************
* FUNCTION NAME: Limit_Switch_Min
* PURPOSE:       Sets a PWM value to neutral (127) if it's less than 127 and the
*                limit switch is on.
* CALLED FROM:   this file
* ARGUMENTS:     
*     Argument       Type             IO   Description
*     --------       -------------    --   -----------
*     switch_state   unsigned char    I    limit switch state
*     *input_value   pointer           O   points to PWM byte value to be limited
* RETURNS:       void
*******************************************************************************/
void Limit_Switch_Min(unsigned char switch_state, unsigned char *input_value)
{
  if (switch_state == CLOSED)
  { 
    if(*input_value < 127)
      *input_value = 127;
  }
}


/*******************************************************************************
* FUNCTION NAME: Limit_Mix
* PURPOSE:       Limits the mixed value for one joystick drive.
* CALLED FROM:   Default_Routine, this file
* ARGUMENTS:     
*     Argument             Type    IO   Description
*     --------             ----    --   -----------
*     intermediate_value    int    I    
* RETURNS:       unsigned char
*******************************************************************************/
unsigned char Limit_Mix (int intermediate_value)
{
  static int limited_value;
  
  if (intermediate_value < 2000)
  {
    limited_value = 2000;
  }
  else if (intermediate_value > 2254)
  {
    limited_value = 2254;
  }
  else
  {
    limited_value = intermediate_value;
  }
  return (unsigned char) (limited_value - 2000);
}


/*******************************************************************************
* FUNCTION NAME: Setup_Who_Controls_Pwms
* PURPOSE:       Each parameter specifies what processor will control the pwm.  
*                 
* CALLED FROM:   User_Initialization
*     Argument             Type    IO   Description
*     --------             ----    --   -----------
*     pwmSpec1              int     I   USER/MASTER (defined in ifi_aliases.h)
*     pwmSpec2              int     I   USER/MASTER
*     pwmSpec3              int     I   USER/MASTER
*     pwmSpec4              int     I   USER/MASTER
*     pwmSpec5              int     I   USER/MASTER
*     pwmSpec6              int     I   USER/MASTER
*     pwmSpec7              int     I   USER/MASTER
*     pwmSpec8              int     I   USER/MASTER
* RETURNS:       void
*******************************************************************************/
static void Setup_Who_Controls_Pwms(int pwmSpec1,int pwmSpec2,int pwmSpec3,int pwmSpec4,
                                    int pwmSpec5,int pwmSpec6,int pwmSpec7,int pwmSpec8)
{
  txdata.pwm_mask = 0xFF;         /* Default to master controlling all PWMs. */
  if (pwmSpec1 == USER)           /* If User controls PWM1 then clear bit0. */
    txdata.pwm_mask &= 0xFE;      /* same as txdata.pwm_mask = txdata.pwm_mask & 0xFE; */
  if (pwmSpec2 == USER)           /* If User controls PWM2 then clear bit1. */
    txdata.pwm_mask &= 0xFD;
  if (pwmSpec3 == USER)           /* If User controls PWM3 then clear bit2. */
    txdata.pwm_mask &= 0xFB;
  if (pwmSpec4 == USER)           /* If User controls PWM4 then clear bit3. */
    txdata.pwm_mask &= 0xF7;
  if (pwmSpec5 == USER)           /* If User controls PWM5 then clear bit4. */
    txdata.pwm_mask &= 0xEF;
  if (pwmSpec6 == USER)           /* If User controls PWM6 then clear bit5. */
    txdata.pwm_mask &= 0xDF;
  if (pwmSpec7 == USER)           /* If User controls PWM7 then clear bit6. */
    txdata.pwm_mask &= 0xBF;
  if (pwmSpec8 == USER)           /* If User controls PWM8 then clear bit7. */
    txdata.pwm_mask &= 0x7F;
}


/*******************************************************************************
* FUNCTION NAME: User_Initialization
* PURPOSE:       This routine is called first (and only once) in the Main function.  
*                You may modify and add to this function.
*                The primary purpose is to set up the DIGITAL IN/OUT - ANALOG IN
*                pins as analog inputs, digital inputs, and digital outputs.
* CALLED FROM:   main.c
* ARGUMENTS:     none
* RETURNS:       void
*******************************************************************************/
void User_Initialization (void)
{
  rom const	char *strptr = "IFI User Processor Initialized ...";

/* FIRST: Set up the pins you want to use as analog INPUTs. */
  IO1 = IO2 = INPUT;        /* Used for analog inputs. */
    /* 
     Note: IO1 = IO2 = IO3 = IO4 = INPUT; 
           is the same as the following:

           IO1 = INPUT;
           IO2 = INPUT;
           IO3 = INPUT;
           IO4 = INPUT;
    */

/* SECOND: Configure the number of analog channels. */
  Set_Number_of_Analog_Channels(TWO_ANALOG);     /* See ifi_aliases.h */

/* THIRD: Set up any extra digital inputs. */
  /* The six INTERRUPTS are already digital inputs. */
  /* If you need more then set them up here. */
  /* IOxx = IOyy = INPUT; */
  IO6 = IO8 = IO10 = INPUT;      /* Used for limit switch inputs. */
  IO12 = IO14 = IO16 = INPUT;    /* Used for limit switch inputs. */

/* FOURTH: Set up the pins you want to use as digital OUTPUTs. */
  IO3 = IO4 = OUTPUT;
  IO5 = IO7 = IO9 = OUTPUT;     /* For connecting to adjacent limit switches. */
  IO11 = IO13 = IO15 = OUTPUT;  /* For connecting to adjacent limit switches. */

/* FIFTH: Initialize the values on the digital outputs. */
  rc_dig_out03 = rc_dig_out04 = 0;
  rc_dig_out05 = rc_dig_out07 = rc_dig_out09 = 0;
  rc_dig_out11 = rc_dig_out13 = rc_dig_out15 = 0;

/* SIXTH: Set your initial PWM values.  Neutral is 127. */
  pwm01 = pwm02 = pwm03 = pwm04 = pwm05 = pwm06 = pwm07 = pwm08 = 127;

/* SEVENTH: Choose which processor will control which PWM outputs. */
  Setup_Who_Controls_Pwms(MASTER,MASTER,MASTER,MASTER,MASTER,MASTER,MASTER,MASTER);

/* EIGHTH: Set your PWM output type.  Only applies if USER controls PWM 1, 2, 3, or 4. */
  /*   Choose from these parameters for PWM 1-4 respectively:                          */
  /*     IFI_PWM  - Standard IFI PWM output generated with Generate_Pwms(...)          */
  /*     USER_CCP - User can use PWM pin as digital I/O or CCP pin.                    */
  Setup_PWM_Output_Type(IFI_PWM,IFI_PWM,IFI_PWM,IFI_PWM);

  /* 
     Example: The following would generate a 40KHz PWM with a 50% duty cycle
              on the CCP2 pin (PWM OUT 1):
          CCP2CON = 0x3C;
          PR2 = 0xF9;
          CCPR2L = 0x7F;
          T2CON = 0;
          T2CONbits.TMR2ON = 1;
          Setup_PWM_Output_Type(USER_CCP,IFI_PWM,IFI_PWM,IFI_PWM);
  */

/* Add any other user initialization code here. */

  Initialize_Serial_Comms();     

  Putdata(&txdata);             /* DO NOT CHANGE! */

  printf("%s\n", strptr);       /* Optional - Print initialization message. */

  User_Proc_Is_Ready();         /* DO NOT CHANGE! - last line of User_Initialization */
}


/*******************************************************************************
* FUNCTION NAME: Process_Data_From_Master_uP
* PURPOSE:       Executes every 17ms when it gets new data from the master 
*                microprocessor.
* CALLED FROM:   main.c
* ARGUMENTS:     none
* RETURNS:       void
*******************************************************************************/
void Process_Data_From_Master_uP(void)
{
  Getdata(&rxdata);   /* Get fresh data from the master microprocessor. */

  Default_Routine();  /* Optional.  See below. */

  /* Add your own code here. */

  printf("PWM OUT 7 = %d, PWM OUT 8 = %d\n",(int)pwm07,(int)pwm08);  /* printf EXAMPLE */
  Putdata(&txdata);             /* DO NOT CHANGE! */
}


/*******************************************************************************
* FUNCTION NAME: Default_Routine
* PURPOSE:       Performs the default mappings of inputs to outputs for the
*                Robot Controller.
* CALLED FROM:   this file, Process_Data_From_Master_uP routine
* ARGUMENTS:     none
* RETURNS:       void
*******************************************************************************/
void Default_Routine(void)
{
  
 /*---------- 1 Joystick Drive -------------------------------------------------
  *-----------------------------------------------------------------------------
  *  This code mixes the Y and X axis on one joystick for driving. 
  *  Joystick forward  = Robot forward
  *  Joystick backward = Robot backward
  *  Joystick right    = Robot rotates right
  *  Joystick left     = Robot rotates left
  *  Connect the left  drive motors to PWM OUT 1 and/or PWM OUT 3 on the RC.
  *  Connect the right drive motors to PWM OUT 2 and/or PWM OUT 4 on the RC.
  */  
   pwm01 = pwm03 = Limit_Mix(2000 + PWM_in1 + PWM_in2 - 127);   /* LEFT  WHEELS */
   pwm02 = pwm04 = Limit_Mix(2000 + PWM_in2 - PWM_in1 + 127);   /* RIGHT WHEELS */
   pwm01 = pwm03 = 255 - pwm01;    /* reverse direction of left side */


 /* ------ Other PWM OUTPUT Mapping (can be used for two-joystick drive ------*/
  pwm05 = PWM_in5;    /* limited by digital inputs 6 & 8 below */
  pwm06 = PWM_in6;    /* limited by digital inputs 10 & 12 below */
  pwm07 = PWM_in7;
  pwm08 = PWM_in8;


 /*---------- Limit Switches to limit PWM OUTPUTs 3 & 4 -------------------*/
  Limit_Switch_Max(rc_dig_in06, &pwm05);  /* PWM05 won't go FWD if rc_dig_in06 is low. */
  Limit_Switch_Min(rc_dig_in08, &pwm05);  /* PWM05 won't go REV if rc_dig_in08 is low. */
  Limit_Switch_Max(rc_dig_in10, &pwm06);  /* PWM06 won't go FWD if rc_dig_in10 is low. */
  Limit_Switch_Min(rc_dig_in12, &pwm06);  /* PWM06 won't go REV if rc_dig_in12 is low. */


 /*---------- R/C PWM INPUTs toggle solenoids -----------------------------*/

  if (PWM_in3 < 95)         /* PWM_in3 forward (typically) */
  {
    solenoid1 = 1;                /* turns on  Solenoid 1 */
    solenoid2 = 0;                /* turns off Solenoid 2 */
  }
  else if (PWM_in3 > 160)   /* PWM_in3 reverse (typically) */
  {
    solenoid1 = 0;                /* turns off Solenoid 1 */
    solenoid2 = 1;                /* turns on  Solenoid 2 */
  }
  else                      /* PWM_in3 neutral band */
  {
    solenoid1 = 0;                /* turns off Solenoid 1 */
    solenoid2 = 0;                /* turns off Solenoid 2 */
  }
  
  if (PWM_in4 < 95)         /* PWM_in4 left (typically) */
  {
    solenoid3 = 1;                /* turns on  Solenoid 3 */
    solenoid4 = 0;                /* turns off Solenoid 4 */
  }
  else if (PWM_in4 > 160)   /* PWM_in4 right (typically) */
  {
    solenoid3 = 0;                /* turns off Solenoid 3 */
    solenoid4 = 1;                /* turns on  Solenoid 4 */
  }
  else                      /* PWM_in4 neutral band */
  {
    solenoid3 = 0;                /* turns off Solenoid 3 */
    solenoid4 = 0;                /* turns off Solenoid 4 */
  }


 /*---------- Switch inputs toggle solenoids ------------------------------*/
  solenoid5 = !rc_dig_in14;
  solenoid6 = !rc_dig_in16;


} /* END Default_Routine(); */


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
