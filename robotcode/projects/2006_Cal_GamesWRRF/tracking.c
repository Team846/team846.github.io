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
#include "user_routines.h"

// This variable is used to signal whether or not the camera
// is pointed at the target. See Get_Tracking_State(), below.
unsigned char Tracking_State = STATE_SEARCHING;

int cameraTiltError = 0;
char gPanAsTilt = 0;
//********************************************************************************
void Servo_Track(void)
{
	static unsigned char Tracking_Initialized = 0;
	static unsigned int old_camera_t_packets = 0;
	static unsigned char new_search = 1;
	static unsigned char loop_count = 0;
		
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
			
			cameraTiltError = (int)T_Packet_Data.my - TILT_TARGET_PIXEL_DEFAULT;
		}
	}
	cameraTiltOverride();
}

//*******************************************************************************

unsigned char GetRawTrackingState(void)	//added by D.Giandomenico
{
	return Tracking_State;		
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




//********************************************************************************
void cameraTiltOverride(void)
{
	if (gPanAsTilt)
	{
		int position = (int) mPanOffset - 127;
		position = mDivideBy4(position);
		
		position += TILT_STATIC_DEFAULT;
		mLimitRange(position, 0, 255);
	
		TILT_SERVO = (unsigned char) position;
		
		if (gLoop.onSecond && gCameraPrinting)
			printf("Pan Offset = %3d ; Tilt Servo = %3d\r", (int) mPanOffset, (int) position);
	}
	else
	{
		TILT_SERVO = TILT_STATIC_DEFAULT;
	}
}

//********************************************************************************
int getAdjustedPanTrackingError(void)
{
	int result;
	//if (Tracking_State == TARGET_IN_VIEW)
	//	return 0;
	//int trim = getPanOffsetFromOI() - 24;
	int trim = getPanOffsetFromOI() - 12;		
	result = trim + (char) (T_Packet_Data.mx - PAN_TARGET_PIXEL_DEFAULT);
	result = LimitRange(result, -127,127);	//necessary?? [dg]
	return result;	
}



//********************************************************************************
char getPanOffsetFromOI(void)
{
	if (!gPanAsTilt)
	{
		char trim = mPanOffset;
		//mPanOffset input is not correctly assigned [dg]
		calibrateInput((unsigned char *) &trim, 12);	//correct for low values -- should have non-zero arg if call is needed
	
		trim -= 127;
		trim /= 4;	//give range of +/- 15 [dg]
		
		
		if (gLoop.onSecondB)
			printf("WARNING: GetPanOffsetFromOI:  trim=%d\r", (int) trim);
		return trim;
	}
	else
		return 0;
}
//********************************************************************************

//returns twice the tilt angle
int get2xTiltAngle(void)	//D.Giandomenico (where did this come from? - we should get data direct from tracking)
{
	int tiltAngle2x = (((int)TILT_SERVO - TILT_CENTER_PWM_DEFAULT));
	return tiltAngle2x;	
}
//********************************************************************************

//returns the tilt error
int getTiltError(void)	//S.Giandomenico (where did this come from? - we should get data direct from tracking)
{
	return cameraTiltError;	
}
