//Author: Brandon Liu

#ifndef LRT_DRIVER_STATION_DISPLAY_H
#define LRT_DRIVER_STATION_DISPLAY_H

class LRTDriverStationDisplay {
public:
	LRTDriverStationDisplay* instance;
	
	LRTDriverStationDisplay* GetInstance();
	void Printf(int line, const char* format, ...);
	void UpdateDisplay();
	
	void ScrollUp();
	void ScrollDown();
	
private:
	LRTDriverStationDisplay();
	
	static const int kBufferLineCount = 20;
	static const int kDisplayLineCount = 6;
	static const int kDisplayLineLength = 21;
	
	char* m_buffer;
	
	int m_currentLineIndex;
};

#endif /* LRT_DRIVER_STATION_DISPLAY_H */
