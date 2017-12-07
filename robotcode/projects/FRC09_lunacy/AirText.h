#ifndef _AIRTEXT_h_
#define _AIRTEXT_h_

#include "SensorBase.h"
#include "Scroll.h"
#include "I2C.h"

class AirText: public SensorBase
{
public:
	explicit AirText(UINT32 slot);
	virtual ~AirText();
	void SendByte(UINT8 kRegister, UINT8 b);
	void Clear();

	void print(UINT8 row, char *format, ...);
	void SetDelay_uS(UINT16 uSec);
	void SetDelay_mS(UINT8 mSec);
	
	void SetRepetitions(UINT8 nReps);
	void StartAir(UINT8 StrNumber);
	void StopAir(void);
	
	void AirTextUpdate(void);
	
private:
	
	static const UINT8 kAddress = 12;\
	
	static const UINT8 kMAX_LINE_LEN = 20;
	static const UINT8 kMAX_NUM_STR = 10;
	
	I2C *m_i2c;
};

#endif
