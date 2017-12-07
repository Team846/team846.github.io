/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2008. All Rights Reserved.							  */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in $(WIND_BASE)/WPILib.  */
/*----------------------------------------------------------------------------*/

//#include "LRTPIDController.h"
#include "Notifier.h"
//#include "PIDSource.h"
//#include "PIDOutput.h"
#include <math.h>
#include "Synchronized.h"
#include "AsynchronousPrinter.h"
#include "LRTDriveTrain.h"
/**
 * Allocate a PID object with the given constants for P, I, D
 * @param Kp the proportional coefficient
 * @param Ki the integral coefficient
 * @param Kd the derivative coefficient
 * @param source The PIDSource object that is used to get values
 * @param output The PIDOutput object that is set to the output value
 * @param period the loop time for doing calculations. This particularly effects calculations of the
 * integral and differental terms. The default is 50ms.
 */
LRTPIDController::LRTPIDController(float Kp, float Ki, float Kd,
									LRTDriveEncoders* source,
									//LRTDriveSpeedControllers* out,
									void* out,
									bool isVel, float period ) :
	m_semaphore (0)
{
	m_semaphore = semBCreate(SEM_Q_PRIORITY, SEM_FULL);

	m_controlLoop = new Notifier(LRTPIDController::CallCalculate, this);
	
	
	m_P = Kp;
	m_I = Ki;
	m_D = Kd;
	m_maximumOutput = 1.0;
	m_minimumOutput = -1.0;
	
	m_maximumInput = 0;
	m_minimumInput = 0;
	
	m_continuous = false;
	m_enabled = false;
	m_setpoint = 0;

	m_prevError = 0;
	m_totalError = 0;
	m_tolerance = .05;

	m_result = 0;
	
//	m_pidInput = source;
//	m_pidOutput = output;
	m_source = source;
	m_out = out;
	m_period = period;

	m_controlLoop->StartPeriodic(m_period);
}

/**
 * Free the PID object
 */
LRTPIDController::~LRTPIDController()
{
	semFlush(m_semaphore);
	delete m_controlLoop;
}

/**
 * Call the Calculate method as a non-static method. This avoids having to prepend
 * all local variables in that method with the class pointer. This way the "this"
 * pointer will be set up and class variables can be called more easily.
 * This method is static and called by the Notifier class.
 * @param controller the address of the PID controller object to use in the background loop
 */
void LRTPIDController::CallCalculate(void *controller)
{
	LRTPIDController *control = (LRTPIDController*) controller;
	control->Calculate();
}

 /**
  * Read the input, calculate the output accordingly, and write to the output.
  * This should only be called by the Notifier indirectly through CallCalculate
  * and is created during initialization.
  */	
void LRTPIDController::Calculate()
{
	AsynchronousPrinter::Printf("PID\n");
	CRITICAL_REGION(m_semaphore)
	{
	if (m_source == 0)
		return;
	if (m_out == 0)
		return;
//	float input = m_pidInput->PIDGet();
	float input = m_isVel?m_source->GetNormalizedRate():m_source->GetTurningSpeedPercent();
	if (m_enabled)
	{
		m_error = m_setpoint - input;
		if (m_continuous)
		{
			if (fabs(m_error) > 
				(m_maximumInput - m_minimumInput) / 2)
			{
				if (m_error > 0)
					m_error = m_error  - m_maximumInput + m_minimumInput;
				else
					m_error = m_error  +
					m_maximumInput - m_minimumInput;
			}
		}

		if (((m_totalError + m_error) * m_I < m_maximumOutput) &&
				((m_totalError + m_error) * m_I > m_minimumOutput))
			m_totalError += m_error;

				
		m_result = m_P * m_error + m_I * m_totalError + m_D * (m_error - m_prevError);
		m_prevError = m_error;
		
		if (m_result > m_maximumOutput)
			m_result = m_maximumOutput;
		else if (m_result < m_minimumOutput)
			m_result = m_minimumOutput;
		
//		m_pidOutput->PIDWrite(m_result);
		//m_isVel?(LRTDriveTrain*)m_out->SetVel(m_result, m_source):(LRTDriveTrain*)m_out->SetTurn(m_result, m_source);
		LRTDriveTrain* o = (LRTDriveTrain* ) m_out;
		if (m_isVel)
		{
			o->SetVel(m_result);
		}
		else
		{
			o->SetTurn(m_result);
		}
	}
	}
	END_REGION;
}


/**
 * Set the PID Controller gain parameters.
 * Set the proportional, integral, and differential coefficients.
 * @param p Proportional coefficient
 * @param i Integral coefficient
 * @param d Differential coefficient
 */
void LRTPIDController::SetPID(float p, float i, float d)
{
	CRITICAL_REGION(m_semaphore)
	{
		m_P = p;
		m_I = i;
		m_D = d;
	}
	END_REGION;
}

/**
 * Get the Proportional coefficient
 * @return proportional coefficient
 */
float LRTPIDController::GetP()
{
	CRITICAL_REGION(m_semaphore)
	{
		return m_P;
	}
	END_REGION;
}

