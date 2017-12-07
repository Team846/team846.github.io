/*******************************************************************************
* FILE NAME: user_routines.c <FRC VERSION>
*
* DESCRIPTION:
*  This file contains the default mappings of inputs  
*  (like switches, joysticks, and buttons) to outputs on the RC.  
*
* USAGE:
*  You can either modify this file to fit your needs, or remove it from your 
*  project and replace it with a modified copy. 
*
*******************************************************************************/
#include <stdio.h>
#include "ifi_aliases.h"
#include "ifi_default.h"
#include "ifi_utilities.h"
#include "user_routines.h"
#include "user_Serialdrv.h"


/*** DEFINE USER VARIABLES AND INITIALIZE THEM HERE ***/
#if _USE_CMU_CAMERA
#include "user_camera.h"
/****************************
**   VISION VARIABLES      **
*****************************/
int pan_dir=0;
int state=0;
int latch=0;
int latch2=0;
extern unsigned int index_ptr;
extern unsigned int data_rdy;
extern cam_struct cam;
extern unsigned int parse_mode;

int color=0;
const int steering_comp = 30; //steering compenstation (0 to 127)
const int speed_setting = 150; //forward speed setting (127 to 254)
extern int pan_position,tilt_position,tracking;
unsigned int speed_control;
unsigned int cam_state_flag;
void Camera_Processing(void);
#endif

extern unsigned char aBreakerWasTripped;


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
* FUNCTION NAME: User_Initialization
* PURPOSE:       This routine is called first (and only once) in the Main function.  
*                You may modify and add to this function.
* CALLED FROM:   main.c
* ARGUMENTS:     none
* RETURNS:       void
*******************************************************************************/
void User_Initialization (void)
{
  Set_Number_of_Analog_Channels(SIXTEEN_ANALOG);    /* DO NOT CHANGE! */

/* FIRST: Set up the I/O pins you want to use as digital INPUTS. */
  digital_io_01 = digital_io_02 = digital_io_03 = digital_io_04 = INPUT;
  digital_io_05 = digital_io_06 = digital_io_07 = digital_io_08 = INPUT;
  digital_io_09 = digital_io_10 = digital_io_11 = digital_io_12 = INPUT;
  digital_io_13 = digital_io_14 = digital_io_15 = digital_io_16 = INPUT;
  digital_io_18 = INPUT;  /* Used for pneumatic pressure switch. */
    /* 
     Note: digital_io_01 = digital_io_02 = ... digital_io_04 = INPUT; 
           is the same as the following:

           digital_io_01 = INPUT;
           digital_io_02 = INPUT;
           ...
           digital_io_04 = INPUT;
    */

/* SECOND: Set up the I/O pins you want to use as digital OUTPUTS. */
  digital_io_17 = OUTPUT;    /* Example - Not used in Default Code. */

/* THIRD: Initialize the values on the digital outputs. */
  rc_dig_out17 = 0;

/* FOURTH: Set your initial PWM values.  Neutral is 127. */
  pwm01 = pwm02 = pwm03 = pwm04 = pwm05 = pwm06 = pwm07 = pwm08 = 127;
  pwm09 = pwm10 = pwm11 = pwm12 = pwm13 = pwm14 = pwm15 = pwm16 = 127;

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
         T2CON = 0;
         T2CONbits.TMR2ON = 1;

         Setup_PWM_Output_Type(USER_CCP,IFI_PWM,IFI_PWM,IFI_PWM);
  */

  /* Add any other initialization code here. */

  Putdata(&txdata);             /* DO NOT CHANGE! */

  Serial_Driver_Initialize();
  printf("IFI 2005 User Processor Initialized ...\r");  /* Optional - Print initialization message. */
  /* Note:  use a '\r' rather than a '\n' with the new compiler (v2.4) */

#if _USE_CMU_CAMERA
  cam_state_flag = 0;
#endif

  User_Proc_Is_Ready();         /* DO NOT CHANGE! - last line of User_Initialization */
}

