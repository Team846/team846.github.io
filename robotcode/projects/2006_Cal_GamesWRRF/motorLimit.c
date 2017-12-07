/*******************************************************************************
*	TITLE:		motorLimit.c 
******************************************************************************/

#include "ifi_aliases.h"
#include "ifi_default.h"
#include "ifi_utilities.h"
#include "connections.h"
#include "MotorDrive.h"
#include <stdio.h>


//********************************************************************************
/* 
Computes a moving average speed based on the input.
*/
//*******************************************************************************

#define kDriveLimit	30
char motorLimitLeft(char driveIn)
{
	static int speed=0;

	char out = driveIn;
	if (speed>0)
	{
		if (driveIn < -kDriveLimit) out = -kDriveLimit;	//Limit power
	}
	else	//going in reverse
	{
		if(driveIn > kDriveLimit) out = kDriveLimit;	//Limit power
	}
	
	speed = speed*15/16 + out;	//compute moving average speed

	return out;		
}
