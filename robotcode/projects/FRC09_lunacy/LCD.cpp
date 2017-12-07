#include "LCD.h"
#include "DigitalModule.h"
#include "I2C.h"
#include "utility.h"
#include "WPIStatus.h"
#include "Scroll.h"

//This needs to be rewritten using a buffered output.
//Recommend writing all commands (register and char) into a general stack or queue
//and then transmitting a few characters each main loop.

#include <stdio.h>

I2C_LCD::I2C_LCD(UINT32 slot, UINT32 upInput, UINT32 dnInput)
	: Scroll(slot, upInput, dnInput), m_i2c(NULL)
	{
		DigitalModule *module = DigitalModule::GetInstance(slot);
		m_i2c = module->GetI2C(kAddress);
		
	
		m_Cur_Line_Index=0;
		
		memset(LCDScrnBuf,32,kLCD_MAX_LINE_CNT*kLCD_LINE_LEN);  //fill with spaces
		
		Clear();	
		
		for(int i=0;i<kLCD_MAX_LINE_CNT;++i)
			print(i,"Line %d",i);
		
		LCDScrn2.nCmdsToSend = 0;
	}

I2C_LCD::~I2C_LCD()
{
	delete m_i2c;
	m_i2c = NULL;
}

void I2C_LCD::Clear()
{
	SendByte(kClear,1);	//What is the '1'? [DG]
}

void I2C_LCD::SendByte(lcd_cmd kRegister, UINT8 b)
{
//	printf("%d%c,",kRegister, b);
	m_i2c->Write(kRegister,b);
}

void I2C_LCD::setCursor(UINT8 x, UINT8 y)
{
	SendByte(kCol,x);
	SendByte(kRow,y);
}


void I2C_LCD::print(UINT8 x, UINT8 y, const char *format, ...)
{
	va_list args;
	char buffer[40];
	
	int i;
	int nc;
	
	va_start(args, format);
	nc = vsprintf(buffer,format,args);
	va_end(args);
	
	
	setCursor(x,y);
	
	puts("Unbuffered LCD Printing!\n");	//This is BAD [DG]
	i = 0;
	while(i<nc)
		SendByte(kDisplay,buffer[i++]);
}

void I2C_LCD::print(UINT8 row, const char *format, ...)
{
	va_list args;
	char buffer[40];
	
//	int i;
	int nc;
	
	va_start(args, format);
	nc = vsprintf(buffer,format,args);
	va_end(args);
	
	memset(&LCDScrnBuf[row][0],32,kLCD_LINE_LEN); //clear the line
	
	// limit the maximum length to write to the line
	if(nc>kLCD_LINE_LEN)
		nc = kLCD_LINE_LEN;
	
	memcpy(LCDScrnBuf[row], buffer,nc);
	
//	for(i=0;i<nc;++i)
//		LCDScrnBuf[row][i]= buffer[i];
	
}
/*
 * LCDBeginUpdate copies the data in the screen buffer to a second buffer,
 * complete with I2C registers for the setCursor() commands
 * Now, the data may be transmitted without knowing the contents.
 * 
 * Using LCDUpdate(), these commands may be transmitted a few bytes at a time over
 * several cycles. [DG]
 */
void I2C_LCD::LCDBeginUpdate(void)
{
	int i=0;
	int row=0;
	LCDScrn2.b = LCDScrn2.buffer;
	LCDScrn2.r = LCDScrn2.regI2C;
	
	do {
		LCDScrn2.regI2C[i]= kCol;
		LCDScrn2.buffer[i]= 0;
		++i;
		LCDScrn2.regI2C[i]= kRow;
		LCDScrn2.buffer[i]= row;
		++i;
		memcpy(&LCDScrn2.buffer[i],LCDScrnBuf[row+m_Cur_Line_Index], kLCD_LINE_LEN );
		memset(&LCDScrn2.regI2C[i],kDisplay, kLCD_LINE_LEN );	//display byte
		i += kLCD_LINE_LEN;
	} while (++row < kLCD_PHY_LINE);
	LCDScrn2.nCmdsToSend = i;
	
//	for(i=0;i<sizeof(LCDScrn2.buffer);i++)
//		putchar(LCDScrn2.buffer[i]);
//	printf("\n");

//	printf("\n\n");
//	for(i=0;i<sizeof(LCDScrn2.regI2C);i++){
//			printf("%c",LCDScrn2.regI2C[i]+48);
//			printf("%c",LCDScrn2.buffer[i]);
//			printf(";");
//	}
//	printf("\n");
}

/* LCDUpdate sends nBytes from the LCDScrn2 buffers.
 * Sending 4 lines of 20 chars (80+4*2=88 bytes] takes ~85ms, or ~1 ms per character.
 * If we are running at 200Hz (5ms cycle time), we can only send a few chars per loop.
 */
void I2C_LCD::LCDUpdate(int nBytes)
{
	while (LCDScrn2.nCmdsToSend > 0 && --nBytes >= 0)
	{
		LCDScrn2.nCmdsToSend--;
		//printf("%d%c;",(lcd_cmd) *LCDScrn2.r++, *LCDScrn2.b++);
		SendByte((lcd_cmd)*LCDScrn2.r++, *LCDScrn2.b++);
	}	
}
//void I2C_LCD::LCDUpdate(int nBytes)
//{
//	if (LCDScrn2.updateComplete)
//		return;
//	
//	while (--nBytes >= 0)
//	{
//		SendByte(2, *LCDScrn2.p++);
//		if (++col >= kLCD_LINE_LEN) {
//			//Start Next Row.
//			col=0;
//			if (++row<kLCD_PHY_LINE) {
//				setCursor(0,row);
//				nBytes -=2;	//sends two bytes
//			}
//			else {
//				LCDScrn2.updateComplete=true;
//				return;
//			}
//		}
//	}
//}
//void I2C_LCD::LCDUpdate(void)
//{
//	for(int row = 0; row<kLCD_PHY_LINE; row++)
//	{
//		setCursor(0,row);
//		for(int col = 0; col<kLCD_LINE_LEN; col++)
//			SendByte(2,LCDScrnBuf[row+m_Cur_Line_Index][col]);
//	}
//}

void I2C_LCD::ScrollUpdateLCD(void)
{
	int direction;
	
	ScrollCheck();
	direction = ScrollDirection();
	
	if(direction < 0 && m_Cur_Line_Index >0)
		m_Cur_Line_Index += direction;
	if(direction > 0 && m_Cur_Line_Index<(kLCD_MAX_LINE_CNT-kLCD_PHY_LINE))
		m_Cur_Line_Index += direction;
}

