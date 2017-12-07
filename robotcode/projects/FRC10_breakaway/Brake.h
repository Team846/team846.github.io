//Author: Brandon Liu and David Liu (2009-2010)

#ifndef BRAKE_H_
#define BRAKE_H_
#include "WPILib.h"

class Brake {
	
public:
	virtual ~Brake() {}
	virtual void Set(int brakeAmount) = 0;
	virtual void SetBrake() = 0;
	virtual void SetCoast() = 0;
	virtual void UpdateOutput() = 0;
};

#endif /* BRAKE_H_ */
