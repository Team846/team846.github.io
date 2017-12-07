#ifndef VIRTUAL_LRT_ENCODER_H_
#define VIRTUAL_LRT_ENCODER_H_

#include "..\General.h"
#include "..\ActionData.h"
#include "..\Util\DutyCycleSubscriber.h"

class VirtualLRTEncoder : public DutyCycleSubscriber
{
private:
    // used to determine if low or high gear
    ActionData& action;

    double rate; // ticks/sec
    double ticks;

    // conversion from ft / s rate input to ticks / s
    // ft / s * in / ft * rev / in * ticks / rev = ticks / s
    // in ticks / s
    static double HIGH_GEAR_MAX_RATE;
    static double LOW_GEAR_MAX_RATE;

    enum {LOW_GEAR = 1 , HIGH_GEAR = 2};

    // used to synchronized rate and tick updates
    SEM_ID semaphore;

public:
    VirtualLRTEncoder(UINT8 sourceA, UINT8 sourceB);
    ~VirtualLRTEncoder();

    void SetDistancePerPulse(double distancePerPulse) {}
    void Start() {}

    INT32 Get();
    double GetRate();

    // called at 50 Hz
    virtual void Update(float dutyCycle);
};

#endif
