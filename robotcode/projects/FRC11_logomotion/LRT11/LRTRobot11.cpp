#include "LRTRobot11.h"
#include "Util/PrintInConstructor.h"
#include "Jaguar\ProxiedCANJaguar.h"
//#include <signal.h>
//#include "Config/joystickdg.h"

LRTRobot11::LRTRobot11()
    :
    firstMember_(
        "\n\n\n---------------------------------------------------------\n"
        "Begin LRTRobot Constructor\n",
        "LRTRobot Destroyed\n\n")
    , brain()
    , dc_CANBus_("CANbus\n")
#ifdef VIRTUAL
    , controller(VirtualCANBusController::GetInstance())
#else
//    , controller(CANBusController::GetInstance())
#endif
    , config(Config::GetInstance())
    , ds(*DriverStation::GetInstance())
//    , switchLED(6)
    , prevState(DISABLED)
    , lastMember_("LRTRobot.LastMember\n") //trace constructor.

{
    components = Component::CreateComponents();

//    mainLoopWatchDog = wdCreate();
    printf("---- Robot Initialized ----\n\n");
}

LRTRobot11::~LRTRobot11()
{
    // Testing shows this to be the entry point for a Kill signal.
    // Start shutting down processes here. -dg
    printf("\n\nBegin Deleting LRTRobot11\n");

    // Kill the main loop, so we don't access deleted objects. -dg
    LRTRobotBase::quitting_ = true;
    printf("LRTRobot11 says to LRTRobotBase: \"Quit Main Loop please\"\n");
    Wait(0.100); //Wait for main loop to exec one last time and then exit.  Should take < 20ms.

    //End background printing; Request print task to stop and die.
    //Premature?  We could move this to ~LRTRobotBase()
    AsynchronousPrinter::Quit();
}

void LRTRobot11::RobotInit()
{
    config.ConfigureAll();
//    config.Save();

    const char* build = (Util::ToString<int>(config.Get<int>("Build", "BuildNumber", -1)) + "-" +
            Util::ToString<int>(config.Get<int>("Build", "RunNumber", -1))).c_str();

#ifdef USE_DASHBOARD
    SmartDashboard::Log(build, "Build/Run");
#endif
    AsynchronousPrinter::Printf(build);

    //Test code for new button assignments.  -dg 6/8/2011
//    int status = driverstation_button::Initalize_All_Buttons();
//    if(status)
//        printf("FATAL ERROR in Initalize_All_Buttons()\n");

}

//static int ExecutionNotify(...)
//{
//    AsynchronousPrinter::Printf("Main execution > 20ms\n");
//    return 0;
//}

void LRTRobot11::MainLoop()
{

    // setup a watchdog to warn us if our loop takes too long
    // sysClkRateGet returns the number of ticks per cycle at the current clock rate.
//    wdStart(mainLoopWatchDog, sysClkRateGet() / 50, ExecutionNotify, 0);
    GameState gameState = DetermineState();

    {
        ProfiledSection ps("Brain Processing");
        brain.Process(gameState);
    }

    //iterate though and output components
    for(list<ComponentWithData>::iterator iter = components->begin(); iter != components->end(); iter++)
    {
        // if we are enabled or the Component does not require the enabled state
        if(gameState != DISABLED || !((*iter).second.RequiresEnabledState))
        {
            int DIO = (*iter).second.DS_DIOToDisableComponent;
            if(DIO == ComponentData::NO_DS_DISABLE_DIO || ds.GetDigitalIn(DIO))
            {
                ProfiledSection ps("Outputting " + (*iter).first->GetName());
                (*iter).first->Output();
            }

        }
    }

#ifndef VIRTUAL
    ProxiedCANJaguar::SetGameState(gameState);
#endif

//    if(prevState != gameState)
//        controller.ResetCache();

    prevState = gameState;

    // if we finish in time, cancel the watchdog's error message
//    wdCancel(mainLoopWatchDog);
}

GameState LRTRobot11::DetermineState()
{
    GameState state = TELEOPERATED;

    if(IsDisabled())
        state = DISABLED;
    else if(IsAutonomous())
        state = AUTONOMOUS;

    return state;
}


/*
 * FRC_UserProgram_StartupLibraryInit()
 *  is the entry point of the program, like main().
 *
 * The FRC_UserProgram_StartupLibraryInit() calls RobotBase::startRobotTask((FUNCPTR)FRC_userClassFactory)
 * which creates a task called "FRC_RobotTask"
 * that  ultimately calls FRC_userClassFactory() to create a new "LRTRobot11"
 * and then calls the virtual RobotBase::StartCompetition()
 * -> LRTRobotBase::StartCompetition();
 * This VxWorks task is named "FRC_RobotTask"
 * See WPILIB Robotbase.cpp
 * -D.Giandomenico (description of WPLIB start code)
 */

//START_ROBOT_CLASS(LRTRobot11); //Expand the macro as below:
RobotBase* FRC_userClassFactory()
{
    return new LRTRobot11();
}
extern "C" {
    INT32 FRC_UserProgram_StartupLibraryInit()
    {
        RobotBase::startRobotTask((FUNCPTR)FRC_userClassFactory);
        return 0;
    }
}

