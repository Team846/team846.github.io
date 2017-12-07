#ifndef ESC_H_
#define ESC_H_

#include "ProxiedCANJaguar.h"
#include "CANJaguarBrake.h"
#include "WPILib.h"
#include "..\Config\Config.h"
#include "..\Config\Configurable.h"
#include "..\Util\AsynchronousPrinter.h"
#include "..\Sensors\DriveEncoders.h"
#include "..\Util\RunningSum.h"
#include <string>
using namespace std;

class Esc : public ProxiedCANJaguar, public CANJaguarBrake, public Configurable
{
public:
#ifdef VIRTUAL
    Esc(int channel, VirtualLRTEncoder& encoder, string name);
    Esc(int channelA, int channelB, VirtualLRTEncoder& encoder, string name);
#else
    Esc(int channel, LRTEncoder& encoder, string name);
    Esc(int channelA, int channelB, LRTEncoder& encoder, string name);
#endif
    ~Esc();
    virtual void Configure();
    void Stop();
private:
    virtual void Set(float speed);  //renaming set() to setDutyCycle()
public:
    void SetDutyCycle(float dutycycle);  //renaming set() to setSpeed()

    void ApplyBrakes();
    void SetBrake(int brakeAmount);

    void ResetCache();

private:
    bool hasPartner;
    Esc* partner;


    class CurrentLimiter
    {
    public:
        CurrentLimiter();
        float Limit(float speed, float robotSpeed);

    private:
        UINT32 timeExtended, timeBurst;
        UINT32 coolExtended, coolBurst;

        const static float kmaxContinous = 40.0 / 133;
        const static float kmaxExtended = 60.0 / 133;
        const static float kmaxBurst = 100.0 / 133;

        const static float kmaxExtendedPeriodSeconds = 2.0;
        const static float kmaxBurstPeriodSeconds = 0.2;

        const static float kminExtendedCooldown = 1.0;
        const static float kminBurstCooldown = 0.5;
    };

    CurrentLimiter currentLimiter;
#ifdef VIRTUAL
    VirtualLRTEncoder& encoder;
#else
    LRTEncoder& encoder;
#endif

    string name;

    float GetNormalizedSpeed();
    float pGain;
    int index;

    const static int kArraySize = 4;
    RunningSum stoppingIntegrator;
    float errorRunning;
    float errors[kArraySize];
};

#endif /* ESC_H_ */
