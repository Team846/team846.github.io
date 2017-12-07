#ifndef BRAKE_H
#define BRAKE_H
#include "WPILib.h"

class Brake {
	DigitalOutput m_brakeLine;
	int m_cycleCount;
	int m_amount;
	
public:
	Brake(UINT32 output);
	void Set(int val8); // between 0 and 8
	void SetBrake() {
		Set(8);
	}
	void SetCoast() {
		Set(0);
	}
	void UpdateOutput();
};

#endif
