#include "AsynchronousFileLogger.h"

AsynchronousFileLogger* AsynchronousFileLogger::instance = NULL;

AsynchronousFileLogger& AsynchronousFileLogger::GetInstance()
{
    if(instance == NULL)
        instance = new AsynchronousFileLogger();
    return *instance;
}

void AsynchronousFileLogger::Printf(const char* format, ...)
{
    const int maxLength = 256;
    char buffer[maxLength];

    AsynchronousFileLogger& me = GetInstance();

    va_list args;
    va_start(args, format);

    vsprintf(buffer, format, args);
    va_end(args);

    {
        Synchronized s(me.semaphore);
        string str(buffer);

        me.queue.push(str);
        me.queueBytes += str.length();

        if(me.queueBytes >= kMaxBuffer)
        {
            while(!me.queue.empty())
                me.queue.pop();

            string overflow("(AsynchronousFileLogger Buffer Overflow)\n");

            me.queue.push(overflow);
            me.queueBytes = overflow.length();
        }
    }
}

AsynchronousFileLogger::AsynchronousFileLogger()
    : semaphore(semMCreate(SEM_Q_PRIORITY | SEM_DELETE_SAFE
            | SEM_INVERSION_SAFE))
    , queueBytes(0)
    , printerTask("AsynchronousFileLogger",
            (FUNCPTR)AsynchronousFileLogger::PrinterTaskRunner)
    , fout("log.txt")
{
    AddToSingletonList();
    printerTask.Start();
}

AsynchronousFileLogger::~AsynchronousFileLogger()
{
    semDelete(semaphore);
}

void AsynchronousFileLogger::PrinterTaskRunner()
{
    AsynchronousFileLogger::GetInstance().PrinterTask();
}
void AsynchronousFileLogger::PrinterTask()
{
    while(true)
    {
        while(!queue.empty())
        {
            string str;

            {
                Synchronized s(semaphore);
                str = queue.front();

                queue.pop();
                queueBytes -= str.length();

                Wait(0.001);   // allow other tasks to run
            }

            fout << str;
            fout.flush();
        }

        Wait(0.002);   // allow other tasks to run
    }
}

void AsynchronousFileLogger::StopPrinterTask()
{
    printerTask.Suspend();
}

void AsynchronousFileLogger::ResumePrinterTask()
{
    printerTask.Resume();
}
