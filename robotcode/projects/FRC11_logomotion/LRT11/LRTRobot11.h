#ifndef LRT_ROBOT_11_H_
#define LRT_ROBOT_11_H_

#include "General.h"
#include "LRTRobotBase.h"
#include "Brain/Brain.h"
#include "Components/Component.h"
#include "Config/Config.h"
#include "wdLib.h"
#include "sysLib.h"

#include "Util/AsynchronousPrinter.h"
#include "Util/PrintInConstructor.h"

class LRTRobot11 : public LRTRobotBase
{
public:
    PrintInConstructor firstMember_;
    LRTRobot11();
    virtual ~LRTRobot11();

    virtual void RobotInit();
    virtual void MainLoop();

private:
    Brain brain;

    PrintInConstructor dc_CANBus_;
#ifdef VIRTUAL
    VirtualCANBusController& controller;
#else
//    CANBusController& controller;
#endif

    Config& config;

    DriverStation& ds;
//    DigitalOutput switchLED;

    GameState prevState;
    GameState DetermineState();

    list<ComponentWithData>* components;

//    AnalogChannel armPot;
//    WDOG_ID mainLoopWatchDog;
    PrintInConstructor lastMember_; //trace constructor/destructor.
};

#endif
