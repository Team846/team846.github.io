/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2008. All Rights Reserved.							  */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in $(WIND_BASE)/WPILib.  */
/*----------------------------------------------------------------------------*/

#include "LRTgyro.h"
#include "AnalogChannel.h"
#include "AnalogModule.h"
#include "Timer.h"
#include "Utility.h"
#include "WPIStatus.h"


/**
 * Initialize the gyro.
 * Calibrate the gyro by running for a number of samples and computing the center value for this
 * part. Then use the center value as the Accumulator center value for subsequent measurements.
 * It's important to make sure that the robot is not moving while the centering calculations are
 * in progress, this is typically done when the robot is first turned on while it's sitting at
 * rest before the competition starts.
 */
void Gyro::InitGyro()
{
	if (!m_analog->IsAccumulatorChannel())
	{
		wpi_fatal(GyroNotAccumulatorChannel);
		if (m_channelAllocated)
		{
			delete m_analog;
			m_analog = NULL;
		}
		return;
	}
	
	printf("Gyro initialized in LRTgyro.cpp\n");

	m_voltsPerDegreePerSecond = kDefaultVoltsPerDegreePerSecond;
	m_analog->SetAverageBits(kAverageBits);
	m_analog->SetOversampleBits(kOversampleBits);
	float sampleRate = kSamplesPerSecond * 
		(1 << (kAverageBits + kOversampleBits));
	
	printf("Setting sample rate to:  %f\n",sampleRate);
	m_analog->GetModule()->SetSampleRate(sampleRate);
	Wait(1.0);
	
	
	printf("Confirming sample rate:  %f\n",m_analog->GetModule()->GetSampleRate());

	m_analog->InitAccumulator();
	Wait(kCalibrationSampleTime);

	INT64 value;
	UINT32 count;
	m_analog->GetAccumulatorOutput(&value, &count);
	
	UINT32 center = (UINT32)((float)value / (float)count + .5);

	m_offset = ((float)value / (float)count) - (float)center;
	
	printf("size of int = %d, size of long = %d\n",sizeof(int),sizeof(long));
	
	printf("value = %ld, count = %d, center = %d, m_offset = %f\n", (long)value, count, center, m_offset );
	
	m_analog->SetAccumulatorCenter(center);
//	m_analog->SetAccumulatorDeadband(0); ///< TODO: compute / parameterize this
	m_analog->SetAccumulatorDeadband((UINT32)(center/512.0*2+.5)); ///< TODO: compute / parameterize this
	m_analog->ResetAccumulator();
	
	
	printf("LSBWeight = %d, AverageBits = %d\n",  m_analog->GetLSBWeight(), m_analog->GetAverageBits());
	printf("2.5 V is %d, 2.5 mV is %d\n",m_analog->GetModule()->VoltsToValue(1,2.5),m_analog->GetModule()->VoltsToValue(1,.0025));
	
			
}

/**
 * Gyro constructor given a slot and a channel.
 * 
 * @param slot The cRIO slot for the analog module the gyro is connected to.
 * @param channel The analog channel the gyro is connected to.
 */
Gyro::Gyro(UINT32 slot, UINT32 channel)
{
	m_analog = new AnalogChannel(slot, channel);
	m_channelAllocated = true;
	InitGyro();
}

/**
 * Gyro constructor with only a channel.
 * 
 * Use the default analog module slot.
 * 
 * @param channel The analog channel the gyro is connected to.
 */
Gyro::Gyro(UINT32 channel)
{
	m_analog = new AnalogChannel(channel);
	m_channelAllocated = true;
	InitGyro();
}

/**
 * Gyro constructor with a precreated analog channel object.
 * Use this constructor when the analog channel needs to be shared. There
 * is no reference counting when an AnalogChannel is passed to the gyro.
 * @param channel The AnalogChannel object that the gyro is connected to.
 */
Gyro::Gyro(AnalogChannel *channel)
{
	m_analog = channel;
	m_channelAllocated = false;
	if (channel == NULL)
	{
		wpi_fatal(NullParameter);
	}
	else
	{
		InitGyro();
	}
}

Gyro::Gyro(AnalogChannel &channel)
{
	m_analog = &channel;
	m_channelAllocated = false;
	InitGyro();
}

/**
 * Reset the gyro.
 * Resets the gyro to a heading of zero. This can be used if there is significant
 * drift in the gyro and it needs to be recalibrated after it has been running.
 */
void Gyro::Reset()
{
	m_analog->ResetAccumulator();
}

/**
 * Delete (free) the accumulator and the analog components used for the gyro.
 */
Gyro::~Gyro()
{
	if (m_channelAllocated)
		delete m_analog;
}

/**
 * Return the actual angle in degrees that the robot is currently facing.
 * 
 * The angle is based on the current accumulator value corrected by the oversampling rate, the
 * gyro type and the A/D calibration values.
 * The angle is continuous, that is can go beyond 360 degrees. This make algorithms that wouldn't
 * want to see a discontinuity in the gyro output as it sweeps past 0 on the second time around.
 * 
 * @return the current heading of the robot in degrees. This heading is based on integration
 * of the returned rate from the gyro.
 */
float Gyro::GetAngle( void )
{
	INT64 rawValue;
	UINT32 count;
	m_analog->GetAccumulatorOutput(&rawValue, &count);

	INT64 value = rawValue - (INT64)((float)count * m_offset);

	double scaledValue = value * 1e-9 * (double)m_analog->GetLSBWeight() * (double)(1 << m_analog->GetAverageBits()) /
		(m_analog->GetModule()->GetSampleRate() * m_voltsPerDegreePerSecond);

	return (float)scaledValue;
}


/**
 * Set the gyro type based on the sensitivity.
 * This takes the number of volts/degree/second sensitivity of the gyro and uses it in subsequent
 * calculations to allow the code to work with multiple gyros.
 * 
 * @param voltsPerDegreePerSecond The type of gyro specified as the voltage that represents one degree/second.
 */
void Gyro::SetSensitivity( float voltsPerDegreePerSecond )
{
	m_voltsPerDegreePerSecond = voltsPerDegreePerSecond;
}

/**
 * Get the angle in degrees for the PIDSource base object.
 * 
 * @return The angle in degrees.
 */
double Gyro::PIDGet()
{
	return GetAngle();
}
