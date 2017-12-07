#include <math.h>
#include "Arm.h"
#include "..\Config\RobotConfig.h"
#include "..\Jaguar\ProxiedCANJaguar.h"
#include "..\Config\Config.h"
#include "..\Sensors\VirtualPot.h"
#include "..\ActionData\ArmAction.h"
#include "..\ActionData\RollerAction.h"


Arm::Arm()
    : Component()
    , config(Config::GetInstance())
    , configSection("Arm")
//   , state(IDLE)
    , oldState(ACTION::ARM_::IDLE)
    , cycleCount(0)
    , presetMode(true)
    , pulseCount(0)
{
    armEsc = new ProxiedCANJaguar(RobotConfig::CAN::ARM_, "Arm");

#ifdef VIRTUAL
    // arm is ~29 inches
    // speed: 29 in * 1 ft / 12 in * 1.3 rps * 2 pi rad / rev = ~19.7 ft/s
    // ft / turn: 29 in * 1 ft / 12 in * 2 pi rad / rev = ~15.2 ft
    armPot = new VirtualPot(RobotConfig::ANALOG::POT_ARM, 1, 15.2, 19.7);
#else
    armPot = new AnalogChannel(RobotConfig::ANALOG::POT_ARM);
#endif


    // brake when set to 0 to keep the arm in place
    armEsc->ConfigNeutralMode(LRTCANJaguar::kNeutralMode_Brake);
    Configure();
    printf("Constructed Arm\n");
}

Arm::~Arm()
{
    delete armEsc;
    delete armPot;
}

void Arm::Configure()
{
    minPosition = config.Get<float>(configSection, "minPosition", 280);
    midPosition = config.Get<float>(configSection, "midPosition", 621);
    maxPosition = config.Get<float>(configSection, "maxPosition", 530);

    midPositionDeadband = config.Get<float>(configSection, "midPositionDeadband", 10);

    maxPowerUp    = config.Get<float>(configSection, "maxPowerUp", 0.30);
    powerRetainUp = config.Get<float>(configSection, "powerRetainUp", 0.10);
    powerDown     = config.Get<float>(configSection, "powerDown", -0.15);

    midPowerUp    = config.Get<float>(configSection, "midPowerUp", 0.2);
    midPowerDown  = config.Get<float>(configSection, "midPowerDown", -0.15);
    
    pGainDown 	  = config.Get<float>(configSection, "pGainDown", 0.0015); 
    pGainUp	      = config.Get<float>(configSection, "pGainUp", 0.003); 
    pGainMid	  = config.Get<float>(configSection, "pGainMid", 0.01);

    timeoutCycles = (int)(config.Get<int>(configSection, "timeoutMs", 1500) * 1.0 / 1000.0 * 50.0 / 1.0);
}

