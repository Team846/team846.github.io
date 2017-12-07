#ifndef ARM_H_
#define ARM_H_

#include "Component.h"
#include "..\Config\Configurable.h"
using namespace std;

class ProxiedCANJaguar;
class Config;
class VirtualPot;

/*!
 * \brief the component that handles the arm. It reads the sensor values and sends the motor commands based on inputs and commands. 
 */
class Arm : public Component, public Configurable
{
private:
    Config& config;
    string configSection;
    ProxiedCANJaguar* armEsc;
#ifdef VIRTUAL
    VirtualPot* armPot;
#else
    AnalogChannel* armPot;
#endif

    int oldState;

    float maxPosition, minPosition, midPosition, midPositionDeadband;
    float maxPowerUp, powerRetainUp, powerDown, midPGain;
    float midPowerUp, midPowerDown;
    float pGainDown, pGainUp, pGainMid;

    int timeoutCycles;
    int cycleCount;

    enum {BOTTOM = 1, TOP = 2};
    bool presetMode;

    int pulseCount;
    int hysteresis;

    const static float ARM_UP_THRESHOLD = 10;

public:
    Arm();
    virtual ~Arm();

    virtual void Configure();
    virtual void Output();

    virtual string GetName();
};

#endif
