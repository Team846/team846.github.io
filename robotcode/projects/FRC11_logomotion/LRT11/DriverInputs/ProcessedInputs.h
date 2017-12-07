#ifndef PROCESSED_JOYSTICK_H_
#define PROCESSED_JOYSTICK_H_

#include "..\General.h"
#include "DebouncedJoystick.h"
#include "..\Config\DriverStationConfig.h"
#include "..\Config\Config.h"
#include "..\Config\Configurable.h"

class ProcessedInputs : public SensorBase, public Configurable
{
private:
    static ProcessedInputs* instance;

    double forwardDeadband;
    double turnDeadband;

    float GetThrottle();
    DebouncedJoystick driverStick, operatorStick;

    //joy_[2] are synoymns for the driver/operator stick, allowing array assignment of buttons
    DebouncedJoystick* const joy_[2 + 1];
protected:
    ProcessedInputs();
    DISALLOW_COPY_AND_ASSIGN(ProcessedInputs);

public:
    ~ProcessedInputs();

    static ProcessedInputs& GetInstance();
    virtual void Configure();

    bool IsServiceMode();
    bool IsDriverTriggerDown();
    bool IsOperatorTriggerDown();

    bool ShouldAbort();

    // drive train
    float GetForward();
    float GetTurn();

    bool ShouldBrakeLeft();
    bool ShouldBrakeRight();

    // lift
    bool ShouldMoveLiftLow();
    bool ShouldMoveLiftMedium();
    bool ShouldMoveLiftHigh();
    bool ShouldManuallyPowerLift();
    float GetLiftPower();
    bool IsHighRow();

    // arm
    bool ShouldMoveArmDown();
    bool ShouldMoveArmUp();
    bool ShouldMoveArmBottomPreset();
    bool ShouldMoveArmTopPreset();
    bool ShouldGrabGamePiece();

    // roller
    bool ShouldRollerSpit();
    bool ShouldRollerSuck();
    bool ShouldRollerRotateUp();
    bool ShouldRollerRotateDown();
    bool ShouldRollerBeAutomated();
    bool ShouldRollerCommenceAutomation();
    bool GetOperatorThrottle();

    // automated ringer deployment with arm controls
    bool ShouldMoveArmToMiddle();
    bool ShouldReleaseRingerWithArm();
    bool ShouldMoveArmUpAndLiftDown();

    // automated ringer deployment with lift controls
    bool ShouldReleaseRingerWithLift();

    // shifting
    bool ShouldShiftHigh();
    bool ShouldToggleGear();
    bool ShouldShiftThird();

    // minibot deployment
    bool ShouldDeployAligner();
    bool ShouldDeployMinibot();

    // encoder data collection
    bool ShouldCollectEncoderData();

    // config
    bool ShouldLoadConfig();
    bool ShouldSaveConfig();
    bool ShouldApplyConfig();

    void UpdateDebouncing();
};

#endif
