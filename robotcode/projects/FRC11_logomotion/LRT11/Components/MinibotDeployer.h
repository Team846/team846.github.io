#ifndef MINIBOT_DEPLOYER_H
#define MINIBOT_DEPLOYER_H

#include "Component.h"
#include "..\Config\Configurable.h"

class Config;
class ProxiedCANJaguar;
class VirtualLRTServo;
class LRTServo;

class MinibotDeployer : public Component, public Configurable
{
public:
    MinibotDeployer();
    virtual ~MinibotDeployer();

    virtual void Output();
    virtual void Configure();
    virtual string GetName();

private:
    Config& config;
    ProxiedCANJaguar* deployerEsc;

    float lockedServoValue;
    float releasedServoValue;

    int currentThreshold;

#ifdef VIRTUAL
    VirtualLRTServo* alignerServo;
#else
    LRTServo* alignerServo;
#endif
};

#endif
