//Author: David Liu

#include "LRTIterativeRobot.h"
#include "DriverStation.h"
#include "Timer.h"
#include "LRTUtil.h"
#include "Profiler.h"
#include "AsynchronousPrinter.h"
#include <algorithm>

/**
 * Constructor for RobotIterativeBase
 * 
 * The constructor initializes the instance variables for the robot to indicate
 * the status of initialization for disabled, autonomous, and teleop code.
 */
LRTIterativeRobot::LRTIterativeRobot()
	: m_commonLoops(0)
	, m_disabledLoops (0)
	, m_autonomousLoops (0)
	, m_teleopLoops (0)
	, m_minCycleTime(1e6)
	, m_maxCycleTime(0)
	, m_disabledInitialized (false)
	, m_autonomousInitialized (false)
	, m_teleopInitialized (false)
{
	AsynchronousPrinter::AsynchronousPrinter::Printf("LRTIterativeRobot Constructor Start\n");

	AsynchronousPrinter::Printf("LRTIterativeRobot Constructor Finish\n");
}

/**
 * Free the resources for a RobotIterativeBase class.
 */
LRTIterativeRobot::~LRTIterativeRobot() {
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
	AsynchronousPrinter::Printf("LRTIterativeRobot StartCompetition() Commenced\n");

	// first and one-time initialization
	RobotInit();
	
	GetWatchdog().SetExpiration(0.5); // seconds
//	GetWatchdog().SetEnabled(true);
	GetWatchdog().SetEnabled(false);

	const int reportPeriod = 50*5;
	double cycleWaitTimes[reportPeriod], cycleRunTimes[reportPeriod];
//	double profilerTimes[reportPeriod];
	int cycleIndex = 0;
	int packetsMissedInLastReportPeriod = 0;
	packetsMissedInLifetime = 0;
	UINT32 lastPacketNumber = 0;
	bool veryFirstPacketInLifetime = true;
	
	// loop forever, calling the appropriate mode-dependent function
	while (true) {
		GetWatchdog().Feed();
		
		double waitStart = Timer::GetFPGATimestamp();

		while (!NextPeriodReady()) {
			Wait(0.0005); // allow other tasks to run... [dcl] experimental!!
		}
		UINT32 dsPacketNumber = m_ds->GetPacketNumber();
		if (veryFirstPacketInLifetime) {
			veryFirstPacketInLifetime = false;
		} else {
			packetsMissedInLastReportPeriod += dsPacketNumber - lastPacketNumber - 1;
		}
		lastPacketNumber = dsPacketNumber;
		
		double cycleStart = Timer::GetFPGATimestamp();
		Profiler::GetInstance().StartNewCycle();
//		double profilerTime = Timer::GetFPGATimestamp() - cycleStart;
		
		{ ProfiledSection pf("WholeCycle");
		
			// Call the appropriate function depending upon the current robot mode
			if (IsDisabled()) {
				if (!m_disabledInitialized) {
					DisabledInit();
					m_disabledLoops = 0;
					m_disabledInitialized = m_autonomousInitialized = m_teleopInitialized = false;
					m_disabledInitialized = true;
					AsynchronousPrinter::Printf("Disabled_Init() completed\n");
				}
				
				CommonPeriodic();
				DisabledPeriodic();
				++m_disabledLoops;
				++m_commonLoops;
				
			} else if (IsAutonomous()) {
				if (!m_autonomousInitialized) {
					AutonomousInit();
					m_autonomousLoops = 0;
					m_disabledInitialized = m_autonomousInitialized = m_teleopInitialized = false;
					m_autonomousInitialized = true;
					AsynchronousPrinter::Printf("Autonomous_Init() completed\n");
				}
				
				CommonPeriodic();
				EnabledPeriodic();
				AutonomousPeriodic();
				++m_autonomousLoops;
				++m_commonLoops;
				
			} else {
				if (!m_teleopInitialized) {
					TeleopInit();
					m_teleopLoops = 0;
					m_disabledInitialized = m_autonomousInitialized = m_teleopInitialized = false;
					m_teleopInitialized = true;
					AsynchronousPrinter::Printf("Teleop_Init() completed\n");
				}
				
				CommonPeriodic();
				EnabledPeriodic();
				TeleopPeriodic();
				++m_teleopLoops;
				++m_commonLoops;
			}
		}
		
		double cycleEnd = Timer::GetFPGATimestamp();

		double cycleRunTime = cycleEnd - cycleStart;
		double cycleWaitTime = cycleStart - waitStart;
		
		cycleRunTimes[cycleIndex] = cycleRunTime * 1000; //ms
		cycleWaitTimes[cycleIndex] = cycleWaitTime * 1000; //ms
//		profilerTimes[cycleIndex] = profilerTime * 1000; //ms
		++cycleIndex;
		if (cycleIndex == reportPeriod) {
			cycleIndex = 0;
			double runMin, runMax, runMean, waitMin, waitMax, waitMean;
			LRTUtil::minMaxMean<double>(cycleRunTimes, reportPeriod, runMin, runMax, runMean);
			LRTUtil::minMaxMean<double>(cycleWaitTimes, reportPeriod, waitMin, waitMax, waitMean);
//			double pfMin, pfMax, pfMean;
//			LRTUtil::minMaxMean<double>(profilerTimes, reportPeriod, pfMin, pfMax, pfMean);
			
			packetsMissedInLifetime += packetsMissedInLastReportPeriod;
			AsynchronousPrinter::Printf("RunTime:[%.2f-%.2f]ms,~%.2f ; Wait:[%.2f-%.2f],~%.2f ; MISS:%d pkts, (%d total)\n"
					, runMin, runMax, runMean
					, waitMin, waitMax, waitMean
					, packetsMissedInLastReportPeriod, packetsMissedInLifetime);
//			AsynchronousPrinter::Printf("ProfilerTime:[%.2f-%.2f]ms,~%.2f\n"
//					, pfMin, pfMax, pfMean);
			packetsMissedInLastReportPeriod = 0;
			
			m_minCycleTime = min(m_minCycleTime, runMin);
			m_maxCycleTime = max(m_maxCycleTime, runMax);
		}
	}
	
	AsynchronousPrinter::Printf("LRTIterativeRobot StartCompetition() Ended\n");
}

