//Author: Brandon Liu and David Liu (2010)

#ifndef CAN_JAGUAR_BRAKE_H_
#define CAN_JAGUAR_BRAKE_H_

#include "WPILib.h"
#include "Brake.h"
#include "CANJaguar\CANJaguar.h"
#include "ProxiedCANJaguar.h"

class CANJaguarBrake : public Brake {
	ProxiedCANJaguar& m_jaguar;
	int m_cycleCount;
	int m_amount;
	
public:
	CANJaguarBrake(ProxiedCANJaguar& jaggie);
	virtual void Set(int val8); // between 0 and 8
	virtual void SetBrake() {
		Set(8);
	}
	virtual void SetCoast() {
		Set(0);
	}
	virtual void UpdateOutput();
};

#endif /* CAN_JAGUAR_BRAKE_H_ */
