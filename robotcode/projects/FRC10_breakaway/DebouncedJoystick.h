//Author: David Liu (2009)

#ifndef DEBOUNCEDJOYSTICK_H_
#define DEBOUNCEDJOYSTICK_H_

#include "Joystick.h"

class DebouncedJoystick : public Joystick {
public:
	DebouncedJoystick(unsigned port, int nBtns, int nAxes);
	~DebouncedJoystick();

	void Init();
	void Update();

	bool ButtonInBounds(int button);
	bool AxisInBounds(int axis);
	
//	float GetYAdjusted(JoystickHand hand = kRightHand);
	
	bool IsButtonJustPressed(int button);
	bool IsButtonJustReleased(int button);
	bool IsButtonDown(int button);
	bool WasButtonDown(int button);

	bool IsHatSwitchJustPressed(int axis, int direction);
	bool IsHatSwitchJustReleased(int axis, int direction);
	bool IsHatSwitchDown(int axis, int direction);
	bool WasHatSwitchDown(int axis, int direction);
private:
	int m_nBtns, m_nAxes;
	bool *m_wasPressed;
	bool *m_isPressed;
	float *m_axisPrevValue;
	float *m_axisValue;
	float m_yOffset;//move to a good place
};

#endif
