#include "Brain.h"
#include "..\ActionData.h"
#include "..\ActionData\DriveAction.h"

Brain::Brain()
    : config(Config::GetInstance())
    , console(Console::GetInstance())
    , lcd(LCD::GetInstance())
    , ds(*DriverStation::GetInstance())
    , action(ActionData::GetInstance())
    , inputs(ProcessedInputs::GetInstance())
    , gameTimer()
    , leftEncoder(DriveEncoders::GetInstance().GetLeftEncoder())
    , rightEncoder(DriveEncoders::GetInstance().GetRightEncoder())
    , driveEncoders(DriveEncoders::GetInstance())
    , lineSensor()
    , prevLinePosition(-1)
    , firstReading(true)
    , leftSide(false)
    , isFinale(false)
    , hasMoved(false)
    , wasDisabledLastCycle(true)
{
    // first reading is bogus; throw it out 3/12/11 -KV
    lineSensor.GetLinePosition();
    printf("Constructed Brain\n");
}

Brain::~Brain()
{

}

void Brain::Process(GameState gameState)
{
    // start game timer when transitioning to teleop mode
    if(previousState != TELEOPERATED && gameState == TELEOPERATED)
        gameTimer.Start();

    if(previousState != DISABLED)
        wasDisabledLastCycle = false;

    Common();

    // mode-specific methods
    switch(gameState)
    {
    case DISABLED:
        Disabled();
        break;

    case TELEOPERATED:
        Teleop();
//        Auton();
        break;

    case AUTONOMOUS:
        Auton();
        break;
    }

    // match is over
    if(gameState == TELEOPERATED)
    {
        LCD::UpdateGameTime(gameTimer.Get());
        if(gameTimer.Get() > 120)
            gameTimer.Stop();
    }

#ifdef USE_DASHBOARD
    {
        ProfiledSection ps("Dashboard Logging");
        UpdateDashboardValues(gameState);
    }
#endif

    previousState = gameState;
}

void Brain::UpdateDashboardValues(GameState gameState)
{
    // drive train and encoder logging
    SmartDashboard::Log((int)(action.driveTrain->rate.rawForward * 100), "Raw Forward (F)");
    SmartDashboard::Log(200 + (int)(action.driveTrain->rate.rawForward * 100), "Raw Forward (B)");

    SmartDashboard::Log((int)(action.driveTrain->rate.rawTurn * 100), "Raw Turn (F)");
    SmartDashboard::Log(200 + (int)(action.driveTrain->rate.rawTurn * 100), "Raw Turn (B)");

//    SmartDashboard::Log(action.driveTrain.brakeLeft, "BL: ");
//    SmartDashboard::Log(action.driveTrain.brakeRight, "BR: ");

    SmartDashboard::Log(leftEncoder.Get(), "Left Ticks");
    SmartDashboard::Log(rightEncoder.Get(), "Right Ticks");

    SmartDashboard::Log(leftEncoder.GetRate(), "Left Rate");
    SmartDashboard::Log(rightEncoder.GetRate(), "Right Rate");

    if(previousState != DISABLED && gameState == DISABLED)
    {
        // log build and run number
        SmartDashboard::Log((Util::ToString<int>(config.Get<int>("Build", "Number", -1)) + "-" +
                Util::ToString<int>(config.Get<int>("Build", "RunNumber", -1))).c_str(), "Build/Run");
    }

    SmartDashboard::Log((float)gameTimer.Get(), "Game Timer");

    // can only deploy minibot with 10 seconds remaining
    if(gameState == TELEOPERATED)
    {
        if(gameTimer.Get() > 110)
        {
            isFinale = true;
            SmartDashboard::Log(true, "Can Deploy Minibot?");
        }
        else
            SmartDashboard::Log(false, "Can Deploy Minibot?");
    }
}
