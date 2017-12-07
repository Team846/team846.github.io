#ifndef BRAKE_H_
#define BRAKE_H_

#include "..\General.h"

// used to switch from digital (PWM) to CAN braking -KV 3/25/11
class Brake
{
public:
    Brake() { }

    virtual ~Brake()
    {
    }

    virtual void SetBrake(int brakeAmount) = 0;
    virtual void SetBrakeMax() = 0;
    virtual void SetBrakeOff() = 0;
};

#endif /* BRAKE_H_ */
