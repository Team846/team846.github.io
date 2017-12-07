#ifndef BRAIN_H_
#define BRAIN_H_

#include "..\General.h"
#include "..\ActionData.h"
#include "..\Util\Console.h"
#include "..\Util\LCD.h"
#include "..\Config\Config.h"
#include "..\DriverInputs\ProcessedInputs.h"
#include "..\Sensors\DriveEncoders.h"
#include "..\Sensors\LRTEncoder.h"
#include "..\Sensors\LineSensor.h"

class Brain
{
public:
    Brain();
    ~Brain();

    // called from the main loop
    void Process(GameState gameState);

private:
    Config& config;
    Console& console;
    LCD& lcd;
    DriverStation& ds;

    ActionData& action;
    GameState previousState;

    ProcessedInputs& inputs;
    Timer gameTimer;

#ifdef VIRTUAL
    VirtualLRTEncoder& leftEncoder;
    VirtualLRTEncoder& rightEncoder;
#else
    LRTEncoder& leftEncoder;
    LRTEncoder& rightEncoder;
#endif

    DriveEncoders& driveEncoders;

    LineSensor lineSensor;
    int prevLinePosition;

    bool firstReading;
    bool leftSide;

    bool isFinale;
    bool hasMoved;

    bool wasDisabledLastCycle;

    // Mode-specific methods
    void Common();
    void Disabled();

    void Auton();
    void Teleop();

    // Subroutines used in multiple modes
    void AutomatedRoutineWithLift();
    void AutomatedRoutines();
    void AutomatedFollowLine();

    // Teleop subroutines
    void TeleopDriveTrain();
    void TeleopShifter();

    void TeleopLift();
    void TeleopArm();

    void TeleopRoller();
    void TeleopMinibot();

    // Autonomous routines
    void Side();
    void EncoderAuton();
    void Middle(int numberOfTubes);

    // used for debugging
    bool PauseOnDS_input(int softKeyNum);

    // SmartDashboard updating
    void UpdateDashboardValues(GameState gameState);
};

#endif
