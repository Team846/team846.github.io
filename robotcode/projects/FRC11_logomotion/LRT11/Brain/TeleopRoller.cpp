#include "Brain.h"
#include "..\ActionData\LiftAction.h"
#include "..\ActionData\RollerAction.h"


void Brain::TeleopRoller()
{
    if(inputs.ShouldRollerSpit())
        action.roller->state = ACTION::ROLLER::SPITTING;
    // grab game piece is a driver-controlled button
    else if(inputs.ShouldGrabGamePiece() || inputs.ShouldRollerSuck())
        action.roller->state = ACTION::ROLLER::SUCKING;
    // spitting the ringer out (automated)
    else if(inputs.ShouldRollerBeAutomated() || action.roller->automated)
    {
        static enum
        {
//            ROTATING,
            MOVING_LIFT_DOWN,
        } state = MOVING_LIFT_DOWN;

        static int timer = 0;

        // begin, or set the state, only when the button is just pressed
        // or from autonomous when action.roller->commenceAutomation == true
        if(inputs.ShouldRollerCommenceAutomation() || action.roller->commenceAutomation)
        {
            action.roller->commenceAutomation = false;
            timer = 0;
            state = MOVING_LIFT_DOWN;
        }

        switch(state)
        {
        case MOVING_LIFT_DOWN:
            action.lift->givenCommand = true;
            action.lift->manualMode = true;
            action.lift->power = -0.4;

            // keep moving down for 2/5 of a second
            if(++timer > 20)
                action.roller->state = ACTION::ROLLER::SPITTING;
            break;
        }
    }
    // rotate roller upward
    else if(inputs.ShouldRollerRotateUp())
    {
        action.roller->rotateUpward = true;
        action.roller->state = ACTION::ROLLER::ROTATING;
    }
    // rotate roller downaward
    else if(inputs.ShouldRollerRotateDown())
    {
        action.roller->rotateUpward = false;
        action.roller->state = ACTION::ROLLER::ROTATING;
    }
    // spit out ringer onto the peg
    else
        action.roller->state = ACTION::ROLLER::STOPPED;
}
