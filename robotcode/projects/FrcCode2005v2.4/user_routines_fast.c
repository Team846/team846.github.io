/*******************************************************************************
* FILE NAME: user_routines_fast.c <FRC VERSION>
*
* DESCRIPTION:
*  This file is where the user can add their custom code within the framework
*  of the routines below. 
*
* USAGE:
*  You can either modify this file to fit your needs, or remove it from your 
*  project and replace it with a modified copy. 
*
* OPTIONS:  Interrupts are disabled and not used by default.
*
*******************************************************************************/

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
extern cam_struct cam;
extern int speed_control;
extern int speed_setting;
extern unsigned char cam_uart_buffer[];
unsigned char char_buffer[2];  // 11
volatile unsigned int data_rdy;
volatile unsigned int cam_index_ptr = 0;  // Zero start of buffer
unsigned int start_string;
unsigned char rx_char;
int packet_start = 0;                     // Used to parse packet types
unsigned int parse_mode = 0;              // Tells it to look for '\r' instead of T packets
int tracking, pan_position, tilt_position, drive_control = 0;
int last_pan_position,begin_x, begin_y, end_y, i, j = 0;
unsigned int transit_char,trcnt = 0;
#endif

/*******************************************************************************
* FUNCTION NAME: InterruptVectorLow
* PURPOSE:       Low priority interrupt vector
* CALLED FROM:   nowhere by default
* ARGUMENTS:     none
* RETURNS:       void
* DO NOT MODIFY OR DELETE THIS FUNCTION 
*******************************************************************************/
#pragma code InterruptVectorLow = LOW_INT_VECTOR
void InterruptVectorLow (void)
{
  _asm
    goto InterruptHandlerLow  /*jump to interrupt routine*/
  _endasm
}


/*******************************************************************************
* FUNCTION NAME: InterruptHandlerLow
* PURPOSE:       Low priority interrupt handler
* If you want to use these external low priority interrupts or any of the
* peripheral interrupts then you must enable them in your initialization
* routine.  Innovation First, Inc. will not provide support for using these
* interrupts, so be careful.  There is great potential for glitchy code if good
* interrupt programming practices are not followed.  Especially read p. 28 of
* the "MPLAB(R) C18 C Compiler User's Guide" for information on context saving.
* CALLED FROM:   this file, InterruptVectorLow routine
* ARGUMENTS:     none
* RETURNS:       void
*******************************************************************************/
#pragma code
#pragma interruptlow InterruptHandlerLow save=PROD /* You may want to save additional symbols. */

void InterruptHandlerLow ()     
{                               
  unsigned char int_byte;       
  if (INTCON3bits.INT2IF && INTCON3bits.INT2IE)       /* The INT2 pin is RB2/DIG I/O 1. */
  { 
    INTCON3bits.INT2IF = 0;
  }
  else if (INTCON3bits.INT3IF && INTCON3bits.INT3IE)  /* The INT3 pin is RB3/DIG I/O 2. */
  {
    INTCON3bits.INT3IF = 0;
  }
  else if (INTCONbits.RBIF && INTCONbits.RBIE)  /* DIG I/O 3-6 (RB4, RB5, RB6, or RB7) changed. */
  {
    int_byte = PORTB;          /* You must read or write to PORTB */
    INTCONbits.RBIF = 0;     /*     and clear the interrupt flag         */
  }                                        /*     to clear the interrupt condition.  */
  else
  { 
    CheckUartInts();    /* For Dynamic Debug Tool or buffered printf features. */
  }
}


/*******************************************************************************
* FUNCTION NAME: User_Autonomous_Code
* PURPOSE:       Execute user's code during autonomous robot operation.
* You should modify this routine by adding code which you wish to run in
* autonomous mode.  It will be executed every program loop, and not
* wait for or use any data from the Operator Interface.
* CALLED FROM:   main.c file, main() routine when in Autonomous mode
* ARGUMENTS:     none
* RETURNS:       void
*******************************************************************************/
void User_Autonomous_Code(void)
{
  /* Initialize all PWMs and Relays when entering Autonomous mode, or else it
     will be stuck with the last values mapped from the joysticks.  Remember, 
     even when Disabled it is reading inputs from the Operator Interface. 
  */
    pwm01 = pwm02 = pwm03 = pwm04 = pwm05 = pwm06 = pwm07 = pwm08 = 127;
    pwm09 = pwm10 = pwm11 = pwm12 = pwm13 = pwm14 = pwm15 = pwm16 = 127;
    relay1_fwd = relay1_rev = relay2_fwd = relay2_rev = 0;
    relay3_fwd = relay3_rev = relay4_fwd = relay4_rev = 0;
    relay5_fwd = relay5_rev = relay6_fwd = relay6_rev = 0;
    relay7_fwd = relay7_rev = relay8_fwd = relay8_rev = 0;

  while (autonomous_mode)   /* DO NOT CHANGE! */
  {
    if (statusflag.NEW_SPI_DATA)      /* 26.2ms loop area */
    {
        Getdata(&rxdata);   /* DO NOT DELETE, or you will be stuck here forever! */

        /* Add your own autonomous code here. */

        Generate_Pwms(pwm13,pwm14,pwm15,pwm16);

        Putdata(&txdata);   /* DO NOT DELETE, or you will get no PWM outputs! */
    }
  }
}

