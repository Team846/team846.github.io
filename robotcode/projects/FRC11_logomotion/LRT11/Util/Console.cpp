#include "Console.h"

Console* Console::instance = NULL;

Console::Console() :
    cycleNum(0)
{
    printf("Constructed Console\n");
}

Console& Console::GetInstance()
{
    if(instance == NULL)
        instance = new Console();
    return *instance;
}

int Console::GetCycleCount()
{
    return cycleNum;
}

void Console::UpdateCycleCount()
{
    cycleNum++;
}

void Console::PrintEverySecond(const char* format, ...)
{
    if(cycleNum % kCyclesPerSecond == 0)
    {
        // 200 characters should be large enough to accommodate all prints
        char buffer[200];

        va_list args;
        va_start(args, format);

        vsprintf(buffer, format, args);   // format string into buffer
        AsynchronousPrinter::Printf(buffer);
        va_end(args);
    }
}

void Console::PrintEveryHalfSecond(const char* format, ...)
{
    if(cycleNum % (kCyclesPerSecond / 2) == 0)
    {
        // 200 characters should be large enough to accommodate all prints
        char buffer[200];

        va_list args;
        va_start(args, format);

        vsprintf(buffer, format, args);   // format string into buffer
        AsynchronousPrinter::Printf(buffer);
        va_end(args);
    }
}

// hertz may be < 1
void Console::PrintMultipleTimesPerSecond(float hertz, const char* format, ...)
{
    if(cycleNum % int(kCyclesPerSecond / hertz) == 0)
    {
        // 200 characters should be large enough to accommodate all prints
        char buffer[200];

        va_list args;
        va_start(args, format);

        vsprintf(buffer, format, args);   // format string into buffer
        AsynchronousPrinter::Printf(buffer);
        va_end(args);
    }
}
