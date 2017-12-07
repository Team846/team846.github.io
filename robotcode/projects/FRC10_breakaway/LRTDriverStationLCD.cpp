//Author: Brandon Liu and David Liu (2009-2010)

#include "LRTDriverStationLCD.h"
#include "NetworkCommunication/FRCComm.h"

#include <cstdio>
#include <Synchronized.h>
#include "LRTUtil.h"
#include "AsynchronousPrinter.h"

LrtLcd* LrtLcd::m_instance = NULL;

LrtLcd::LrtLcd()  {
	m_textBuffer = new char[kNumBufferLines * kNumBufferColumns];
	m_outputBuffer = new char[USER_DS_LCD_DATA_SIZE];
	
	m_curLineIndex = m_curColumnIndex = 0;
	memset(m_textBuffer, ' ', kNumBufferLines * kNumBufferColumns);
	
	m_textBufferSemaphore = semMCreate(SEM_DELETE_SAFE | SEM_INVERSION_SAFE);
	
	AddToSingletonList();
}

LrtLcd::~LrtLcd() {
	semDelete(m_textBufferSemaphore);
	delete [] m_textBuffer;
	delete [] m_outputBuffer;
	m_instance = NULL; //actual object is not deleted.
}

LrtLcd& LrtLcd::GetInstance() {
	if (m_instance == NULL)
		m_instance = new LrtLcd();
	return *m_instance;
}

void LrtLcd::print(UINT8 line, const char* format, ...) {
	if (line > kNumBufferLines) {
		AsynchronousPrinter::Printf("LRTDSLCD: out of bounds %d\n", line);
		return;
	}
	
	va_list args;
	char buffer[120]; // FIXME: how is this size determined? originally 40
					  // BL: AFAIK, It's just an arbitrary size that should be larger than
					  // the largest possible string that would be passed
	
	int len;
	
	va_start(args, format);
	Synchronized sync(m_textBufferSemaphore);
	len = vsprintf(buffer,format,args);
	va_end(args);
	
	// clear the line
	memset(m_textBuffer + line * kNumBufferColumns, ' ', kNumBufferColumns);
	
	// limit the maximum length to write to the line
	if(len > kNumBufferColumns)
		len = kNumBufferColumns;
	
	memcpy(m_textBuffer + line * kNumBufferColumns, buffer, len);
}

void LrtLcd::LCDUpdate() {
	Synchronized sync(m_textBufferSemaphore);
	
	memset(m_outputBuffer, ' ', USER_DS_LCD_DATA_SIZE); //clear the buffer
	*((UINT16 *)m_outputBuffer) = kFullDisplayTextCommand;
	
	char *outputBufferTextStart = m_outputBuffer + sizeof(UINT16);
	
	for (int line = 0; line < kNumLcdPhysicalLines; ++line) {
		int y = m_curLineIndex + line;
		memcpy(outputBufferTextStart + kNumLcdPhysicalColumns * line,
				m_textBuffer + kNumBufferColumns * y + m_curColumnIndex,
				kNumLcdPhysicalColumns
		);
	}
	setUserDsLcdData(m_outputBuffer, USER_DS_LCD_DATA_SIZE, kSyncTimeout_ms);
}

void LrtLcd::ScrollLCD(int x, int y) {
	m_curLineIndex = LRTUtil::clamp<int>(m_curLineIndex + y,
			0, kNumBufferLines - kNumLcdPhysicalLines);
	m_curColumnIndex = LRTUtil::clamp<int>(m_curColumnIndex + x,
			0, kNumBufferColumns - kNumLcdPhysicalColumns);
	AsynchronousPrinter::Printf("DS LCD Scroll %d %d => %d,%d\n", x, y, m_curLineIndex, m_curColumnIndex);
}
