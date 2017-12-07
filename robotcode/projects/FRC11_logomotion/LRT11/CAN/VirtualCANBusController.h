#ifndef VIRTUAL_CAN_BUS_CONTROLLER_H_
#define VIRTUAL_CAN_BUS_CONTROLLER_H_

#include "..\General.h"
#include "..\Util\AsynchronousPrinter.h"
#include "..\Util\DutyCycleSubscriber.h"
#include "..\Config\RobotConfig.h"
#include "..\Sensors\VirtualPot.h"
#include <vector>
using namespace std;

class VirtualCANBusController : public SensorBase
{
public:
    virtual ~VirtualCANBusController()
    {
        delete * jaguarLabels;
    }
    static VirtualCANBusController& GetInstance();

    void ResetCache() {}
    void ResetCache(int id) {}
    void Set(int id, float val);

    float Get(int id)
    {
        return 0.0;
    }

    // blocking jaguar configuration functions
    void SetPID(int id, double p, double i, double d)
    {
        liftPGain = p;
    }

    void SetPositionReference(int id, VirtualPot* pot)
    {
        liftPot = pot;
    }

    void SetPotentiometerTurns(int id, UINT16 turns) {}

    CANJaguar::PositionReference GetPositionReference(int channel)
    {
        return CANJaguar::kPosRef_None;
    }

    void SetControlMode(int id, CANJaguar::ControlMode controlMode)
    {
        liftMode = controlMode;
    }

    CANJaguar::ControlMode GetControlMode(int id)
    {
        return CANJaguar::kPercentVbus;
    }

    void EnableControl(int id, double encoderInitialPosition = 0.0) {}

    // blocking functions with no parameter
    void DisableControl(int channel)
    {
        if((UINT32) channel == RobotConfig::CAN::LIFT && liftMode == CANJaguar::kPosition)
        {
            liftMode = CANJaguar::kPercentVbus;
            Set(channel, 0);
            liftMode = CANJaguar::kPosition;
        }
    }

    float GetOutputCurrent(int channel)
    {
        return 0.0;
    }

    float GetTemperature(int channel)
    {
        return 0.0;
    }

    float GetBusVoltage(int channel)
    {
        return 0.0;
    }

    float GetOutputVoltage(int channel)
    {
        return 0.0;
    }

    double GetSpeed(int channel)
    {
        return 0.0;
    }

    double GetPosition(int channel)
    {
        return 0.0;
    }

    void ConfigNeutralMode(int id, CANJaguar::NeutralMode mode) {}
    void PrintOnlineStatus() {}

    void SetDutyCycleSubscriber(int channel, DutyCycleSubscriber* subscriber);

private:
    VirtualCANBusController();
    DISALLOW_COPY_AND_ASSIGN(VirtualCANBusController);

    static VirtualCANBusController* instance;
    int BusIdToIndex(int id);

    // CAN jaguar ids (should be a contiguous block)

    static void BusWriterTaskRunner();
    void BusWriterTask();

    const static int kMinJaguarId = 1;
    const static int kMaxJaguarId = 10;
    const static int kNumJaguars = kMaxJaguarId - kMinJaguarId + 1;

    char jaguarLabels[kNumJaguars][30];
    volatile float setpoints[kNumJaguars];

    // can't put volatile in front of DutyCycleSubscriber*, as that would mean
    // that the pointee is volatile; instead, the contents of the array, or the
    // pointers themselves, should be volatile
    DutyCycleSubscriber* volatile subscribers[kNumJaguars];

    Task busWriterTask;
    SEM_ID semaphore;

    CANJaguar::ControlMode liftMode;
    VirtualPot* liftPot;
    double liftPGain;
};

#endif
