#ifndef GENERAL_H_
#define GENERAL_H_

#include "WPILib.h"

//#define VIRTUAL
#define LRT_ROBOT_2011
#define USE_DASHBOARD

typedef enum GameState
{
    DISABLED,
    AUTONOMOUS,
    TELEOPERATED
};

enum Side { LEFT, RIGHT };

#define LINEAR_DRIVE_INPUT
#ifndef LINEAR_DRIVE_INPUT
#define INPUT_POWER 2
#endif //LINEAR_DRIVE_INPUT

//different forms of loop timing
//#define INTERRUPT_CYCLES
#ifndef INTERRUPT_CYCLES
#define TIMER_CYCLES
#endif //INTERRUPT_CYCLES

// viable combinations are LRT_ROBOT_2011, LRT_ROBOT_2011 + VIRTUAL, neither
#ifdef VIRTUAL
#ifndef LRT_ROBOT_2011
#error "VIRTUAL mode only works with LRT_ROBOT_2011"
#endif //LRT_ROBOT_2011
#endif //Virtual

#endif //GENERAL_H_
