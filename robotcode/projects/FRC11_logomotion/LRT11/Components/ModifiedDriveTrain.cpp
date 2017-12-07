#include "ModifiedDriveTrain.h"
#include "..\Config\Config.h"
#include "DriveTrain\CLRateTrain.h"
#include "DriveTrain\CLPositionDriveTrain.h"
#include "..\Sensors\DriveEncoders.h"
#include "..\Jaguar\Esc.h"
#include "..\Config\RobotConfig.h"
#include "..\ActionData\DriveAction.h"
#include "..\ActionData\ShifterAction.h"
#include <math.h>

ModifiedDriveTrain::ModifiedDriveTrain()
    : Component()
    , driveEncoders(DriveEncoders::GetInstance()) //TODO: If this is a singleton, why create it here? -dg
    , config(Config::GetInstance())
{
    closedRateTrain = new ClosedLoopRateDrivetrain();
    closedPositionTrain = new CLPositionDriveTrain(*closedRateTrain);
    
#ifdef LRT_ROBOT_2011
    leftESC = new Esc(RobotConfig::CAN::DRIVE_LEFT_A, RobotConfig::CAN::DRIVE_LEFT_B,
            driveEncoders.GetLeftEncoder(), "left");
    rightESC = new Esc(RobotConfig::CAN::DRIVE_RIGHT_A, RobotConfig::CAN::DRIVE_RIGHT_B,
            driveEncoders.GetRightEncoder(), "right");
#else
    // TODO fix initialization
    leftESC = new Esc(RobotConfig::CAN::DRIVE_LEFT,  driveEncoders.GetLeftEncoder(),  "left");
    rightESC = new Esc(RobotConfig::CAN::DRIVE_RIGHT, driveEncoders.GetRightEncoder(), "right");
#endif

    Configure();
    synchronizedCyclesLeft = 0;

//    leftESC->CollectCurrent();
//    rightESC->CollectCurrent();
    printf("Constructed Drive (ModifiedDriveTrain)\n");
}

ModifiedDriveTrain::~ModifiedDriveTrain()
{
    driveEncoders.~DriveEncoders();

    delete closedRateTrain;
    delete closedPositionTrain;
    delete leftESC;
    delete rightESC;
}

void ModifiedDriveTrain::Configure()
{
    cyclesToSynchronize = config.Get<int>("Drivetrain", "CyclesToSynchronize", 60);
}

