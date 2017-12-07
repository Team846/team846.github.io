#include "DriveEncoders.h"

DriveEncoders* DriveEncoders::instance = NULL;

DriveEncoders& DriveEncoders::GetInstance()
{
    if(instance == NULL)
        instance = new DriveEncoders();
    return *instance;
}

DriveEncoders::DriveEncoders()
    : encoderLeft(
        RobotConfig::DIGITAL_IO::ENCODER_LEFT_A,
        RobotConfig::DIGITAL_IO::ENCODER_LEFT_B /*, 534.0 / 574.0 */)
#ifndef VIRTUAL
    , uselessEncoder(3, 6)
#endif
    , encoderRight(
        RobotConfig::DIGITAL_IO::ENCODER_RIGHT_A,
        RobotConfig::DIGITAL_IO::ENCODER_RIGHT_B)
    , isHighGear(false)
{
    // want to stay with ticks/second
    encoderLeft.SetDistancePerPulse(1);
    encoderRight.SetDistancePerPulse(1);

    encoderLeft.Start();
    encoderRight.Start();
    printf("Construct Drive Encoders\n");
}

DriveEncoders::~DriveEncoders() {}


double DriveEncoders::RawForwardSpeed()
{
    return (encoderLeft.GetRate() + encoderRight.GetRate()) / 2;
}

double DriveEncoders::NormalizedForwardMotorSpeed()
{
	double forwardSpeed = RawForwardSpeed() / ENCODER_RATE_HIGH_GEAR;

	if (!isHighGear)
		forwardSpeed *= LOW_GEAR_MULTIPLIER;
	
	return forwardSpeed;
}

/***************** Turning Functions ***************************/
double DriveEncoders::GetNormalizedLowGearTurningSpeed()
{
    return GetNormalizedTurningSpeed() * LOW_GEAR_MULTIPLIER;
}

double DriveEncoders::GetTurningSpeed()
{
    // WPILib GetRate error still exists with LRTEncoder?
    return encoderRight.GetRate() - encoderLeft.GetRate();
}

double DriveEncoders::GetNormalizedTurningSpeed()
{
    return GetTurningSpeed() / MAX_TURNING_RATE;
}

double DriveEncoders::GetNormalizedTurningMotorSpeed()
{
    return isHighGear ? GetNormalizedTurningSpeed() :
            GetNormalizedLowGearTurningSpeed();
}

double DriveEncoders::GetRobotDist()
{
    return (GetWheelDist(LEFT) + GetWheelDist(RIGHT)) / 2;
}

int DriveEncoders::GetTurnTicks()
{
    // CCW is positive, CW is negative
    return encoderRight.Get() - encoderLeft.Get();
}

double DriveEncoders::GetTurnRevolutions()
{
    return GetTurnTicks() / TICKS_PER_FULL_TURN;
}

double DriveEncoders::GetTurnAngle()
{
    return GetTurnRevolutions() * 360.0;
}

/************* Distance functions **************************************/
double DriveEncoders::GetWheelDist(int side)
{
    // pulses / ( pulses / revolution ) * distance / revolution = inch distance
    LRTEncoder& e = (side == LEFT ? encoderLeft : encoderRight);
    return e.Get() / PULSES_PER_REVOLUTION * WHEEL_DIAMETER * PI;
}

double DriveEncoders::GetLeftSpeed()
{
    return encoderLeft.GetRate();
}

double DriveEncoders::GetNormalizedLeftOppositeGearMotorSpeed()
{
    return Util::Clamp<double>(
            encoderLeft.GetRate() /
            (!isHighGear ? ENCODER_RATE_HIGH_GEAR : (ENCODER_RATE_HIGH_GEAR / LOW_GEAR_MULTIPLIER))
            , -1.0, 1.0);
}

double DriveEncoders::GetRightSpeed()
{
    return encoderRight.GetRate();
}

double DriveEncoders::GetNormalizedMotorSpeed(LRTEncoder& encoder)
{
    return encoder.GetRate() /
            (isHighGear ? ENCODER_RATE_HIGH_GEAR : (ENCODER_RATE_HIGH_GEAR / LOW_GEAR_MULTIPLIER));
}

double DriveEncoders::GetNormalizedOpposingGearMotorSpeed(LRTEncoder& encoder)
{
    return encoder.GetRate() /
            (isHighGear ? (ENCODER_RATE_HIGH_GEAR / LOW_GEAR_MULTIPLIER) : ENCODER_RATE_HIGH_GEAR );
}

double DriveEncoders::GetNormalizedRightOppositeGearMotorSpeed()
{
    return encoderRight.GetRate() /
            (isHighGear ? (ENCODER_RATE_HIGH_GEAR / LOW_GEAR_MULTIPLIER) : ENCODER_RATE_HIGH_GEAR );
}

void DriveEncoders::SetHighGear(bool isHighGear)
{
    this->isHighGear = isHighGear;
}

bool DriveEncoders::IsHighGear()
{
	return isHighGear;
}

#ifdef VIRTUAL
VirtualLRTEncoder& DriveEncoders::GetLeftEncoder()
#else
LRTEncoder& DriveEncoders::GetLeftEncoder()
#endif
{
    return encoderLeft;
}

#ifdef VIRTUAL
VirtualLRTEncoder& DriveEncoders::GetRightEncoder()
#else
LRTEncoder& DriveEncoders::GetRightEncoder()
#endif
{
    return encoderRight;
}
