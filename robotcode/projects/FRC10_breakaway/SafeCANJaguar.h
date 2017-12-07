
#ifndef SAFECANJAGUAR_H
#define SAFECANJAGUAR_H

#include "CANJaguar/CANJaguar.h"
#include "WPILib.h"
#include "AsynchronousPrinter.h"

/**
 * Luminary Micro Jaguar Speed Control
 */
class SafeCANJaguar : public CANJaguar
{
public:
	explicit SafeCANJaguar(UINT8 deviceNumber)
	: CANJaguar(deviceNumber)
	, m_isEnabled(true)
	{
		CheckConnectivity();
	}
	virtual ~SafeCANJaguar(){}
	
	bool IsOnline() {
		float volts = GetBusVoltage();
		AsynchronousPrinter::Printf("CAN %d : %f\n", m_deviceNumber, volts);
		return volts > 2.0;
	}
	void CheckConnectivity() {
		m_isEnabled = true;
		m_isEnabled = IsOnline();
	}
	
	virtual void setTransaction(UINT32 messageID, const UINT8 *data, UINT8 dataSize)
	{
		if (m_isEnabled) {
			CANJaguar::setTransaction(messageID, data, dataSize);
		} else {
//			AsynchronousPrinter::Printf("skip set on CAN#%d\n", m_deviceNumber);
			// ne faire rien...
		}
	}

	virtual void getTransaction(UINT32 messageID, UINT8 *data, UINT8 *dataSize)
	{
		if (m_isEnabled) {
			CANJaguar::getTransaction(messageID, data, dataSize);
		} else {
//			AsynchronousPrinter::Printf("skip get on CAN#%d\n", m_deviceNumber);
			// ne faire rien...
		}
	}

private:
	bool m_isEnabled;
};
#endif

