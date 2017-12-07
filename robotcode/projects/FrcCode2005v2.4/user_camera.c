/******************************************************************************************************
FILE NAME: user_camera.c

MODIFICATION HISTORY:
		11/28/04 First Version by Anthony Rowe

DESCRIPTION:
	This file contains a set of function that communicate with the CMUcam.  It also requires a 
	modification to the PicSerialdrv.c file that is standard with the FRC distribution. This 
	modification allows the uart interrupt routine to buffer the CMUcam packets.

LIMITATION:
  Sometimes, after pressing the reset button, the camera will not come up correctly.  If you 
  power cycle the camera when it is stuck - it will work correctly.  It seems to always work
  from an RC power up.

******************************************************************************************************/
#include <stdio.h>
#include "user_Serialdrv.h"
#include "user_camera.h"

#if _USE_CMU_CAMERA
extern unsigned int parse_mode;
extern volatile unsigned int cam_index_ptr;
extern volatile unsigned int data_rdy;
unsigned char cam_uart_buffer[64]; 


// Different exposure values
int camera_exp_yellow;
int camera_exp_green;
int camera_exp_red;

// This keeps the state of the servo handy, for when the camera is reset like when tracking a new color
int servo_mode=0;
int servo_pan=127;
int servo_tilt=127;


cam_struct cam;   // Main camera struct that is defined in camera.h, use this to access all camera data


// AEC - Automatic Exposure Control
// AGC - Automatic Gain Control
// RGB - Red Green Blue color space
// YCrCb - Another color space (not RGB), that is less effected by lighting changes
rom const char *noise_filter = "nf 6";  // Set noise filter to build up a tolerance to stray pixels
rom const char *agc_level = "CR 0 32"; // Set AGC gain to mid level
rom const char *yCrCb_mode = "CR 18 0"; // Set into yCrCb mode instead of RGB
rom const char *track_green = "TC 85 120 0 255 80 150"; // Track green color in yCrCb mode
														// Cr (85, 120)
														// Cb (80, 150)
														// Y  (0, 255)
rom const char *track_yellow = "TC 100 255 75 255 0 20";// Track yellow color in RGB mode
														// R (100, 255)
														// G (75, 255 )
														// B (0 , 20 )
rom const char *track_white = "TC 150 255 150 255 50 255";// Track yellow color in RGB mode
rom const char *track_red = "TC 190 255 0 255 0 40";// Track red color in YCrCb mode
rom const char *track_blue = "TC 0 150 0 150 70 255";// Track blue color in RGB mode
													
rom const char *aec_disable = "CR 41 128"; 
rom const char *manual_agc = "CR 19 32"; 
rom const char *raw_mode = "rm 1";

int camera_find_color(int color)
{
  unsigned char buf_str[16];   // buffer used for exposure register value
  int chk=0,correct;
  
  buf_str[0]='C'; buf_str[1]='R'; buf_str[2]=' '; buf_str[3]='1'; buf_str[4]='6'; buf_str[5]=' ';
  // This is the start of the CR 16 xx string that gets filled in below
  
  camera_stop();          // Stop the camera if it is already tracking
  chk+=camera_reset();      // Reset the camera, and make sure it reset (actually check for true return)
  if(servo_mode) camera_auto_servo(1);  // Put it into servo mode if need be
  chk+=camera_const_cmd(noise_filter);  // Enable the same noise filter for all tracking
  chk+=camera_const_cmd(raw_mode);      // Put into raw mode, this makes it send output numbers as bytes instead
  // of being ascii readable
  switch(color)
  {
  case RED:
    correct=9;  // This is the total number of commands that need to return true as a checksum
    // Increase this if you add more camera commands or it will trigger an error below
    write_int_to_buffer(&(buf_str[6]),camera_exp_red);
    // This writes an integer value into the buffer, after the "CR 16"
    chk+=camera_const_cmd(yCrCb_mode);  // YCrCb mode!!!
    chk+=camera_const_cmd(aec_disable);
    chk+=camera_const_cmd(manual_agc);
    chk+=camera_buffer_cmd( buf_str );  // Note camera_buffer_cmd, not camera_const_cmd
    chk+=camera_const_cmd(agc_level);
    chk+=camera_const_cmd(track_red);
    break;
  case YELLOW: case WHITE: case BLUE:   // Yellow, White, and Blue use similiar settings
    correct=8;
    write_int_to_buffer(&(buf_str[6]),camera_exp_yellow);
    chk+=camera_const_cmd(aec_disable);
    chk+=camera_const_cmd(manual_agc);
    chk+=camera_const_cmd(agc_level);
    chk+=camera_buffer_cmd( buf_str );
    if( color==YELLOW ) chk+=camera_const_cmd( track_yellow );
    if( color==WHITE ) chk+=camera_const_cmd( track_white );
    if( color==BLUE ) chk+=camera_const_cmd( track_blue );
    break;
  case GREEN:
    correct=9;
    write_int_to_buffer(&(buf_str[6]),camera_exp_green);
    chk+=camera_const_cmd(yCrCb_mode);    // YCrCb mode!!!
    chk+=camera_const_cmd(aec_disable);
    chk+=camera_const_cmd(manual_agc);
    chk+=camera_buffer_cmd( buf_str );
    chk+=camera_const_cmd(agc_level);
    chk+=camera_const_cmd(track_green);
    break;
  default:
    
    return 0;
    break;
  }
  // Do not add commands below here, because it will interrupt tracking
  if(chk!=correct)  // Checks to see if the correct number of camera commands returned true
  {
    printf( "* Error setting color tracking parameters\rExpected %d, and got %d\r",correct,chk );
#if FREEZE_ON_ERROR
    while(1);
#endif
    return 0;  // only if not FREEZE_ON_ERROR
  }
  parse_mode=1; // Now waiting for Track Color Packet in interrupt
  return 1;
}

