#include "Brain.h"
#include "..\ActionData.h"

void Brain::Teleop()
{
    //teleop driveTrain must be called before teleop shifter to work
    TeleopDriveTrain();
    TeleopShifter();

    TeleopLift();
    TeleopArm();
    TeleopRoller();

    TeleopMinibot();

    AutomatedRoutineWithLift();

    if(inputs.ShouldAbort())
        action.master.abort = true;
    else
        action.master.abort = false;
}
