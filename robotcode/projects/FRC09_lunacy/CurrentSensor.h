/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2008. All Rights Reserved.							  */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in $(WIND_BASE)/WPILib.  */
/*----------------------------------------------------------------------------*/

#ifndef CURRENT_SENSOR_H_
#define CURRENT_SENSOR_H_

#include "AnalogChannel.h"
#include "DigitalOutput.h"
#include "SensorBase.h"
#include "Timer.h"

/** 
 * Handle operation of the CurrentSensor.
 * The CurrentSensor reads acceleration directly through the sensor. Many sensors have
 * multiple axis and can be treated as multiple devices. Each is calibrated by finding
 * the center value over a period of time.
 */
class CurrentSensor : public SensorBase {
public:
	explicit CurrentSensor(UINT32 channel);
	CurrentSensor(UINT32 slot, UINT32 channel);
	explicit CurrentSensor(AnalogChannel *channel);
	virtual ~CurrentSensor();

	void Measure();
	float GetCurrent();
	void SetSensitivity(float sensitivity);
	void SetZero(float zero);
	
	void ResetPulse();

private:
	AnalogChannel *m_analogChannel;
	DigitalOutput *m_resetChannel;
	float m_voltsPerAmp;
	float m_zeroAmpVoltage;
	bool m_allocatedChannel;
	float m_value;
	double m_time;
	Timer m_timer;
};

#endif
