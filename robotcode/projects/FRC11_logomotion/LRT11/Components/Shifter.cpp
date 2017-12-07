#include "Shifter.h"
#include "../Util/AsynchronousPrinter.h"
#include "Shifter\LRTServo.h"
#include "Shifter\VirtualLRTServo.h"
#include "..\Sensors\DriveEncoders.h"
#include "..\Config\RobotConfig.h"
#include "..\Config\Config.h"
#include "..\ActionData\DriveAction.h"
#include "..\ActionData\ShifterAction.h"


Shifter::Shifter()
    : Component()
    , encoders(DriveEncoders::GetInstance())
    , config_section("Shifter")
{
#ifdef VIRTUAL
    leftServo = new VirtualLRTServo(RobotConfig::PWM::LEFT_GEARBOX_SERVO, "Left Shift Servo")
    rightServo = new VirtualLRTServo(RobotConfig::PWM::RIGHT_GEARBOX_SERVO, "Right Shift Servo")
#else
    leftServo  = new LRTServo(RobotConfig::PWM::LEFT_GEARBOX_SERVO, "Left Shift Servo");
    rightServo = new LRTServo(RobotConfig::PWM::RIGHT_GEARBOX_SERVO, "Right Shift Servo");
#endif
    puts("Constructed Shifter");
}

Shifter::~Shifter()
{
    delete leftServo;
    delete rightServo;

}

void Shifter::Configure()
{
	// TODO: add servo values into config
    Config& config = Config::GetInstance();
    lowGearServoValLeft = config.Get<int>(config_section, "leftLowGearServoVal", 1700);
    highGearServoValLeft = config.Get<int>(config_section, "leftHighGearServoVal", 1050);
    lowGearServoValRight = config.Get<int>(config_section, "rightLowGearServoVal", 1100);
    highGearServoValRight = config.Get<int>(config_section, "rightHighGearServoVal", 1850);
    servoDisableDelay = config.Get<int>(config_section, "servoDisableDelay", 5 * 50);
}

void Shifter::Output()
{
#ifdef USE_DASHBOARD
//    SmartDashboard::Log(leftServo->Get(), "Left servo position");
//    SmartDashboard::Log(rightServo->Get(), "Right servo position");
#endif

//    DriverStation& station = *DriverStation::GetInstance();
//    leftServo->Set(station.GetAnalogIn(1));
//    rightServo->Set(station.GetAnalogIn(2));
//    return;

    //Power down servos if robot is not moving for several seconds; governed by servoDisableTimer -dg
    if(servoDisableTimer > 0)
        servoDisableTimer--;

    const bool robotTryingToMove =
        (action.driveTrain->rate.rawForward != 0.0 || action.driveTrain->rate.rawTurn != 0.0);

    if(robotTryingToMove || action.shifter->force)
        servoDisableTimer = servoDisableDelay; //reset timer

    bool enableServo = servoDisableTimer > 0 ;

    leftServo->SetEnabled(enableServo);
    rightServo->SetEnabled(enableServo);

    switch(action.shifter->gear)
    {
    case ACTION::GEARBOX::LOW_GEAR:
        leftServo->SetMicroseconds(lowGearServoValLeft);
        rightServo->SetMicroseconds(lowGearServoValRight);
        encoders.SetHighGear(false);
        break;

    case ACTION::GEARBOX::HIGH_GEAR:
        leftServo->SetMicroseconds(highGearServoValLeft);
        rightServo->SetMicroseconds(highGearServoValRight);
        encoders.SetHighGear(true);
        break;

    default:
        AsynchronousPrinter::Printf("Fatal: %s:%d\n", __FILE__, __LINE__);
        break;
    }
}

string Shifter::GetName()
{
    return "Shifter";
}

