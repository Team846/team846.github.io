#ifndef DEBOUNCEDJOYSTICK_H_
#define DEBOUNCEDJOYSTICK_H_

#include "Joystick.h"

class DebouncedJoystick : public Joystick {
public:
	DebouncedJoystick::DebouncedJoystick(UINT32 port, int nBtns);
	~DebouncedJoystick();

	void Init();
	void Update();
	bool IsButtonJustPressed(int button);
	bool IsButtonJustReleased(int button);
	bool IsButtonDown(int button);
private:
	int m_nBtns;
	bool *m_wasPressed;
	bool *m_isPressed;
};

#endif
