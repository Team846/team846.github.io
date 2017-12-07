#ifndef LIFT_ACTION_H
#define LIFT_ACTION_H
#include "..\ActionData.h"

struct LiftAction
{
    bool givenCommand;
    bool manualMode;
    float power;

    bool highColumn;
    ACTION::LIFT::ePresets lift_preset;
    ACTION::eCompletionStatus completion_status;
};
#endif //LIFT_ACTION_H