/**
 * Get the Integral coefficient
 * @return integral coefficient
 */
float LRTPIDController::GetI()
{
	CRITICAL_REGION(m_semaphore)
	{
		return m_I;
	}
	END_REGION;
}

/**
 * Get the Differential coefficient
 * @return differential coefficient
 */
float LRTPIDController::GetD()
{
	CRITICAL_REGION(m_semaphore)
	{
		return m_D;
	}
	END_REGION;
}

/**
 * Return the current PID result
 * This is always centered on zero and constrained the the max and min outs
 * @return the latest calculated output
 */
float LRTPIDController::Get()
{
	float result;
	CRITICAL_REGION(m_semaphore)
	{
		result = m_result;
	}
	END_REGION;
	return result;
}

/**
 *  Set the PID controller to consider the input to be continuous,
 *  Rather then using the max and min in as constraints, it considers them to
 *  be the same point and automatically calculates the shortest route to
 *  the setpoint.
 * @param continuous Set to true turns on continuous, false turns off continuous
 */
void LRTPIDController::SetContinuous(bool continuous)
{
	CRITICAL_REGION(m_semaphore)
	{
		m_continuous = continuous;
	}
	END_REGION;

}

/**
 * Sets the maximum and minimum values expected from the input.
 * 
 * @param minimumInput the minimum value expected from the input
 * @param maximumInput the maximum value expected from the output
 */
void LRTPIDController::SetInputRange(float minimumInput, float maximumInput)
{
	CRITICAL_REGION(m_semaphore)
	{
		m_minimumInput = minimumInput;
		m_maximumInput = maximumInput;	
	}
	END_REGION;

	SetSetpoint(m_setpoint);
}

/**
 * Sets the minimum and maximum values to write.
 * 
 * @param minimumOutput the minimum value to write to the output
 * @param maximumOutput the maximum value to write to the output
 */
void LRTPIDController::SetOutputRange(float minimumOutput, float maximumOutput)
{
	CRITICAL_REGION(m_semaphore)
	{
		m_minimumOutput = minimumOutput;
		m_maximumOutput = maximumOutput;
	}
	END_REGION;
}

/**
 * Set the setpoint for the PIDController
 * @param setpoint the desired setpoint
 */
void LRTPIDController::SetSetpoint(float setpoint)
{
	CRITICAL_REGION(m_semaphore)
	{
		if (m_maximumInput > m_minimumInput) {
			if (setpoint > m_maximumInput)
				m_setpoint = m_maximumInput;
			else if (setpoint < m_minimumInput)
				m_setpoint = m_minimumInput;
			else
				m_setpoint = setpoint;
		}
		else
			m_setpoint = setpoint;
	}
	END_REGION;	
}

/**
 * Returns the current setpoint of the PIDController
 * @return the current setpoint
 */
float LRTPIDController::GetSetpoint()
{
	float setpoint;
	CRITICAL_REGION(m_semaphore)
	{
		setpoint = m_setpoint;
	}
	END_REGION;
	return setpoint;
}

/**
 * Retruns the current difference of the input from the setpoint
 * @return the current error
 */
float LRTPIDController::GetError()
{
	float  error;
	CRITICAL_REGION(m_semaphore)
	{
		error = m_error;
	}
	END_REGION;
	return error;
}

/*
 * Set the percentage error which is considered tolerable for use with
 * OnTarget.
 * @param percentage error which is tolerable
 */
void LRTPIDController::SetTolerance(float percent)
{
	CRITICAL_REGION(m_semaphore)
	{
		m_tolerance = percent;
	}
	END_REGION;
}

/*
 * Return true if the error is within the percentage of the total input range,
 * determined by SetTolerance. This asssumes that the maximum and minimum input
 * were set using SetInput.
 */
bool LRTPIDController::OnTarget()
{
	bool temp;
	CRITICAL_REGION(m_semaphore)
	{
		temp = (fabs(m_error)<m_tolerance / 100 * 
				(m_maximumInput - m_minimumInput));
	}
	END_REGION;
	return temp;
}

/**
 * Begin running the PIDController
 */
void LRTPIDController::Enable()
{
	CRITICAL_REGION(m_semaphore)
	{
			
		m_enabled = true;
	}
	END_REGION;	
}
/**
 * Stop running the PIDController, this sets the output to zero before stopping.
 */
void LRTPIDController::Disable()
{

	CRITICAL_REGION(m_semaphore)
	{
		//m_pidOutput->PIDWrite(0);
		LRTDriveTrain* o = (LRTDriveTrain*)m_out;
		m_isVel?o->SetVel(0):o->SetTurn(0);
		m_enabled = false;
	}
	END_REGION;
}

/**
 * Reset the previous error,, the integral term, and disable the controller.
 */
void LRTPIDController::Reset()
{
	Disable();
	
	CRITICAL_REGION(m_semaphore)
	{
		m_prevError = 0;
		m_totalError = 0;
		m_result = 0;
	}
	END_REGION;
}


