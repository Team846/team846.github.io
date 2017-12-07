#ifndef __user_camera_h_
#define __user_camera_h_

#define MAX_BUF_SIZE        64
#define CAMERA_PORT         1
#define FREEZE_ON_ERROR     0

/* Default Trackable Colors */
#define YELLOW  1
#define GREEN   2
#define WHITE   3
#define RED     4
#define BLUE    5

/* Two possible color modes*/
#define RGB     0
#define YCrCb   1

/* 
	This is the main struct definition that gets filled when camera_track_update
    is called.  The servo values are only updated if the servo mode is enabled.
*/
typedef struct
{
  unsigned int  x,y;
  unsigned int  x1,y1,x2,y2;
  unsigned int  size,conf,pan_servo,tilt_servo;
} cam_struct;


/* 
	These commands can be used by the user to control the camera.
	Look in camera.c for more detailed comments.
*/
int camera_init( int exp_yellow, int exp_green, int exp_red);
int camera_stop(void);
int camera_track_update(void);
int camera_find_color(int color);
int camera_auto_servo(int enable);
int camera_set_servos( int pan, int tilt );

/*
	These commands are used internally by the camera driver.
	They could be used by you, but be careful.  
	Read the CMUcam manual!
*/
int camera_const_cmd(rom const char *cmd_str);
int camera_buffer_cmd(unsigned char *cmd_str);
int camera_configure( int exposure, int gain, int color_mode );
int camera_reset(void);
int camera_getACK(void);
int wait_for_data(void);
void reset_rx_buffer(void);
void write_int_to_buffer(unsigned char *buf, int val );

#endif
