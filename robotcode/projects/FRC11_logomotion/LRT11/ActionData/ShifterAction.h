#ifndef SHIFTER_ACTION_H
#define SHIFTER_ACTION_H
#include "..\ActionData.h"

struct ShifterAction
{
    ACTION::GEARBOX::eGearboxState gear;
    bool force;
};
#endif //SHIFTER_ACTION_H
