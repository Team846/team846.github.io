#ifndef LRT_ASYNCPRINT_H_
#define LRT_ASYNCPRINT_H_

#include <queue>
#include "WPILib.h"

/*!
 * \brief Provides an asynchronous equivilent of printf. 
 */
class AsynchronousPrinter
{
public:
    virtual ~AsynchronousPrinter(); //why is this virtual? -dg
    static AsynchronousPrinter& Instance();

    /*!
     * \brief Asynchronous alternative to Printf
     */
    static int Printf(const char* format, ...);
    static void Quit();
    static bool QueueEmpty()
    {
        return Instance().queue_.empty();
    }

protected:
    AsynchronousPrinter();

private:
    static int PrinterTaskWrapper(); //match (FUNCPTR):  int *FUNCPTR(...)
    int PrinterTask();

    bool quitting_; //if true, then we are exiting and closing down the print queue.
    bool running_;  //true if print task is running.

    semaphore* semaphore_;
    int queue_bytes_;

    Task printerTask; //T vsWorks background task that prints the buffered output.
    queue<string> queue_;

    const static int kMaxBuffer_ = 4096;
    DISALLOW_COPY_AND_ASSIGN(AsynchronousPrinter);
};

#endif
