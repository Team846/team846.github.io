#ifndef VIRTUAL_LRT_SERVO_H_
#define VIRTUAL_LRT_SERVO_H_

#include "..\..\General.h"

class VirtualLRTServo
{
private:
    bool enabled;
    float value; // value of this servo

public:
    VirtualLRTServo(UINT32 channel);
    ~VirtualLRTServo();

    void SetEnabled(bool enabled);
    bool IsEnabled();

    void Set(float value);
    float Get();
};

#endif
