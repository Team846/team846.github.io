#include "DebouncedJoystick.h"
#include <cstdio>
#include "..\Util\AsynchronousPrinter.h"

DebouncedJoystick::DebouncedJoystick(UINT32 port, int nBtns, int nAxes) :
    Joystick(port)
    , nBtns(nBtns)
    , nAxes(nAxes)
{
    // N.B. indexes are 1-based
    wasPressed = new bool[nBtns + 1];
    isPressed = new bool[nBtns + 1];
    axisPrevValue = new float[nAxes + 1];
    axisValue = new float[nAxes + 1];

    Init();
}

DebouncedJoystick::~DebouncedJoystick()
{
    delete wasPressed;
    delete isPressed;
    delete axisPrevValue;
    delete axisValue;
}

bool DebouncedJoystick::ButtonInBounds(int button)
{
    if(button <= 0 || button > nBtns)
    {
        AsynchronousPrinter::Printf(
            "[!]DebouncedJoystick: Button %d out of bounds!\n", button);
        return false;
    }
    return true;
}

bool DebouncedJoystick::AxisInBounds(int axis)
{
    if(axis <= 0 || axis > nAxes)
    {
        AsynchronousPrinter::Printf(
            "[!]DebouncedJoystick: Axis %d out of bounds!\n", axis);
        return false;
    }
    return true;
}

void DebouncedJoystick::Init()
{
    for(int i = 1; i <= nBtns; ++i)
    {
        wasPressed[i] = isPressed[i] = false;
    }

    for(int i = 1; i <= nAxes; ++i)
    {
        axisPrevValue[i] = axisValue[i] = 0;
    }
}
void DebouncedJoystick::Update()
{
    for(int i = 1; i <= nBtns; ++i)
    {
        wasPressed[i] = isPressed[i];
        isPressed[i] = GetRawButton(i);
    }

    for(int i = 1; i <= nAxes; ++i)
    {
        axisPrevValue[i] = axisValue[i];
        axisValue[i] = GetRawAxis(i);
    }
}

float DebouncedJoystick::GetRawAxisDelta(int axis)
{
    // return positive when stick is pushed forward
    return axisPrevValue[axis] - axisValue[axis];
}

bool DebouncedJoystick::IsButtonJustPressed(int button)
{
    if(!ButtonInBounds(button))
        return false;
    return isPressed[button] && !wasPressed[button];
}

bool DebouncedJoystick::IsButtonJustReleased(int button)
{
    if(!ButtonInBounds(button))
        return false;
    return !isPressed[button] && wasPressed[button];
}

bool DebouncedJoystick::IsButtonDown(int button)
{
    if(!ButtonInBounds(button))
        return false;
    return isPressed[button];
}

bool DebouncedJoystick::WasButtonDown(int button)
{
    if(!ButtonInBounds(button))
        return false;
    return wasPressed[button];
}

bool DebouncedJoystick::IsHatSwitchJustPressed(int axis, int direction)
{
    return !WasHatSwitchDown(axis, direction) && IsHatSwitchDown(axis,
            direction);
}

bool DebouncedJoystick::IsHatSwitchJustReleased(int axis, int direction)
{
    return WasHatSwitchDown(axis, direction) && !IsHatSwitchDown(axis,
            direction);
}

bool DebouncedJoystick::WasHatSwitchDown(int axis, int direction)
{
    if(!AxisInBounds(axis))
        return false;
    return (axisPrevValue[axis] * direction > 0.5);
}

bool DebouncedJoystick::IsHatSwitchDown(int axis, int direction)
{
    if(!AxisInBounds(axis))
        return false;
    return (axisValue[axis] * direction > 0.5);
}
