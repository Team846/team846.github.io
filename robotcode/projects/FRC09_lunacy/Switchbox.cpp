#include "Switchbox.h"

Switchbox::Switchbox(DigitalInput *leftSwitch, DigitalInput *rightSwitch,
		DigitalInput *spinOrCornerSwitch) :
	m_leftSwitch(leftSwitch), m_rightSwitch(rightSwitch),
			m_spinOrCornerSwitch(spinOrCornerSwitch) {
}

AutonomousTask Switchbox::GetTask() const{
	// note: switches are LOW when ON,  HIGH when OFF
	bool left = !m_leftSwitch->Get();
	bool right = !m_rightSwitch->Get();
	bool spinOrCorner = !m_spinOrCornerSwitch->Get();
	
	if (left && !right) { //left
		if (spinOrCorner)
			return kGoCornerLeft;
		else
			return kCornerToCornerLeft;
	}
	else if (!left && right) { //right
		if (spinOrCorner)
			return kGoCornerRight;
		else
			return kCornerToCornerRight;
	}
	else if (left && right)
		return kGoStraightAndSpinLeft;
	else
		return kGoStraightAndSpinRight;
}
