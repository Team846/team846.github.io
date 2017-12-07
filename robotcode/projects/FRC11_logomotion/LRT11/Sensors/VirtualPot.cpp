#include "VirtualPot.h"
#include "../Util/AsynchronousPrinter.h"

VirtualPot::VirtualPot(UINT32 channel, int potTurns, float ftPerTurn,
        float motorMaxSpeedFps, float defaultPosition)
    : turns(potTurns)
    , distancePerTurn(ftPerTurn)
    , maxSpeed(motorMaxSpeedFps)
{
    // if default position is out of range, assume the half-way point
    position = defaultPosition;
    if(position < 0 || position > turns)
        position = turns / 2.0;

    // if tracking the arm pot, subscribe to the CAN arm
    if(channel == RobotConfig::ANALOG::POT_ARM)
        Subscribe(RobotConfig::CAN::ARM_);
    // all other pots are connected to the jaguars directly,
    // so the pot channel and CAN id are the same
    else
        Subscribe(channel);
}

VirtualPot::~VirtualPot()
{

}

float VirtualPot::GetPosition()
{
    return position;
}

INT32 VirtualPot::GetAverageValue()
{
    // range of [0,1023]
    return (INT32)(1023 * position / turns);
}

// called at 50 Hz
void VirtualPot::Update(float dutyCycle)
{
    // use dutyCycle to determine rate and add to the turn count
    double rate = maxSpeed * dutyCycle;
    double distTraveled = rate * 1.0 / 50.0; // ticks / s * s = ticks; s = period = 1 / 50 Hz

//    AsynchronousPrinter::Printf("Dist traveled: %.2f, Rate: %.2f, DPT: %.2f\n", distTraveled, rate, distancePerTurn);
    position += distTraveled / distancePerTurn;
}
