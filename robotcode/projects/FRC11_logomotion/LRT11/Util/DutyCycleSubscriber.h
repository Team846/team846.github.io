#ifndef DUTY_CYCLE_SUBSCRIBER_H_
#define DUTY_CYCLE_SUBSCRIBER_H_

#include "..\General.h"

class DutyCycleSubscriber
{
public:
    DutyCycleSubscriber();
    virtual ~DutyCycleSubscriber();

    void Subscribe(int channel);
    virtual void Update(float dutyCycle) = 0;
};

#endif
