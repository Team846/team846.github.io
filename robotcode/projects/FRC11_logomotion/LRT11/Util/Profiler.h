/**
 *  Usage:
 *
 *  {
 *      ProfiledSection pf( "NameOfActivity" );
 *      // do something time consuming...
 *  }
 */

#ifndef LRT_PROFILER_H_
#define LRT_PROFILER_H_


#include <map>
#include <string>
#include <sstream>
#include <iomanip>
#include "../General.h"
#include "AsynchronousPrinter.h"

class Profiler : public SensorBase
{
public:
    const static int reportPeriod = 50 * 5;
    const static int reportLimit = 10;

    virtual ~Profiler()
    {
    }
    static Profiler& GetInstance();

    void StartNewCycle();
    void Log(std::string name, double timeTaken);
    void ClearLogBuffer();

protected:
    Profiler();
    DISALLOW_COPY_AND_ASSIGN(Profiler);

private:
    static Profiler* instance;

    map<string, int> loggedCounts;
    map<string, double> loggedMins;
    map<string, double> loggedMaxs;
    map<string, double> loggedSums;

    int cycleIndex;
};

class ProfiledSection
{
public:
    ProfiledSection(std::string name) :
        name(name), start(Timer::GetFPGATimestamp())
    {
    }

    ~ProfiledSection()
    {
        double ms = (Timer::GetFPGATimestamp() - start) * 1000;
        Profiler::GetInstance().Log(name, ms);
    }

protected:
    string name;
    double start;
};

class ProfilerHelper
{
public:
    ProfilerHelper()
    {
    }

    void Start(string name)
    {
        name = name;
        start = Timer::GetFPGATimestamp();
    }

    void Finish()
    {
        double ms = (Timer::GetFPGATimestamp() - start) * 1000;
        Profiler::GetInstance().Log(name, ms);
    }

protected:
    string name;
    double start;
};

#endif
