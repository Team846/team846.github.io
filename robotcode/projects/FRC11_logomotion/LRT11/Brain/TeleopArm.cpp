#include "Brain.h"
#include "..\ActionData\ArmAction.h"

void Brain::TeleopArm()
{
    static int timer = 0;

    if(inputs.ShouldMoveArmDown())
        action.arm->state = ACTION::ARM_::MANUAL_DOWN;
    else if(inputs.ShouldMoveArmUp())
        action.arm->state = ACTION::ARM_::MANUAL_UP;
    // driver wants the arm down and the roller to rotate
    else if(inputs.ShouldGrabGamePiece())
    {
        action.arm->state = ACTION::ARM_::PRESET_BOTTOM;
        timer = 0;
    }
    else if(inputs.ShouldMoveArmToMiddle())
    {
        action.arm->state = ACTION::ARM_::PRESET_MIDDLE;
        timer = 0;
    }
    else if(action.arm->completion_status == ACTION::FAILURE)
    {
        if(++timer > 100)
        {
            action.arm->state = ACTION::ARM_::IDLE;
            timer = 0;
        }
    }
    else if(action.arm->state != ACTION::ARM_::PRESET_MIDDLE)
        // default to the arm at the top state
        action.arm->state = ACTION::ARM_::PRESET_TOP;

    if(wasDisabledLastCycle)
        // must set to IDLE to register state change
        action.arm->state = ACTION::ARM_::IDLE;

    // operator preset control currently disabled, as driver
    // has the ability to move the arm up and down
//    else if(inputs.ShouldMoveArmTopPreset())
//        action.arm->presetTop = true;
//    else if(inputs.ShouldMoveArmBottomPreset())
//        action.arm->presetTop = false;
//    else
//        // no command given
//        action.arm->givenCommand = false;
}
