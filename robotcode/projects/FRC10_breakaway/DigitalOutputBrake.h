//Author: David Liu (2009)

#ifndef DIGITAL_OUTPUT_BRAKE_H_
#define DIGITAL_OUTPUT_BRAKE_H_

#include "WPILib.h"
#include "Brake.h"

class DigitalOutputBrake : public Brake {
	DigitalOutput m_brakeLine;
	int m_cycleCount;
	int m_amount;
	
public:
	DigitalOutputBrake(UINT32 channel);
	virtual void Set(int val8); // between 0 and 8
	virtual void SetBrake() {
		Set(8);
	}
	virtual void SetCoast() {
		Set(0);
	}
	virtual void UpdateOutput();
};

#endif /* DIGITAL_OUTPUT_BRAKE_H_ */