/**********************************************************************
camera_set_servos

	This function sets the pan and the tilt servo position.
	Note, that this function will stop the camera from tracking.
	If you are tilting the head to see if something is there, you need
	to send a fresh track command.  Maybe it would be more efficient to
	use the RC's servo output.
	
	int pan  - servo pan value from 46 to 210, with 128 as the middle
			 - Yes, we know that seems strange, but there is a good reason
	int tilt - servo tilt value from 46 to 210, with 128 as the middle

	Return: 0 missing ACK, or timeout
			1 Both commands are good

**********************************************************************/
int camera_set_servos( int pan, int tilt )
{
  int chk=0;
  unsigned char buf[32];
  servo_pan=pan;
  servo_tilt=tilt;
  buf[0]='S';
  buf[1]='V';
  buf[2]=' ';
  buf[3]='0';  // Servo 0 is usually the pan
  buf[4]=' ';
  camera_stop();
  write_int_to_buffer(&buf[5], pan );
  chk+=camera_buffer_cmd(buf);
  buf[3]='1';  // Servo 1, yes there are up to 5 servos
  write_int_to_buffer(&buf[5], tilt );
  chk+=camera_buffer_cmd(buf);
  if(chk!=2) return 0;
  return 1;
}


/**********************************************************************
write_int_to_buffer

This function takes a buffer and adds an ascii value to the end of it.
	unsigned char *buf - Location in buffer to start appending a value
	int val	- the integer value that is to be added to the buffer

**********************************************************************/
void write_int_to_buffer(unsigned char *buf, int val )
{
  int digit;
  int index;
  index=0;
  digit=val/100;
  digit=digit%10; // This grabs the hundreds place digit
  if(digit>0)    // If it is a zero, we don't need to print it
  {
    buf[index]=digit+48;
    index++;
  }
  digit=val/10;
  digit=digit%10;    // This grabs the tens place digit
  if(index>0 || digit>0 )  // If the hundreds place was a 0 and this is also
  {					// a 0, skip it, else print the number
    buf[index]=digit+48;
    index++;
  }
  digit=val%10;  // Print out the ones place digit no matter what
  buf[index]=digit+48;
  index++;
  buf[index]=0;  // Add a null character to terminate the string
  
}

