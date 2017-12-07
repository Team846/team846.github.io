#ifndef ARM_ACTION_H
#define ARM_ACTION_H
#include "..\ActionData.h"

struct ArmAction
{
    ACTION::ARM_::eStates state;
    ACTION::eCompletionStatus completion_status;
};
#endif //ARM_ACTION_H
