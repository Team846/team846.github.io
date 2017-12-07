#ifndef DRIVE_ACTION_H
#define DRIVE_ACTION_H
#include "..\ActionData.h"

struct DriveAction
{
    ACTION::DRIVETRAIN::eMode mode;
    struct
    {
        float rawForward, rawTurn;
        bool brakeLeft, brakeRight;
        bool usingClosedLoop;
        bool thirdGear;
    } rate;

    struct
    {
        bool givenCommand;
        bool shouldMoveDistance, shouldTurnAngle;

        float distanceSetPoint, turnSetPoint; // inches, degrees
        float maxFwdSpeed, maxTurnSpeed;

        bool done;
    } position;

    struct
    {
        bool givenCommand;

        float distanceDutyCycle;
        float distanceSetPoint;

        bool done;
    } distance;

    bool test;

};
#endif //DRIVE_ACTION_H