/*******************************************************************************
* FUNCTION NAME: Process_Data_From_Master_uP
* PURPOSE:       Executes every 26.2ms when it gets new data from the master 
*                microprocessor.
* CALLED FROM:   main.c
* ARGUMENTS:     none
* RETURNS:       void
*******************************************************************************/
void Process_Data_From_Master_uP(void)
{
  static unsigned char i;
  static unsigned char delay;

  Getdata(&rxdata);   /* Get fresh data from the master microprocessor. */

  Default_Routine();  /* Optional.  See below. */

  Generate_Pwms(pwm13,pwm14,pwm15,pwm16);

  /* Eample code to check if a breaker was ever tripped. */

  if (aBreakerWasTripped)
  {
    for (i=1;i<29;i++)
    {
      if (Breaker_Tripped(i))
        User_Byte1 = i;  /* Update the last breaker tripped on User_Byte1 (to demonstrate the use of a user byte) 
                            Normally, you do something else if a breaker got tripped (ex: limit a PWM output)     */
    }
  }

#if _USE_CMU_CAMERA
  switch (cam_state_flag)
  {
    case 0:
      delay = 38;              //Wait for at least a second before talking to the camer (26.2ms * 38 = ~1s)
      cam_state_flag = 4;
      break;
    case 1:
      if (camera_init(64,85,50))  // Set the 3 exposure values yellow, green and red
      {
        cam_state_flag = 2;
        Pwm1_red = Pwm2_red = Relay1_red = Relay2_red = 0; 
      }
      else
      {
        cam_state_flag = 4;   //Issue continual retries until camera responds
        delay = 19;           //Set delay for 1/2 sec (26.2ms * 19 = ~.5s)
        Pwm1_red ^= 1;      //Flash All Color LED Indicators when camera is not responding
        Pwm2_red ^= 1; 
        Relay1_red ^= 1; 
        Relay2_red ^= 1; 
      }
      break;
    case 2:
      camera_auto_servo(1);  // Turn on auto-servo mode and set servo parameters
      cam_state_flag = 3;
      break;
    case 3:                  // Continue processing
      Camera_Processing();
      break;
    case 4:
      if (delay)
        delay--;
      else
      {
        cam_state_flag = 1;
      }
      break;
  }
#else
  /* Add your own code here. (a printf will not be displayed when connected to the breaker panel unless a Y cable is used) */

  printf("Port1 Y %3d, X %3d, Fire %d, Top %d\r",(int)p1_y,(int)p1_x,(int)p1_sw_trig,(int)p1_sw_top);  /* printf EXAMPLE */
#endif

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
  
 /*---------- Analog Inputs (Joysticks) to PWM Outputs-----------------------
  *--------------------------------------------------------------------------
  *   This maps the joystick axes to specific PWM outputs.
  */
  pwm01 = p1_y;   
  pwm02 = p2_y;   
  pwm03 = p3_y;   
  pwm04 = p4_y;   
  pwm05 = p1_x;   
  pwm06 = p2_x;   
  pwm07 = p3_x;   
  pwm08 = p4_x;   
  pwm09 = p1_wheel;
  pwm10 = p2_wheel;   
  pwm11 = p3_wheel;   
  pwm12 = p4_wheel;   
  
 /*---------- 1 Joystick Drive ----------------------------------------------
  *--------------------------------------------------------------------------
  *  This code mixes the Y and X axis on Port 1 to allow one joystick drive. 
  *  Joystick forward  = Robot forward
  *  Joystick backward = Robot backward
  *  Joystick right    = Robot rotates right
  *  Joystick left     = Robot rotates left
  *  Connect the right drive motors to PWM13 and/or PWM14 on the RC.
  *  Connect the left  drive motors to PWM15 and/or PWM16 on the RC.
  */  
#if _USE_CMU_CAMERA
  if (p1_sw_trig > 0 && tracking > 0) { //If vision active and tracking use camera data
    p1_y = speed_control; //set forward speed
    p1_x = pan_position;  //set turning rate
    p1_x = 255 - p1_x;    //invert turn direction
    // Steering Compensation
    if (p1_x > 135 && p1_x < 225 && steering_comp > 0)
      p1_x = p1_x + steering_comp;
    if (p1_x < 120 && p1_x > steering_comp && steering_comp > 0)
      p1_x = p1_x - steering_comp;
    pwm11 = Limit_Mix(2000 + p1_y + p1_x - 127);
    pwm12 = Limit_Mix(2000 + p1_y - p1_x + 127);
  }
  else 
    if (p1_sw_trig < 1) { //If vision not active use joystick
      pwm11 = Limit_Mix(2000 + p1_y + p1_x - 127);
      pwm12 = Limit_Mix(2000 + p1_y - p1_x + 127);
    }
#else
  pwm13 = pwm14 = Limit_Mix(2000 + p1_y + p1_x - 127);
  pwm15 = pwm16 = Limit_Mix(2000 + p1_y - p1_x + 127);
#endif
  
 /*---------- Buttons to Relays----------------------------------------------
  *--------------------------------------------------------------------------
  *  This default code maps the joystick buttons to specific relay outputs.  
  *  Relays 1 and 2 use limit switches to stop the movement in one direction.
  *  The & used below is the C symbol for AND                                
  */
  relay1_fwd = p1_sw_trig & rc_dig_in01;  /* FWD only if switch1 is not closed. */
  relay1_rev = p1_sw_top  & rc_dig_in02;  /* REV only if switch2 is not closed. */
  relay2_fwd = p2_sw_trig & rc_dig_in03;  /* FWD only if switch3 is not closed. */
  relay2_rev = p2_sw_top  & rc_dig_in04;  /* REV only if switch4 is not closed. */
  relay3_fwd = p3_sw_trig;
  relay3_rev = p3_sw_top;
  relay4_fwd = p4_sw_trig;
  relay4_rev = p4_sw_top;
  relay5_fwd = p1_sw_aux1;
  relay5_rev = p1_sw_aux2;
  relay6_fwd = p3_sw_aux1;
  relay6_rev = p3_sw_aux2;
  relay7_fwd = p4_sw_aux1;
  relay7_rev = p4_sw_aux2;
  relay8_fwd = !rc_dig_in18;  /* Power pump only if pressure switch is off. */
  relay8_rev = 0;
  
  /*---------- PWM outputs Limited by Limit Switches  ------------------------*/
  
  Limit_Switch_Max(rc_dig_in05, &pwm03);
  Limit_Switch_Min(rc_dig_in06, &pwm03);
  Limit_Switch_Max(rc_dig_in07, &pwm04);
  Limit_Switch_Min(rc_dig_in08, &pwm04);
  Limit_Switch_Max(rc_dig_in09, &pwm09);
  Limit_Switch_Min(rc_dig_in10, &pwm09);
  Limit_Switch_Max(rc_dig_in11, &pwm10);
  Limit_Switch_Min(rc_dig_in12, &pwm10);
  Limit_Switch_Max(rc_dig_in13, &pwm11);
  Limit_Switch_Min(rc_dig_in14, &pwm11);
  Limit_Switch_Max(rc_dig_in15, &pwm12);
  Limit_Switch_Min(rc_dig_in16, &pwm12);
  
#if _USE_CMU_CAMERA
  return;     //Don't execute the ROBOT FEEDBACK if the CMU camera is used
#endif

 /*---------- ROBOT FEEDBACK LEDs------------------------------------------------
  *------------------------------------------------------------------------------
  *   This section drives the "ROBOT FEEDBACK" lights on the Operator Interface.
  *   The lights are green for joystick forward and red for joystick reverse.
  *   Both red and green are on when the joystick is centered.  Use the
  *   trim tabs on the joystick to adjust the center.     
  *   These may be changed for any use that the user desires.                       
  */	
  
  if (user_display_mode == 0) /* User Mode is Off */
    
  { /* Check position of Port 1 Joystick */
    if (p1_y >= 0 && p1_y <= 56)
    {                     /* Joystick is in full reverse position */
      Pwm1_green  = 0;    /* Turn PWM1 green LED - OFF */
      Pwm1_red  = 1;      /* Turn PWM1 red LED   - ON  */
    }
    else if (p1_y >= 125 && p1_y <= 129)
    {                     /* Joystick is in neutral position */
      Pwm1_green  = 1;    /* Turn PWM1 green LED - ON */
      Pwm1_red  = 1;      /* Turn PWM1 red LED   - ON */
    }
    else if (p1_y >= 216 && p1_y <= 255)
    {                     /* Joystick is in full forward position*/
      Pwm1_green  = 1;    /* Turn PWM1 green LED - ON  */
      Pwm1_red  = 0;      /* Turn PWM1 red LED   - OFF */
    }
    else
    {                     /* In either forward or reverse position */
      Pwm1_green  = 0;    /* Turn PWM1 green LED - OFF */
      Pwm1_red  = 0;      /* Turn PWM1 red LED   - OFF */
    }  /*END Check position of Port 1 Joystick
    
    /* Check position of Port 2 Y Joystick 
           (or Port 1 X in Single Joystick Drive Mode) */
    if (p2_y >= 0 && p2_y <= 56)
    {                     /* Joystick is in full reverse position */
      Pwm2_green  = 0;    /* Turn pwm2 green LED - OFF */
      Pwm2_red  = 1;      /* Turn pwm2 red LED   - ON  */
    }
    else if (p2_y >= 125 && p2_y <= 129)
    {                     /* Joystick is in neutral position */
      Pwm2_green  = 1;    /* Turn PWM2 green LED - ON */
      Pwm2_red  = 1;      /* Turn PWM2 red LED   - ON */
    }
    else if (p2_y >= 216 && p2_y <= 255)
    {                     /* Joystick is in full forward position */
      Pwm2_green  = 1;    /* Turn PWM2 green LED - ON  */
      Pwm2_red  = 0;      /* Turn PWM2 red LED   - OFF */
    }
    else
    {                     /* In either forward or reverse position */
      Pwm2_green  = 0;    /* Turn PWM2 green LED - OFF */
      Pwm2_red  = 0;      /* Turn PWM2 red LED   - OFF */
    }  /* END Check position of Port 2 Joystick */
    
    /* This drives the Relay 1 and Relay 2 "Robot Feedback" lights on the OI. */
    Relay1_green = relay1_fwd;    /* LED is ON when Relay 1 is FWD */
    Relay1_red = relay1_rev;      /* LED is ON when Relay 1 is REV */
    Relay2_green = relay2_fwd;    /* LED is ON when Relay 2 is FWD */
    Relay2_red = relay2_rev;      /* LED is ON when Relay 2 is REV */

    Switch1_LED = !(int)rc_dig_in01;
    Switch2_LED = !(int)rc_dig_in02;
    Switch3_LED = !(int)rc_dig_in03;
    
  } /* (user_display_mode = 0) (User Mode is Off) */
  
  else  /* User Mode is On - displays data in OI 4-digit display*/
  {
    User_Mode_byte = backup_voltage*10; /* so that decimal doesn't get truncated. */
  }   
  
} /* END Default_Routine(); */


