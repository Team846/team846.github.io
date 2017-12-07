#ifndef ROLLER_ACTION_H
#define ROLLER_ACTION_H
#include "..\ActionData.h"

struct RollerAction
{
    ACTION::ROLLER::eStates state;

    // true to rotate upward, false to rotate downward;
    // only active in ROTATING state
    bool rotateUpward;

    // used to automate roller spitting (rotate + reverse roller)
    bool automated;
    bool commenceAutomation;

    float maxSuckPower;
};
#endif //ROLLER_ACTION_H
