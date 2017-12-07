#include "CLPositionDriveTrain.h"
#include <math.h>
CLPositionDriveTrain::CLPositionDriveTrain(ClosedLoopRateDrivetrain& train)
    : drive(train)
    , encoders(DriveEncoders::GetInstance())
{
    // set default values for all structs
    movePositionInfo.target = 0.0;
    movePositionInfo.initialBearing = 0;
    movePositionInfo.hasCommand = false;

    turnAngleInfo.target = 0;
    turnAngleInfo.initialDistance = 0.0;
    turnAngleInfo.hasCommand = false;

    moveDistanceInfo.target = 0.0;
    moveDistanceInfo.initialBearing = 0;
    moveDistanceInfo.goingForward = true;
    moveDistanceInfo.hasCommand = false;

    printf("Constructed CLPositionDriveTrain\n");
}

void CLPositionDriveTrain::Configure()
{
    Config& config = Config::GetInstance();
    const string configSection = "CLPositionDriveTrain";

    pGainFwd = config.Get<float>(configSection, "pGainFwd", 1.5);
    pGainFwdTurnCorrection = config.Get<float>(configSection, "pGainFwdTurnCorrection", 0.01);
    pGainTurn = config.Get<float>(configSection, "pGainTurn", 1.5);
    pGainTurnFwdCorrection = config.Get<float>(configSection, "pGainTurnFwdCorrection", 0.02);

    fwdDeadband = config.Get<float>(configSection, "fwdDeadband", 0.05);
    turnDeadband = config.Get<float>(configSection, "turnDeadband", 0.05);
}

DriveCommand CLPositionDriveTrain::Drive(float maxFwdSpeed, float maxTurnSpeed)
{
    bool done = true; // assume no command, so the drive train is done
    float fwdCorrection = 0.0, turnCorrection = 0.0;

    if(movePositionInfo.hasCommand)
    {
        float error = movePositionInfo.target - encoders.GetRobotDist();
        fwdCorrection = error * pGainFwd;

//        AsynchronousPrinter::Printf("Moving error: %.2f\n", error);

        int turnError = movePositionInfo.initialBearing - encoders.GetTurnTicks();
        turnCorrection = turnError * pGainFwdTurnCorrection;
        done = fabs(error) < fwdDeadband;
    }
    else if(turnAngleInfo.hasCommand)
    {
        int error = turnAngleInfo.target - encoders.GetTurnTicks();
        turnCorrection = error * pGainTurn;

//        AsynchronousPrinter::Printf("Turning error: %d\n", error);

        float fwdError = turnAngleInfo.initialDistance - encoders.GetRobotDist();
        fwdCorrection = pGainTurnFwdCorrection * fwdError;
        done = fabs(error) < turnDeadband;
    }

    fwdCorrection = Util::Clamp<float>(fwdCorrection, -maxFwdSpeed, maxFwdSpeed);
    turnCorrection = Util::Clamp<float>(turnCorrection, -maxTurnSpeed, maxTurnSpeed);

    return drive.Drive(fwdCorrection, turnCorrection);
}

CLPositionCommand CLPositionDriveTrain::DriveAtLeastDistance(float dutyCycle)
{
    CLPositionCommand command;
    dutyCycle = fabs(dutyCycle); // duty cycle is always >= 0

    float fwdCorrection = 0, turnCorrection = 0;

    if(moveDistanceInfo.hasCommand)
    {
        float turnError = moveDistanceInfo.initialBearing - encoders.GetTurnTicks();
        turnCorrection = turnError * pGainFwdTurnCorrection;

        if(moveDistanceInfo.goingForward)
            fwdCorrection = dutyCycle;
        else
            fwdCorrection = -dutyCycle;

        float error = moveDistanceInfo.target - encoders.GetRobotDist();
        if(moveDistanceInfo.goingForward)
            command.done = error <= 0;
        else
            command.done = error >= 0;
    }

    command.drive = drive.Drive(fwdCorrection, turnCorrection);
    return command;
}

