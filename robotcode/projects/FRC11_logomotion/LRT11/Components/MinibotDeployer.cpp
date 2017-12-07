#include "MiniBotDeployer.h"
#include "..\Config\RobotConfig.h"
#include "..\Config\Config.h"
#include "..\Jaguar\ProxiedCANJaguar.h"
#include "Shifter\LRTServo.h"
#include "Shifter\VIRTUALLRTServo.h"
#include "..\ActionData\DeployerAction.h"

MinibotDeployer::MinibotDeployer()
    : Component()
    , config(Config::GetInstance())
{
    deployerEsc = new ProxiedCANJaguar(RobotConfig::CAN::DEPLOYER, "Minibot Deployer");
#ifdef VIRTUAL
    alignerServo = new VirtualLRTServo(RobotConfig::PWM::ALIGNER_SERVO, "Minibot Deployment Servo");
#else
    alignerServo = new LRTServo(RobotConfig::PWM::ALIGNER_SERVO, "Minibot Deployment Servo");
#endif
    Configure();
    printf("Constructed Minibot Deployer\n");
}

MinibotDeployer::~MinibotDeployer()
{
    delete deployerEsc;
    delete alignerServo;
}

void MinibotDeployer::Configure()
{
    string configSection = "MinibotDeployer";
    lockedServoValue = config.Get<float>(configSection, "lockedServoValue", 0.5);
    releasedServoValue = config.Get<float>(configSection, "releasedServoValue", 1.0);
    currentThreshold = config.Get<int>(configSection, "currentThreshold", 15);
}

void MinibotDeployer::Output()
{
    if(action.deployer->shouldAlignerRelease)
        // set servo to release position
        alignerServo->Set(releasedServoValue);
    else
        // set servo to locked position
        alignerServo->Set(lockedServoValue);

    if(action.wasDisabled)
        deployerEsc->ConfigNeutralMode(LRTCANJaguar::kNeutralMode_Brake);

    static enum
    {
        IDLE,
        ABORT,
        ACCELERATING,
        FIRST_PUSH,
        RELAX,
        SECOND_PUSH
    } state = IDLE;

    static int timer = 0;
    if(action.deployer->shouldDeployMinibot)
    {
        timer = 0; // reset timer
        state = ACCELERATING;

        // deployment already commenced
        deployerEsc->ShouldCollectCurrent(true);
        action.deployer->shouldDeployMinibot = false;
    }

    // abort overrides everything
    if(action.master.abort)
        state = ABORT;

    float setPoint;
    switch(state)
    {
    case IDLE:
        // no movement
        setPoint = 0;
        break;

    case ABORT:
        // stop movement
        setPoint = 0;
        break;

    case ACCELERATING:
        // full power
        setPoint = 1.0;

        // two fifths second pulse of full power
        if(++timer % 20 == 0)
        {
            state = FIRST_PUSH;
            timer = 0;
        }

        break;

    case FIRST_PUSH:
        // full power
        setPoint = 0.5;

        // used to detect current spike
        float current = deployerEsc->GetCurrent();

#ifdef USE_DASHBOARD
        SmartDashboard::Log(current, "Minibot Deployment Current");
#endif

        // wait until a current spike or 1 second timeout
        if(/* current > currentThreshold || */ ++timer % 50 == 0)
        {
            state = RELAX; // relax for a bit
            timer = 0;
        }

        break;

    case RELAX:
        // no power
        setPoint = 0.0;

        // half a second of relaxing
        if(++timer % 25 == 0)
        {
            state = SECOND_PUSH; // next push to actually deploy the minibot
            timer = 0;
        }

        break;

    case SECOND_PUSH:
        // second push requires less power
        setPoint = 0.5;

        // one second of pushing
        if(++timer % 50 == 0)
        {
            state = IDLE; // done!
            timer = 0;
        }

        break;
    }

    deployerEsc->SetDutyCycle(setPoint);
}

string MinibotDeployer::GetName()
{
    return "MinibotDeployer";
}
