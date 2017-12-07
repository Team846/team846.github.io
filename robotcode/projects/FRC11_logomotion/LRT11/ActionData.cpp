#include "ActionData.h"
#include "ActionData\ArmAction.h"
#include "ActionData\LiftAction.h"
#include "ActionData\DriveAction.h"
#include "ActionData\RollerAction.h"
#include "ActionData\DeployerAction.h"
#include "ActionData\ShifterAction.h"
#include "ActionData\ConfigAction.h"


ActionData* ActionData::instance = NULL;

ActionData& ActionData::GetInstance()
{
    if(instance == NULL)
        instance = new ActionData();
    return *instance;
}

ActionData::ActionData()
{
    // used to abort movements
    master.abort = false;

    driveTrain = (DriveAction*) malloc(sizeof(DriveAction));
    driveTrain->mode = ACTION::DRIVETRAIN::SPEED;

    driveTrain->rate.rawForward = 0;
    driveTrain->rate.rawTurn = 0;
    driveTrain->rate.brakeLeft = false;
    driveTrain->rate.brakeRight = false;
    driveTrain->rate.thirdGear = false;
    // closed loop should default to on
    driveTrain->rate.usingClosedLoop = true;

    driveTrain->position.givenCommand = false;
    driveTrain->position.shouldMoveDistance = false;
    driveTrain->position.shouldTurnAngle = false;
    driveTrain->position.turnSetPoint = 0.0;
    driveTrain->position.distanceSetPoint = 0.0;
    driveTrain->position.maxFwdSpeed = 1.0;
    driveTrain->position.maxTurnSpeed = 1.0;
    driveTrain->position.done = false;

    driveTrain->distance.givenCommand = false;
    driveTrain->distance.distanceDutyCycle = 0.0;
    driveTrain->distance.distanceSetPoint = 0.0;
    driveTrain->distance.done = false;

    wasDisabled = true;

//    positionTrain.shouldMoveDistance = false;
//    positionTrain.shouldTurnAngle = false;
//    positionTrain.shouldOutputMoveDistance = false;
//    positionTrain.shouldOutputTurnAngle = false;
//    positionTrain.moveDistance = 0;
//    positionTrain.turnAngle = 0;
//    positionTrain.pivotLeft = false;
//    positionTrain.pivotRight = false;
//    positionTrain.done = false;
//    positionTrain.enabled = false;
//    // closed loop should default to on
//    positionTrain.usingClosedLoop = true;

    lift = (LiftAction*) malloc(sizeof(LiftAction));
    lift->givenCommand = false;
    lift->manualMode = false;
    lift->power = 0;
    lift->highColumn = false;
    lift->lift_preset = ACTION::LIFT::STOWED;
    lift->completion_status = ACTION::IN_PROGRESS;

    demoLift.power = 0;

    arm = (ArmAction*) malloc(sizeof(ArmAction));//I'm sure there is a c++ way to do this, if you see it please fix it
    arm->state = ACTION::ARM_::IDLE;
    arm->completion_status = ACTION::IN_PROGRESS;

    roller = (RollerAction*) malloc(sizeof(RollerAction));
    roller->state = ACTION::ROLLER::STOPPED;
    // if in roller->ROTATING state, default to rotating upward
    roller->rotateUpward = true;
    roller->automated = false;
    roller->commenceAutomation = false;
    roller->maxSuckPower = 1.0;

    deployer = (DeployerAction*) malloc(sizeof(DeployerAction));
    deployer->shouldAlignerRelease = false;
    deployer->shouldDeployMinibot = false;

    encoderData.shouldCollect = false;

    shifter = (ShifterAction*) malloc(sizeof(ShifterAction));
    shifter->gear = ACTION::GEARBOX::LOW_GEAR;
    shifter->force = false;

    config = (ConfigAction*) malloc(sizeof(ConfigAction));
    config->load = false;
    config->save = false;
    config->apply = false;
}

ActionData::~ActionData()
{
    free(arm);
    free(lift);
    free(driveTrain);
    free(roller);
    free(deployer);
    free(shifter);
    free(config);
}
