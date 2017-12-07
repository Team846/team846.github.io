#ifndef LIFT_H_
#define LIFT_H_

#include "Component.h"
#include "..\Config\Configurable.h"


class Config;
class ProxiedCANJaguar;
class VirtualPot;

/*!
 * \brief the component that handles all io and abstraction of the lift.
 */ 
class Lift : public Component, public Configurable
{
private:
    Config& config;
    string configSection;
    ProxiedCANJaguar* liftEsc;
#ifdef VIRTUAL
    VirtualPot* liftPot;
#endif

    const static float inchesToTurns = 1.0 / 12.0;

    int timeoutCycles;
    int cycleCount;

    float minPosition;
    float maxPosition;

    void StartTimer();

    enum {MANUAL = 1, PRESET = 2} prevMode;
    float potDeadband;
    bool positionMode;

public:
    Lift();
    virtual ~Lift();

    virtual void Configure();
    void ConfigureManualMode();

    virtual void Output();
    virtual string GetName();
};

#endif
