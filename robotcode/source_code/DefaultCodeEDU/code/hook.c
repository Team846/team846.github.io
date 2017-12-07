#include "hook.h"
#include "lrtConnections.h"



void ToggleHook(void)
{
	static char debounce;
	static char hook;
	#error


}



/********************************************************************************
* FUNCTION: 
*
* DESCRIPTION: 
*
********************************************************************************/

/*******************************************************************************/
//raises the flag on drivers command
//lowered on second push
//lowered on disable

#include "lrtConnections.h"
#include "lrtUtilities.h"		//not really the place to put these prototypes.

static struct {
	char up;	//state of flag; default down
	unsigned char debounceTimer;	//don't respond to multiple pushes
	unsigned char btnLastCycle:1;	//don't respond to multiple pushes
} gHook;


static char up = 1;
enum {kDown,kUp};

void ToggleHook(void)
{
	(mBtnHook==0) !=
if (gHook.releasedLastCycle && 
	
if (gHook.debounceTimer!=0) gHook.debounceTimer--;
	else //
		gHook.debounceTimer = 4;	//4*26.2ms
	gHook.up = kUp;
}

void hookPWM(void)
{
	if (hook.debounceTimer>0) hook.debounceTimer--;
	mPWMhook = hook.up ? 0:255;
}

		if (mBtnHook) gHookPosition=kHookUp;
		if (mBtnHookRelease) gHookPosition=kHookDown;
