/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2008. All Rights Reserved.							  */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in $(WIND_BASE)/WPILib.  */
/*----------------------------------------------------------------------------*/

#include "CurrentSensor.h"
#include "AnalogModule.h"
#include "Utility.h"
#include "WPIStatus.h"

/**
 * Create a new instance of an CurrentSensor.
 * 
 * The CurrentSensor is assumed to be in the first analog module in the given analog channel. The
 * constructor allocates desired analog channel.
 */
CurrentSensor::CurrentSensor(UINT32 senseChannel, UINT32 resetChannel) {
	printf("Init CurrentSensor Analog %d\n",senseChannel);
	m_analogChannel = new AnalogChannel(senseChannel);
	printf("Fin CurrentSensor Analog %d\n",senseChannel);
	printf("Init CurrentSensor resetChannel %d\n", resetChannel);
	m_resetChannel = new DigitalOutput(resetChannel);
	printf("Fin CurrentSensor resetChannel %d\n",resetChannel);
	
	m_allocatedChannel = true;
	m_value = 0;
	
	m_voltsPerAmp = 1.0;
	m_zeroAmpVoltage = 2.5;

	m_resetChannel->Set(true); // normally high
	
	m_timer.Start();
}

///**
// * Create new instance of CurrentSensor.
// * 
// * Make a new instance of the CurrentSensor given a module and channel. The constructor allocates
// * the desired analog channel from the specified module
// */
//CurrentSensor::CurrentSensor(UINT32 slot, UINT32 channel)
//{
//	m_analogChannel = new AnalogChannel(slot, channel);
//	m_allocatedChannel = true;
//	InitCurrentSensor();
//}

/**
 * Create a new instance of CurrentSensor from an existing AnalogChannel.
 * Make a new instance of CurrentSensor given an AnalogChannel. This is particularly
 * useful if the port is going to be read as an analog channel as well as through
 * the CurrentSensor class.
 */
//CurrentSensor::CurrentSensor(AnalogChannel *channel)
//{
//	if (channel == NULL)
//	{
//		wpi_fatal(NullParameter);
//	}
//	else
//	{
//		m_analogChannel = channel;
//		InitCurrentSensor();
//	}
//	m_allocatedChannel = false;
//}

/**
 * Delete the analog components used for the CurrentSensor.
 */
CurrentSensor::~CurrentSensor() {
	if (m_allocatedChannel)
	{
		delete m_analogChannel;
	}
}

void CurrentSensor::Measure() {
	m_value = m_analogChannel->GetVoltage();
//	m_value = m_value - m_zeroAmpVoltage) / m_voltsPerAmp;
	m_time = m_timer.Get();
	ResetPulse();
	m_timer.Reset();
	
//	m_value = m_value / m_time * 0.005;
}

/**
 * Return the current in Amps.
 * 
 * The current is returned in units of Amps.
 * 
 * @return The current sensed, in Amps.
 */
float CurrentSensor::GetCurrent() {
	return m_value;
}

/**
 * Set the CurrentSensor sensitivity.
 * 
 * This sets the sensitivity of the CurrentSensor used for calculating the current.
 * The sensitivity varys by CurrentSensor model. There are constants defined for various models.
 * 
 * @param sensitivity The sensitivity of CurrentSensor in Volts per Amp.
 */
void CurrentSensor::SetSensitivity(float sensitivity) {
	m_voltsPerAmp = sensitivity;
}

/**
 * Set the voltage that corresponds to 0 Amps.
 * 
 * The zero G voltage varys by CurrentSensor model. There are constants defined for various models.
 * 
 * @param zero The zero G voltage.
 */
void CurrentSensor::SetZero(float zero) {
	m_zeroAmpVoltage = zero;
}

/**
 * Send a reset pulse
 */
void CurrentSensor::ResetPulse() {
//	m_resetChannel->Pulse(0.000013);
	m_resetChannel->Pulse(20e-6);
	
	// Note: DigitalOutput::Pulse has a resolution of about 6.7 us.
	// From testing:
	// 0-13us input => 6.7us pulse
	// 14-?us input => 13us pulse
	// 20us input => 20us
	
//	// send low pulse
//	m_resetChannel->Set(false);
//	taskDelay(1);
//	m_resetChannel->Set(true);
}
