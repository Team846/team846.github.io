#ifndef ROBOT_CONFIG_H_
#define ROBOT_CONFIG_H_

#include "../General.h"
#define CHANGEME 0

namespace RobotConfig
{
    const UINT32 INVALID = ~0;  //use to mark unused ports below. Will cause errors. -dg
#ifdef LRT_ROBOT_2011

    namespace CAN //ports 1-16 (2CAN limitation)
    {
        const  UINT32 DRIVE_RIGHT_B = 1;
        const  UINT32 DRIVE_LEFT_A  = 2;
        const  UINT32 DRIVE_LEFT_B  = 3;
        const  UINT32 ARM_          = 4;  //name "ARM" seems to be used -dg
        const  UINT32 ROLLER_BOTTOM = 5;
        const  UINT32 DEPLOYER      = 6;
        const  UINT32 ROLLER_TOP    = 7;
        const  UINT32 LIFT          = 8;
        //9
        const  UINT32 DRIVE_RIGHT_A = 10;
    }

    namespace PWM //ports 1-10
    {
        const  UINT32 RIGHT_GEARBOX_SERVO = 1;
        const  UINT32 LEFT_GEARBOX_SERVO  = 2;
        const  UINT32 ALIGNER_SERVO       = 3;
    };

    namespace ANALOG //ports 1-7; 8 is used by FRC's 12V battery monitor
    {
        const  UINT32 POT_ARM              = 3;
        const  UINT32 LINE_SENSOR_ADC      = 6;
    }
    namespace DIGITAL_IO //ports 1-14
    {
        const  UINT32 LINE_SENSOR_SI       = 4;
        const  UINT32 LINE_SENSOR_CLOCK    = 5;
        const  UINT32 ENCODER_LEFT_A       = 9;
        const  UINT32 ENCODER_LEFT_B       = 10;
        const  UINT32 ENCODER_RIGHT_A      = 13;
        const  UINT32 ENCODER_RIGHT_B      = 14;
    }
#else  // not LRT_ROBOT_2011
    namespace CAN //ports 1-16 (2CAN limitation)
    {
        const  UINT32 DRIVE_LEFT      = 2;
        const  UINT32 DRIVE_RIGHT     = 3;

        const  UINT32 ROLLER_TOP      = 32;
        const  UINT32 ROLLER_BOTTOM   = 22;

        const  UINT32 ARM_          = INVALID;
        const  UINT32 DEPLOYER      = INVALID;
        const  UINT32 LIFT          = INVALID;
    }

    namespace PWM //ports 1-10 In use: 10,8,7
    {
        enum { UNUSED_A = 1, UNUSED_B = 2 }; //2010 unused ports available for dummy assignment on 2011 robot
        const  UINT32 LEFT_GEARBOX_SERVO  = 8;
        const  UINT32 RIGHT_GEARBOX_SERVO = 10;

        // invalid servos
        const  UINT32 ALIGNER_SERVO       = UNUSED_A;
    }
    namespace ANALOG //ports 1-7; 8 is used by FRC's 12V battery monitor; In Use: 5,7.
    {
        enum { UNUSED_A = 1, UNUSED_B = 2 }; //2010 unused ports available for dummy assignment on 2011 robot
        const  UINT32 POT_ARM             = UNUSED_A;
        const  UINT32 LINE_SENSOR_ADC     = UNUSED_B;

    }
    namespace DIGITAL_IO //ports 1-14;  In Use on 2010: 1,2,3,13,14
    {
        enum { UNUSED_A = 5, UNUSED_B = 6 }; //2010 unused ports available for dummy assignment on 2011 robot
        const  UINT32 ENCODER_LEFT_A  = 1;
        const  UINT32 ENCODER_LEFT_B  = 2;
        const  UINT32 ENCODER_RIGHT_A = 13;
        const  UINT32 ENCODER_RIGHT_B = 14;
        const  UINT32 LINE_SENSOR_SI      = UNUSED_A;
        const  UINT32 LINE_SENSOR_CLOCK   = UNUSED_B;
    }
#endif // LRT_ROBOT_2011
}
#endif  //ROBOT_CONFIG_H_
