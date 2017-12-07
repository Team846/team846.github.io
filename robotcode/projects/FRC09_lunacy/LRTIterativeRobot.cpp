/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2008. All Rights Reserved.							  */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in $(WIND_BASE)/WPILib.  */
/*----------------------------------------------------------------------------*/

#include "LRTIterativeRobot.h"
#include "DriverStation.h"

/**
 * Constructor for RobotIterativeBase
 * 
 * The constructor initializes the instance variables for the robot to indicate
 * the status of initialization for disabled, autonomous, and teleop code.
 */
LRTIterativeRobot::LRTIterativeRobot() {
	printf("RobotIterativeBase Constructor Start\n");
	// set status for initialization of disabled, autonomous, and teleop code.
	m_disabledInitialized = m_autonomousInitialized = m_teleopInitialized = false;

	// keep track of the number of continuous loops performed per period
	m_disabledLoops = 0;
	m_autonomousLoops = 0;
	m_teleopLoops = 0;

	SetPeriod(kDefaultPeriod);

	ds = DriverStation::GetInstance();

	// Start the timer for the main loop
	m_mainLoopTimer.Start();

	printf("RobotIterativeBase Constructor Finish\n");
}

/**
 * Free the resources for a RobotIterativeBase class.
 */
LRTIterativeRobot::~LRTIterativeRobot() {
}


/**
 * Set the period for the periodic functions.
 * 
 * The period is set in seconds for the length of time between calls to the
 * periodic functions.  Default period is 0.005 seconds (200Hz iteration loop).
 */
void LRTIterativeRobot::SetPeriod(double period) {
	m_period = period;
	m_frequency = 1/period;
	m_freqInt = (int)m_frequency;
}

/**
 * Get the number of loops per second for the IterativeRobot
 * 
 * Get the number of loops per second for the IterativeRobot.  The default period of
 * 0.005 seconds results in 200 loops per second.  (200Hz iteration loop).
 */
double LRTIterativeRobot::GetLoopsPerSec() {
	return m_frequency;
}

/**
 * Provide an alternate "main loop" via StartCompetition().
 * 
 * This specific StartCompetition() implements "main loop" behavior like that of the FRC
 * control system in 2008 and earlier, with a primary (slow) loop that is
 * called periodically, and a "fast loop" (a.k.a. "spin loop") that is 
 * called as fast as possible with no delay between calls. 
 */
void LRTIterativeRobot::StartCompetition() {
	printf("RobotIterativeBase StartCompetition() Commenced\n");

	// first and one-time initialization
	RobotInit();
	
	// Timing Test code
//	Timer timer;
//	timer.Start();
//	int loopTime = 0;
//	while(true) {
//		if (NextPeriodReady()) {
//			++loopTime;
//			timer.Reset();
//			CommonPeriodic();
//			double commonPTime = timer.Get();
//			if ((loopTime % 200) == 0) {
//				printf("Loop Time: %d\n", loopTime/200);
//				printf("CommonPeriodic Time: %f\n", commonPTime);
//			}
//		}
//	}

	// loop forever, calling the appropriate mode-dependent function
	while (true) {
		// Call the appropriate function depending upon the current robot mode
		if (IsDisabled()) {
			// call DisabledInit() if we are now just entering disabled mode from
			// either a different mode or from power-on
			if (!m_disabledInitialized) {
				DisabledInit();
				m_disabledInitialized = m_autonomousInitialized = m_teleopInitialized = false;
				m_disabledInitialized = true;
				printf("Disabled_Init() completed\n");
			}
			
			if (NextPeriodReady()) {
				CommonPeriodic();
				DisabledPeriodic();
				if (IsNewDataAvailable()) {
					CommonNewData();
					DisabledNewData();
				}
			}
			CommonContinuous();
			DisabledContinuous();
			
			
		} else if (IsAutonomous() || IsAutonomousExtended()) {
			// call AutonomousInit() if we are now just entering autonomous mode from
			// either a different mode or from power-on
			if (!m_autonomousInitialized) {
				// KBS NOTE:  old code reset all PWMs and relays to "safe values"
				// whenever entering autonomous mode, before calling
				// "Autonomous_Init()"
				if (!IsAutonomousExtended()) {
					AutonomousInit();
					m_disabledInitialized = m_autonomousInitialized = m_teleopInitialized = false;
					printf("Autonomous_Init() completed\n");
				}
				m_autonomousInitialized = true;
			}
			if (NextPeriodReady()) {
				CommonPeriodic();
				AutonomousPeriodic();
				if (IsNewDataAvailable()) {
					CommonNewData();
					AutonomousNewData();
				}
			}
			CommonContinuous();
			AutonomousContinuous();
			
			
		} else {
			// call TeleopInit() if we are now just entering teleop mode from
			// either a different mode or from power-on
			if (!m_teleopInitialized) {
				TeleopInit();
				m_disabledInitialized = m_autonomousInitialized = m_teleopInitialized = false;
				m_teleopInitialized = true;
				printf("Teleop_Init() completed\n");
			}
			if (NextPeriodReady()) {
				CommonPeriodic();
				TeleopPeriodic();
				if (IsNewDataAvailable()) {
					CommonNewData();
					TeleopNewData();
				}
			}
			CommonContinuous();
			TeleopContinuous();
		}
	}
	printf("RobotIterativeBase StartCompetition() Ended\n");
}

