#include "LRTDriverStationLCD.h"
#include "NetworkCommunication/FRCComm.h"

#include <cstdio>

LRTDriverStationLCD* LRTDriverStationLCD::m_instance = NULL;

LRTDriverStationLCD::LRTDriverStationLCD()  {
	m_textBuffer = new char[kLCD_MAX_LINE_CNT * kLCD_LINE_LEN];
	m_outputBuffer = new char[USER_DS_LCD_DATA_SIZE];
	
	m_Cur_Line_Index = 0;
	memset(m_textBuffer, ' ', kLCD_MAX_LINE_CNT * kLCD_LINE_LEN);
	
	m_textBufferSemaphore = semMCreate(SEM_DELETE_SAFE | SEM_INVERSION_SAFE);
	
	AddToSingletonList();
}

LRTDriverStationLCD::~LRTDriverStationLCD() {
	semDelete(m_textBufferSemaphore);
	delete [] m_textBuffer;
	m_instance = NULL; //actual object is not deleted.
}

LRTDriverStationLCD* LRTDriverStationLCD::GetInstance() {
	if (m_instance == NULL)
		m_instance = new LRTDriverStationLCD();
	return m_instance;
}

void LRTDriverStationLCD::print(UINT8 line, const char* format, ...) {
	if (line >= kLCD_MAX_LINE_CNT)
		return;
	
	va_list args;
	char buffer[40];
	
	int len;
	
	va_start(args, format);
	Synchronized sync(m_textBufferSemaphore);
	len = vsprintf(buffer,format,args);
	va_end(args);
	
	memset(m_textBuffer + line * kLCD_LINE_LEN,' ',kLCD_LINE_LEN); //clear the line
	
	// limit the maximum length to write to the line
	if(len>kLCD_LINE_LEN)
		len = kLCD_LINE_LEN;
	
	memcpy(m_textBuffer + line * kLCD_LINE_LEN, buffer, len);

	
//	for(i=0;i<nc;++i)
//		m_textBuffer[line*kLCD_LINE_LEN + i]= buffer[i];
	
}

void LRTDriverStationLCD::LCDUpdate() {
	Synchronized sync(m_textBufferSemaphore);
	
	memset(m_outputBuffer, ' ', USER_DS_LCD_DATA_SIZE); //clear the buffer
	*((UINT16 *)m_outputBuffer) = kFullDisplayTextCommand;
	memcpy(m_outputBuffer + sizeof(UINT16), m_textBuffer + kLCD_LINE_LEN * m_Cur_Line_Index, kLCD_LINE_LEN * kLCD_PHY_LINE);
	setUserDsLcdData(m_outputBuffer, USER_DS_LCD_DATA_SIZE, kSyncTimeout_ms);
}

void LRTDriverStationLCD::ScrollUpLCD() {
	printf("DS LCD Scroll Up\n");
	if (m_Cur_Line_Index == 0)
		return;
	m_Cur_Line_Index--;
}

void LRTDriverStationLCD::ScrollDownLCD() {
	printf("DS LCD Scroll Down\n");
	m_Cur_Line_Index++;
	if (m_Cur_Line_Index > kLCD_MAX_LINE_CNT - kLCD_PHY_LINE)
		m_Cur_Line_Index = kLCD_MAX_LINE_CNT - kLCD_PHY_LINE;
}
