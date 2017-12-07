//Author: David Liu (2010)

#include "Profiler.h"
#include <string>
#include <fstream>
#include <sstream>
#include "LRTUtil.h"
#include "LRTConsole.h"
#include "AsynchronousPrinter.h"

using namespace std;

Profiler* Profiler::m_instance = NULL;

Profiler& Profiler::GetInstance ()
{
	if (m_instance == NULL) {
		m_instance = new Profiler();	
	}
	return *m_instance;
}

Profiler::Profiler()
: m_cycleIndex(0)
{
	AddToSingletonList();
	
	// anything else...
}

template <class PairT>
struct SortBySecondValue {
	bool operator()(const PairT& lhs, const PairT& rhs) const {
		return lhs.second > rhs.second;
	}
};

void Profiler::StartNewCycle() {
	++m_cycleIndex;
	
	if (m_cycleIndex >= m_reportPeriod) {
		double reportStart = Timer::GetFPGATimestamp();
		
		AsynchronousPrinter::Printf("----------------------\n");
		AsynchronousPrinter::Printf("| PROFILER (%d cycles)\n", m_reportPeriod);
		typedef map<string, double>::value_type paired;
		typedef set< paired ,  SortBySecondValue<paired>  > SetSortedBySecond;
		SetSortedBySecond vals;

		for (map<string, double>::iterator it = m_loggedMaxs.begin(); it != m_loggedMaxs.end(); ++it) {
//			vals.insert( paired(it->first, it->second/m_loggedCounts[it->first]) ); // to sort by means
			vals.insert( paired(it->first, it->second) );
		}
		
		int i = 0;
		for (SetSortedBySecond::iterator it = vals.begin(); it != vals.end(); ++it) {
			double min = m_loggedMins[it->first];
			double max = m_loggedMaxs[it->first];
			int count = m_loggedCounts[it->first];
			double mean = m_loggedSums[it->first] / count;

			AsynchronousPrinter::Printf("| %-30s ~%.2f [%.2f-%.2f] x%d\n", it->first.c_str()
					, mean, min, max, count );
			
			++i;
			if (i > m_reportLimit) {
				break;
			}
		}
		AsynchronousPrinter::Printf("----------------------\n");
		double reportTime = (Timer::GetFPGATimestamp() - reportStart)*1000;

		m_cycleIndex = 0;
		ClearLogBuffer();
		
		AsynchronousPrinter::Printf("| Report generation took %.2f ms\n", reportTime);
	}
}

void Profiler::Log(std::string name, double timeTaken) {
	if (m_loggedCounts.find(name) == m_loggedCounts.end()) {
		m_loggedCounts[name] = 1;
		m_loggedMins[name] = m_loggedMaxs[name] = m_loggedSums[name] = timeTaken;
	} else {
		++m_loggedCounts[name];
		if (timeTaken < m_loggedMins[name])
			m_loggedMins[name] = timeTaken;
		if (timeTaken > m_loggedMaxs[name])
			m_loggedMaxs[name] = timeTaken;
		m_loggedSums[name] += timeTaken;
	}
}

void Profiler::ClearLogBuffer() {
	m_loggedCounts.clear();
	m_loggedMins.clear();
	m_loggedMaxs.clear();
	m_loggedSums.clear();
}
