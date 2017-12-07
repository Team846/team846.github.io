#include "LCD.h"
#include "..\Util\Util.h"
#include "..\Util\AsynchronousPrinter.h"
#include "NetworkCommunication/FRCComm.h"
#include <cstdio>
#include <Synchronized.h>

LCD* LCD::instance = NULL;
double LCD::gameTime = -1.0;

LCD::LCD() :
    loadArray("\\|/-")
{
    AddToSingletonList(); //TODO Make this a static object. -dg

    textBuffer = new char[kNumBufferLines * kNumBufferColumns];
    outputBuffer = new char[USER_DS_LCD_DATA_SIZE];

    curLineIndex = curColumnIndex = 0;
    memset(textBuffer, ' ', kNumBufferLines * kNumBufferColumns);

    textBufferSemaphore = semMCreate(SEM_DELETE_SAFE | SEM_INVERSION_SAFE);

    printf("Constructed LCD\n");
}

LCD::~LCD()
{
    semDelete(textBufferSemaphore);
    delete [] textBuffer;
    delete [] outputBuffer;
    instance = NULL; //actual object is not deleted.
}

LCD& LCD::GetInstance()
{
    if(instance == NULL)
        instance = new LCD();
    return *instance;
}

void LCD::Print(UINT8 line, const char* format, ...)
{
    if(line > kNumBufferLines)
    {
        AsynchronousPrinter::Printf("LCD: out of bounds %d\n", line);
        return;
    }

    va_list args;

    // How is this size determined?
    // BL: AFAIK, It's just an arbitrary size that should be larger than
    // the largest possible string that would be passed
    char buffer[120];
    int len;

    va_start(args, format);

    Synchronized sync(textBufferSemaphore);
    len = vsprintf(buffer, format, args);

    va_end(args);

    // clear the line
    memset(textBuffer + line * kNumBufferColumns, ' ', kNumBufferColumns);

    // limit the maximum length to write to the line
    if(len > kNumBufferColumns)
        len = kNumBufferColumns;

    memcpy(textBuffer + line * kNumBufferColumns, buffer, len);
}

void LCD::LCDUpdate()
{
    Synchronized sync(textBufferSemaphore);

    // clear the buffer
    memset(outputBuffer, ' ', USER_DS_LCD_DATA_SIZE);
    *((UINT16*)outputBuffer) = kFullDisplayTextCommand;

    char* outputBufferTextStart = outputBuffer + sizeof(UINT16);

    for(int line = 0; line < kNumLcdPhysicalLines; ++line)
    {
        int y = curLineIndex + line;
        memcpy(outputBufferTextStart + kNumLcdPhysicalColumns * line,
                textBuffer + kNumBufferColumns * y + curColumnIndex,
                kNumLcdPhysicalColumns);
    }

    setUserDsLcdData(outputBuffer, USER_DS_LCD_DATA_SIZE, kSyncTimeout_ms);
}

void LCD::ScrollLCD(int x, int y)
{
    curLineIndex = Util::Clamp<int>(curLineIndex + y,
            0, kNumBufferLines - kNumLcdPhysicalLines);
    curColumnIndex = Util::Clamp<int>(curColumnIndex + x,
            0, kNumBufferColumns - kNumLcdPhysicalColumns);

    AsynchronousPrinter::Printf("DS LCD Scroll %d %d => %d,%d\n", x, y, curLineIndex, curColumnIndex);
}

void LCD::UpdateHeartbeat(bool isServiceMode)
{
    static int loops = 0;

    if(loops % 12 == 0)
        LCDUpdate();

    string mode = "Normal Mode";
    if(isServiceMode)
        mode = "Service Mode";

    char heartbeat = loadArray[(loops / 12) % 4];
    Print(kHeartbeatLine, "%c %s %.2f", heartbeat, mode.c_str(), gameTime);

    loops++;
}

void LCD::UpdateGameTime(double time)
{
    gameTime = time;
}
