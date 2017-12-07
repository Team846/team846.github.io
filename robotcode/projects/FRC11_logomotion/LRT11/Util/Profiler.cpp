#include "Profiler.h"

Profiler* Profiler::instance = NULL;

Profiler& Profiler::GetInstance()
{
    if(instance == NULL)
        instance = new Profiler();
    return *instance;
}

Profiler::Profiler() :
    cycleIndex(0)
{
    AddToSingletonList();
}

template <class PairT> struct SortBySecondValue
{
    bool operator()(const PairT& lhs, const PairT& rhs) const
    {
        return lhs.second > rhs.second;
    }
};

void Profiler::StartNewCycle()
{
    ++cycleIndex;
    return;

    if(cycleIndex >= reportPeriod)
    {
        double reportStart = Timer::GetFPGATimestamp();

        AsynchronousPrinter::Printf("----------------------\n");
        AsynchronousPrinter::Printf("PROFILER (%d cycles)\n", reportPeriod);

        typedef map<string, double>::value_type paired;
        typedef set< paired , SortBySecondValue<paired> > SetSortedBySecond;
        SetSortedBySecond vals;

        for(map<string, double>::iterator it = loggedMaxs.begin(); it
                != loggedMaxs.end(); ++it)
        {
            // vals.insert( paired(it->first, it->second/loggedCounts[it->first]) ); // to sort by means
            vals.insert(paired(it->first, it->second));
        }

        int i = 0;
        for(SetSortedBySecond::iterator it = vals.begin(); it != vals.end(); ++it)
        {
            double min = loggedMins[it->first];
            double max = loggedMaxs[it->first];
            int count = loggedCounts[it->first];
            double mean = loggedSums[it->first] / count;


            AsynchronousPrinter::Printf("| %-30s ~%.2f [%.2f-%.2f] x%d\n", it->first.c_str()
                    , mean, min, max, count);

            ++i;
            if(i > reportLimit)
                break;
        }

        double reportTime = (Timer::GetFPGATimestamp() - reportStart) * 1000;

        cycleIndex = 0;
        ClearLogBuffer();

        AsynchronousPrinter::Printf("End report (%.2f ms)\n", reportTime);
    }
}

void Profiler::Log(std::string name, double timeTaken)
{
    if(loggedCounts.find(name) == loggedCounts.end())
    {
        loggedCounts[name] = 1;
        loggedMins[name] = loggedMaxs[name] = loggedSums[name]
                = timeTaken;
    }
    else
    {
        ++loggedCounts[name];
        if(timeTaken < loggedMins[name])
            loggedMins[name] = timeTaken;
        if(timeTaken > loggedMaxs[name])
            loggedMaxs[name] = timeTaken;
        loggedSums[name] += timeTaken;
    }
}

void Profiler::ClearLogBuffer()
{
    loggedCounts.clear();
    loggedMins.clear();
    loggedMaxs.clear();
    loggedSums.clear();
}
