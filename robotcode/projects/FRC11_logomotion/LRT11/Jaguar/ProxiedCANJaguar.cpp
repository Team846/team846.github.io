#include <stdio.h>
#include "../Jaguar/ProxiedCANJaguar.h"
#include "../CAN/VirtualCANBusController.h"

#define DISABLE_SETPOINT_CACHING 0

GameState ProxiedCANJaguar::gameState = DISABLED;
//ProxiedCANJaguar::JaguarList ProxiedCANJaguar::jaguars = {0};
ProxiedCANJaguar::ProxiedCANJaguar* ProxiedCANJaguar::jaguar_list_ = NULL;


ProxiedCANJaguar::ProxiedCANJaguar(UINT8 channel, char* name)
    : LRTCANJaguar(channel)
    , taskName_("JAG#" + Util::ToString<int>(channel))
    , print_ctor_dtor(taskName_.c_str(), (taskName_ + "\n").c_str())
    , channel(channel)
    , name_(name)

    , setpoint(0.0)
    , lastSetpoint(0.0)
    , shouldCacheSetpoint(false)
    , cacheSetpointCounter(0)
    , mode(LRTCANJaguar::kNeutralMode_Coast)
    , lastMode(LRTCANJaguar::kNeutralMode_Coast)
    , shouldCacheMode(false)
    , current(0.0)
    , collectCurrent(false)
    , potValue(0.0)
    , collectPotValue(false)
#ifdef VIRTUAL
    , controller(VirtualCANBusController::GetInstance())
#else
    , lastState(DISABLED)
//    : controller(CANBusController::GetInstance())
#endif
//    , index(jaguars.num)
    , commTask(taskName_.c_str(), (FUNCPTR) ProxiedCANJaguar::CommTaskWrapper)
    , commSemaphore(semBCreate(SEM_Q_PRIORITY, SEM_EMPTY))
    , running_(false)
    , quitting_(false)
{

    next_jaguar_ = jaguar_list_;
    jaguar_list_ = this;

    if(name_ == NULL) name_ = "?";
    commTask.Start((UINT32) this);
    printf("Created Jaguar %2d: %s\n", channel, name_);
}

ProxiedCANJaguar::~ProxiedCANJaguar()
{
    //Before we delete the jaguar object,
    //the Jaguar reader task should be killed
    // and the main loop that accesses jags should be killed.
    // currently the main loop is killed in the dtor of LRTRobot11.
//   JaguarReader::GetInstance().StopTask(); //kill the jag reader that accesses this object.
    StopBackgroundTask();
    int error = semDelete(commSemaphore);
    if(error)
        printf("SemDelete Error=%d\n", error);
}
void ProxiedCANJaguar::StopBackgroundTask()
{
    if(running_)
    {
        INT32 task_id = commTask.GetID(); //for info only. no safety check.
        commTask.Stop();
        printf("Task 0x%x killed for CANid=%d:%s\n", task_id, channel, name_);
    }
}


void ProxiedCANJaguar::ShouldCollectCurrent(bool shouldCollect)
{
    collectCurrent = shouldCollect;
}

void ProxiedCANJaguar::ShouldCollectPotValue(bool shouldCollect)
{
    collectPotValue = shouldCollect;
}

float ProxiedCANJaguar::GetCurrent()
{
    return current;
}

float ProxiedCANJaguar::GetPotValue()
{
    return potValue;
}

int ProxiedCANJaguar::CommTaskWrapper(UINT32 proxiedCANJaguarPointer)
{
    ProxiedCANJaguar* jaguar = (ProxiedCANJaguar*) proxiedCANJaguarPointer;
    if(jaguar->channel != 0)    // ignore jags on channel #0 -dg
    {
        jaguar->running_ = true;
        jaguar->CommTask();
    }
    jaguar->running_ = false;
    printf("Ending task for Jaguar %s %d\n", jaguar->name_, jaguar->channel);
    return 0; // return no error
}

