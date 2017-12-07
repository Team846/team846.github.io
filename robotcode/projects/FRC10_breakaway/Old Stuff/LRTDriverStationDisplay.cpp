//Author: Brandon Liu

#include <cstring>
#include "SensorBase.h"
#include "DriverStation.h"
#include "LRTDriverStationDisplay.h"

LRTDriverStationDisplay* LRTDriverStationDisplay::GetInstance() {
	if (instance == 0)
		instance = new LRTDriverStationDisplay();
	return instance;
}

void LRTDriverStationDisplay::Printf(int line, const char* format, ...) {
	if (line < 0 || line >= kBufferLineCount)
		return;
	
	va_list args;
	char buffer[40];
	int len;
	
	va_start(args, format);
	len = vsprintf(buffer,format,args);
	va_end(args);
	
	//if the line is longer than the screen, cut off the excess
	if (len > kDisplayLineLength)
		len = kDisplayLineLength;
	
	memset(m_buffer + line * kDisplayLineLength, ' ', kDisplayLineLength); //clear the entire line first
	memcpy(m_buffer + line * kDisplayLineLength, buffer, len); //now copy in the line
}

void LRTDriverStationDisplay::UpdateDisplay() {
	//print one line at a time
	for (int line = m_currentLineIndex; line < m_currentLineIndex + kDisplayLineCount; line++) {
		char line_text[kDisplayLineLength];
		memcpy(line_text, m_buffer + line * kDisplayLineLength, kDisplayLineLength);
		line_text[kDisplayLineLength - 1] = '\n'; //add newlines to the end of each line
		DriverStation::GetInstance()->GetLowPriorityDashboardPacker().Printf(line_text);
	}
}

void LRTDriverStationDisplay::ScrollUp() {
	m_currentLineIndex--;
	if (m_currentLineIndex < 0)
		m_currentLineIndex = 0;
}

void LRTDriverStationDisplay::ScrollDown() {
	m_currentLineIndex++;
	if (m_currentLineIndex + kDisplayLineCount > kBufferLineCount)
		m_currentLineIndex = kBufferLineCount - kDisplayLineCount;
}

LRTDriverStationDisplay::LRTDriverStationDisplay() {
	m_buffer = new char[kBufferLineCount * kDisplayLineLength];
	memset(m_buffer, ' ', kBufferLineCount * kDisplayLineLength);
	
	m_currentLineIndex = 0;
	
//	AddToSingletonList();
}
