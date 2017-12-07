//Author: Brandon Liu (2010)

#include "LRTConsole.h"
#include "WPILib.h"
#include "Notifier.h"
#include <cstdio>
#include "AsynchronousPrinter.h"

LRTConsole* LRTConsole::m_instance = NULL;

LRTConsole::LRTConsole() :
	m_cycleNum(0) {
}

LRTConsole& LRTConsole::GetInstance() {
	if (m_instance == NULL) {
		m_instance = new LRTConsole();
	}
	return *m_instance;
}

int LRTConsole::GetCycleCount() {
	return m_cycleNum;
}

void LRTConsole::NextCycle() {
	m_cycleNum++;
}

void LRTConsole::PrintEverySecond(const char* format, ...) {
	if (m_cycleNum % kCyclesPerSecond == 0) {
		char buffer[200]; //200 characters should be large enough to accommodate all prints
		va_list args;
		va_start(args, format);
		vsprintf(buffer, format, args); //format string into buffer
		AsynchronousPrinter::Printf(buffer);
		va_end(args);
	}
}

void LRTConsole::PrintEveryHalfSecond(const char* format, ...) {
	if (m_cycleNum % (kCyclesPerSecond / 2) == 0) {
		char buffer[200]; //200 characters should be large enough to accommodate all prints
		va_list args;
		va_start(args, format);
		vsprintf(buffer, format, args); //format string into buffer
		AsynchronousPrinter::Printf(buffer);
		va_end(args);
	}
}

//hertz may be < 1
void LRTConsole::PrintMultipleTimesPerSecond(float hertz, const char* format, ...) {
	if (m_cycleNum % int(kCyclesPerSecond / hertz) == 0) {
		char buffer[200]; //200 characters should be large enough to accommodate all prints
		va_list args;
		va_start(args, format);
		vsprintf(buffer, format, args); //format string into buffer
		AsynchronousPrinter::Printf(buffer);
		va_end(args);
	}
}
