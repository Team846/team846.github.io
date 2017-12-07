#ifndef PROXIED_CAN_JAGUAR_H_
#define PROXIED_CAN_JAGUAR_H_

#include "SpeedController.h"
#include "CANBusController.h"
#include "CANJaguar/CANJaguar.h"
#include "WPILib.h"

class ProxiedCANJaguar : public SpeedController {
public:
	ProxiedCANJaguar(int channel);
	
	virtual void Set(float speed);
	virtual float Get();
	
	float GetCurrent();
	void ConfigNeutralMode(CANJaguar::NeutralMode mode);
	
private:
	CANBusController& m_controller;
	
	int m_channel;
};

#endif /* PROXIED_CAN_JAGUAR_H_ */
