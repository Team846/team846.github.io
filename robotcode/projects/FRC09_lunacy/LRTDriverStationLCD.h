#ifndef LRT_DRIVER_STATION_LCD_H_
#define LRT_DRIVER_STATION_LCD_H_

#include "SensorBase.h"
#include "Scroll.h"

/**
 * Provides LCD output on the Driver Station LCD. Utilizes scrolling.
 */

//class LRTDriverStationLCD : public SensorBase, public Scroll {
class LRTDriverStationLCD : public SensorBase {
public:
	static const UINT32 kSyncTimeout_ms = 20;
	static const UINT16 kFullDisplayTextCommand = 0x9FFF;
	
	virtual ~LRTDriverStationLCD();
	static LRTDriverStationLCD *GetInstance();
	
//	void ScrollUpdateLCD();
	void LCDUpdate();
	void print(UINT8 line, const char *format, ...);
	void ScrollUpLCD();
	void ScrollDownLCD();
	
protected:
	LRTDriverStationLCD();
	
private:
	static LRTDriverStationLCD *m_instance;
	
	DISALLOW_COPY_AND_ASSIGN(LRTDriverStationLCD);
	
	UINT8 m_Cur_Line_Index;
	
	static const UINT8 kLCD_PHY_LINE = 6;
	static const UINT8 kLCD_MAX_LINE_CNT = 20;
	static const UINT8 kLCD_LINE_LEN = 21;
	char *m_textBuffer;
	char *m_outputBuffer;
	SEM_ID m_textBufferSemaphore;
};

#endif //LRT_DRIVER_STATION_LCD_H_
