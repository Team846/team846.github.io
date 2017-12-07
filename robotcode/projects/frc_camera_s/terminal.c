/*******************************************************************************
*
*	TITLE:		terminal.c 
*
*	VERSION:	0.1 (Beta)                           
*
*	DATE:		25-Nov-2005
*
*	AUTHOR:		R. Kevin Watson
*				kevinw@jpl.nasa.gov
*
*	COMMENTS:	You are free to use this source code for any non-commercial
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
*	25-Nov-2005  0.1  RKW - Original code.
*
*******************************************************************************/
#include <stdio.h>
#include "ifi_aliases.h"
#include "ifi_default.h"
#include "camera.h"
#include "tracking.h"

/*******************************************************************************
*
*	FUNCTION:		Tracking_Info_Terminal()
*
*	PURPOSE:		This function is designed to send tracking information
*					to a terminal attached to your robot controller's 
*					programming port.
*
*	CALLED FROM:	user_routines.c/Process_Data_From_Master_uP()
*
*	PARAMETERS:		none
*
*	RETURNS:		nothing
*
*	COMMENTS:		This code assumes that the terminal serial port is 
*					properly set in camera.h
*
*******************************************************************************/
void Tracking_Info_Terminal(void)
{
	static unsigned char count = 0;
	static unsigned int old_camera_t_packets = 0;

	// has a new camera tracking packet arrived since we last checked?
	if(camera_t_packets != old_camera_t_packets)
	{
		count++;

		// only show data on every five t-packets
		if(count >= 5)
		{
			// reset the counter
			count = 0;

			// does the camera have a tracking solution?
			if(T_Packet_Data.my == 0)
			{
				printf("Searching...\r\n");
			}
			else
			{
				printf("\r\n");

				// pan angle = ((current pan PWM) - (pan center PWM)) * degrees/pan PWM step
				printf(" Pan Angle (degrees) = %d\r\n", (((int)PAN_SERVO - 124) * 65)/124);

				// tilt angle = ((current tilt PWM) - (tilt center PWM)) * degrees/tilt PWM step
				printf("Tilt Angle (degrees) = %d\r\n", (((int)TILT_SERVO - 144) * 25)/50);

				printf(" Pan Error (Pixels)  = %d\r\n", (int)T_Packet_Data.mx - PAN_TARGET_PIXEL_DEFAULT);
				printf("Tilt Error (Pixels)  = %d\r\n", (int)T_Packet_Data.my - TILT_TARGET_PIXEL_DEFAULT);
				printf(" Blob Size (Pixels)  = %u\r\n", (unsigned int)T_Packet_Data.pixels);
				printf("Confidence (Pixels)  = %u\r\n", (unsigned int)T_Packet_Data.confidence);
			}
		}
	}
}
