#include "Brain.h"
#include "..\ActionData\LiftAction.h"
#include "..\ActionData\DriveAction.h"
#include "..\ActionData\DeployerAction.h"

void Brain::Disabled()
{
    // reset action data values
    action.driveTrain->rate.rawForward = 0;
    action.driveTrain->rate.rawTurn    = 0;

    action.driveTrain->position.givenCommand = false;
    action.driveTrain->distance.givenCommand = false;

    action.lift->givenCommand = false;

    // default to locked aligner
    action.deployer->shouldAlignerRelease = false;

    hasMoved = false;
    wasDisabledLastCycle = true;

    static int cycleCount = 0;
    if(++cycleCount % 50 == 0) // check every second
        Config::GetInstance().CheckForFileUpdates();
}
