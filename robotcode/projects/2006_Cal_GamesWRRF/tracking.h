/*******************************************************************************
*
*	TITLE:		tracking.h 
*
*	VERSION:	0.2 (Beta)                           
*
*	DATE:		21-Feb-2006
*
*	AUTHOR:		R. Kevin Watson
*				kevinw@jpl.nasa.gov
*
*	COMMENTS:	This is the "streamlined" version of tracking.h
*
*				You are free to use this source code for any non-commercial
*				use. Please do not make copies of this source code, modified
*				or un-modified, publicly available on the internet or elsewhere
*				without permission. Thanks.
*
*				Copyright ©2005-2006 R. Kevin Watson. All rights are reserved.
*
********************************************************************************
*
*	CHANGE LOG:
*
*	DATE         REV  DESCRIPTION
*	-----------  ---  ----------------------------------------------------------
*	01-Jan-2006  0.1  RKW - Original code.
*	21-Feb-2006  0.2  RKW - Provided two new functions to set the pan and
*	                  tilt servo position. This was done to provide a level
*	                  of indirection so that the servos could be commanded
*	                  from the robot controller or the CMUcam2.
*	                  RKW - Fixed bug in search initialization code where
*	                  temp_pan_servo was initialized to zero instead of
*	                  Tracking_Config_Data.Pan_Min_PWM.
*	                  RKW - Altered tracking algorithm to use the t-packet
*	                  confidence value to determine whether the code should
*	                  track or search.
*	                  RKW - Added Get_Tracking_State() function, which can
*	                  be used to determine if the camera is on target.
*
*******************************************************************************/
#ifndef _TRACKING_H
#define _TRACKING_H

// By default, PWM output one is used for the pan servo.
// Change it to another value if you'd like to use PWM
// output one for another purpose.
#define PAN_SERVO pwm09

// By default, PWM output two is used for the tilt servo.
// Change it to another value if you'd like to use PWM
// output two for another purpose.
#define TILT_SERVO pwm10

// This value defines how many "slow loops" to wait before
// sending the tracking servo(s) to their next destination
// while in search mode. This provides a small delay for the 
// camera to lock onto the target between position changes.
#define SEARCH_DELAY_DEFAULT 6

// This defines the minimum t-packet confidence value needed
// before the tracking software will transition from search
// mode to tracking mode.
#define CONFIDENCE_THRESHOLD_DEFAULT 20

// These values define how quickly the camera will attempt 
// to track the object. If these are set too high, the camera 
// will take longer to settle, too low and the camera will
// overshoot the target and oscillate.
#define PAN_GAIN_DEFAULT 3
#define TILT_GAIN_DEFAULT 8

// If your camera suddenly moves away from the target once
// it finds it, you'll need to change the sign on one or
// both of these values. Start with the tilt first.
#define PAN_ROTATION_SIGN_DEFAULT 1
#define TILT_ROTATION_SIGN_DEFAULT -1

// These two values define the image pixel that we're
// going to try to keep the tracked object on. By default
// the center of the image is used.
#define PAN_TARGET_PIXEL_DEFAULT 79
#define TILT_TARGET_PIXEL_DEFAULT 119

// These values define how much error, in pixels, is 
// allowable when trying to keep the center of the tracked
// object on the center pixel of the camera's imager. Too 
// high a value and your pointing accuracy will suffer, too
// low and your camera may oscillate because the servos 
// don't have enough pointing resolution to get the center 
// of the tracked object into the square/rectangle defined 
// by these values 
#define PAN_ALLOWABLE_ERROR_DEFAULT 6
#define TILT_ALLOWABLE_ERROR_DEFAULT 6

// These values define the lower and upper bound on the
// pan servo travel
//#define PAN_MIN_PWM_DEFAULT 0   // -65 degrees
//#define PAN_MAX_PWM_DEFAULT 248 // +65 degrees
#define PAN_MIN_PWM_DEFAULT 124   // -65 degrees
#define PAN_MAX_PWM_DEFAULT 124 // +65 degrees

// This value defines how far the pan servo will step
// each time while in search mode. This value was chosen
// to provide five evenly spaced stopping points (i.e., 0,
// 62, 124, 186 and 248) for the pan servo.
//#define PAN_SEARCH_STEP_SIZE_DEFAULT 62
#define PAN_SEARCH_STEP_SIZE_DEFAULT 0

// These values define the PWM values of the pan and tilt
// servos when the camera is pointing directly ahead
// (i.e., when pan and tilt angles are zero).
#define PAN_CENTER_PWM_DEFAULT 124
#define TILT_CENTER_PWM_DEFAULT 144

// This defines whether the camera servo is tracking the
// vertical/tilt error
//#define _TRACK_TILT

// This value defines the default position for the camera
// when staying at a static position.
#define TILT_STATIC_DEFAULT 127

// These values define the lower and upper bound on the
// tilt servo travel
//#define TILT_MIN_PWM_DEFAULT 94  // -25 degrees	Original
//#define TILT_MAX_PWM_DEFAULT 194 // +25 degrees	Original
#define TILT_MIN_PWM_DEFAULT ((unsigned char) (TILT_CENTER_PWM_DEFAULT - 2*35)) // -35 degrees
#define TILT_MAX_PWM_DEFAULT ((unsigned char) (TILT_CENTER_PWM_DEFAULT + 2*25)) // +25 degrees

// This value defines how far the tilt servo will step
// each time while in search mode. This value was chosen
// to provide three evenly spaced stopping points (i.e.,
// 94, 144 and 194) for the tilt servo.
//#define TILT_SEARCH_STEP_SIZE_DEFAULT 50
#define TILT_SEARCH_STEP_SIZE_DEFAULT 0

// parameters for CMUcam2 with OV7620 camera module
#define IMAGE_WIDTH 159
#define IMAGE_HEIGHT 239

// Tracking_State values
#define STATE_SEARCHING 0
#define STATE_TARGET_IN_VIEW 1
#define STATE_PAN_ON_TARGET 2
#define STATE_TILT_ON_TARGET 4

// Get_Tracking_State() return values
#define SEARCHING 0
#define TARGET_IN_VIEW 1
#define CAMERA_ON_TARGET 2


// function prototypes
void Servo_Track(void);
unsigned char Get_Tracking_State(void);
void Set_Pan_Servo_Position(unsigned char);
void Set_Tilt_Servo_Position(unsigned char);

//LRT functions/variables
void cameraTiltOverride(void);
int getAdjustedPanTrackingError(void);
char getPanOffsetFromOI(void);
int get2xTiltAngle(void);
int getTiltError(void);
unsigned char GetRawTrackingState(void);

extern char gPanAsTilt;

#endif