/*******************************************************************************
* FUNCTION NAME: Process_Data_From_Local_IO
* PURPOSE:       Execute user's realtime code.
* You should modify this routine by adding code which you wish to run fast.
* It will be executed every program loop, and not wait for fresh data 
* from the Operator Interface.
* CALLED FROM:   main.c
* ARGUMENTS:     none
* RETURNS:       void
*******************************************************************************/
void Process_Data_From_Local_IO(void)
{
  /* Add code here that you want to be executed every program loop. */

#if _USE_CMU_CAMERA
  if (speed_setting < 128 || speed_control > 254)   //Check speed_setting for valid forward speed
    speed_setting = 150;            //If not valid number set to default
  if (p1_sw_trig > 0){               // RUN ONLY IF TRIGGER IS HELD
    if (tracking > 0)
      speed_control = speed_setting;
    else {
      trcnt ++;
      if (trcnt > 1000)
        speed_control = 127;
      else
        trcnt = 0;
    }
  }
  
  if (tracking < 1 && p1_sw_trig < 1){
    speed_control = 127;
    pan_position = 127;
  }
#endif
}

/*******************************************************************************
* FUNCTION NAME: Serial_Char_Callback
* PURPOSE:       Interrupt handler for the TTL_PORT.
* CALLED FROM:   user_SerialDrv.c
* ARGUMENTS:     
*     Argument             Type    IO   Description
*     --------             ----    --   -----------
*     data        unsigned char    I    Data received from the TTL_PORT
* RETURNS:       void
*******************************************************************************/

void Serial_Char_Callback(unsigned char tmp)
{
  /* Add code to handle incomming data (remember, interrupts are still active) */

#if _USE_CMU_CAMERA
/*******************************************************************************
	This is the section of the UART rx interrupt routine that handle the camera 
	buffer. There are two different modes parse_mode=0 and parse_mode=1.  
	
	parse_mode=0 
	is for reading ACKs back from the camera.  It will buffer an entire line until
	it reads a '\r' character.  It should be used for non streaming data from the
    camera.

	parse_mode=1
	is for reading tracking packets that are streaming from the camera. It assumes
	the camera is in raw mode and will read until it gets a 'T' character.  It then
	buffers until it gets a 255 bytes which in raw mode signifies the end of line.

	cam_index_ptr - is the counter that keeps track of the current location in the
					buffer.

	data_rdy - is a flag that remains 0 until the entire data packet is ready at
				which point it becomes 1.  Once data_rdy is 1, new packets will be
				ignored. 
********************************************************************************/
  if (data_rdy==0)
  {
    if (parse_mode==0)  // Grab single line packets, such as ACKS or NCKS
    {
      if (tmp=='\r' || cam_index_ptr==MAX_BUF_SIZE)
      {
        // Once the end of a packet is found, assert data_rdy and wait
        cam_uart_buffer[cam_index_ptr]=0;
        data_rdy=1;
      }
      else
      {
        cam_uart_buffer[cam_index_ptr]=tmp;
        cam_index_ptr++;
      }
    }
    if (parse_mode==1)   // Grab streaming packets
    {
      
      if (tmp=='T' )
      {
        
        data_rdy=0;
        cam_uart_buffer[0]=tmp;
        cam_index_ptr=1;
        return;
      }
      if (cam_index_ptr>0) // Added this to potentially stop unnecessary delays
      {
        if (tmp==255 || cam_index_ptr==MAX_BUF_SIZE)
        {
          if (cam_index_ptr==0 )return;
          
          // Once the end of a packet is found, assert data_rdy and wait
          cam_uart_buffer[cam_index_ptr]=0;
          data_rdy=1;
        }
        else
        {
          cam_uart_buffer[cam_index_ptr]=tmp;
          cam_index_ptr++;
        }
      }
    }
  }
#endif
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
