#ifndef SHIFTER_H_
#define SHIFTER_H_

#include "..\General.h"
#include "Component.h"
#include "..\Config\Configurable.h"

class Config;
class DriveEncoders;
class LRTServo;
class VirtualLRTServo;

class Shifter : public Component, public Configurable
{
private:
#ifdef VIRTUAL
    VirtualLRTServo* leftServo, *rightServo;
#else
    LRTServo* leftServo, *rightServo;
#endif

    DriveEncoders& encoders;

    string config_section;
    
    //only let servo become disabled after a delay.
    //when Hitec HS322 servo is disabled, it jumps from it's last set point. -dg
    int servoDisableTimer;
    int servoDisableDelay; // 5 sec

    int lowGearServoValLeft;
    int highGearServoValLeft;
    int lowGearServoValRight;
    int highGearServoValRight;
/*
#ifdef LRT_ROBOT_2011
    //2011 robot
    float leftLowGearServoVal = 0.25; // = 0.33;
    float leftHighGearServoVal = 0.25 + 0.29; //= 0.62;

    float rightLowGearServoVal = 0.67;
    float rightHighGearServoVal = 0.39;
#else
    //2010 robot
    float leftLowGearServoVal = 0.85; //.62
    float leftHighGearServoVal = 0.36; //.33

    float rightLowGearServoVal = 0.67;
    float rightHighGearServoVal = 0.30;
#endif //LRT_ROBOT_2011 - servo endpoints
*/
public:
    Shifter();
    virtual ~Shifter();

    void virtual Output();
    virtual void Configure();
    virtual string GetName();
};

#endif //SHIFTER_H_
