#ifndef CL_POSITION_DRIVE_TRAIN_H_
#define CL_POSITION_DRIVE_TRAIN_H_

#include "..\..\General.h"
#include "..\..\Sensors\DriveEncoders.h"
#include "..\..\Config\Config.h"
#include "..\..\Config\Configurable.h"
#include "..\..\Util\RunningSum.h"
#include "CLRateTrain.h"
#include "DitheredBrakeTrain.h"

typedef struct
{
    DriveCommand drive;
    bool done;
} CLPositionCommand;

class CLPositionDriveTrain : public Configurable
{
public:
    CLPositionDriveTrain(ClosedLoopRateDrivetrain& train);

    virtual void Configure();

    DriveCommand Drive(float maxFwdSpeed, float maxTurnSpeed);
    CLPositionCommand DriveAtLeastDistance(float dutyCycle);

    // position drive
    void SetMovePosition(float distance_in);
    void SetTurnAngle(float angle_dg);

    // distance drive
    void SetMoveDistance(float distance_in);

private:
    ClosedLoopRateDrivetrain& drive;
    DriveEncoders& encoders;

    float pGainFwd;
    float pGainFwdTurnCorrection;

    float pGainTurn;
    float pGainTurnFwdCorrection;

    float fwdDeadband;
    float turnDeadband;

    struct
    {
        float target;
        int initialBearing;
        bool hasCommand;
    } movePositionInfo;

    struct
    {
        int target;
        float initialDistance;
        bool hasCommand;
    } turnAngleInfo;

    struct
    {
        float target;
        int initialBearing;
        bool goingForward;
        bool hasCommand;
    } moveDistanceInfo;
};

#endif