void Arm::Output()
{

    float potValue = armPot->GetAverageValue();

#ifdef USE_DASHBOARD
    SmartDashboard::Log(potValue, "Arm Pot Value");
#endif

    if(action.master.abort)
    {
        armEsc->SetDutyCycle(0.0);
        action.arm->state = ACTION::ARM_::IDLE;
        action.arm->completion_status = ACTION::ABORTED;
        return; // do not allow normal processing (ABORT is not printed...should fix this -dg)
    }

    const bool state_change = (oldState != action.arm->state);
    if(state_change)
        AsynchronousPrinter::Printf("Arm: %s\n", ACTION::ARM_::state_string[action.arm->state]);

    if(state_change)
        cycleCount = timeoutCycles; // reset timeout

    switch(action.arm->state)
    {
    case ACTION::ARM_::PRESET_TOP:
        action.arm->completion_status = ACTION::IN_PROGRESS;
        // overriden below to change roller speed while moving the arm up
        action.roller->maxSuckPower = 1.0;

        // don't merely switch to the IDLE state, as the caller will likely
        // set the state each time through the loop
        if(--cycleCount < 0)
        {
            action.arm->completion_status = ACTION::FAILURE;
            armEsc->SetDutyCycle(0.0);
            break; // timeout overrides everything
        }

        if(potValue >= maxPosition)
        {
            action.arm->completion_status = ACTION::SUCCESS;
            armEsc->SetDutyCycle(powerRetainUp);
            // cycleCount will never get decremented below 0, so powerRetainUp
            // will be maintained
            cycleCount = 100;
        }
        else //we have not yet hit the setpoint
        {
            action.arm->completion_status = ACTION::IN_PROGRESS;
		    float error = maxPosition - potValue;
            
            armEsc->SetDutyCycle(Util::Max<float>(Util::Min<float>(maxPowerUp*1.5, error*pGainUp), 0.15));

            action.roller->state = ACTION::ROLLER::SUCKING;
            action.roller->maxSuckPower = 0.3; // lower duty cycle

            // make roller suck while moving up to keep
            // game piece in
            if(++pulseCount % 2 == 0)
                action.roller->state = ACTION::ROLLER::SUCKING;
            else
                action.roller->state = ACTION::ROLLER::STOPPED;
        }
        break;

    case ACTION::ARM_::PRESET_BOTTOM:
        action.arm->completion_status = ACTION::IN_PROGRESS;
        action.roller->maxSuckPower = 1.0;

        if(--cycleCount < 0)
        {
            action.arm->completion_status = ACTION::FAILURE;
            armEsc->SetDutyCycle(0.0);
            break; // timeout overrides everything
        }

        if(potValue <= minPosition)
        {
            action.arm->completion_status = ACTION::SUCCESS;
            armEsc->SetDutyCycle(0.0); // don't go below the min position
            // cycleCount will never get decremented below 0, so powerRetainUp
            // will be maintained
            cycleCount = 100;
        }
        else
        {
            action.arm->completion_status = ACTION::IN_PROGRESS;
            float error = minPosition - potValue;
            armEsc->SetDutyCycle(Util::Max<float>(powerDown, error*pGainDown));
//            AsynchronousPrinter::Printf("setpoint %.3f\n",Util::Max<float>(powerDown, error*0.0015));
        }
        break;

    case ACTION::ARM_::PRESET_MIDDLE:
        {
        	action.arm->completion_status = ACTION::IN_PROGRESS;
	    	float error = midPosition - potValue;
	    	if (fabs(error) > midPositionDeadband) 
	        	error -= midPositionDeadband * Util::Sign<float>(error);
	    	else 
	    	{
	    		error = 0.0;
	        	action.arm->completion_status = ACTION::SUCCESS;
	    	}
	
	    	float dutyCycle = pGainMid * error;
	    		    	
	    	dutyCycle = Util::Clamp<float>(dutyCycle,-0.35, 0.35);
	
			armEsc->SetDutyCycle(dutyCycle);
        }
        
        break;
        
    case ACTION::ARM_::MANUAL_UP:
        if(potValue < maxPosition)
            armEsc->SetDutyCycle(maxPowerUp);
        else
            armEsc->SetDutyCycle(0.0);

        action.arm->completion_status = ACTION::IN_PROGRESS;
        // operator must hold button to stay in manual mode
        action.arm->state = ACTION::ARM_::IDLE;
        break;

    case ACTION::ARM_::MANUAL_DOWN:
        if(potValue > minPosition)
            armEsc->SetDutyCycle(powerDown);
        else
            armEsc->SetDutyCycle(0.0);

        action.arm->completion_status = ACTION::IN_PROGRESS;
        // operator must hold button to stay in manual mode
        action.arm->state = ACTION::ARM_::IDLE;
        break;

    case ACTION::ARM_::IDLE:
        action.arm->completion_status = ACTION::SUCCESS;
        armEsc->SetDutyCycle(0.0);
        break;
    default:
        AsynchronousPrinter::Printf("Arm: ERROR Unknown State\n");

    }

    oldState = action.arm->state;

    //Print diagnostics
    static ACTION::eCompletionStatus lastDoneState = ACTION::UNSET;
    if(lastDoneState != action.arm->completion_status)
        AsynchronousPrinter::Printf("Arm: Status=%s\n", ACTION::status_string[action.arm->completion_status]);
    lastDoneState = action.arm->completion_status;
}

string Arm::GetName()
{
    return "Arm";
}
