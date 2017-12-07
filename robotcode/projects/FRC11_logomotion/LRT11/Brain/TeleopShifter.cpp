#include "Brain.h"
#include "..\ActionData\DriveAction.h"
#include "..\ActionData\ShifterAction.h"

void Brain::TeleopShifter()
{
    // used to only force shift the first time
//    static bool forceShiftedLow = false;
//    static bool forceShiftedHigh = false;

    //must set this way since is static
    static int lastGear = action.shifter->gear;

    // assume no force shift
    action.shifter->force = false;

    if(inputs.ShouldToggleGear())
    {
        action.shifter->force = true;

        if(lastGear == ACTION::GEARBOX::LOW_GEAR)
            action.shifter->gear = ACTION::GEARBOX::HIGH_GEAR;
        else
            action.shifter->gear = ACTION::GEARBOX::LOW_GEAR;
    }

    // always low gear unless shift high button is pushed down
//    if(inputs.ShouldShiftHigh())
//    {
//        // only force shift the first time
//        if(!forceShiftedHigh)
//        {
//            action.shifter->gear = action.shifter->HIGH_GEAR;
//            action.shifter->force = true;
//            forceShiftedHigh = true;
//        }
//        // shifted to high gear; set low gear flag to false
//        forceShiftedLow = false;
//    }
//    else
//    {
//        // only force shift the first time
//        if(!forceShiftedLow)
//        {
//            action.shifter->gear = action.shifter->LOW_GEAR;
//            action.shifter->force = true;
//            forceShiftedLow = true;
//        }
//
//        // shifted to low gear; set high gear flag to false
//        forceShiftedHigh = false;
//    }

    // commenced a shift; synchronize the drive train
    if(lastGear != action.shifter->gear)
        action.driveTrain->mode = ACTION::DRIVETRAIN::SYNCHRONIZING;

    lastGear = action.shifter->gear;
}
