//Author: Brandon Liu (2010)

#ifndef LRTCONSOLE_H_
#define LRTCONSOLE_H_

#include "Notifier.h"

class LRTConsole {
public:
	static LRTConsole& GetInstance();

	int GetCycleCount();
	void PrintEverySecond(const char* format, ...);
	void PrintEveryHalfSecond(const char* format, ...);
	void PrintMultipleTimesPerSecond(float hertz, const char* format, ...);

private:
	LRTConsole();
	LRTConsole(const LRTConsole&); // Prevent copy-construction
	LRTConsole& operator=(const LRTConsole&); // Prevent assignment
	
	friend class LRTBot10;
	void NextCycle(); //increment cycle count; should be only called by LRTBot10
	
	static LRTConsole *m_instance;
	
	volatile int m_cycleNum;
	
	const static int kCyclesPerSecond = 50;
};
#endif /*LRTCONSOLE_H_*/
