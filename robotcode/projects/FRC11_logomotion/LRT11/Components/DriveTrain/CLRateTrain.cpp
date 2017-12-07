#include "CLRateTrain.h"
#include <math.h>

ClosedLoopRateDrivetrain::ClosedLoopRateDrivetrain()
//    : DriveMethod(escLeft, escRight)
//    , escLeft(escLeft)
//    , escRight(escRight)
    : encoders(DriveEncoders::GetInstance())
    , config(Config::GetInstance())
    , fwdRunningError(FWD_DECAY)
    , turnRunningError(TURN_DECAY)
    , brakeLeft(false)
    , brakeRight(false)
    , usingClosedLoop(true)
    , highGear(true)
{
    printf("Constructed CLRateTrain\n");
}

void ClosedLoopRateDrivetrain::Configure()
{
    // confiure parent class
    DitheredBrakeTrain::Configure();

    const static string configSection = "CLRateDriveTrain";

    pGainTurnHighGear = config.Get<float>(configSection, "pGainTurnHighGear", 1.5);
    pGainFwdHighGear = config.Get<float>(configSection, "pGainFwdHighGear", 1.5);

    pGainTurnLowGear = config.Get<float>(configSection, "pGainTurnLowGear", 1.5);
    pGainFwdLowGear = config.Get<float>(configSection, "pGainFwdLowGear", 1.5);
}

DriveCommand ClosedLoopRateDrivetrain::Drive(float rawFwd, float rawTurn)
{
    if(brakeLeft && brakeRight)
        Stop();
    else if(brakeLeft)
        PivotLeft(rawFwd);
    else if(brakeRight)
        PivotRight(rawFwd);
    // TODO FIX
//    if(brakeLeft || brakeRight)
//        return;

    if(!usingClosedLoop)
    {
#ifdef USE_DASHBOARD
//        SmartDashboard::Log(rawFwd, "Raw Forward (CLDT)");
//        SmartDashboard::Log(rawTurn, "Raw Turn (CLDT)");
#endif
        return DitheredBrakeTrain::Drive(rawFwd, rawTurn);
    }

    float pGainTurn = highGear ? pGainTurnHighGear : pGainTurnLowGear;
    float pGainFwd = highGear ? pGainFwdHighGear : pGainFwdLowGear;
    
//    AsynchronousPrinter::Printf("gain %.3f\n", pGainTurn);

    float turningRate = highGear ? encoders.GetNormalizedTurningSpeed()
            : encoders.GetNormalizedLowGearTurningSpeed();

    // eliminate spurrious measurements above mag |1|
    // values over mag |1| will cause the closed loop to slow down
    SmartDashboard::Log(turningRate, "Normalized Turning Speed");
    turningRate = Util::Clamp<float>(turningRate, -1, 1);

    // update the running sum with the error
    float turningError = rawTurn - turningRate;
    
    if (fabs(rawTurn) < 0.05) //if the joystick is not near zero switch off the integral.
    	turningError = turnRunningError.UpdateSum(turningError);
    else
    	turnRunningError.Clear();
    

    float turningCorrection = turningError * pGainTurn;
    float newTurn = rawTurn + turningCorrection;

    float robotSpeed = encoders.NormalizedForwardMotorSpeed();
    // don't want to limit the top speed of the drivetrain
    SmartDashboard::Log(robotSpeed, "Normalized Speed");
    SmartDashboard::Log(encoders.IsHighGear(), "Is High Gear");
    robotSpeed = Util::Clamp<float>(robotSpeed, -1.0, 1.0);

    float fwdError = rawFwd - robotSpeed;
    
    if (fabs(rawFwd ) < 0.05) //if the joystick is not near zero switch off the integral.
    	fwdError = fwdRunningError.UpdateSum(fwdError);
    else
    	fwdRunningError.Clear();

    float fwdCorrection = fwdError * pGainFwd;
    float newFwd = rawFwd + fwdCorrection;
    
    if (fabs(rawFwd) < 1e-3 && fabs(robotSpeed) < 0.05)
    {
    	newFwd = 0.0;
    }
    

#ifdef USE_DASHBOARD
//    SmartDashboard::Log(turningRate, "Turning Rate");
    SmartDashboard::Log(pGainFwd, "Forward Gain");
    SmartDashboard::Log(pGainTurn, "Turn Gain");
//    SmartDashboard::Log(rawFwd, "Raw Forward (CLDT)");
//    SmartDashboard::Log(rawTurn, "Raw Turn (CLDT)");
//    SmartDashboard::Log(newFwd, "Forward");
//    SmartDashboard::Log(newTurn, "Turn");
#endif

    return DitheredBrakeTrain::Drive(newFwd, newTurn);
}

void ClosedLoopRateDrivetrain::PivotLeft(float rightSpeed)
{
//    escLeft.Stop();
//    escRight.Set(rightSpeed);
}

void ClosedLoopRateDrivetrain::PivotRight(float leftSpeed)
{
//    escRight.Stop();
//    escLeft.Set(leftSpeed);
}

void ClosedLoopRateDrivetrain::SetBrakeLeft(bool brakeLeft)
{
    this->brakeLeft = brakeLeft;
}

void ClosedLoopRateDrivetrain::SetBrakeRight(bool brakeRight)
{
    this->brakeRight = brakeRight;
}

void ClosedLoopRateDrivetrain::Stop()
{
//    escRight.Stop();
//    escLeft.Stop();
}

void ClosedLoopRateDrivetrain::SetClosedLoopEnabled(bool enabled)
{
    usingClosedLoop = enabled;
}

void ClosedLoopRateDrivetrain::SetHighGear(bool isHighGear)
{
    highGear = isHighGear;
}
