//Author: Brandon Liu and David Liu (2009-2010)

#ifndef LRT_DRIVER_STATION_LCD_H_
#define LRT_DRIVER_STATION_LCD_H_

#include "SensorBase.h"

/**
 * Provides LCD output on the Driver Station LCD. Utilizes scrolling.
 */


//class LRTDriverStationLCD : public SensorBase, public Scroll {
class LrtLcd : public SensorBase {
public:
	static const UINT32 kSyncTimeout_ms = 20;
	static const UINT16 kFullDisplayTextCommand = 0x9FFF;

	enum LrtDsLcdLineNumber {
		kHeartbeatLine,
		kDriveLine,
		kDriveLine2,
		kEncoderLine,
		kLiftExtenderLine,
		kBallDetectorLine,
		kWinchLine,
		kRollerLine,
		kKickerDiagnosticsLine,
		kENDLINES
	};
	
	virtual ~LrtLcd();
	static LrtLcd& GetInstance();
	
	void LCDUpdate();
	void print(UINT8 line, const char *format, ...);
	void ScrollLCD(int x, int y);
	
protected:
	LrtLcd();
	
private:
	static LrtLcd *m_instance;
	
	DISALLOW_COPY_AND_ASSIGN(LrtLcd);
	
	int m_curLineIndex, m_curColumnIndex;
	
	static const UINT8 kNumBufferLines = 20;
	static const UINT8 kNumBufferColumns = 40;
	static const UINT8 kNumLcdPhysicalLines = 6;
	static const UINT8 kNumLcdPhysicalColumns = 21; //even on the new DriverStation, still 21char
	char *m_textBuffer;
	char *m_outputBuffer;
	SEM_ID m_textBufferSemaphore;
};

#endif //LRT_DRIVER_STATION_LCD_H_
