//Author: David Liu (2009,2010)

#ifndef LRT_ITERATIVE_ROBOT_H_
#define LRT_ITERATIVE_ROBOT_H_

#include "Timer.h"
#include "RobotBase.h"

/**
 * IterativeRobot implements a specific type of Robot Program framework, extending the RobotBase class.
 * 
 * The IterativeRobot class is intended to be subclassed by a user creating a robot program.
 * 
 * This class is intended to implement the "old style" default code, by providing
 * the following functions which are called by the main loop, StartCompetition(), at the appropriate times:
 * 
 * RobotInit() -- provide for initialization at robot power-on
 * 
 * Init() functions -- each of the following functions is called once when the
 *                     appropriate mode is entered:
 *  - DisabledInit()   -- called only when first disabled
 *  - AutonomousInit() -- called each and every time autonomous is entered from another mode
 *  - TeleopInit()     -- called each and every time teleop is entered from another mode
 * 
 * Periodic() functions -- each of these functions is called iteratively at the
 *                         appropriate periodic rate (aka the "slow loop").  The default period of
 *                         the iterative robot is 0.005 seconds, giving a periodic frequency
 *                         of 200Hz (200 times per second).
 *   - DisabledPeriodic()
 *   - AutonomousPeriodic()
 *   - TeleopPeriodic()
 * 
 * Continuous() functions -- each of these functions is called repeatedly as
 *                           fast as possible:
 *   - DisabledContinuous()
 *   - AutonomousContinuous()
 *   - TeleopContinuous()
 * 
 */

class LRTIterativeRobot : public RobotBase {
public:
//	static const double kDefaultPeriod = 5e-3; /** default period for periodic functions **/
//	static const double kDefaultPeriod = 1/50.; /** default period for periodic functions **/
	virtual void StartCompetition();

	virtual void RobotInit();
	virtual void DisabledInit();
	virtual void AutonomousInit();
	virtual void TeleopInit();

	virtual void CommonPeriodic();
	virtual void DisabledPeriodic();
	virtual void AutonomousPeriodic();
	virtual void TeleopPeriodic();
	virtual void EnabledPeriodic();

//	void SetPeriod(double period);
//	double GetLoopsPerSec();
//	int Ticks(double seconds);

protected:
	virtual ~LRTIterativeRobot();
	LRTIterativeRobot();
	
	UINT32 m_commonLoops;
	UINT32 m_disabledLoops;
	UINT32 m_autonomousLoops;
	UINT32 m_teleopLoops;
	
	double m_minCycleTime, m_maxCycleTime;

//private:
	bool NextPeriodReady();

	bool m_disabledInitialized;
	bool m_autonomousInitialized;
	bool m_teleopInitialized;
	int packetsMissedInLifetime;
	
//	double m_period;
//	double m_frequency;

//	Timer m_mainLoopTimer;
};

#endif

