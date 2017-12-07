#ifndef _LCD_h_
#define _LCD_h_

#include "SensorBase.h"
#include "Scroll.h"
#include "I2C.h"

class I2C_LCD: public SensorBase, public Scroll
{
public:
	explicit I2C_LCD(UINT32 slot,UINT32 upPin, UINT32 dnPin);
	virtual ~I2C_LCD();
	
	enum lcd_cmd { kClear=0, kDisplay=2, kCol=3, kRow=4  };
	void SendByte(lcd_cmd kRegister, UINT8 b);
	void Clear();
	
	void print(UINT8 x, UINT8 y, const char*format, ...);
	void print(UINT8 row, const char *format, ...);
	
	void ScrollUpdateLCD(void);
	
	void LCDBeginUpdate(void); //Prepares for transmission of buffer over many cycles.
	void LCDUpdate(int nBytes); //tranmits a few bytes each cycle
	
private:
	void setCursor(UINT8 x, UINT8 y);
	void setCursorInBuffer(UINT8 x, UINT8 y);
	
	static const UINT8 kAddress = 4;
	static const UINT8 kLCD_LINE_LEN = 20;
	static const UINT8 kLCD_PHY_LINE = 4;
	static const UINT8 kLCD_MAX_LINE_CNT = 30;

	
	char LCDScrnBuf[kLCD_MAX_LINE_CNT][kLCD_LINE_LEN];
	UINT32 m_Cur_Line_Index;
	
	struct FourLineScreenBuffer{
		UINT8 buffer[kLCD_PHY_LINE * (2+kLCD_LINE_LEN)];  //allot 2 extra bytes
		UINT8 regI2C[kLCD_PHY_LINE * (2+kLCD_LINE_LEN)];	
		UINT8 *b; //will point into buffer and regI2C
		UINT8 *r;	//will point into buffer and regI2C
		int nCmdsToSend;
	} LCDScrn2;
	
	
	I2C *m_i2c;
};

#endif
