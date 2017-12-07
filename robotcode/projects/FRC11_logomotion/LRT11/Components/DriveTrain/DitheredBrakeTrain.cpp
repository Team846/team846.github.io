#include "DitheredBrakeTrain.h"

DitheredBrakeTrain::DitheredBrakeTrain()
// TODO fix initialization
    : leftEncoder(DriveEncoders::GetInstance().GetLeftEncoder())
    , rightEncoder(DriveEncoders::GetInstance().GetRightEncoder())
{
    Configure();
}

DitheredBrakeTrain::~DitheredBrakeTrain()
{
    // nothing
}

void DitheredBrakeTrain::Configure()
{
//    Config& config = Config::GetInstance();
//    string prefix = "DitheredBrakeDrive.";

    // proportional constant used to determine amount of braking
    // brake gain * ( joystick input - robot speed ) = desired braking
//    brakeGain = config.Get<float>(prefix + "brakeGain", 1.0);
}
//
//DriveCommand DitheredBrakeTrain::Drive(float forwardInput, float turnInput)
//{
//    DriveCommand drive;
//
//    // positive turn is clockwise
//    drive.leftDC = forwardInput - turnInput;
//    drive.rightDC = forwardInput + turnInput;
//
//    if(Util::Abs<float>(drive.leftDC) > 1.0)
//    {
//        // TODO must decide whether forward or turn takes precedence
//        drive.leftDC = Util::Sign<float>(drive.leftDC) * 1.0;
//    }
//
//    if(Util::Abs<float>(drive.rightDC) > 1.0)
//    {
//        // TODO must decide whether forward or turn takes precedence
//        drive.rightDC = Util::Sign<float>(drive.rightDC) * 1.0;
//    }
//
//    // TODO make this based on low/high gear
//    // normalize speeds
//    float leftSpeed = leftEncoder.GetRate() / DriveEncoders::MAX_ENCODER_RATE;
//    float rightSpeed = rightEncoder.GetRate() / DriveEncoders::MAX_ENCODER_RATE;
//
//    // clamp speeds to [-1.0,1.0] interval
//    leftSpeed = Util::Clamp<float>(leftSpeed, -1.0, 1.0);
//    rightSpeed = Util::Clamp<float>(rightSpeed, -1.0, 1.0);
//
//    // assume no braking is necessary
//    drive.leftBrakeDC = 0.0;
//    drive.rightBrakeDC = 0.0;
//
//    // braking is necessary on the left side if leftSpeed > leftInput
//    // signs must be taken into account when comparing
//    // TODO add minimum difference to start braking
//
//    //  S    I   B
//    // 0.5, 1.0, N   - Check
//    // 1.0, 0.5, Y   - Check
//    // 1.0, -0.1, Y  - Check
//    // 0.0, -0.1, N  - Check
//    // 0.0, 0.1, N   - Check
//    // -1.0, 0.1, Y  - Check
//    // -1.0, -0.5, Y - Check
//    // -0.5, -1.0, N - Check
//    if(Util::Abs<float>(rightSpeed) > 3e-2 && (Util::Abs<float>(leftSpeed) > Util::Abs<float>(drive.leftDC) ||
//            Util::Sign<float>(leftSpeed) != Util::Sign<float>(drive.leftDC)))
//    {
//        float error = Util::Abs<float>(leftSpeed - drive.leftDC);
//
//        // braking is proportional to the error
//        float desiredBraking = brakeGain * error;
//
//        // brake duty cycle is equal to desired stall torque / normalized speed
//        drive.leftBrakeDC = desiredBraking / Util::Abs<float>(leftSpeed);
//        drive.leftDC = 0.0; // can only brake if speed controller is set to 0
//    }
//
//    // braking is necessary on the right side if rightSpeed > rightInput
//    // signs must be taken into account when comparing
//    // TODO add minimum difference to start braking
//    if(Util::Abs<float>(rightSpeed) > 3e-2 && (Util::Abs<float>(rightSpeed) > Util::Abs<float>(drive.rightDC) ||
//            Util::Sign<float>(rightSpeed) != Util::Sign<float>(drive.rightDC)))
//    {
//        float error = Util::Abs<float>(rightSpeed - drive.rightDC);
//
//        // braking is proportional to the error
//        float desiredBraking = brakeGain * error;
//
//        // brake duty cycle is equal to desired stall torque / normalized speed
//        drive.rightBrakeDC = desiredBraking / Util::Abs<float>(rightSpeed);
//        drive.rightDC = 0.0; // can only brake if speed controller is set to 0
//    }
//
//    return drive;
//}

// linearizes speed controller - motor response
ESCCommand DitheredBrakeTrain::CalculateBrakeAndDutyCycle(float input, float speed)
{
    ESCCommand command;

    command.dutyCycle = 0.0;
    command.brakingDutyCycle = 0.0;

    if(speed < 0)
    {
        command = CalculateBrakeAndDutyCycle(-input, -speed);
        command.dutyCycle = -command.dutyCycle;
        return command;
    }

    // speed >= 0 at this point
    if(input >= speed) // trying to go faster
    {
        command.dutyCycle = input;
        command.brakingDutyCycle = 0.0;
    }
    // trying to slow down
    else
    {
        float error = input - speed; // error always <= 0

        if(input >= 0) // braking is based on speed alone; reverse power unnecessary
        {
            command.dutyCycle = 0.0; // must set 0 to brake

            if(speed > -error)
                command.brakingDutyCycle = -error / speed; // speed always > 0
            else
                command.brakingDutyCycle = 1.0;
        }
        else // input < 0; braking with reverse power
        {
            command.brakingDutyCycle = 0.0; // not braking
            command.dutyCycle = error / (1.0 + speed); // dutyCycle <= 0 because error <= 0
        }
    }

    return command;
}

DriveCommand DitheredBrakeTrain::Drive(float forwardInput, float turnInput)
{
    DriveCommand drive;

    float leftInput = forwardInput - turnInput;
    float rightInput = forwardInput + turnInput;

    if(Util::Abs<float>(leftInput) > 1.0)
    {
        // TODO must decide whether forward or turn takes precedence
        leftInput = Util::Sign<float>(leftInput) * 1.0;
    }

    if(Util::Abs<float>(rightInput) > 1.0)
    {
        // TODO must decide whether forward or turn takes precedence
        rightInput = Util::Sign<float>(rightInput) * 1.0;
    }

    float leftSpeed = leftEncoder.GetRate() / DriveEncoders::ENCODER_RATE_HIGH_GEAR;
    float rightSpeed = rightEncoder.GetRate() / DriveEncoders::ENCODER_RATE_HIGH_GEAR;

    drive.leftCommand = CalculateBrakeAndDutyCycle(leftInput, leftSpeed);
    drive.rightCommand = CalculateBrakeAndDutyCycle(rightInput, rightSpeed);

    return drive;
}