void ModifiedDriveTrain::Output()
{
    DriveCommand drive;

//    static int cycleCount = 0;
//    if(++cycleCount % 10 == 0)
//    {
//        AsynchronousPrinter::Printf("Left: %6.3f ", leftESC->GetCurrent());
//        AsynchronousPrinter::Printf("Right: %6.3f\n", rightESC->GetCurrent());
//    }

    closedRateTrain->SetHighGear(action.shifter->gear == ACTION::GEARBOX::HIGH_GEAR);
    closedRateTrain->SetClosedLoopEnabled(action.driveTrain->rate.usingClosedLoop);

//    closedRateTrain->SetClosedLoopEnabled(false);


    // calculate left duty cycle, right duty cycle, left brake, and
    // right brake based off of joystick inputs and mode
    switch(action.driveTrain->mode)
    {
    case ACTION::DRIVETRAIN::SPEED:
        if(action.driveTrain->rate.thirdGear)
            // scale raw turn to a max of 0.3
            drive = closedRateTrain->Drive(action.driveTrain->rate.rawForward, action.driveTrain->rate.rawTurn * 0.3);
        else
            drive = closedRateTrain->Drive(action.driveTrain->rate.rawForward, action.driveTrain->rate.rawTurn);
        break;

    case ACTION::DRIVETRAIN::POSITION:
        if(action.driveTrain->position.givenCommand)
        {
            if(action.driveTrain->position.shouldMoveDistance)
            {
                AsynchronousPrinter::Printf("Move distance command");
                closedPositionTrain->SetMovePosition(action.driveTrain->position.distanceSetPoint);
            }
            else if(action.driveTrain->position.shouldTurnAngle)
            {
                AsynchronousPrinter::Printf("Turn angle command");
                closedPositionTrain->SetTurnAngle(action.driveTrain->position.turnSetPoint);
            }

            action.driveTrain->position.givenCommand = false;
            action.driveTrain->position.shouldMoveDistance = false;
            action.driveTrain->position.shouldTurnAngle = false;
        }

        drive = closedPositionTrain->Drive(action.driveTrain->position.maxFwdSpeed,
                action.driveTrain->position.maxTurnSpeed);
        break;

    case ACTION::DRIVETRAIN::DISTANCE:
        if(action.driveTrain->distance.givenCommand)
        {
            closedPositionTrain->SetMoveDistance(action.driveTrain->distance.distanceSetPoint);
            action.driveTrain->distance.givenCommand = false;
        }

        CLPositionCommand command =
            closedPositionTrain->DriveAtLeastDistance(action.driveTrain->distance.distanceDutyCycle);
        action.driveTrain->distance.done = command.done;

        drive = command.drive;
        break;
    case ACTION::DRIVETRAIN::SYNCHRONIZING:
        synchronizedCyclesLeft = cyclesToSynchronize;  //set shift timer from value in config file
        break;
    }

    if(synchronizedCyclesLeft > 0)
        synchronizedCyclesLeft--;

//    AsynchronousPrinter::Printf("sp:%.2f\n", driveEncoders.GetNormalizedRightMotorSpeed());

//    AsynchronousPrinter::Printf("R: %.3f\n",driveEncoders.GetNormalizedOpposingGearMotorSpeed(driveEncoders.GetRightEncoder()));
    
    if(0 && synchronizedCyclesLeft > 20)   //disabled for now; -dg
    {
    	drive.rightCommand.dutyCycle = driveEncoders.GetNormalizedOpposingGearMotorSpeed(driveEncoders.GetRightEncoder());
    	drive.leftCommand.dutyCycle = driveEncoders.GetNormalizedOpposingGearMotorSpeed(driveEncoders.GetLeftEncoder());
    	
//        drive.rightCommand.dutyCycle = GetSynchronizedSpeed(driveEncoders.GetNormalizedRightOppositeGearMotorSpeed());
//        drive.leftCommand.dutyCycle = GetSynchronizedSpeed(driveEncoders.GetNormalizedLeftOppositeGearMotorSpeed());
//        drive.rightCommand.dutyCycle = 0;
//        drive.leftCommand.dutyCycle = 0;
    }
    else if(synchronizedCyclesLeft > 0)
    {

    	drive.rightCommand.dutyCycle = driveEncoders.GetNormalizedOpposingGearMotorSpeed(driveEncoders.GetRightEncoder());
        drive.rightCommand.dutyCycle = GetSynchronizedSpeed(drive.rightCommand.dutyCycle ); //limits to not less than 10%
        
        drive.leftCommand.dutyCycle  = driveEncoders.GetNormalizedOpposingGearMotorSpeed(driveEncoders.GetLeftEncoder());
        drive.leftCommand.dutyCycle = GetSynchronizedSpeed(drive.leftCommand.dutyCycle );
//       drive.leftCommand.dutyCycle = GetSynchronizedSpeed(driveEncoders.GetNormalizedLeftMotorSpeed());
        drive.rightCommand.dutyCycle *= 1.0;  //reduce power, since the motors are unloaded.
        drive.leftCommand.dutyCycle *= 1.0;
    }
//    AsynchronousPrinter::Printf("speed:%.2f\n", driveEncoders.GetNormalizedLowGearForwardSpeed());
//    AsynchronousPrinter::Printf("speed:%.2f\n", driveEncoders.GetNormalizedForwardMotorSpeed());

    // leftDC and rightDC are set to 0 if there is a need to brake;
    // see DitheredBrakeTrain's Drive method
    leftESC->SetDutyCycle(drive.leftCommand.dutyCycle);
    rightESC->SetDutyCycle(drive.rightCommand.dutyCycle);

    if(synchronizedCyclesLeft > 0) //trying to shift?  Then don't apply brakes
    {
        leftESC->SetBrake(0);
        rightESC->SetBrake(0);
    }
    else //Handle normal braking
    {
        // leftBrakeDC and rightBrakeDC must be converted from a percent to a
        // value in range [1,8]; 1 means no braking while 8 means max braking
//        leftESC->SetBrake((int)(drive.leftCommand.brakingDutyCycle * 8));
//        rightESC->SetBrake((int)(drive.rightCommand.brakingDutyCycle * 8));`
#warning Brakes turned off 
    	leftESC->SetBrake(0);
        rightESC->SetBrake(0);
    }   //end of normal braking

    // apply brakes only has an effect if SetBrake is called with a
    // non-zero parameter
    leftESC->ApplyBrakes();
    rightESC->ApplyBrakes();

    if(action.wasDisabled)
    {
        leftESC->ResetCache();
        rightESC->ResetCache();
    }
}

//returns a minumum of 10% speed, so the gears can mesh when stopped or low speed.
float ModifiedDriveTrain::GetSynchronizedSpeed(float motorSpeed) //motor speed refers to the speed of the motor if it were engaged
{
    return motorSpeed;

    float absMotorSpeed = fabs(motorSpeed);
    
    if(absMotorSpeed < 1E-4) // for the case where sign returns zero
        return 0.1; //We can't shift without moving so if we are stopped we spin forward at 10%
    else if(absMotorSpeed < .10)
        return 0.10 * Util::Sign<float>(motorSpeed); //If we are moving very slowly apply 10% power in the direction of movement to ensure the motor actually spins
    else 
    	return motorSpeed; //Otherwise just spin the motor close the the speed of the output shaft
}

string ModifiedDriveTrain::GetName()
{
    return "ModifiedDriveTrain";
}
