#include "VirtualLRTServo.h"

VirtualLRTServo::VirtualLRTServo(UINT32 channel)
    : enabled(true)
    , value(0)
{

}

VirtualLRTServo::~VirtualLRTServo()
{
    // nothing to do
}

void VirtualLRTServo::SetEnabled(bool enabled)
{
    this->enabled = enabled;
}

bool VirtualLRTServo::IsEnabled()
{
    return enabled;
}

void VirtualLRTServo::Set(float value)
{
    if(enabled)
        this->value = value;
}

float VirtualLRTServo::Get()
{
    return value;
}
