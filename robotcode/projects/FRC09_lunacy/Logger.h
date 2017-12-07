#ifndef LOGGER_H
#define LOGGER_H
#include "WPILib.h"
#include <iostream>
#include <fstream>

// FIXME: This doesn't work in the 50Hz NewData loops, because they don't happen on the right loops
// (loopCounter is out of sync)

class Logger
{
	static Logger *instance;
	UINT32 loopCounter;
	ofstream logout;
	
public:
	
	typedef enum {k1hz, k2hz} Value;
	
	static Logger *GetInstance();
	void printf(Value type, char *format, ...);
	void startNextCycle();

	ofstream& Logger::file();
	
	static void log(Value type, char *format, ...) {
		Logger *p = GetInstance();
		va_list args;
		va_start(args, format);
		p->printf(type, format, args);
		va_end(args);
	}
	static void log1hz(char *format, ...) {
		Logger *p = GetInstance();
		va_list args;
		va_start(args, format);
		p->printf(k1hz, format, args);
		va_end(args);
	}
	static void log2hz(char *format, ...) {
		Logger *p = GetInstance();
		va_list args;
		va_start(args, format);
		p->printf(k2hz, format, args);
		va_end(args);
	}
	
protected:
	Logger();
};

#endif
