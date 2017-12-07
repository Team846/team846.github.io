#ifndef ROLLER_H_
#define ROLLER_H_

#include "Component.h"
#include "..\Config\Configurable.h"

class ProxiedCANJaguar;

class Roller : public Component, public Configurable
{
private:
    ProxiedCANJaguar* topRoller;
    ProxiedCANJaguar* bottomRoller;

    string configSection;

    float dutyCycleSucking;

    float dutyCycleSpittingTop;
    float dutyCycleSpittingBottom;

    float dutyCycleRotatingIn;
    float dutyCycleRotatingOut;

    int ignoreCycles;
    bool detected;

    void RollInward();
    void RollOutward();
    void RollOpposite(bool rotateUpward);
    void Stop();

public:
    Roller();
    virtual ~Roller();

    virtual void Output();
    virtual void Configure();
    virtual string GetName();
};

#endif
