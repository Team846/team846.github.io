#ifndef SWITCHBOX_H_
#define SWITCHBOX_H_

#include "DigitalInput.h"

enum AutonomousTask {
	kGoStraightAndSpinLeft, kGoStraightAndSpinRight,
	kGoCornerLeft, kGoCornerRight,
	kGoAllLeft, kGoAllRight,
	kLoopTheLoopLeft, kLoopTheLoopRight, //go fwd, loop the loop, go fwd
	kCornerToCornerLeft, kCornerToCornerRight //turn 45, go fwd (until you get to the next corner, turn 45
};

class Switchbox {
public:
	
	Switchbox(DigitalInput *leftSwitch, DigitalInput *rightSwitch, DigitalInput *spinOrCornerSwitch);
	
	AutonomousTask GetTask() const;
	
private:
	DigitalInput *m_leftSwitch;
	DigitalInput *m_rightSwitch;
	DigitalInput *m_spinOrCornerSwitch;
};

#endif /*SWITCHBOX_H_ */