void ProxiedCANJaguar::CommTask()
{
    while(!quitting_)
    {
        semTake(commSemaphore, WAIT_FOREVER);
        if(quitting_)
            break;

        //Determine if we can cache the setpoint and mode.
        shouldCacheMode = shouldCacheSetpoint = true; //default, unless...
        if(lastState != gameState)
            shouldCacheMode = shouldCacheSetpoint = false;
        if(mode != lastMode)
            shouldCacheMode = shouldCacheSetpoint = false;
        if(setpoint != lastSetpoint)
            shouldCacheSetpoint = false;

        // cache if value has been cached for over
        // half a second -KV -DG championships 4/28/11
        if(++cacheSetpointCounter >= 25)
            shouldCacheSetpoint = false;
        if(false == shouldCacheSetpoint)    //*always* clear the counter if caching is false.
            cacheSetpointCounter = 0;



#if DISABLE_SETPOINT_CACHING
        shouldCacheSetpoint = false;
#endif

        //change the mode, then do the set point.
        if(shouldCacheMode == false)
        {
            LRTCANJaguar::ConfigNeutralMode(mode);
            lastMode = mode;
        }
        if(shouldCacheSetpoint == false)
        {
            LRTCANJaguar::Set(setpoint);
            lastSetpoint = setpoint;
        }




        if(collectCurrent)
        {
            float current = LRTCANJaguar::GetOutputCurrent();
            if(StatusOK())
                this->current = current;
            else
                AsynchronousPrinter::Printf("Invalid current; not storing\n");
        }

        if(collectPotValue)
        {
            float potValue = LRTCANJaguar::GetPosition();
            if(StatusOK())
                this->potValue = potValue;
            else
                AsynchronousPrinter::Printf("Invalid pot value; not storing\n");
        }

        lastState = gameState;
//        AsynchronousPrinter::Printf("%d\n", channel);
    }
}

void ProxiedCANJaguar::BeginComm()
{
    semGive(commSemaphore);
}

#ifdef VIRTUAL
void ProxiedCANJaguar::Set(float setpoint, UINT8 syncGroup)
{
    controller.Set(channel, setpoint);
}

void ProxiedCANJaguar::SetPID(double p, double i, double d)
{
    controller.SetPID(channel, p, i, d);
}

float ProxiedCANJaguar::Get()
{
    return controller.Get(channel);
}

void ProxiedCANJaguar::Disable()
{
    controller.Set(channel, 0);
}

float ProxiedCANJaguar::GetCurrent()
{
    return controller.GetOutputCurrent(channel);
}

float ProxiedCANJaguar::GetTemperature()
{
    return controller.GetTemperature(channel);
}

float ProxiedCANJaguar::GetBatteryVoltage()
{
    return controller.GetBusVoltage(channel);
}

float ProxiedCANJaguar::GetOutputVoltage()
{
    return controller.GetOutputVoltage(channel);
}

double ProxiedCANJaguar::GetPosition()
{
    return controller.GetPosition(channel);
}

double ProxiedCANJaguar::GetSpeed()
{
    return controller.GetSpeed(channel);
}

void ProxiedCANJaguar::ConfigNeutralMode(CANJaguar::NeutralMode mode)
{
    controller.ConfigNeutralMode(channel, mode);
}

void ProxiedCANJaguar::SetPositionReference(VirtualPot* reference)
{
    controller.SetPositionReference(channel, reference);
}

void ProxiedCANJaguar::ConfigPotentiometerTurns(UINT16 turns)
{
    controller.SetPotentiometerTurns(channel, turns);
}

CANJaguar::PositionReference ProxiedCANJaguar::GetPositionReference()
{
    return controller.GetPositionReference(channel);
}

void ProxiedCANJaguar::ChangeControlMode(CANJaguar::ControlMode controlMode)
{
    controller.SetControlMode(channel, controlMode);
}

CANJaguar::ControlMode ProxiedCANJaguar::GetControlMode()
{
    return controller.GetControlMode(channel);
}

void ProxiedCANJaguar::EnableControl(double encoderInitialPosition)
{
    controller.EnableControl(channel, encoderInitialPosition);
}

void ProxiedCANJaguar::DisableControl()
{
    controller.DisableControl(channel);
}

void ProxiedCANJaguar::ResetCache()
{
    controller.ResetCache(channel);
}
#else


//Set() is ambiguous, since it doesn't include the mode.
//We've replaced them with specific command.  They are still in progress.  TODO -dg
void ProxiedCANJaguar::SetDutyCycle(float duty_cycle)
{
    this->setpoint = duty_cycle;
}

void ProxiedCANJaguar::SetPosition(float position)
{
    this->setpoint = position;
}
void ProxiedCANJaguar::Set(float setpoint, UINT8 syncGroup)
{
    // send the value if there is a setpoint or game state change
//    if(setpoint != lastSetpoint || lastState != gameState)
//        LRTCANJaguar::Set(setpoint);
    printf("ERROR: Calling Set() in ProxiedCANJaguar: %s\n;", name_);
    this->setpoint = setpoint;

//    lastSetpoint = setpoint;
//    lastState = gameState;
}

void ProxiedCANJaguar::ConfigNeutralMode(NeutralMode mode)
{
    this->mode = mode;
}

void ProxiedCANJaguar::SetGameState(GameState state)
{
    gameState = state;
}

void ProxiedCANJaguar::ResetCache()
{
    // bogus value to reset cache
    lastSetpoint = -1.0e6;
}
#endif
