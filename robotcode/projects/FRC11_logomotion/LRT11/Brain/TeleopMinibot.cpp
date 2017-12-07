#include "Brain.h"
#include "..\ActionData\DeployerAction.h"

void Brain::TeleopMinibot()
{
    // aligner to square up with the tower
    if(inputs.ShouldDeployAligner())
        action.deployer->shouldAlignerRelease = true;

    // used to determine if the button is let go of and
    // pressed again
    static bool deployButtonJustPressed = false;

    // assume that the minibot should not be deployed
    action.deployer->shouldDeployMinibot = false;

    // only deploy minibot in the finale; let operator push
    // button down while waiting
//    if(isFinale)
//    {
    if(inputs.ShouldDeployMinibot())
    {
        // only reactivate minibot deployment if button
        // is pressed again
        if(!deployButtonJustPressed)
        {
            action.deployer->shouldDeployMinibot = true;
            deployButtonJustPressed = true;
        }
    }
    else
        // operator let button go; reset just pressed flag
        deployButtonJustPressed = false;
//    }
}