bool LRTIterativeRobot::NextPeriodReady() {
	return m_ds->IsNewControlData();
}

/**
 * Robot-wide initialization code should go here.
 * 
 * Users should override this method for default Robot-wide initialization which will
 * be called when the robot is first powered on.  It will be called exactly 1 time.
 */
void LRTIterativeRobot::RobotInit() {
	AsynchronousPrinter::Printf("Default RobotIterativeBase::RobotInit() method running\n");
}

/**
 * Initialization code for modes should go here.
 * 
 * Users should override this method for initialization code which will be called each time
 * the robot enters each mode.
 */
void LRTIterativeRobot::DisabledInit() {
	AsynchronousPrinter::Printf("Default RobotIterativeBase::DisabledInit() method running\n");
}
void LRTIterativeRobot::AutonomousInit() {
	AsynchronousPrinter::Printf("Default RobotIterativeBase::AutonomousInit() method running\n");
}
void LRTIterativeRobot::TeleopInit() {
	AsynchronousPrinter::Printf("Default RobotIterativeBase::TeleopInit() method running\n");
}

/**
 * Periodic code should go here.
 * 
 * Users should override this method for code which will be called periodically at a regular
 * rate while the robot.
 */
void LRTIterativeRobot::DisabledPeriodic() {
//	AsynchronousPrinter::Printf("D - %d\n", m_disabledLoops);
}
void LRTIterativeRobot::AutonomousPeriodic() {
//	AsynchronousPrinter::Printf("A - %d\n", m_autonomousLoops);
}
void LRTIterativeRobot::TeleopPeriodic() {
//	AsynchronousPrinter::Printf("T - %d", m_teleopLoops);
}

void LRTIterativeRobot::EnabledPeriodic() {
	AsynchronousPrinter::Printf("EnabledPeriodic\n");	
}

void LRTIterativeRobot::CommonPeriodic() {
	AsynchronousPrinter::Printf("Default CommonPeriodic\n");
}

