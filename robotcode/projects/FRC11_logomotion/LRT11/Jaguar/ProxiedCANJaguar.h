#ifndef PROXIED_CAN_JAGUAR_H_
#define PROXIED_CAN_JAGUAR_H_

#include "taskLib.h"

#include "LRTCANJaguar.h"

#include "../General.h"
#include "../Util/Util.h"
#include "../Util/PrintInConstructor.h"

class VirtualCANBusController;

class ProxiedCANJaguar : public LRTCANJaguar
{
private:
    string taskName_;
    PrintInConstructor print_ctor_dtor;
    int channel;
    char* name_;

    volatile float setpoint;
    volatile float lastSetpoint;

    volatile bool shouldCacheSetpoint;
    int cacheSetpointCounter;   //timer for how long cache may be held.

    volatile NeutralMode mode;
    volatile NeutralMode lastMode;
    volatile bool shouldCacheMode;

    volatile float current;
    volatile bool collectCurrent;

    volatile float potValue;
    volatile bool collectPotValue;

    static GameState gameState;
    GameState lastState;

    int index;
    Task commTask;

    semaphore* commSemaphore;
    bool running_; //implementation in progress - controlled termination of task -dg
    bool quitting_; // ditto

public:
    ProxiedCANJaguar(UINT8 channel, char* name);
    ~ProxiedCANJaguar();
    void StopBackgroundTask();

    static ProxiedCANJaguar* jaguar_list_;
    ProxiedCANJaguar* next_jaguar_;

    void SetDutyCycle(float duty_cycle);
    void SetPosition(float position);

private:    //don't let external objects control the ESC with the ambiguous Set() cmd
    void Set(float setpoint, UINT8 syncGroup = 0);
public:
    void ConfigNeutralMode(LRTCANJaguar::NeutralMode mode);

    void ShouldCollectCurrent(bool shouldCollect);
    void ShouldCollectPotValue(bool shouldCollect);

    float GetCurrent();
    float GetPotValue();

    static int CommTaskWrapper(UINT32 proxiedCANJaguarPointer);
    void CommTask();

    void BeginComm();

#ifdef VIRTUAL
    virtual float Get();
    virtual void Disable();

    void SetPID(double p, double i, double d);
    void SetPositionReference(VirtualPot* reference);
    void ConfigPotentiometerTurns(UINT16 turns);
    CANJaguar::PositionReference GetPositionReference(void);

    void ChangeControlMode(CANJaguar::ControlMode controlMode);
    CANJaguar::ControlMode GetControlMode();
    void EnableControl(double encoderInitialPosition = 0.0);
    void DisableControl();

    float GetCurrent();
    float GetTemperature();
    float GetBatteryVoltage();
    float GetOutputVoltage();

    double GetPosition();
    double GetSpeed();

    void ResetCache();
#else
    static void SetGameState(GameState state);
    void ResetCache();
#endif

protected:
#ifdef VIRTUAL
    VirtualCANBusController& controller;
#else
//    CANBusController& controller;
#endif
};

#endif /* PROXIED_CAN_JAGUAR_H_ */
