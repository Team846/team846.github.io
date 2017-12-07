#include "Lift.h"
#include "..\Config\RobotConfig.h"
#include "..\Config\Config.h"
#include "..\Sensors\VirtualPot.h"
#include "..\Jaguar\ProxiedCANJaguar.h"
#include "..\ActionData\ArmAction.h"
#include "..\ActionData\LiftAction.h"

Lift::Lift()
    : Component()
    , config(Config::GetInstance())
    , configSection("Lift")
    , timeoutCycles(0)
    , cycleCount(0)
    , prevMode(PRESET)
    , potDeadband(0)
    , positionMode(true)
{
    liftEsc = new ProxiedCANJaguar(RobotConfig::CAN::LIFT, "Lift");  //Pot is directly connected to Jaguar ESC, not the cRio
#ifdef VIRTUAL
    liftPot = new VirtualPot(RobotConfig::CAN::LIFT, 10, 1.0, 6.5);
#endif


    printf("Lift Constructed. CANid: %d\n", RobotConfig::CAN::LIFT);
}

Lift::~Lift()
{
    delete liftEsc;
#ifdef VIRTUAL
    delete liftPot;
#endif
}

void Lift::Configure()
{
//    liftEsc->SetControlMode(CANJaguar::kPosition);
    liftEsc->ChangeControlMode(LRTCANJaguar::kPosition);
#ifdef VIRTUAL
    liftEsc->SetPositionReference(&liftPot);
#else
    liftEsc->SetPositionReference(LRTCANJaguar::kPosRef_Potentiometer);
#endif

    liftEsc->SetPID(config.Get<double>(configSection, "pGain", 100), config.Get<double>(configSection, "iGain", 0),
            config.Get<double>(configSection, "dGain", 0));

//    liftEsc->ConfigSoftPositionLimits(config.Get<double>(prefix + "forwardLimit", 0),
//            config.Get<double>(prefix + "reverseLimit", 10));
//    liftEsc->SetPotentiometerTurns(10);
    liftEsc->ConfigPotentiometerTurns(10);

    Config& config = Config::GetInstance();

    // convert from ms into cycles
    timeoutCycles = (int)(config.Get<int>(configSection, "timeoutMs", 1500) * 1.0 / 1000.0 * 50.0 / 1.0);

    // bottom of low row is the lowest position
//    minPosition = config.Get<float>(prefix + "lowRowReference", 1.17)
//            + config.Get<float>(prefix + "lowRowLowPegRelative", 0.7);

    minPosition = config.Get<float>(configSection, "lowColumn.lowPeg", 1.72);

    // bottom of high row + high peg relative is the highest position
//    maxPosition = config.Get<float>(prefix + "highRowReference", 1.87)
//            + config.Get<float>(prefix + "highPegRelative", 6.5);

    maxPosition = config.Get<float>(configSection, "highColumn.highPeg", 8.64);

    potDeadband = config.Get<float>(configSection, "deadband", 0.4);
}

void Lift::ConfigureManualMode()
{
//    liftEsc->SetControlMode(CANJaguar::kPercentVbus);
    liftEsc->ChangeControlMode(LRTCANJaguar::kPercentVbus);
    liftEsc->EnableControl();
}

