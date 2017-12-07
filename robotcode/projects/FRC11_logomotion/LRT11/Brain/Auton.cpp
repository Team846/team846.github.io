#include "Brain.h"
#include "..\ActionData\ArmAction.h"
#include "..\ActionData\LiftAction.h"
#include "..\ActionData\DriveAction.h"
#include "..\ActionData\ShifterAction.h"
#include "..\ActionData\RollerAction.h"

void Brain::Auton()
{
//    // TODO remove return statement after auton is legitimate
//    return;
//
//    // robot starts at either a side or the middle
//    static enum
//    {
//        SIDE,
//        MIDDLE,
//        DUMB
//    } startPosition;
//
//    // position determined by digital inputs
//    if(ds.GetDigitalIn(1))
//        startPosition = SIDE;
//    else if(ds.GetDigitalIn(2))
//        startPosition = MIDDLE;
//    else
//        startPosition = DUMB;
//
//    // call state-specific methods
//    switch(startPosition)
//    {
//    case SIDE:
//        Side();
//        break;
//    case MIDDLE:
//        Middle(ds.GetDigitalIn(2) ? 2 : 1);
//        break;
//    case DUMB:
    TeleopRoller();
    EncoderAuton();
//        break;
//    }
//
//    // includes automated routines such as line sensing and
//    // dead-reckoning autonomous mode
////    AutomatedRoutines();
}

