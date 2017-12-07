//Author: David Liu (2010)

// Usage:
// in CommonPeriodic:
//	Profiler::GetInstance().StartNewCycle();
//
// in the code:
//	{
//		ProfiledSection pf("NameOfActivity");
//		// do something time consuming...
//	}

#ifndef LRT_PROFILER_H_
#define LRT_PROFILER_H_

#include "WPILib.h"
#include <map>
#include <string>

class Profiler : public SensorBase {
	
public:
	const static int m_reportPeriod = 50*5;
	const static int m_reportLimit = 10;
	
	virtual ~Profiler() { }
	static Profiler& GetInstance ();
	
	void StartNewCycle();
	void Log(std::string name, double timeTaken);
	void ClearLogBuffer();

protected:
	Profiler();
	
private:
	static Profiler *m_instance;

	DISALLOW_COPY_AND_ASSIGN(Profiler);

	std::map<std::string, int> m_loggedCounts;
	std::map<std::string, double> m_loggedMins;
	std::map<std::string, double> m_loggedMaxs;
	std::map<std::string, double> m_loggedSums;
	
	int m_cycleIndex;
};

class ProfiledSection {
public:
	ProfiledSection(std::string name)
	: m_name(name)
	, m_start(Timer::GetFPGATimestamp())
	{
	}
	~ProfiledSection() {
		double ms = (Timer::GetFPGATimestamp() - m_start) * 1000;
		Profiler::GetInstance().Log(m_name, ms);
	}
protected:
	string m_name;
	double m_start;
};

class ProfilerHelper {
public:
	ProfilerHelper()
	{
	}
	void Start(std::string name) {
		m_name = name;
		m_start = Timer::GetFPGATimestamp();
	}
	void Finish() {
		double ms = (Timer::GetFPGATimestamp() - m_start) * 1000;
		Profiler::GetInstance().Log(m_name, ms);
	}
protected:
	string m_name;
	double m_start;
};

#endif
