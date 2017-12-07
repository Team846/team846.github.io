/*******************************************************************************
*
*	TITLE:		tracking.c 
*
*	VERSION:	0.2 (Beta)                           
*
*	DATE:		21-Feb-2006
*
*	AUTHOR:		R. Kevin Watson
*				kevinw@jpl.nasa.gov
*
*	COMMENTS:	This is the "streamlined" version of tracking.c
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
#include <stdio.h>
#include "ifi_default.h"
#include "ifi_aliases.h"
#include "camera.h"
#include "tracking.h"

//#include "turret.h"
#include "utilities.h"
#include "connections.h"
// This variable is used to signal whether or not the camera
// is pointed at the target. See Get_Tracking_State(), below.
unsigned char Tracking_State = STATE_SEARCHING;

/*******************************************************************************
*
*	FUNCTION:		Servo_Track()
*
*	PURPOSE:		This function reads data placed in the T_Packet_Data
*					structure by the Camera_Handler() function and if new
*					tracking data is available, attempts to keep the center
*					of the tracked object in the center of the camera's
*					image using two servos that drive a pan/tilt platform.
*					If the camera doesn't have the object within it's field 
*					of view, this function will execute a search algorithm 
*					in an attempt to find the object.		
*
*	CALLED FROM:	user_routines.c/Process_Data_From_Master_uP()
*
*	PARAMETERS:		None.
*
*	RETURNS:		Nothing.
*
*	COMMENTS:		This version of the tracking code uses a proportional
*					feedback controller to track the object.
*
*******************************************************************************/
void Servo_Track(void)
{
	static unsigned char Tracking_Initialized = 0;
	static unsigned int old_camera_t_packets = 0;
	static unsigned char new_search = 1;
	static unsigned char loop_count = 0;
	static unsigned char pan_servo_position;
	static unsigned char tilt_servo_position;
	int temp_pan_servo;
	int temp_tilt_servo;
	int servo_step;
	int pan_error;
	int tilt_error;

	// if needed, initialize the tracking code
	if(Tracking_Initialized == 0)
	{
		Tracking_Initialized = 1;

		// get center values for the servos
		pan_servo_position = PAN_CENTER_PWM_DEFAULT;
		tilt_servo_position = TILT_CENTER_PWM_DEFAULT;

		// command servos to center position
		Set_Pan_Servo_Position(pan_servo_position);
		Set_Tilt_Servo_Position(tilt_servo_position);
	}

	// Has a new camera t-packet arrived since we last checked?
	if(camera_t_packets != old_camera_t_packets)
	{
		old_camera_t_packets = camera_t_packets;

		// Reset the Tracking_State variable to indicate that
		// we're in the searching state. If it turns out the
		// target is in view, Tracking_State will be updated
		// to reflect this below.
		Tracking_State = SEARCHING;

		// Does the camera have a tracking solution? If so,
		// do we need to move the servos to keep the center
		// of the tracked object centered within the image?
		// If not, we need to drop down below to start or
		// continue a search.
		if(T_Packet_Data.confidence >= CONFIDENCE_THRESHOLD_DEFAULT)
		{
			// if we're tracking, reset the search
			// algorithm so that a new search pattern
			// will start should we lose tracking lock
			new_search = 1;

			// update Tracking_State to indicate that the target
			// is at least in view of the camera
			Tracking_State = TARGET_IN_VIEW;

			////////////////////////////////
			//                            //
			//	x-axis/pan tracking code  //
			//                            //
			////////////////////////////////

			// save the current pan servo PWM value into a local
			// integer variable so that we can detect and correct 
			// underflow and overflow conditions before we update 
			// the pan servo PWM value with a new value
			temp_pan_servo = (int)pan_servo_position;

			// calculate how many image pixels we're away from the
			// vertical center line.

//			pan_error = (int)T_Packet_Data.mx - PAN_TARGET_PIXEL_DEFAULT;
			pan_error = (int)getAdjustedPanTrackingError();

			// Are we too far to the left or right of the vertical 
			// center line? If so, calculate how far we should step
			// the pan servo to reduce the error.
			if(pan_error > PAN_ALLOWABLE_ERROR_DEFAULT)
			{
				// calculate how far we need to step the pan servo
				servo_step = pan_error / PAN_GAIN_DEFAULT;

				// Due to rounding error in the division calculation above,
				// the step may be calculated as zero, which will make it
				// impossible to converge on the target when pan_error is
				// smaller than PAN_GAIN_DEFAULT. To get around this problem,  
				// we just test for the zero case and set the step size to one. 
				if(servo_step == 0)
				{
					servo_step = 1;
				}
			}
			else if(pan_error < -1 * PAN_ALLOWABLE_ERROR_DEFAULT)
			{
				// calculate how far we need to step the pan servo
				servo_step = pan_error / PAN_GAIN_DEFAULT;


				// Due to rounding error in the division calculation above,
				// the step may be calculated as zero, which will make it
				// impossible to converge on the target when pan_error is
				// smaller than PAN_GAIN_DEFAULT. To get around this problem,  
				// we just test for the zero case and set the step size to one
				if(servo_step == 0)
				{
					servo_step = -1;
				}
			}
			else
			{
				// if we've fallen through to here, it means that we're
				// neither too far to the left or too far to the right
				// of the vertical center line of the image and don't 
				// need to move the servo
				servo_step = 0;

				// signal that the pan servo is on target
				Tracking_State += STATE_PAN_ON_TARGET;
			}

			// add the step to the current servo position, taking into
			// account the direction set by the user in tracking.h
			temp_pan_servo += (PAN_ROTATION_SIGN_DEFAULT * servo_step);

			// check the pan servo PWM value for under/overflow
			if(temp_pan_servo < PAN_MIN_PWM_DEFAULT)
			{
				temp_pan_servo = PAN_MIN_PWM_DEFAULT;
			}
			else if(temp_pan_servo > PAN_MAX_PWM_DEFAULT)
			{
				temp_pan_servo = PAN_MAX_PWM_DEFAULT;
			}

			pan_servo_position = (unsigned char)temp_pan_servo;

			// update pan servo PWM value
			Set_Pan_Servo_Position(pan_servo_position);


			/////////////////////////////////
			//                             //
			//	y-axis/tilt tracking code  //
			//                             //
			/////////////////////////////////

			// save the current tilt servo PWM value into a local
			// integer variable so that we can detect and correct 
			// underflow and overflow conditions before we update 
			// the tilt servo PWM value with a new value
			temp_tilt_servo = (int)tilt_servo_position;

			// calculate how many image pixels we're away from the
			// horizontal center line.
			tilt_error = (int)T_Packet_Data.my - TILT_TARGET_PIXEL_DEFAULT;

			// Are we too far above or below the horizontal center line?
			// If so, calculate how far we should step the tilt servo to 
			// reduce the error.
			if(tilt_error > TILT_ALLOWABLE_ERROR_DEFAULT)
			{
				// calculate how far we need to step the tilt servo
				servo_step = tilt_error / TILT_GAIN_DEFAULT;

				// Due to rounding error in the division calculation above,
				// the step may be calculated as zero, which will make it
				// impossible to converge on the target when tilt_error is
				// smaller than TILT_GAIN_DEFAULT. To get around this problem, 
				// we just test for the zero case and set the step size to one. 
				if(servo_step == 0)
				{
					servo_step = 1;
				}
			}
			else if (tilt_error < -1 * TILT_ALLOWABLE_ERROR_DEFAULT)
			{
				// calculate how far we need to step the tilt servo
				servo_step = tilt_error / TILT_GAIN_DEFAULT;

				// Due to rounding error in the division calculation above,
				// the step may be calculated as zero, which will make it
				// impossible to converge on the target when tilt_error is
				// smaller than TILT_GAIN_DEFAULT. To get around this problem, 
				// we just test for the zero case and set the step size to one. 
				if(servo_step == 0)
				{
					servo_step = -1;
				}
			}
			else
			{
				// if we've fallen through to here, it means that we're
				// neither too far below or to far above the horizontal
				// center line of the image and don't need to move the
				// servo
				servo_step = 0;

				// signal that the tilt servo is on target
				Tracking_State += STATE_TILT_ON_TARGET;
			}

			// add the step to the current servo position, taking into
			// account the direction set by the user in tracking.h
			temp_tilt_servo += (TILT_ROTATION_SIGN_DEFAULT * servo_step);

			// check the tilt PWM value for under/overflow
			if(temp_tilt_servo < TILT_MIN_PWM_DEFAULT)
			{
				temp_tilt_servo = TILT_MIN_PWM_DEFAULT;
			}
			else if(temp_tilt_servo > TILT_MAX_PWM_DEFAULT)
			{
				temp_tilt_servo = TILT_MAX_PWM_DEFAULT;
			}

			tilt_servo_position = (unsigned char)temp_tilt_servo;

			// update tilt servo PWM value
			Set_Tilt_Servo_Position(tilt_servo_position);
		}
		else // matching else to if(T_Packet_Data.confidence >= CONFIDENCE_THRESHOLD_DEFAULT) above
		{
			///////////////////
			//               //
			//  search code  //
			//               //
			///////////////////

			// To provide a delay for the camera to lock onto the
			// target between position changes, we only step the camera
			// to a new position every SEARCH_DELAY times while we're 
			// in search mode. SEARCH_DELAY_DEFAULT is #define'd in 
			// tracking.h
			loop_count++;

			if(loop_count > SEARCH_DELAY_DEFAULT)
			{
				// reset the loop counter
				loop_count = 0;

				// If we're starting a new search, initialize the pan
				// and tilt servos to the search starting point.
				// Otherwise, just continue the search pattern from
				// where we left off. The variable new_search is reset
				// to one each time the tracking code (above) executes.
				if(new_search == 1)
				{
					new_search = 0;
					temp_pan_servo = PAN_MIN_PWM_DEFAULT;
					temp_tilt_servo = TILT_CENTER_PWM_DEFAULT;
										
				}
				else
				{
					// calculate new servo position(s) based upon our
					// current servo position(s)
					temp_pan_servo = (int)pan_servo_position;
					temp_tilt_servo = (int)tilt_servo_position;
	
					// if the pan servo is at the end of its travel, 
					// send it back to the start and step the tilt
					// servo to its next position
					if(temp_pan_servo >= PAN_MAX_PWM_DEFAULT)
					{
						temp_pan_servo = PAN_MIN_PWM_DEFAULT;
		
						// if the tilt servo is at the end of its
						// travel, send it back to the start
						if(temp_tilt_servo >= TILT_MAX_PWM_DEFAULT)
						{
							temp_tilt_servo = TILT_MIN_PWM_DEFAULT;
						}
						else
						{
							// step the tilt servo to its next destination
							temp_tilt_servo += TILT_SEARCH_STEP_SIZE_DEFAULT;
			
							// make sure its new position isn't beyond
							// the maximum value set in tracking.h
							if(temp_tilt_servo >= TILT_MAX_PWM_DEFAULT)
							{
								temp_tilt_servo = TILT_MAX_PWM_DEFAULT;
							}
						}
					}
					else
					{
						// step the pan servo to its next destination
						temp_pan_servo += PAN_SEARCH_STEP_SIZE_DEFAULT;
		
						// make sure its new position isn't beyond
						// the maximum value set in tracking.h
						if(temp_pan_servo >= PAN_MAX_PWM_DEFAULT)
						{
							temp_pan_servo = PAN_MAX_PWM_DEFAULT;
						}
					}
				}

				pan_servo_position = (int)temp_pan_servo;
				tilt_servo_position = (int)temp_tilt_servo;

				// update the pan and tilt servo PWM value
				Set_Pan_Servo_Position((unsigned char)pan_servo_position);
				Set_Tilt_Servo_Position((unsigned char)tilt_servo_position);
			}
		}
	}
}

