#include "VirtualLRTEncoder.h"
#include "DriveEncoders.h"
#include "..\ActionData\ShifterAction.h"


#define FPS_TO_TPS(fps) ((fps) * 12.0 * 1 / (4.0 * DriveEncoders::PI)* DriveEncoders::TICKS_PER_FULL_TURN)

double VirtualLRTEncoder::HIGH_GEAR_MAX_RATE = FPS_TO_TPS(16.3);
double VirtualLRTEncoder::LOW_GEAR_MAX_RATE = FPS_TO_TPS(6.4);

VirtualLRTEncoder::VirtualLRTEncoder(UINT8 sourceA, UINT8 sourceB)
    : action(ActionData::GetInstance())
    , rate(0)
    , ticks(0)
    , semaphore(semMCreate(SEM_Q_PRIORITY | SEM_DELETE_SAFE
            | SEM_INVERSION_SAFE))
{
    // working with left encoder, so subscribe to left CAN drive
    if(sourceA == RobotConfig::DIGITAL_IO::ENCODER_LEFT_A)
#ifdef LRT_ROBOT_2011
        Subscribe(RobotConfig::CAN::DRIVE_LEFT_A);
#else
        Subscribe(RobotConfig::CAN::DRIVE_LEFT);
#endif
    // working with right encoder, so subscribe to right CAN drive
    else if(sourceA == RobotConfig::DIGITAL_IO::ENCODER_RIGHT_A)
#ifdef LRT_ROBOT_2011
        Subscribe(RobotConfig::CAN::DRIVE_RIGHT_A);
#else
        Subscribe(RobotConfig::CAN::DRIVE_RIGHT);
#endif
}

VirtualLRTEncoder::~VirtualLRTEncoder()
{
    // nothing
}

INT32 VirtualLRTEncoder::Get()
{
    return (INT32) ticks;
}

double VirtualLRTEncoder::GetRate()
{
    return rate;
}

// called at 50 Hz
void VirtualLRTEncoder::Update(float dutyCycle)
{
    double maxRate = 0.0;

    switch(action.shifter->gear)
    {
    case LOW_GEAR:
        maxRate = LOW_GEAR_MAX_RATE;
        break;

    case HIGH_GEAR:
        maxRate = HIGH_GEAR_MAX_RATE;
        break;
    }

    double tempRate, tempTicks;

    // use dutyCycle to determine rate and add to the tick count
    tempRate = maxRate * dutyCycle;
    tempTicks = tempRate * 1.0 / 50.0; // ticks / s * s = ticks; s = period = 1 / 50 Hz

    {
        Synchronized s(semaphore);

        rate = tempRate;
        ticks += tempTicks;
    }
}
