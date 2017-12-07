#include "Compass.h"
#include "DigitalModule.h"
#include "I2C.h"

#include "WPIStatus.h"



#include <stdio.h>

Compass::Compass(UINT32 slot)
	: m_i2c (NULL)
	{
		DigitalModule *module = DigitalModule::GetInstance(slot);
		m_i2c = module->GetI2C(kAddress);
		
	
		
	}

Compass::~Compass()
{
	delete m_i2c;
	m_i2c = NULL;
}

float Compass::GetBearing(void)
{
	UINT8 HByte;
	UINT8 LByte;
	
	m_i2c->Read(2,sizeof(HByte),&HByte);
	m_i2c->Read(3,sizeof(LByte),&LByte);
	
	float bearing = (float)(((int)HByte<<8)+LByte)/10.;

	return bearing;
}

