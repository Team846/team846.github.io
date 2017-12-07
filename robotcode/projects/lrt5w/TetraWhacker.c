#include "lrtConnections.h"
#include "tetraWhacker.h"

#include "lrtUtilities.h"
#include "lrtResultCodes.h"



static struct {
	char phase;	
	char side;
	char returnValue;

} gTetraWhacker;	//private variables for whackTetra

/*
 * whackTetraStart(side)
 * call once to initiate the tetrawhacker.
 * side is the side on which the target is located.
 * if on right, swing clockwise (CW) then counterclockwise(CCW)
 * opposite if on left.
 */


void whackTetraStart(char side)
{
	gTetraWhacker.side=side;
	gTetraWhacker.phase = 1;
	gTetraWhacker.returnValue = kResultRunning;
}


/* whackTetraRun()
 * called each time in autonomous mode
 * once initialized, calling whacktetraRun will operate the solenoid valves
 * to  strike the hanging tetra.
 */




enum { kRelayTime=(int)(250/26.2), kDelayTime=(int)(1500/26.2) };
char whackTetraRun(void)
{
	static unsigned char timer;

	if (gTetraWhacker.returnValue == kResultRunning)
		switch (gTetraWhacker.phase)
		{
			case 0:	//not running - do nothing
				gTetraWhacker.returnValue = kResultNotRunning;
				break;
	
			case 1: //activate relay
				gTetraWhacker.phase++;
				timer = kRelayTime;
	
				//activate relays to drive valve; swing CC or CCW depending on which side target is located.
				mTetraWhackCW=(gTetraWhacker.side==kRight);
				mTetraWhackCCW = !mTetraWhackCW;
				//fall through
			case 2:	//hold valve for short time.
				if (timer!=0)
				{
					timer--;
					break;
				}
				
				gTetraWhacker.phase++;
				timer=kDelayTime;
				mTetraWhackCW=mTetraWhackCCW=0;
				//fall through	
			
			case 3: //wait - 
				if (timer!=0)
				{
					timer--;
					break;
				}
				//set up next phase - wait
				gTetraWhacker.phase++;
				timer=kRelayTime;
				mTetraWhackCCW=(gTetraWhacker.side==kRight);
				mTetraWhackCW = !mTetraWhackCCW;
				//fall through	
			
			case 4: //wait - 
				if (timer!=0)
				{
					timer--;
					break;
				}
				gTetraWhacker.returnValue = kResultSuccess;
				//finished; set phase to 0
				gTetraWhacker.phase=0;
		} //End of switch
	return gTetraWhacker.returnValue;
}
