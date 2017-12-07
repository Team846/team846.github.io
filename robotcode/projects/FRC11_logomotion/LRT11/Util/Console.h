#ifndef LRTCONSOLE_H_
#define LRTCONSOLE_H_

#include "../General.h"
#include "AsynchronousPrinter.h"

class Console
{
public:
    static Console& GetInstance();
    virtual void UpdateCycleCount();

    int GetCycleCount();
    void PrintEverySecond(const char* format, ...);
    void PrintEveryHalfSecond(const char* format, ...);
    void PrintMultipleTimesPerSecond(float hertz, const char* format, ...);

private:
    Console();
    DISALLOW_COPY_AND_ASSIGN(Console);

    static Console* instance;
    volatile int cycleNum;

    const static int kCyclesPerSecond = 50;
};
#endif /* LRTCONSOLE_H_*/
