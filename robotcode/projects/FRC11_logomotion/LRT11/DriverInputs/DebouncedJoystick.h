#ifndef DEBOUNCED_JOYSTICK_H_
#define DEBOUNCED_JOYSTICK_H_

#include "Joystick.h"

class DebouncedJoystick : public Joystick
{
public:
    DebouncedJoystick(UINT32 port, int nBtns, int nAxes);
    ~DebouncedJoystick();

    void Init();
    void Update();

    bool ButtonInBounds(int button);
    bool AxisInBounds(int axis);

    bool IsButtonJustPressed(int button);
    bool IsButtonJustReleased(int button);
    bool IsButtonDown(int button);
    bool WasButtonDown(int button);

    bool IsHatSwitchJustPressed(int axis, int direction);
    bool IsHatSwitchJustReleased(int axis, int direction);
    bool IsHatSwitchDown(int axis, int direction);
    bool WasHatSwitchDown(int axis, int direction);

    float GetRawAxisDelta(int axis);

private:
    int nBtns, nAxes;
    bool* wasPressed;
    bool* isPressed;
    float* axisPrevValue;
    float* axisValue;
};

#endif
