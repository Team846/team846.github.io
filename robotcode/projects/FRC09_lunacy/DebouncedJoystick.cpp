#include "DebouncedJoystick.h"
#include <cstdio>

DebouncedJoystick::DebouncedJoystick(UINT32 port, int nBtns)
: Joystick(port) {
	m_nBtns = nBtns+1; // avoid out-of-bounds
	m_wasPressed = new bool[m_nBtns];
	m_isPressed = new bool[m_nBtns];
	Init();
}

DebouncedJoystick::~DebouncedJoystick() {
	delete [] m_wasPressed;
	delete [] m_isPressed;
}

void DebouncedJoystick::Init() {
	for (int i=0; i<m_nBtns; ++i) {
		m_wasPressed[i]=m_isPressed[i]=false;
	}
}
void DebouncedJoystick::Update() {
	for (int i=0; i<m_nBtns; ++i) {
		m_wasPressed[i] = m_isPressed[i];
		m_isPressed[i] = GetRawButton(i);
	}
}
bool DebouncedJoystick::IsButtonJustPressed(int button) {
	if (button < 0 || button >= m_nBtns) {
		printf("[!]DebouncedJoystick: Button %d out of bounds!\n", button);
		return false;
	}
	return m_isPressed[button] && !m_wasPressed[button];
}
bool DebouncedJoystick::IsButtonJustReleased(int button) {
	if (button < 0 || button >= m_nBtns) {
		printf("[!]DebouncedJoystick: Button %d out of bounds!\n", button);
		return false;
	}
	return !m_isPressed[button] && m_wasPressed[button];
}
bool DebouncedJoystick::IsButtonDown(int button) {
	if (button < 0 || button >= m_nBtns) {
		printf("[!]DebouncedJoystick: Button %d out of bounds!\n", button);
		return false;
	}
	return m_isPressed[button];
}
