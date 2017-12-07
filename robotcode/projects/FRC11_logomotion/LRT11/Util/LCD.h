#ifndef LCD_H_
#define LCD_H_

#include "..\General.h"

/**
 * Provides LCD output on the Driver Station LCD. Utilizes scrolling.
 */

class LCD : public SensorBase
{
public:
    static const UINT32 kSyncTimeout_ms = 20;
    static const UINT16 kFullDisplayTextCommand = 0x9FFF;

    enum LrtDsLcdLineNumber
    {
        kHeartbeatLine,
        kDriveLine,
        kDriveLine2,
        kDriveTrainLine,
        kEncoderLine,
        kLiftExtenderLine,
        kBallDetectorLine,
        kWinchLine,
        kRollerLine,
        kKickerDiagnosticsLine,
        kENDLINES
    };

    virtual ~LCD();
    static LCD& GetInstance();

    void LCDUpdate();
    void Print(UINT8 line, const char* format, ...);

    void ScrollLCD(int x, int y);
    void UpdateHeartbeat(bool isServiceMode);

    static void UpdateGameTime(double time);

protected:
    LCD();

private:
    static LCD* instance;
    DISALLOW_COPY_AND_ASSIGN(LCD);

    int curLineIndex;
    int curColumnIndex;

    static const UINT8 kNumBufferLines = 20;
    static const UINT8 kNumBufferColumns = 40;

    static const UINT8 kNumLcdPhysicalLines = 6;
    // even on the new DriverStation, still 21 char
    static const UINT8 kNumLcdPhysicalColumns = 21;

    const char* loadArray;
    char* textBuffer;
    char* outputBuffer;
    semaphore* textBufferSemaphore;

    static double gameTime;
};

#endif //LRT_DRIVER_STATION_LCD_H_
