#include "LRTServo.h"
#include "../../Util/AsynchronousPrinter.h"
#include "../../Util/Util.h"

LRTServo::LRTServo(UINT32 channel, char* name)
    : Servo(channel)
    , enabled(true)
    , previous_value_(999.0) //an out of range value
    , name_(name)
{
    if(!name_) name_ = "servo";
    printf("Created %s on channel %d\n", name_, channel);
}

LRTServo::~LRTServo()
{
    // nothing to do
}

void LRTServo::SetEnabled(bool enabled)
{
    if(this->enabled != enabled)  //state change; TODO delete this.
        printf("Servo: %d\n", enabled);

    this->enabled = enabled;
    if(!enabled)
        this->SetOffline();
}

bool LRTServo::IsEnabled()
{
    return enabled;
}

void LRTServo::Set(float value)
{
    if(enabled)
    {
        if(previous_value_ != value)
            AsynchronousPrinter::Printf("%s set: %4f\n", name_, previous_value_ = value);

        Servo::Set(value);
    }
}

void LRTServo::SetMicroseconds(int ms) 
{
    if(enabled)
    {
    	// TODO: CHANGE CONSTANTS TO CONFIG
    	const int MIN_VAL = 727;
    	const int MAX_VAL = 2252;
    	ms = Util::Clamp(ms, MIN_VAL, MAX_VAL);
    	float val = (float)(ms - MIN_VAL) / (MAX_VAL - MIN_VAL);
        Servo::Set(val);
    }
}

void LRTServo::SetAngle(float angle)
{
    if(enabled)
        Servo::SetAngle(angle);
}
