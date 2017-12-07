#ifndef DITHERED_BRAKE_DRIVE
#define DITHERED_BRAKE_DRIVE

#include "..\..\General.h"
#include "..\..\Sensors\DriveEncoders.h"
#include "..\..\Config\Config.h"
#include "..\..\Config\Configurable.h"

typedef struct
{
    float dutyCycle;
    float brakingDutyCycle;
} ESCCommand;

typedef struct
{
    ESCCommand leftCommand;
    ESCCommand rightCommand;
} DriveCommand;

class DitheredBrakeTrain : public Configurable
{
private:
    LRTEncoder& leftEncoder;
    LRTEncoder& rightEncoder;

//    float brakeGain;

public:
    DitheredBrakeTrain();
    ~DitheredBrakeTrain();

    virtual void Configure();
    DriveCommand Drive(float forwardInput, float turnInput);
    ESCCommand CalculateBrakeAndDutyCycle(float input, float speed);
};

#endif
