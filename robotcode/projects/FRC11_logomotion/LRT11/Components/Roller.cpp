#include "Roller.h"
#include "..\Config\RobotConfig.h"
#include "..\Jaguar\ProxiedCANJaguar.h"
#include "..\Util\Util.h"
#include "..\Config\Config.h"
#include "..\Config\RobotConfig.h"
#include "..\ActionData\RollerAction.h"


Roller::Roller()
    : Component()
    , configSection("Roller")
    , ignoreCycles(25)
    , detected(false)
{
    topRoller = new ProxiedCANJaguar(RobotConfig::CAN::ROLLER_TOP, "Top Roller"); // change port numbers later
    bottomRoller = new ProxiedCANJaguar(RobotConfig::CAN::ROLLER_BOTTOM, "Bottom Roller");

    topRoller->ConfigNeutralMode(LRTCANJaguar::kNeutralMode_Coast);
    bottomRoller->ConfigNeutralMode(LRTCANJaguar::kNeutralMode_Coast);
    printf("Constructed Rollers\n");
}

Roller::~Roller()
{
    delete topRoller;
    delete bottomRoller;
}

void Roller::RollInward()
{
//    topRoller->ShouldCollectCurrent(true);
//    bottomRoller->ShouldCollectCurrent(true);

//    float topCurrent, bottomCurrent;
//
//    {
//        ProfiledSection ps("Get Roller Currents");
//        topCurrent = topRoller->GetCurrent();
//        bottomCurrent = bottomRoller->GetCurrent();
//    }

//    static int cycleCount = 0;
//    if(++cycleCount % 10 == 0)
//    {
//        AsynchronousPrinter::Printf("I: %d, T: %.2f\n", ignoreCycles,
//                topCurrent + bottomCurrent);
//        fflush(stdout);
//    }

//    if(--ignoreCycles <= 0 && topCurrent + bottomCurrent > 15)
//        detected = true;

    // observe currents
#ifdef USE_DASHBOARD
//    SmartDashboard::Log(topRoller->GetOutputCurrent(), "Top Current");
//    SmartDashboard::Log(bottomRoller->GetOutputCurrent(), "Bottom Current");
#endif

//    if(detected)
//    {
//        topRoller->Set(0);
//        bottomRoller->Set(0);
//    }
//    else
//    {
    topRoller->SetDutyCycle(Util::Sign<float>(dutyCycleSucking) * Util::Min<float>(action.roller->maxSuckPower,
            Util::Abs<float>(dutyCycleSucking)));
    bottomRoller->SetDutyCycle(Util::Sign<float>(dutyCycleSucking) * Util::Min<float>(action.roller->maxSuckPower,
            Util::Abs<float>(dutyCycleSucking)));
//    }
}

void Roller::RollOutward()
{
    topRoller->SetDutyCycle(dutyCycleSpittingTop);
    bottomRoller->SetDutyCycle(dutyCycleSpittingBottom);
}

void Roller::Stop()
{
    topRoller->SetDutyCycle(0.0);
    bottomRoller->SetDutyCycle(0.0);
}

void Roller::RollOpposite(bool rotateUpward)
{
//    static int cycleCount = 0;

    // set duty cycles based on rotation direction
    if(rotateUpward)
    {
        topRoller->SetDutyCycle(dutyCycleRotatingIn);
        bottomRoller->SetDutyCycle(dutyCycleRotatingOut);
    }
    // pulse rotate and suck when rotating downward
    else
    {
        topRoller->SetDutyCycle(dutyCycleRotatingOut);
        bottomRoller->SetDutyCycle(dutyCycleRotatingIn);
    }
//    else if(++cycleCount < 20)
//    {
//        topRoller->Set(dutyCycleSucking);
//        bottomRoller->Set(dutyCycleSucking);
//    }
//    else
//        cycleCount = 0;
}

void Roller::Output()
{
    // abort overrides everything
    if(action.master.abort)
        // stop moving rollers
        action.roller->state = ACTION::ROLLER::STOPPED;

    if(action.roller->state != ACTION::ROLLER::SUCKING)
    {
        ignoreCycles = 25;
        detected = false;

        topRoller->ShouldCollectCurrent(false);
        bottomRoller->ShouldCollectCurrent(false);
    }

    switch(action.roller->state)
    {
    case ACTION::ROLLER::STOPPED:
        Stop();
        break;
    case ACTION::ROLLER::SUCKING:
        RollInward();
        break;
    case ACTION::ROLLER::SPITTING:
        RollOutward();
        break;
    case ACTION::ROLLER::ROTATING:
        RollOpposite(action.roller->rotateUpward);
        break;
    }
}

void Roller::Configure()
{
    Config& config = Config::GetInstance();

    // default values empirically determined on 3/11/11 in room 612 -KV
    dutyCycleSucking = config.Get<float>(configSection, "dutyCycleSucking", -1.0);

    // independent so that the ringer may be rotated when it is ejected
    dutyCycleSpittingTop = config.Get<float>(configSection, "dutyCycleSpittingTop", 1.0);
    dutyCycleSpittingBottom = config.Get<float>(configSection, "dutyCycleSpittingBottom", 0.6);

    // duty cycle for roller rotating inward is higher so that the ringer stays
    // inside the grabber (no tendency to move out)
    dutyCycleRotatingOut = config.Get<float>(configSection, "dutyCycleRotatingOut", -0.9);
    dutyCycleRotatingIn = config.Get<float>(configSection, "dutyCycleRotatingIn", 1.0);
}

string Roller::GetName()
{
    return "Roller";
}