#if _USE_CMU_CAMERA
/*******************************************************************************
* FUNCTION NAME: Camera_Processing
* PURPOSE:       Handles trigger buttons from OI and track updates
* CALLED FROM:   Process_Data_From_Master_uP
* ARGUMENTS:     none
* RETURNS:       void
*******************************************************************************/
void Camera_Processing(void)
{
  static unsigned char i;

  if (p2_sw_top==1)
  {
    latch2++;
    if (latch2==1)
    {
      pan_dir++;
      if (pan_dir>2) pan_dir=0;
      switch(pan_dir)
      {
        case 0: 
          camera_set_servos( 128, 128 ); 
          break;
        case 1: 
          camera_set_servos( 41, 128 ); 
          break;
        case 2: 
          camera_set_servos( 210, 128 ); 
          break;
      }
    }
  } else latch2=0;
  /* Add your own code here. */
  if (p2_sw_trig==1)
  {
    latch++;
    if (latch==1 ) // This stops the trigger from rapidly cycling through the colors
    {
      // Clear all of the LEDs
      Pwm1_green=0; Pwm2_green=0; Relay1_green=0; Relay2_green=0; Switch1_LED=0;
      color++; // Cycle to next color (must start counting at 1)
      if (color>5) color=1;    // The colors are defined as {1-5} = YELLOW, GREEN, WHITE, RED, BLUE
      // This means you can also say: camera_find_color( GREEN );
      i=camera_find_color(color);
      printf( "COLOR SWITCH %d = %d\r",color,i );
      // This part below sets the LEDs so you can see which color you are tracking.
      switch(color)  
      {
        case 1: 
          Pwm1_green=1; 
          break;   // Yellow
        case 2: 
          Pwm2_green=1; 
          break;   // Green
        case 3: 
          Relay1_green=1; 
          break; // White
        case 4: 
          Relay2_green=1; 
          break; // Red
        case 5: 
          Switch1_LED=1; 
          break;  // Blue
      }
    }
  } 
  else 
    latch=0;
  
  if (camera_track_update()==1)
  {
    // Put vision processing here, because we have a good frame!
    printf( "Got T packet %d %d %d %d servo: %d %d\r",cam.x,cam.y,cam.size,cam.conf,cam.pan_servo,cam.tilt_servo );
    if (cam.size > 0 && p1_sw_trig>0) {     //Check to see if camera is tracking
      tracking = 1;               // if yes then set 'tracking' flag
      pan_position = cam.pan_servo;
    }
    else
      tracking = 0;               // if there is no track, clear the flag
  }
}
#endif

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
