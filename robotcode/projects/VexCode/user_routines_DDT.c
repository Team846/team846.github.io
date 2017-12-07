/*******************************************************************************
* FILE NAME: user_routines_DDT.c <VEX VERSION>
*
* DESCRIPTION:
*  This is a modified version of the Default Code.  It includes additions
*  to allow it to be used with the Dynamic Debug Tool and the PicSerialDrv.*
*  files.
*  This file contains the default mappings of inputs  
*  (like switches, joysticks, and buttons) to outputs on the VEX RC.  
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
#include "PicSerialdrv.h"

tx_data_record txdataBufr;         


void Limit_Switch_Max(unsigned char switch_state, unsigned char *input_value){}
void Limit_Switch_Min(unsigned char switch_state, unsigned char *input_value){}
unsigned char Limit_Mix (int intermediate_value){}


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

/* Add any other user initialization code here. */

  Serial_Driver_Initialize();

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
  static unsigned char FirstTimeFlag;

  Getdata(&rxdata);   /* Get fresh data from the master microprocessor. */

  if (PC_Is_Not_In_Control())
  {
    if ((FirstTimeFlag & 2) == 0)  /* Do the the 1st time */
    {
      CCP2CON = 0x3C;
      PR2 = 0xF9;
      CCPR2L = 0x7F;
      T2CON = 0;
      T2CONbits.TMR2ON = 1;
      Setup_PWM_Output_Type(USER_CCP,IFI_PWM,IFI_PWM,IFI_PWM);
      FirstTimeFlag |= 2;    /* Set one shot Flag */
      txdata.user_cmd |= 0x02;       /* Tell master you want to be in auton mode. */
    }
    Default_Routine();       /* Optional.  See below.  Modified for DDT !!! */
    FirstTimeFlag &= 0xFE;   /* Clear Flag for else condition (when !PC_Is_Not_In_Control())*/
  }
  else
  {
    if ((FirstTimeFlag & 1) == 0)  /* Do the the 1st time */
    {
      T2CONbits.TMR2ON = 0;
      Setup_PWM_Output_Type(IFI_PWM,IFI_PWM,IFI_PWM,IFI_PWM);  
      FirstTimeFlag |= 1;   /* Set one shot Flag */
      txdata.user_cmd &= 0xFD;       /* Tell master you want to be in normal mode. */
    }
    FirstTimeFlag &= 0xFD;  /* Clear Flag for else condition (when PC_Is_Not_In_Control()) */
  }
  
  INTCONbits.GIEL  = 0;             /*Enable global interrupts*/
  txdataBufr = txdata;              /*Protect txdataBufr copy */
  INTCONbits.GIEL  = 1;             /*Enable global interrupts*/
  Putdata(&txdataBufr);             /* DO NOT CHANGE! */
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
  

} /* END Default_Routine(); */


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