/*******************************************************************************
*
*	FUNCTION:		Get_Tracking_State()
*
*	PURPOSE:		This function can be used to determine if both the pan 
*					and tilt servos have the camera positioned such that the 
*					centroid (center) of the green light is located at the 
*					center of the camera's imager.
*
*	CALLED FROM:	User code.
*
*	PARAMETERS:		None.
*
*	RETURNS:		SEARCHING if in search mode.
*
*					TARGET_IN_VIEW if the target is in view of the camera
*					but not locked on.
*
*					CAMERA_ON_TARGET if the camera is locked onto the
*					target
*
*	COMMENTS:		The return values are defined in tracking.h.
*
*******************************************************************************/
unsigned char Get_Tracking_State(void)
{
	unsigned char return_value;

	if(Tracking_State == STATE_SEARCHING)
	{
		return_value = SEARCHING; 
	}
	else if(Tracking_State < STATE_TARGET_IN_VIEW + STATE_PAN_ON_TARGET + STATE_TILT_ON_TARGET)
	{
		return_value = TARGET_IN_VIEW;
	}
	else if(Tracking_State == STATE_TARGET_IN_VIEW + STATE_PAN_ON_TARGET + STATE_TILT_ON_TARGET)
	{
		return_value = CAMERA_ON_TARGET;
	}

	return(return_value);
}

