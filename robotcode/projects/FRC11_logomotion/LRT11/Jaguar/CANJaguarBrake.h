#ifndef CAN_JAGUAR_BRAKE_H_
#define CAN_JAGUAR_BRAKE_H_

#include "..\General.h"
#include "Brake.h"
#include "CANJaguar.h"
#include "ProxiedCANJaguar.h"
#include "..\Util\Util.h"

class CANJaguarBrake : public Brake
{
    ProxiedCANJaguar& jaguar;
    int cycleCount;
    int amount;

public:
    CANJaguarBrake(ProxiedCANJaguar& jaggie);

    // [0, 8]; 0 is no braking and 8 is max braking
    virtual void SetBrake(int brakeAmount);

    // does nothing if setpoint is non-zero
    // applies brake based on call to SetBrake
    virtual void ApplyBrakes();

    // braking is dependent on speed; higher speeds will result in
    // more dramatic braking. speed is not considered in this method
    virtual void SetBrakeMax()
    {
        SetBrake(8);
    }

    virtual void SetBrakeOff()
    {
        SetBrake(0);
    }
};

#endif /* CAN_JAGUAR_BRAKE_H_ */