/**********************************************************************
camera_init

This function checks to see if the camera is available, and sets
the 3 predefined exposures that should be given from the calibration
program.

	exp_yellow - exposure value for yellow, white and blue RGB
	exp_green  - exposure value for green in YCrCb
	exp_red	   - exposure value for red in YCrCb

**********************************************************************/
int camera_init(int exp_yellow, int exp_green, int exp_red)
{
  int r;
  if(camera_reset()==0) 
  {
    printf( "\r\rCamera not found!\rPlease check, power and cable connections.\r\r" );
#if FREEZE_ON_ERROR
		  while(1);
#endif
    return 0;
  }
  camera_exp_yellow=exp_yellow;
  camera_exp_green=exp_green;
  camera_exp_red=exp_red;
  r=0;
  r+=camera_const_cmd(aec_disable ); // Disable AEC
  r+=camera_const_cmd(manual_agc ); // Enable manual AEC and AGC
  r+=camera_const_cmd(noise_filter ); // Set noise filter to 6 pixels
  r+=camera_const_cmd(raw_mode ); // Set noise filter to 6 pixels
  if(r<4)
  {
    printf( "\r\rOne of the camera settings on initilization failed\r\r" );
#if FREEZE_ON_ERROR
    while(1);
#endif
  }
  return 1;
}

/********************************************************************************
camera_track_update

This function should be called after a color tracking call. The interrupt
will fill this with a T packet from the camera if parse_mode is set to 1.
parse_mode is set to 1 by track color.

	Return:  1 when new data is ready (only read packet when 1)
			 0 when waiting for the camera
*********************************************************************************/
int camera_track_update(void)
{
  int i;
  
  if(wait_for_data()==0) return 0;
  
  if(cam_uart_buffer[0]=='T' && cam_index_ptr>=9)
  {
    cam.x=cam_uart_buffer[1];
    cam.y=cam_uart_buffer[2];
    cam.x1=cam_uart_buffer[3];
    cam.y1=cam_uart_buffer[4];
    cam.x2=cam_uart_buffer[5];
    cam.y2=cam_uart_buffer[6];
    cam.size=cam_uart_buffer[7];
    cam.conf=cam_uart_buffer[8];
    if(cam_index_ptr>9)
    {
      cam.pan_servo=cam_uart_buffer[9];
      cam.tilt_servo=cam_uart_buffer[10];
    }
    reset_rx_buffer();
    return 1;
  }
  // Bad data in buffer
  reset_rx_buffer();
  return 0;
}

/**********************************************************************
camera_configure

This function shows how you can set the exposure gain and color_mode
of the camera manually. 
	int exposure - sets the exposure level
				 - lower values make the image darker
	int gain	 - this sets the color gain of the image
				 - lower values make the image less intense
	int color_mode - RGB puts the camera in RGB mode
				   - YCrCb puts the camera in YCrCb mode

    Return: 1 ACK
			0 no ACK, maybe a NCK or a timeout
**********************************************************************/
int camera_configure( int exposure, int gain, int color_mode )
{
  unsigned char buf[32];
  int chk=0;
  
  if(color_mode==RGB)
    chk+=camera_const_cmd( "cr 18 8" );  // RGB mode
  else
    chk+=camera_const_cmd( yCrCb_mode );  // YCrCb mode
  
  chk+=camera_const_cmd(aec_disable);  // Disable automatic AEC
  chk+=camera_const_cmd(manual_agc);   // Allow manual settings
  buf[0]='C';
  buf[1]='R';
  buf[2]=' ';
  buf[3]='1';
  buf[4]='6';
  buf[5]=' ';
  write_int_to_buffer(&buf[6], exposure ); // Send the constructed exposure
  chk+=camera_buffer_cmd( buf ); 
  buf[0]='C';
  buf[1]='R';
  buf[2]=' ';
  buf[3]='0';
  buf[4]=' ';
  write_int_to_buffer(&buf[5], gain );  // Send the constructed gain value
  chk+=camera_buffer_cmd( buf ); 
  if(chk<5)
  {
    printf( "\r\rOne of the camera settings in camera_configure failed\r\r" );
#if FREEZE_ON_ERROR
    while(1);
#endif
  }
}

/**********************************************************************
camera_getACK

This function sends the final '\r' character and waits to see if the
camera returns an ACK or a NCK. This only works when parse_mode = 0 and
should be used for control commands to the camera, and not for tracking
commands.

    Return: 1 ACK
			0 no ACK, maybe a NCK or a timeout
**********************************************************************/
int camera_getACK(void)
{
  int cnt,i;
  Serial_Write(CAMERA_PORT,"\r",1);
  reset_rx_buffer();
  if( wait_for_data()==0) return 0;
  if(cam_uart_buffer[0]==65) 
  {
    reset_rx_buffer();
    return 1;
  }
  if(cam_uart_buffer[0]==84) 
  {
    return 2;
  }
  reset_rx_buffer();
  return 0;
  
}