void CLPositionDriveTrain::SetMovePosition(float distance_in)
{
    movePositionInfo.target = encoders.GetRobotDist() + distance_in;
    movePositionInfo.initialBearing = encoders.GetTurnTicks();
    movePositionInfo.hasCommand = true;
    turnAngleInfo.hasCommand = false; // either run move or turn
}

void CLPositionDriveTrain::SetTurnAngle(float angle_dg)
{
    turnAngleInfo.target = (int)(encoders.GetTurnTicks() + angle_dg / 360.0 * DriveEncoders::TICKS_PER_FULL_TURN);
    turnAngleInfo.initialDistance = encoders.GetRobotDist();
    turnAngleInfo.hasCommand = true;
    movePositionInfo.hasCommand = false; // either run move or turn
}

void CLPositionDriveTrain::SetMoveDistance(float distance_in)
{
    moveDistanceInfo.target = encoders.GetRobotDist() + distance_in;
    moveDistanceInfo.initialBearing = encoders.GetTurnTicks();
    moveDistanceInfo.goingForward = distance_in > 0;
    moveDistanceInfo.hasCommand = true;
}

/*

void CLPositionDriveTrain::TurnAngle(float angle, bool pivotLeft, bool pivotRight)
{
    turnAngleInfo.target = encoders.GetTurnAngle() - angle;
    turnAngleInfo.distance = encoders.GetRobotDist();
    turnAngleInfo.pivotLeft = pivotLeft;
    turnAngleInfo.pivotRight = pivotRight;
    turnAngleInfo.hasCommand = true;
}

bool CLPositionDriveTrain::MoveDistanceOutput()
{
    if(!moveDistanceInfo.hasCommand)
        return true;

    drive.SetClosedLoopEnabled(true);
    drive.SetHighGear(false);

    float error = (moveDistanceInfo.target - encoders.GetRobotDist());
//    float newError = forwardRunningError.UpdateSum(error);

    // arcade drive assumes inputs are within [-1,1] interval
    // limit max speed to 50%
    float correction = Util::Clamp<float>(error * pGainFwd, -0.1, 0.1);

    float turnError = moveDistanceInfo.delta - encoders.GetTurnTicks();
    float turnCorrection = turnError * pGainFwdTurnCorrection;

    AsynchronousPrinter::Printf("E: %.2f, C: %.2f, TE: %.2f, TC: %.2f\n", error,
            correction, turnError, turnCorrection);
//    drive.ArcadeDrive(correction, turnCorrection);
    drive.ArcadeDrive(0.1, 0);

#ifdef USE_DASHBOARD
//    SmartDashboard::Log(newError, "CLPosition Error");
//    SmartDashboard::Log(correction, "CLPosition Correction");
#endif

    return Util::Abs<float>(error) < fwdDeadband;
}

bool CLPositionDriveTrain::TurnAngleOutput()
{
    if(!turnAngleInfo.hasCommand)
        return true;

    float error = (turnAngleInfo.target - encoders.GetTurnAngle());
    float newError = turnRunningError.UpdateSum(error);

    // arcade drive, pivot left, and pivot right all assume inputs are within [-1,1] interval
    float correction = Util::Clamp<float>(newError * pGainTurn, -1, 1);

    // output based on pivot flags
    if(turnAngleInfo.pivotLeft)
        drive.PivotLeft(correction);
    else if(turnAngleInfo.pivotRight)
        drive.PivotRight(correction);
    else
    {
        float fwdError = turnAngleInfo.distance - encoders.GetRobotDist();
        float fwdCorrection = pGainTurnFwdCorrection * fwdError;

        drive.ArcadeDrive(fwdCorrection, correction);
    }

#ifdef USE_DASHBOARD
//    SmartDashboard::Log(newError, "Turning error (position)");
//    SmartDashboard::Log(correction, "Turning correction (position)");
#endif

    return Util::Abs<float>(error) < turnDeadband;
}

void CLPositionDriveTrain::Stop()
{
    drive.Stop();
}

void CLPositionDriveTrain::SetClosedLoopEnabled(bool enabled)
{
    drive.SetClosedLoopEnabled(enabled);
}
*/
