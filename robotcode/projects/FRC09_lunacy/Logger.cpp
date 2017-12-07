#include "WPILib.h"
#include "Logger.h"
#include <stdio.h>

Logger *Logger::instance = NULL;

Logger *Logger::GetInstance() {
	if (instance == NULL) {
		//printf("Creating Logger\n");
		instance = new Logger();
		//printf("Logger Created\n");
	}
	return instance;
}

//virtual ~Logger::Logger();
void Logger::printf(Value type, char *format, ...) {
	if (
		type == k1hz && (loopCounter % 200 == 0)
		|| type == k2hz && (loopCounter % 100 == 0)
	)
	{
		va_list args;
		
		va_start(args, format);
		vprintf(format,args);
		va_end(args);
	}
}

ofstream& Logger::file() {
	return logout;
}

void Logger::startNextCycle() {
	++loopCounter;
}

Logger::Logger()
	: logout("/tmp/846.log")
{
	loopCounter = 0;
}
