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

#ifndef LRT_ASYNCPRINT_H_
#define LRT_ASYNCPRINT_H_

#include "WPILib.h"
#include <queue>
#include <string>

class AsynchronousPrinter : public SensorBase {
	
public:
	virtual ~AsynchronousPrinter();
	static AsynchronousPrinter& GetInstance();
	static void Printf(const char *format, ...);
	void StopPrinterTask();
	void ResumePrinterTask();
//	void ClearBuffer();

private:
	AsynchronousPrinter();
	DISALLOW_COPY_AND_ASSIGN(AsynchronousPrinter);
	
	static void PrinterTaskRunner();
	void PrinterTask();
	
	bool m_enabled;
	static AsynchronousPrinter *m_instance;
	
	Task m_printerTask;
	SEM_ID m_semaphore;

	std::queue<std::string> m_queue;
	int m_queueBytes;
	
	const static int kMaxBuffer = 1024;
};

#endif
