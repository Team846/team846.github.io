#include "AirText.h"
#include "DigitalModule.h"
#include "I2C.h"
#include "utility.h"
#include "WPIStatus.h"

#include <stdio.h>

AirText::AirText(UINT32 slot)
	: m_i2c (NULL)
	{
		DigitalModule *module = DigitalModule::GetInstance(slot);
		m_i2c = module->GetI2C(kAddress);
	
		Clear();
		
		StopAir();
//		SetDelay(16000);
//		print(0,"846");
//		print(1,"Moonkey");
//		print(2,"SCORE");
//		print(3,"Slot %d",slot);
//		print(4,"string 5");
//				
//		StartAir(4);
	}

AirText::~AirText()
{
	delete m_i2c;
	m_i2c = NULL;
}

void AirText::Clear()
{
	SendByte(0,1);
}

//StartAir - starts display airtext using designated string
void AirText::StartAir(UINT8 StrNumber)
{
	SendByte(1,StrNumber);
}

//StopAir - stops airtext display
void AirText::StopAir(void)
{
	SendByte(2,1);
}

void AirText::SetDelay_uS(UINT16 uSec)
{
	UINT8 highByte, lowByte;
	lowByte = (UINT8)(uSec & 0x00FF);
	highByte = (UINT8)((uSec&0XFF00)>>8);
	SendByte(10, highByte);
	SendByte(11, lowByte);
}

void AirText::SetDelay_mS(UINT8 mSec)
{
	SendByte(12, mSec);
}

void AirText::SetRepetitions(UINT8 nReps)
{
	SendByte(3,nReps);
}

void AirText::SendByte(UINT8 kRegister, UINT8 b)
{
	m_i2c->Write(kRegister,b);
}

void AirText::print(UINT8 row, char *format, ...)
{
	va_list args;
	char buffer[40];
	
	if(row>kMAX_NUM_STR-1)
	{
		printf("ERROR:  AirText::print - row > kMAX_NUM_STR-1\n");
		return;
	}

	int nc;
	
	va_start(args, format);
	nc = vsprintf(buffer,format,args);
	va_end(args);
	
	
	SendByte(50,row);	// Select which string to write to
	
	// limit the maximum length to write to the line
	if(nc>kMAX_LINE_LEN)
		nc = kMAX_LINE_LEN;
	for(int i=0;i<nc;++i){
		// convert lower case to upper case since our charmaps have no lower case
		if (buffer[i]>=97 && buffer[i]<=122)//a-z
		            buffer[i]-=32;
		SendByte(51,buffer[i]);
	}
			
}

void AirText::AirTextUpdate(void)
{
//	for(int row = 0; row<kLCD_PHY_LINE; row++)
//	{
//		setCursor(0,row);
//		for(int col = 0; col<kLCD_LINE_LEN; col++)
//			SendByte(2,LCDScrnBuf[row+m_Cur_Line_Index][col]);
//	}
}
