#include "Brain.h"
#include "..\ActionData\LiftAction.h"

void Brain::TeleopLift()
{
    action.lift->manualMode = false;

    // assume command is given; set to false in last else
    // statement (see below)
    action.lift->givenCommand = true;

    // determine which presets to use, as peg rows are staggered
    action.lift->highColumn = inputs.IsHighRow();

    // determine lift movement with else ifs for interlocks
    if(inputs.ShouldMoveLiftLow())
        action.lift->lift_preset = ACTION::LIFT::LOW_PEG;
    else if(inputs.ShouldMoveLiftMedium())
        action.lift->lift_preset = ACTION::LIFT::MED_PEG;
    else if(inputs.ShouldMoveLiftHigh())
        action.lift->lift_preset = ACTION::LIFT::HIGH_PEG;
    else if(inputs.ShouldManuallyPowerLift())
    {
        action.lift->manualMode = true;
        action.lift->power = inputs.GetLiftPower();
    }
    else
        // no command given
        action.lift->givenCommand = false;

    // if preset mode
//    if(action.lift->givenCommand && !action.lift->manualMode)
//    {
//        // do not run lift up unless arm is at top preset
//        if(!action.arm.presetTop ||
//                action.arm.doneState != action.arm.SUCCESS)
//            // setting givenCommand to false disables lift
//            action.lift->givenCommand = false;
//    }
}
