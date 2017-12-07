#include "utilities.h"

float drive_comp_filter(int x0)	//tested w/debugger against Excel spreadsheet[dg]
{
	//first order IIR
	const float a1=1.9042, a2=-1.6790, b1 = 0.1137;
	static int x1=0;
	static float y1=0;
	y1 = a1*x0 + a2*x1 - b1*y1;
	x1 = x0;
	return y1;
}


/* Slew_limit() caps the rate at which the applied pwm value can increase.
 * There is no limit on how fast the pwm value can decrease, except if it changes sign.
 */
int slew_limit(int fwd)
{
	enum { slewLimit=128/6};	//max rate of increase on the fwd
	static int oldFwd;

	//cases: fwd is increasing past oldFwd ->apply slew limit if rate exceeds limit.
	//fwd is decreasing toward zero from oldFwd ->no change
	//fwd is going opposite polarity ->set to zero
	//fwd is zero, oldFwd is anything -> no change; fwd remains zero
	if (fwd > 0)
	{
		if (oldFwd <0) //opposite signs
			fwd=0;
		else if (fwd > oldFwd + slewLimit)
			fwd = oldFwd + slewLimit;
	}
	else if (fwd < 0)
	{
		if (oldFwd >0) //opposite signs
			fwd=0;
		else if (fwd < oldFwd - slewLimit)
			fwd = oldFwd - slewLimit;
	}
	return oldFwd = fwd;
}
