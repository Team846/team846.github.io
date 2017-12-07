#ifndef LRT_DIGITAL_OUTPUT_H_
#define LRT_DIGITAL_OUTPUT_H_

#include "WPILib.h"
#include "../Util/AsynchronousPrinter.h"

class LRTDigitalOutput : public DigitalOutput
{
private:
    static UINT32 delay;

public:
    LRTDigitalOutput(UINT32 channel)
        : DigitalOutput(channel)
    {

    }

    void Set(UINT32 value)
    {
        DigitalOutput::Set(value);
        Delay(delay); // TODO refine delay?
    }

    void Delay(UINT32 ticks)
    {
        while(ticks > 0)
            ticks--;
    }

    static void SetDelay(int delayTicks)
    {
        delay = delayTicks;
    }
};

#endif
