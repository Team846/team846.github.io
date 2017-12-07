//This header is included in "lrtconnections.h"
//It overrides #defined variables on the OI & FRC board to
//allow testing on the prior year's robot.
//It is included only if _04Robot is defined.


#ifndef __2004Robot_h
#define __2004Robot_h

#ifndef _04Robot
#error This file should only be included if _04Robot is #defined.
#endif //_04Robot

extern char _NullPWM;	// a global to write to.
//These definition are for testing code on 2004 robot

#undef mPWMcimLeft
#undef mPWMfpriceLeft
#undef mPWMcimRight
#undef mPWMfpriceRight
#undef mPWMshoulder
#undef mPWMforearm
#undef mPWMsignalFlag

#define mPWMcimLeft pwm01
#define mPWMfpriceLeft _NullPWM
#define mPWMcimRight pwm02
#define mPWMfpriceRight _NullPWM
#define mPWMshoulder pwm03
#define mPWMforearm pwm04
#define mPWMsignalFlag _NullPWM


#undef mGearShiftHighRelay
#undef mGearShiftLowRelay
#undef mCompressorRelay	//on separate power circuit
#undef mHookRelay	//0 must reflect disabled position (hook up)
#undef mTetraWhackCW
#undef mTetraWhackCCW

#define mGearShiftHighRelay _NullPWM
#define mGearShiftLowRelay _NullPWM
#define mCompressorRelay _NullPWM
#define mHookRelay _NullPWM
#define mTetraWhackCW _NullPWM
#define mTetraWhackCCW _NullPWM
#endif	//__2004Robot_h
