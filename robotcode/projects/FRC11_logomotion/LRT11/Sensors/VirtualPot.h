#ifndef VIRTUAL_POT_H_
#define VIRTUAL_POT_H_

#include "..\General.h"
#include "..\Util\DutyCycleSubscriber.h"
#include "..\Config\RobotConfig.h"

class VirtualPot : public DutyCycleSubscriber
{
private:
    float turns;
    float distancePerTurn; // ft
    float maxSpeed; // max speed of motor in ft / s, taking into account the gearbox
    float position;

public:
    VirtualPot(UINT32 channel, int potTurns, float ftPerTurn,
            float motorMaxSpeedFps, float defaultPosition = -1);
    ~VirtualPot();

    // in the range [0.0,turns]
    float GetPosition();

    // in the range[0, 1023]
    INT32 GetAverageValue();

    // called at 50 Hz
    void Update(float dutyCycle);
};

#endif
