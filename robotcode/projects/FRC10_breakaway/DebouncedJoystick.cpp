//Author: David Liu (2009)

#include "DebouncedJoystick.h"
#include <cstdio>
#include "AsynchronousPrinter.h"

DebouncedJoystick::DebouncedJoystick(unsigned port, int nBtns, int nAxes)
: Joystick(port)
, m_nBtns(nBtns)
, m_nAxes(nAxes)
{
	// N.B. indexes are 1-based
	m_wasPressed = new bool[m_nBtns+1];
	m_isPressed = new bool[m_nBtns+1];
	m_axisPrevValue = new float[m_nAxes+1];
	m_axisValue = new float[m_nAxes+1];
//	m_yOffset = 0.008;
	Init();
}
//float DebouncedJoystick::GetYAdjusted(JoystickHand hand )
//{
//	return GetY() + m_yOffset;
//}

DebouncedJoystick::~DebouncedJoystick() {
	delete [] m_wasPressed;
	delete [] m_isPressed;
	delete [] m_axisPrevValue;
	delete [] m_axisValue;
}

bool DebouncedJoystick::ButtonInBounds(int button) {
	if (button <= 0 || button > m_nBtns) {
		AsynchronousPrinter::Printf("[!]DebouncedJoystick: Button %d out of bounds!\n", button);
		return false;
	}
	return true;
}

bool DebouncedJoystick::AxisInBounds(int axis) {
	if (axis <= 0 || axis > m_nAxes) {
		AsynchronousPrinter::Printf("[!]DebouncedJoystick: Axis %d out of bounds!\n", axis);
		return false;
	}
	return true;
}

void DebouncedJoystick::Init() {
	for (int i=1; i<=m_nBtns; ++i) {
		m_wasPressed[i]=m_isPressed[i]=false;
	}
	for (int i=1; i<=m_nAxes; ++i) {
		m_axisPrevValue[i]=m_axisValue[i]=0;
	}
}
void DebouncedJoystick::Update() {
	for (int i=1; i<=m_nBtns; ++i) {
		m_wasPressed[i] = m_isPressed[i];
		m_isPressed[i] = GetRawButton(i);
	}
	for (int i=1; i<=m_nAxes; ++i) {
		m_axisPrevValue[i]=m_axisValue[i];
		m_axisValue[i] = GetRawAxis(i);
//		if (i == 5)
//			printf("%d %f\n", i, GetRawAxis(5));
	}
}
bool DebouncedJoystick::IsButtonJustPressed(int button) {
	if (!ButtonInBounds(button))
		return false;
	return m_isPressed[button] && !m_wasPressed[button];
}
bool DebouncedJoystick::IsButtonJustReleased(int button) {
	if (!ButtonInBounds(button))
		return false;
	return !m_isPressed[button] && m_wasPressed[button];
}
bool DebouncedJoystick::IsButtonDown(int button) {
	if (!ButtonInBounds(button))
		return false;
	return m_isPressed[button];
}
bool DebouncedJoystick::WasButtonDown(int button) {
	if (!ButtonInBounds(button))
		return false;
	return m_wasPressed[button];
}


bool DebouncedJoystick::IsHatSwitchJustPressed(int axis, int direction) {
	return !WasHatSwitchDown(axis, direction) && IsHatSwitchDown(axis, direction);
}
bool DebouncedJoystick::IsHatSwitchJustReleased(int axis, int direction) {
	return WasHatSwitchDown(axis, direction) && !IsHatSwitchDown(axis, direction);
}
bool DebouncedJoystick::WasHatSwitchDown(int axis, int direction) {
	if (!AxisInBounds(axis))
		return false;
//	assert(direction == 1 || direction == -1);
	return (m_axisPrevValue[axis] * direction > 0.5);
}
bool DebouncedJoystick::IsHatSwitchDown(int axis, int direction) {
	if (!AxisInBounds(axis))
		return false;
//	assert(direction == 1 || direction == -1);
	return (m_axisValue[axis] * direction > 0.5);
}