void Lift::Output()
{
    static enum
    {
        IDLE,
        ABORT,
        MANUAL,
        PRESET,
        PULSING
    } state = IDLE;

    if(action.lift->givenCommand)
    {
        if(action.lift->manualMode)
        {
            if(positionMode)
            {
                // configure jaguar for percent voltage mode
                ConfigureManualMode();
                positionMode = false;
            }

            state = MANUAL;
        }
        else
        {
            if(!positionMode)
            {
                // configure jaguar for position mode
                Configure();
                positionMode = true;
            }

            cycleCount = timeoutCycles;
            state = PRESET;
        }

        action.lift->givenCommand = false; // command has been processed
        liftEsc->EnableControl();
    }

    float potValue = 0.0;
#ifdef VIRTUAL
    potValue = liftPot->GetPotValue();
#else
    potValue = liftEsc->GetPotValue();
#endif

#ifdef USE_DASHBOARD
    SmartDashboard::Log(potValue, "Lift Pot Value");
#endif

    // abort overrides everything
    if(action.master.abort)
        state = ABORT;

    static int potCycleCount = 0;
//    static bool shouldMoveArmToMiddle = false;

    switch(state)
    {
    case IDLE:
//        AsynchronousPrinter::Printf("Idle\n");
        liftEsc->DisableControl();
        liftEsc->ShouldCollectPotValue(false);

        if(++potCycleCount % 50 == 0)
            liftEsc->ShouldCollectPotValue(true);

        if(!positionMode)
        {
            // exited from manual mode; done with maneuver
            action.lift->completion_status = ACTION::SUCCESS;
            liftEsc->SetDutyCycle(0.0);
        }
//        else if(shouldMoveArmToMiddle)
//            action.arm.state = action.arm.PRESET_MIDDLE;
        break;

    case ABORT:
//        AsynchronousPrinter::Printf("Abort\n");
        liftEsc->DisableControl();
        liftEsc->ShouldCollectPotValue(false);

        if(!positionMode)
            liftEsc->SetDutyCycle(0.0);

        action.lift->completion_status = ACTION::ABORTED;
        break;

    case PULSING:
//        AsynchronousPrinter::Printf("Pulsing\n");
        liftEsc->ShouldCollectPotValue(true);
        if(positionMode)
        {
            // configure jaguar for voltage mode
            ConfigureManualMode();
            liftEsc->SetPosition(0.0); // clear any old setpoint values from position mode
            positionMode = false;
        }

        if(potValue >= minPosition)
        {
            liftEsc->SetPosition(-0.1);
//            liftEsc->ResetCache();
        }
        else
            liftEsc->SetPosition(0.0);
        break;

    case MANUAL:
//        AsynchronousPrinter::Printf("Manual\n");
        liftEsc->ShouldCollectPotValue(true);
        action.lift->completion_status = ACTION::IN_PROGRESS; // not done yet

        if((action.lift->power > 0 && potValue < maxPosition) ||
                (action.lift->power < 0 && potValue > minPosition))
        {
            liftEsc->ResetCache();
            liftEsc->SetDutyCycle(action.lift->power);
        }
        else
            // don't power past the minimum and maximum positions
            liftEsc->SetDutyCycle(0.0);

        state = IDLE;
        break;

    case PRESET:
//        AsynchronousPrinter::Printf("Preset\n");
        liftEsc->ShouldCollectPotValue(true);
        action.lift->completion_status = ACTION::IN_PROGRESS; // not done yet
        string key;

        float setpoint = 0.0;
        if(action.lift->highColumn)
            key = "highColumn.";
//            setPoint = config.Get<float>(prefix + "highRowReference");
        else
            key = "lowColumn.";
//            setPoint = config.Get<float>(prefix + "lowRowReference");

        switch(action.lift->lift_preset)
        {
        case ACTION::LIFT::STOWED:
            setpoint = 0; // no movement
            break;
        case ACTION::LIFT::LOW_PEG:
            key += "lowPeg";
            break;
        case ACTION::LIFT::MED_PEG:
            key += "mediumPeg";
            break;
        case ACTION::LIFT::HIGH_PEG:
            key += "highPeg";
            break;
        }

        if(ACTION::LIFT::STOWED != action.lift->lift_preset)
            setpoint = config.Get<float>(configSection, key, 2.0); // relative to bottom

//        AsynchronousPrinter::Printf("Status: %.2f\n", Util::Abs<float>(potValue - setpoint));
        // update done flag
        if(Util::Abs<float>(potValue - setpoint) < potDeadband)
        {
//            AsynchronousPrinter::Printf("Updating done flag");
            action.lift->completion_status = ACTION::SUCCESS;
//            cycleCount = 1; // will get decremented to 0

//            if(action.lift->preset == action.lift->MED_PEG || action.lift->preset == action.lift->HIGH_PEG)
//            {
////                AsynchronousPrinter::Printf("Lift success; moving arm to middle position\n");
//                action.arm.state = action.arm.PRESET_MIDDLE;
//                shouldMoveArmToMiddle = true;
//            }
        }
        else
            // keep arm upright when the lift is moving
            action.arm->state = ACTION::ARM_::PRESET_TOP; //This does not belong here

        SmartDashboard::Log(setpoint, "Lift Set Point");
        liftEsc->SetPosition(setpoint);

        if(cycleCount > 0)
            cycleCount--;

        if(cycleCount == 0)
        {
//            AsynchronousPrinter::Printf("Success: %d\n", action.lift->doneState == ACTION::SUCCESS);

            if(action.lift->completion_status != ACTION::SUCCESS)
            {
                action.lift->completion_status = ACTION::FAILURE;
//                shouldMoveArmToMiddle = false;
            }

            if(action.lift->lift_preset == ACTION::LIFT::LOW_PEG &&
                    action.lift->completion_status == ACTION::SUCCESS)
                state = PULSING;
            else
                state = IDLE;
        }

#ifdef USE_DASHBOARD
//        SmartDashboard::Log(setPoint, "Lift Set Point");
#endif
        break;
    }
}

string Lift::GetName()
{
    return "Lift";
}

/*
void Lift::Output()
{
    float potValue;

    {
        ProfiledSection ps("Lift Log Position");
//        potValue = liftEsc->GetPosition();
        potValue = liftPot.GetPosition();
        SmartDashboard::Log(potValue, "Lift Pot Value");
    }

    if(!action.lift->givenCommand && cycleCount == 0)
    {
        {
            ProfiledSection ps("Lift disable control");
            liftEsc->DisableControl();
        }
        return;
    }
    else if((cycleCount == 0 || action.lift->givenCommand) && !action.lift->manualMode)
    {
        action.lift->givenCommand = false;
        StartTimer();

        // reset preset flags
        action.lift->done = false;
        liftEsc->EnableControl();
    }

    if(action.lift->manualMode)
    {
        liftEsc->EnableControl();

        if(prevMode == PRESET)
            ConfigureVoltageMode();

        if((action.lift->power > 0 && potValue < maxPosition) ||
                (action.lift->power < 0 && potValue > minPosition))
        {
            liftEsc->ResetCache();
            liftEsc->Set(action.lift->power);
        }

        action.lift->givenCommand = false;
        prevMode = MANUAL;
    }
    else
    {
        if(prevMode == MANUAL)
            Configure(); // position control mode

        string key = prefix;

        float setPoint;
        if(action.lift->highRow)
            setPoint = config.Get<float>(prefix + "highRowBottom");
        else
            setPoint = config.Get<float>(prefix + "lowRowBottom");

        switch(action.lift->position)
        {
        case STOWED:
            break; // no relative position
        case LOW_PEG:
            key += "lowPegRelative";
            break;
        case MED_PEG:
            key += "mediumPegRelative";
            break;
        case HIGH_PEG:
            key += "highPegRelative";
            break;
        }

        if(action.lift->position != action.lift->STOWED)
            setPoint += config.Get<float>(key); // relative to bottom

        // update done flag
        if(Util::Abs<float>(potValue - setPoint) < potDeadband)
            action.lift->done = true;

        liftEsc->Set(setPoint);
        cycleCount--;

        SmartDashboard::Log(setPoint, "Lift Set Point");
        prevMode = PRESET;
    }
}
*/
