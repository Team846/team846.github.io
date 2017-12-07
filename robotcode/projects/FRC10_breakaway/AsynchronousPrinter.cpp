//Author: David Liu (2010)

#include "AsynchronousPrinter.h"
#include <string>
#include <fstream>
#include <sstream>
#include "LRTUtil.h"
#include "LRTConsole.h"

using namespace std;

AsynchronousPrinter* AsynchronousPrinter::m_instance = NULL;

AsynchronousPrinter& AsynchronousPrinter::GetInstance()
{
	if (m_instance == NULL) {
		m_instance = new AsynchronousPrinter();	
	}
	return *m_instance;
}

void AsynchronousPrinter::Printf(const char *format, ...) {
//	va_list args;
//	GetInstance().Printf(format, args);
	
	const int maxLength = 256;
	char buffer[maxLength];
	
	AsynchronousPrinter& me = GetInstance();
	
	va_list args;
	va_start(args, format);
//	vsnprintf(buffer, maxLength, format, args); // WTF? THIS CRASHES!
	vsprintf(buffer, format, args);
	va_end(args);

	{
		Synchronized s (me.m_semaphore);
		string str (buffer);
		me.m_queue.push(str);
		me.m_queueBytes += str.length();
		
		if (me.m_queueBytes >= kMaxBuffer) {
			while (!me.m_queue.empty()) {
				me.m_queue.pop();
			}
			
			string overflow("(AsyncPrinter Buffer Overflow)\n");
			me.m_queue.push(overflow);
			me.m_queueBytes = overflow.length();
		}
	}
}

AsynchronousPrinter::AsynchronousPrinter()
: m_printerTask("AsynchronousPrinter", (FUNCPTR)AsynchronousPrinter::PrinterTaskRunner)
, m_semaphore(semMCreate(SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE))
, m_queueBytes(0)
{
	AddToSingletonList();

	m_printerTask.Start();
	// anything else...
}

AsynchronousPrinter::~AsynchronousPrinter() {
	semDelete(m_semaphore);
}

void AsynchronousPrinter::PrinterTaskRunner() {
	AsynchronousPrinter::GetInstance().PrinterTask();
}
void AsynchronousPrinter::PrinterTask() {
	while(true) {
		while (!m_queue.empty()) {
			string str;
			{ Synchronized s (m_semaphore);
				str = m_queue.front();
				m_queue.pop();
				m_queueBytes -= str.length();
				Wait(0.001); // allow other tasks to run
			}
			printf(str.c_str());
		}
		Wait(0.002); // allow other tasks to run
	}
	//
}

void AsynchronousPrinter::StopPrinterTask()
{
	m_printerTask.Suspend();
}

void AsynchronousPrinter::ResumePrinterTask()
{
	m_printerTask.Resume();
}