/**********************************************************************
camera_auto_servo

This function turns on auto servo mode.  It also sets servo_mode=1 or 0
which allows commands that reset the camera, to make sure they call this
function again to restore the lost servo state.

     int enable  - 1 for on, 0 for off

	 Return: 0 - command failed with NCK or timeout
			 1 - got ACK and everything is good
**********************************************************************/
int camera_auto_servo(int enable)
{
  int i;
  if (enable)
  {
    i = camera_const_cmd( "SP 50 25 15 48 24 11" );
    i &= camera_const_cmd( "SM 15" );
    camera_set_servos( servo_pan, servo_tilt );
    servo_mode = 1;
    return i;
  }
  else
  {
    servo_mode=0;
    return camera_const_cmd( "SM 0" );
  }
}

/**********************************************************************
camera_const_cmd

This function is used to send constant string commands to the camera.
This means you have to send something in quotes.  If you are sending from
a buffer, you must use camera_buffer_cmd.  See camera_find_color for examples.

		rom const char *cmd_str - This is a constant string to be sent to the camera
								- It does not require the '\r' at the end

		Return: 0 - no ACK, or timed out
				1 - ACK, the command is good
**********************************************************************/
int camera_const_cmd(rom const char *cmd_str)
{
  int i;
  int len;
  for (i=0; i<MAX_BUF_SIZE; i++ )
  {
    if (cmd_str[i]=='\r' || cmd_str[i]==0 )
    {
      len = i;
      //	cmd_str[i]=0;
      break;
    }
  }
  Serial_Write(CAMERA_PORT,cmd_str,len);
  return camera_getACK();
}

/**********************************************************************
camera_const_cmd

This function is used to send a string from memory to the camera. It is
camera_const_cmd's twin, but is used for arrays that you make yourself.

		unsigned char *cmd_str - This is a string to be sent to the camera
								- It does not require the '\r' at the end

		Return: 0 - no ACK, or timed out
				1 - ACK, the command is good
**********************************************************************/
int camera_buffer_cmd(unsigned char *cmd_str)
{
  int i;
  int len;
  for(i=0; i<MAX_BUF_SIZE; i++ )
  {
    if(cmd_str[i]=='\r' || cmd_str[i]==0 )
    {
      len=i;
      break;
    }
  }
  Serial_Write_Bufr(CAMERA_PORT,cmd_str,len);
  return camera_getACK();
}

/**********************************************************************
camera_stop

This function stops the camera from streaming.  This should be used when
you are in the middle of tracking, and want to send new control commands.

		Return: 0 - no ACK, or timed out and may not have stopped
				1 - ACK, the command is good, the camera stopped
**********************************************************************/
int camera_stop(void)
{
  parse_mode=0;
  camera_getACK();
  return camera_getACK();
}

/**********************************************************************
reset_rx_buffer

This function will reset the pointer index and the data_rdy flag that
is used by the interrupt in PicSerialdrv.c
This is called right before the interrupt routine is supposed to look
for new data.
**********************************************************************/
void reset_rx_buffer(void)
{
	cam_index_ptr=0;
	data_rdy=0;
}

/**********************************************************************
wait_for_data

This function checks to see if the serial buffer for the camera has new
data ready.  Depending on what mode it is in, it may wait for a full T
packet, or just a '\r' terminated line. It only waits a short period of
time, and then it returns 0 if no data is ready.  This short period of
time is just long enough to catch ACKs from messages.

		Return: 0 - no new data
				1 - new packet is ready
**********************************************************************/
int wait_for_data(void)
{
  int i;

  // This loop below is a counter that gives just enough time to catch
  // an ACK from a normal command.  
  for(i=0; i<20000; i++ )     
	  if(data_rdy!=0 ) return 1;
	
  return 0;
}

/**********************************************************************
camera_reset

This function resets the camera.  This will clear all register.  It then
checks to see if the camera is responding.

		Return: 0 - no ACK, and reset may not have happened
				1 - reset occured, and all is well
**********************************************************************/
int camera_reset(void)
{
  int i;
  parse_mode=0;
  Serial_Write(CAMERA_PORT,"rs\r",3);
  i=camera_getACK();
  return camera_getACK();
}

#endif