/*******************************************************************************
*
*	FUNCTION:		Set_Pan_Servo_Position()
*
*	PURPOSE:		Commands the pan servo to a new position
*
*	CALLED FROM:	Servo_Track(), above.
*
*	PARAMETERS:		None.
*
*	RETURNS:		Nothing.
*
*	COMMENTS:
*
*******************************************************************************/
void Set_Pan_Servo_Position(unsigned char new_pan_position)
{
	PAN_SERVO = new_pan_position;
}

/*******************************************************************************
*
*	FUNCTION:		Set_Tilt_Servo_Position()
*
*	PURPOSE:		Commands the tilt servo to a new position
*
*	CALLED FROM:	Servo_Track(), above.
*
*	PARAMETERS:		None.
*
*	RETURNS:		Nothing.
*
*	COMMENTS:
*
*******************************************************************************/
void Set_Tilt_Servo_Position(unsigned char new_tilt_position)
{
	TILT_SERVO = new_tilt_position;
}



int getAdjustedPanTrackingError(void)
{
	int result;
	int trim = getPanOffsetFromOI() - 25;
	result = trim + (char) (T_Packet_Data.mx - PAN_TARGET_PIXEL_DEFAULT);
	result = LimitRange(result, -127,127);	//necessary?? [dg]
	return result;	
}



//********************************************************************************
char getPanOffsetFromOI(void)
{
	char trim = mPanOffset;
	//mPanOffset input is not correctly assigned [dg]
	calibrateInput((unsigned char *) &trim, 12);	//correct for low values -- should have non-zero arg if call is needed

	trim -= 127;
	trim /= 4;	//give range of +/- 15 [dg]
	
	
	if (gLoop.onSecond)
		printf("WARNING: GetPanOffsetFromOI:  trim=%d\r", (int) trim);
	return trim;
}
//********************************************************************************
