//raises the flag on drivers command
//lowered on second push
//lowered on disable

#include "lrtConnections.h"
#include "lrtUtilities.h"		//not really the place to put these prototypes.
#include "hook.h"

static struct {
	char up;	//state of flag; default down
	unsigned char debounceTimer;	//don't respond to multiple pushes
} signalflag;

static struct {
	char up;	//state of flag; default down
	unsigned char debounceTimer;	//don't respond to multiple pushes
} signalflag;


void ToggleSignalFlag(void)
{
	if (signalflag.debounceTimer>0) return;
	signalflag.debounceTimer = 19;	//one half second
	signalflag.up = !signalflag.up;
}

void SignalFlagPWM(void)
{
	if (signalflag.debounceTimer>0) signalflag.debounceTimer--;
	mPWMsignalFlag = signalflag.up ? 0:255;
}


