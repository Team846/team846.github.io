//Author: David Liu (2010)

#ifndef CLOSEDLOOPSPEEDCONTROLLER_H_
#define CLOSEDLOOPSPEEDCONTROLLER_H_

#include "Encoder.h"
#include "SpeedController.h"
#include "PIDController.h"
#include "PIDOutput.h"
#include "PIDSource.h"

class ClosedLoopSpeedController : public SpeedController, public PIDOutput, public PIDSource {
private:
	SpeedController &m_esc;
	CounterBase &m_encoder;
	float m_maxRate;
	float m_setPoint;
	bool m_isClosedLoopEnabled;

	PIDController m_controller;

public:
	ClosedLoopSpeedController(SpeedController& m_esc, CounterBase& encoder, float maxRate,
			bool isClosedLoopEnabled, float p, float i, float d);
	virtual ~ClosedLoopSpeedController();

	virtual void Set(float speed);
	virtual float Get();
	
	void SetClosedLoopEnabled(bool enabled);
	bool IsClosedLoopEnabled();
	
	PIDController& GetPIDController();
	
	float GetMaxRate();
	void SetMaxRate(float maxRate);

	void SetPGain(float pGain);
	
	SpeedController& GetSpeedController();
	CounterBase& GetEncoder();

	virtual void PIDWrite(float correction);
	virtual double PIDGet();
};
#endif /*CLOSEDLOOPSPEEDCONTROLLER_H_*/
