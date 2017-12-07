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
#include "gyro.h"
#include "adc.h"

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
void User_Initialization(void)
{
  rom const	char *strptr = "IFI User Processor Initialized ...";

/* FIRST: Set up the pins you want to use as analog INPUTs. */
  IO1 = IO2 = IO3 = IO4 = INPUT;        /* Used for analog inputs. */
    /* 
     Note: IO1 = IO2 = IO3 = IO4 = INPUT; 
           is the same as the following:

           IO1 = INPUT;
           IO2 = INPUT;
           IO3 = INPUT;
           IO4 = INPUT;
    */

/* SECOND: Configure the number of analog channels. */
  Set_Number_of_Analog_Channels(FOUR_ANALOG);     /* See ifi_aliases.h */

/* THIRD: Set up any extra digital inputs. */
  /* The six INTERRUPTS are already digital inputs. */
  /* If you need more then set them up here. */
  /* IOxx = IOyy = INPUT; */
  IO6 = IO8 = IO10 = INPUT;      /* Used for limit switch inputs. */
  IO12 = IO14 = IO16 = INPUT;    /* Used for limit switch inputs. */

/* FOURTH: Set up the pins you want to use as digital OUTPUTs. */
  IO5 = IO7 = IO9 = INPUT;     /* For connecting to adjacent limit switches. */
  IO11 = IO13 = IO15 = INPUT;  /* For connecting to adjacent limit switches. */

/* FIFTH: Initialize the values on the digital outputs. */
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

  Initialize_Serial_Comms();

  Initialize_Gyro();     

  Initialize_ADC();

  Putdata(&txdata);             /* DO NOT CHANGE! */

//  printf("%s\n", strptr);       /* Optional - Print initialization message. */

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
	static unsigned int i = 0;
	static unsigned int j = 0;
	int temp_gyro_rate;
	long temp_gyro_angle;
	int temp_gyro_bias;

	i++;
	j++; // this will rollover every ~1000 seconds

	if(j == 10)
		printf("\rCalculating Gyro Bias...");

	if(j == 60)
	{
		// start a gyro bias calculation
		Start_Gyro_Bias_Calc();
	}

	if(j == 300)
	{
		// terminate the gyro bias calculation
		Stop_Gyro_Bias_Calc();

		// reset the gyro heading angle
		Reset_Gyro_Angle();

		printf("Done\r");
	}

	if(i >= 30 && j >= 300)
	{
		temp_gyro_bias = Get_Gyro_Bias();
		temp_gyro_rate = Get_Gyro_Rate();
		temp_gyro_angle = Get_Gyro_Angle();
		printf(" Gyro Bias=%d\r", temp_gyro_bias);
		printf(" Gyro Rate=%d\r", temp_gyro_rate);
		printf("Gyro Angle=%d\r\r", (int)temp_gyro_angle);

		i = 0;
	}

	Getdata(&rxdata);
	Putdata(&txdata);
}
