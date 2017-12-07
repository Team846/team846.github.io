#ifndef _COMPASS_h_
#define _COMPASS_h_

#include "SensorBase.h"

class I2C;

class Compass: public SensorBase
{
public:
	explicit Compass(UINT32 slot);
	virtual ~Compass();
	
	float GetBearing(void);
	
	
private:
	static const UINT8 kAddress = 0xC0;
	
	I2C *m_i2c;
};

#endif
