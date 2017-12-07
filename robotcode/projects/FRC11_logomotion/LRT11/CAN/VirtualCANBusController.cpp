#include "VirtualCANBusController.h"

VirtualCANBusController* VirtualCANBusController::instance = NULL;

VirtualCANBusController& VirtualCANBusController::GetInstance()
{
    if(instance == NULL)
        instance = new VirtualCANBusController();
    return *instance;
}

VirtualCANBusController::VirtualCANBusController()
    : busWriterTask("VirtualCANBusController",
            (FUNCPTR)VirtualCANBusController::BusWriterTaskRunner)
    , semaphore(semMCreate(SEM_Q_PRIORITY | SEM_DELETE_SAFE
            | SEM_INVERSION_SAFE))
{
    AddToSingletonList();

    // fill arrays with default values
    for(int id = kMinJaguarId; id <= kMaxJaguarId; ++id)
    {
        int idx = BusIdToIndex(id);
        setpoints[idx] = 0.0;
        subscribers[idx] = NULL;
        // idx is the array index; id is the CAN-bus ID
        sprintf(jaguarLabels[idx], "Esc %d Setpoint: ", id);
    }

    busWriterTask.Start();
}

int VirtualCANBusController::BusIdToIndex(int id)
{
    if(id > kMaxJaguarId || id < kMinJaguarId)
    {
        AsynchronousPrinter::Printf("VirtualCANBusController: %d out of range!\n", id);
        return 0;
    }

    return id - kMinJaguarId;
}

void VirtualCANBusController::Set(int id, float val)
{
    int idx = BusIdToIndex(id);

    // position mode for the lift must be handled differently
    if((UINT32) id == RobotConfig::CAN::LIFT && liftMode == CANJaguar::kPosition)
    {
        if(liftPot == NULL)
        {
            AsynchronousPrinter::Printf("Error! Lift pot is null.\n");
            return;
        }

        float position = liftPot->GetPosition();
        float error = val - position;

        AsynchronousPrinter::Printf("Position = %.2f, Val = %.2f, Error = %.2f\n", position, val, error);
        // val is set to the correction, which is the value that
        // needs to be sent to the jaguar
        val = error * liftPGain / 100.0;
    }

    setpoints[idx] = val;
#ifdef USE_DASHBOARD
    SmartDashboard::Log(val, jaguarLabels[idx]);
#endif
}

void VirtualCANBusController::SetDutyCycleSubscriber(int channel, DutyCycleSubscriber* subscriber)
{
    int idx = BusIdToIndex(channel);
    subscribers[idx] = subscriber;
}

void VirtualCANBusController::BusWriterTaskRunner()
{
    VirtualCANBusController::GetInstance().BusWriterTask();
}

void VirtualCANBusController::BusWriterTask()
{
    while(true)
    {
        for(int id = kMinJaguarId; id <= kMaxJaguarId; ++id)
        {
            int idx = BusIdToIndex(id);

            if(subscribers[idx] != NULL)
                subscribers[idx]->Update(setpoints[idx]);
        }

        Wait(0.02); // run at 50 Hz
    }
}