void Brain::EncoderAuton()
{
//    static int e = 0;
//    if(++e % 10 == 1)
//    {
//        for(int i = 1; i <= 8; i++)
//            AsynchronousPrinter::Printf("%d ", ds.GetDigitalIn(i));
//        AsynchronousPrinter::Printf("\n");
//    }
////  if (!PauseOnDS_input(8))
////      AsynchronousPrinter::Printf("Switch\n" );
//    return;

    static char* stateName = "unknown state";

    enum
    {
        DRIVE_FORWARD,
        WAIT_FOR_DRIVE_FORWARD,
        STALL_DETECTION,
        STEP_BACK,
        MOVE_LIFT_UP,
        WAIT_FOR_MOVE_LIFT_UP,
        ROTATE_ROLLER,
        RELEASE_GAMEPIECE_START,
        RELEASE_GAMEPIECE,
        MOVE_LIFT_DOWN_START,
        WAIT_FOR_MOVE_LIFT_DOWN,
        SETUP_DRIVE_BACK,
        DRIVE_BACK,
        TURN_AROUND,
        IDLE,
    };
    static int state = DRIVE_FORWARD;

    static int timer = 0;
    static int timeout = 0;
    static int wait  = 0;

    if(wasDisabledLastCycle)
    {
        timer = 0;
        timeout = 0;
        wait  = 0;
        state = DRIVE_FORWARD;
    }

    static bool advanceState = false;
    static bool canPause     = true;

    if(wait > 0)
    {
        wait--;
        return;
    }

    if(advanceState)
    {
//#define PAUSE_AUTON
#ifdef PAUSE_AUTON
        // waits until key is released
        if(canPause && PauseOnDS_input(8))
        {
            AsynchronousPrinter::Printf("Pausing\n", stateName);
            return;
        }
#endif
        AsynchronousPrinter::Printf("Finished %s\n", stateName);
        state++;
        canPause = true;
        advanceState = false;
        timer = 0;
        timeout = 0;
    }

    // disabled because of excessive printing -KV -DG championships 4/28/11
#define PRINTSTATE() do { } while(false) // AsynchronousPrinter::Printf("Entering %s\n", stateName)
    switch(state)
    {
    case DRIVE_FORWARD: // move drive to a location; move arm up; shift to low gear
        stateName = "DRIVE_FORWARD";
        PRINTSTATE();
        action.driveTrain->mode = ACTION::DRIVETRAIN::DISTANCE;
        action.driveTrain->distance.givenCommand = true;

        action.driveTrain->distance.distanceSetPoint = 15.0 * 12; // 15 feet
//        action.driveTrain->distance.distanceSetPoint = 1.0 * 12; // 15 feet
        action.driveTrain->distance.distanceDutyCycle = 0.8;

        action.driveTrain->distance.done = false;
        // arm should stay in top position
        action.arm->state = ACTION::ARM_::PRESET_TOP;

        // low gear driving
        action.shifter->gear = ACTION::GEARBOX::LOW_GEAR;
        action.shifter->force = true;

        advanceState = true;
        canPause = false;
        break;

    case WAIT_FOR_DRIVE_FORWARD: //detects when we have driven our distance
        stateName = "WAIT_FOR_DRIVE_FORWARD";
        PRINTSTATE();
        if(action.driveTrain->distance.done)
        {
            advanceState = true;
            action.driveTrain->mode = ACTION::DRIVETRAIN::SPEED;
            action.driveTrain->rate.rawForward = 0.0;
            action.driveTrain->rate.rawTurn = 0.0;
        }
        break;

    case STALL_DETECTION:
        // turns off closed loop and tells the drive to move at a speed
        // looks if speed is below a threshold to detect a stall

        stateName = "STALL_DETECTION";
//        AsynchronousPrinter::Printf("entering %s\n", stateName);
        action.driveTrain->mode = ACTION::DRIVETRAIN::SPEED;
        action.driveTrain->rate.usingClosedLoop = false;

        action.driveTrain->rate.rawForward = 0.2;
        action.driveTrain->rate.rawTurn = 0.0;

        // Wait until robot has accelerated
        if(driveEncoders.NormalizedForwardMotorSpeed() > 0.1)
            timer = 51; // bypass the timer below

        // advance state when stalled
        // or after a timeout for testing in pits
        if((++timer > 50 &&
                driveEncoders.NormalizedForwardMotorSpeed() < 0.05) || ++timeout > 200)
        {
            action.driveTrain->rate.usingClosedLoop = true;
            action.driveTrain->rate.rawForward = 0.0;
            action.driveTrain->rate.rawTurn = 0.0;
            advanceState = true;
        }
        break;

    case STEP_BACK:
        stateName = "STEP_BACK";
//        AsynchronousPrinter::Printf("entering %s\n", stateName);

        action.driveTrain->mode = ACTION::DRIVETRAIN::POSITION;
        action.driveTrain->position.givenCommand = true;

        action.driveTrain->position.shouldMoveDistance = true;
        action.driveTrain->position.shouldTurnAngle = false;

        action.driveTrain->position.distanceSetPoint = -3.0; // 3 inches back
        action.driveTrain->position.turnSetPoint = 0.0;

        action.driveTrain->position.maxFwdSpeed = 0.2;
        action.driveTrain->position.maxTurnSpeed = 1.0;

        wait = 50; //give us 1s to move back
        advanceState = true;
        break;

    case MOVE_LIFT_UP:
#ifndef LRT_ROBOT_2011
        state = SETUP_DRIVE_BACK; //skip to driving back
        AsynchronousPrinter::Printf("skipping lift operations\n");
        break;
#endif
        stateName = "MOVE_LIFT_UP";
//        AsynchronousPrinter::Printf("entering %s\n", stateName);
        action.lift->givenCommand = true;
        // read from the driverstation if it is a low/high peg.
        action.lift->highColumn = ds.GetDigitalIn(2);

        action.lift->lift_preset = ACTION::LIFT::HIGH_PEG;
        action.lift->manualMode = false;
        advanceState = true;

        break;

    case WAIT_FOR_MOVE_LIFT_UP:
        stateName = "WAIT_FOR_MOVE_LIFT_UP";
//        AsynchronousPrinter::Printf("entering %s\n", stateName);
        if(action.lift->completion_status != ACTION::IN_PROGRESS) // message is available
        {
            if(action.lift->completion_status == ACTION::SUCCESS)
                advanceState = true;
            else // lift operation failed; abort
                state = IDLE;
        }
        break;

    case ROTATE_ROLLER:
        stateName = "ROTATE_ROLLER";
//        AsynchronousPrinter::Printf("entering %s\n", stateName);
        action.roller->rotateUpward = false; //go down
        action.roller->state = ACTION::ROLLER::ROTATING;

        if(++timer >= 75)
            advanceState = true;
        break;

    case RELEASE_GAMEPIECE_START:
        // automated roller routine starts moving the lift down and spits the gamepiece out
        stateName = "RELEASE_GAMEPIECE_START";
//        AsynchronousPrinter::Printf("entering %s\n", stateName);
        action.roller->automated = true;
        action.roller->commenceAutomation = true;
        advanceState = true;

        canPause = false; // pausing will cause the lift to keep moving down
        break;

    case RELEASE_GAMEPIECE:
        stateName = "RELEASE_GAMEPIECE";
//        AsynchronousPrinter::Printf("entering %s\n", stateName);

        // release gamepiece for 1/2 seconds
        if(++timer >= 25)
        {
            // stop automating; stop rollers
            action.roller->automated = false;
            advanceState = true;
        }
        break;

    case MOVE_LIFT_DOWN_START:
        stateName = "MOVE_LIFT_DOWN_START";
//        AsynchronousPrinter::Printf("entering %s\n", stateName);
        action.lift->givenCommand = true;
        // depends on if the robot is in the middle or on the side
        action.lift->highColumn = ds.GetDigitalIn(2);

        action.lift->lift_preset = ACTION::LIFT::LOW_PEG;
        action.lift->manualMode = false;
        advanceState = true;
        break;

    case WAIT_FOR_MOVE_LIFT_DOWN:
        stateName = "WAIT_FOR_MOVE_LIFT_DOWN";
//        AsynchronousPrinter::Printf("entering %s\n", stateName);

        if(action.lift->completion_status == ACTION::SUCCESS)
            advanceState = true;
        else if(action.lift->completion_status == ACTION::FAILURE ||
                action.lift->completion_status == ACTION::ABORTED)
            state = IDLE; // abort auton routine

        break;

    case SETUP_DRIVE_BACK:
        stateName = "SETUP_DRIVE_BACK";
//        AsynchronousPrinter::Printf("entering %s\n", stateName);
        action.driveTrain->mode = ACTION::DRIVETRAIN::POSITION;
        action.driveTrain->position.givenCommand = true;

        action.driveTrain->position.shouldMoveDistance = true;
        action.driveTrain->position.shouldTurnAngle = false;

        action.driveTrain->position.distanceSetPoint = -14.5 * 12; // 14.5 feet back
        action.driveTrain->position.turnSetPoint = 0.0;

        action.driveTrain->position.maxFwdSpeed = 0.8;
        action.driveTrain->position.maxTurnSpeed = 1.0;

        advanceState = true;
        break;

    case DRIVE_BACK:
        stateName = "DRIVE_BACK";
//        AsynchronousPrinter::Printf("entering %s\n", stateName);
        if(++timer > 150)
            advanceState = true;
        break;

    case TURN_AROUND:
        stateName = "TURN_AROUND";
//        AsynchronousPrinter::Printf("entering %s\n", stateName);
        action.driveTrain->mode = ACTION::DRIVETRAIN::POSITION;
        action.driveTrain->position.givenCommand = true;

        action.driveTrain->position.shouldMoveDistance = false;
        action.driveTrain->position.shouldTurnAngle = true;

        action.driveTrain->position.distanceSetPoint = 0.0;
        action.driveTrain->position.turnSetPoint = 180.0; // 180 degrees

        action.driveTrain->position.maxFwdSpeed = 1.0;
        action.driveTrain->position.maxTurnSpeed = 1.0;

        advanceState = true;
        break;

    case IDLE:
        stateName = "IDLE";
//        AsynchronousPrinter::Printf("entering %s\n", stateName);
        // wait for turning to complete and do nothing
        break;

    default: //state not handled
        stateName = "Default";
        break;
    }
}

// returns false if the ds input has been released
// otherwise returns true
bool Brain::PauseOnDS_input(int softKeyNum)
{
    bool keyReleased = false;

    static bool wasPressed = false;
    bool isPressed = ds.GetDigitalIn(softKeyNum);
    if(wasPressed && !isPressed)
        keyReleased = true;
    wasPressed = isPressed;

    return !keyReleased;
}