/**
 * Determine if the appropriate next periodic function should be called.
 * 
 * This function makes adjust for lateness in the cycle by keeping track of how late
 * it was and crediting the next period with that amount.
 */
//TODO: decide what this should do if it slips more than one cycle.

bool LRTIterativeRobot::NextPeriodReady() {
//	static double adjustment = 0.0;
//	double elapsed_time = m_mainLoopTimer.Get();
//
//	if ((elapsed_time + adjustment) >= m_period) {
//		// immediately reset the timer and calculate the next adjustment
//		m_mainLoopTimer.Reset();
//		adjustment = (elapsed_time + adjustment) - m_period;
//		// check for slippage of more than one cycle
//		// TODO:  Should really notify the user somehow when this happens
//		if (adjustment > m_period) {
//			adjustment = 0.0;
////			printf("[!] lost cycle\n");
//		}
//		return true;
//	}
//	return false;
	static double nextTimeToRun = 0.0;
	long periodNumber;
	static long expectedPeriodNumber=0;
	static int timer=0;
	const double currentTime = m_mainLoopTimer.Get();	//Can't we use the FPGA timer? [DG]
	if(currentTime < nextTimeToRun)
		return false;
	periodNumber = (long) (currentTime / m_period);

	if (1 && --timer <= 0)
	{
		timer=200;
//		printf("periodNumber=%ld; %10.6f sec\n", periodNumber, nextTimeToRun);
	}	
	
	if (expectedPeriodNumber && periodNumber - expectedPeriodNumber>0)
	{
		printf("[!]Lost %ld Cycles after #%ld\n",	periodNumber - expectedPeriodNumber, expectedPeriodNumber );
	}
	++periodNumber;  //Get time of next period
	nextTimeToRun = periodNumber * m_period;
	expectedPeriodNumber = periodNumber;

	return true;
}

/**
 * Robot-wide initialization code should go here.
 * 
 * Users should override this method for default Robot-wide initialization which will
 * be called when the robot is first powered on.  It will be called exactly 1 time.
 */
void LRTIterativeRobot::RobotInit() {
	printf("Default RobotIterativeBase::RobotInit() method running\n");
}

/**
 * Initialization code for disabled mode should go here.
 * 
 * Users should override this method for initialization code which will be called each time
 * the robot enters disabled mode.
 */
void LRTIterativeRobot::DisabledInit() {
	printf("Default RobotIterativeBase::DisabledInit() method running\n");
}

/**
 * Initialization code for autonomous mode should go here.
 * 
 * Users should override this method for initialization code which will be called each time
 * the robot enters autonomous mode.
 */
void LRTIterativeRobot::AutonomousInit() {
	printf("Default RobotIterativeBase::AutonomousInit() method running\n");
}

/**
 * Initialization code for teleop mode should go here.
 * 
 * Users should override this method for initialization code which will be called each time
 * the robot enters teleop mode.
 */
void LRTIterativeRobot::TeleopInit() {
	printf("Default RobotIterativeBase::TeleopInit() method running\n");
	
}

/**
 * Periodic code for disabled mode should go here.
 * 
 * Users should override this method for code which will be called periodically at a regular
 * rate while the robot is in disabled mode.
 */
void LRTIterativeRobot::DisabledPeriodic() {
//	printf("D - %d\n", m_disabledLoops);
	m_disabledLoops = 0;
}

/**
 * Periodic code for autonomous mode should go here.
 *
 * Users should override this method for code which will be called periodically at a regular
 * rate while the robot is in autonomous mode.
 */
void LRTIterativeRobot::AutonomousPeriodic() {
//	printf("A - %d\n", m_autonomousLoops);
	m_autonomousLoops = 0;
}

/**
 * Periodic code for teleop mode should go here.
 *
 * Users should override this method for code which will be called periodically at a regular
 * rate while the robot is in teleop mode.
 */
void LRTIterativeRobot::TeleopPeriodic() {
//	printf("T - %d", m_teleopLoops);
	m_teleopLoops = 0;
}

/**
 * Continuous code for disabled mode should go here.
 *
 * Users should override this method for code which will be called repeatedly as frequently
 * as possible while the robot is in disabled mode.
 */
void LRTIterativeRobot::DisabledContinuous() {
}

/**
 * Continuous code for autonomous mode should go here.
 *
 * Users should override this method for code which will be called repeatedly as frequently
 * as possible while the robot is in autonomous mode.
 */
void LRTIterativeRobot::AutonomousContinuous() {
}

/**
 * Continuous code for teleop mode should go here.
 *
 * Users should override this method for code which will be called repeatedly as frequently
 * as possible while the robot is in teleop mode.
 */
void LRTIterativeRobot::TeleopContinuous() {
}


void LRTIterativeRobot::CommonPeriodic() {
	printf("Default CommonPeriodic\n");
}
void LRTIterativeRobot::CommonNewData() {}
void LRTIterativeRobot::DisabledNewData() {}
void LRTIterativeRobot::AutonomousNewData() {}
void LRTIterativeRobot::TeleopNewData() {}
void LRTIterativeRobot::CommonContinuous() {}

/*
 * 
 * IsAutonomousExtended() used if we want  continue the autonomous routines in teleop.
 * If called, we are in TeleOp because IsAutonomous() returned false
 * Overriding to return true will cause teleop cycles to operate as autnomous.
 */
bool LRTIterativeRobot::IsAutonomousExtended(){ return false; }
int LRTIterativeRobot::Ticks(double seconds)
{
	return (int)m_frequency*seconds;
}
