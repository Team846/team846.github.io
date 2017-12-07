// connections.h: Connections information

#ifndef connections_h_
#define connections_h_

#if !defined(__18F8520) && !defined(__18F8722)
#error Unknown processor target
#endif

//**************  FRC CONNECTIONS *****************


// PWM outputs ==================================

// pwm01 used for power tap
#define PWM_LIFT				pwm02
//#define PWM_FORK				pwm03
#define PWM_FORKL				pwm03
#define PWM_FORKR				pwm04

#define PWM_CIM_L				pwm11
#define PWM_CIM_R				pwm12

#define PWM_SHIFTER_L			pwm15
#define PWM_SHIFTER_R			pwm16

#define RELAY_VACUUM			relay1_fwd

// Analog Inputs ================================
#define SENS_LIFT_POS rc_ana_in01
#define SENS_FORK_POS rc_ana_in02
//#define SENS_GYRO	3
//#define SENS_TILT_H	4
//#define SENS_TILT_V	5
#define SENS_VACUUM_PRESSURE	rc_ana_in06
//#define SENS_LIFT_POS 1
//#define SENS_FORK_POS 2
//#define SENS_GYRO	3
//#define SENS_TILT_H	4
//#define SENS_TILT_V	5
//#define SENS_VACUUM_PRESSURE	6

// Digital I/O ==================================

// Interrupts
//#define mLeftEncoderCount		rc_dig_in01
//#define mRightEncoderCount		rc_dig_in02
//#define DIAGNOSTIC_LED			rc_dig_in09		//This is the i/o pin
//#define DIAGNOSTIC_LED_PORT		digital_io_09	// Configure as output
#define ULTRASONIC_LED			rc_dig_in10		//This is the i/o pin
#define ULTRASONIC_LED_PORT		digital_io_10	// Configure as output

#define SENS_LEFTENC_BCOUNT		rc_dig_in07
#define SENS_RIGHTENC_BCOUNT	rc_dig_in08

#define DIGOUT_PRGM					rc_dig_in09
#define DIGOUT_PRGM_PORT				digital_io_09	// Configure as output

#define SENS_LIFT_LOLIMIT 0 //rc_dig_in09
#define SENS_LIFT_HILIMIT 0 //rc_dig_in10

#define LCD_SCROLL_DN_INPUT	rc_dig_in11
#define LCD_SCROLL_UP_INPUT rc_dig_in12

//Brake outputs on Victor ESC's
#define DIGOUT_COASTL					rc_dig_in13
#define DIGOUT_COASTL_PORT				digital_io_13	// Configure as output
#define DIGOUT_COASTR					rc_dig_in14
#define DIGOUT_COASTR_PORT				digital_io_14	// Configure as output

// Ultrasonic Sensor Connection
#define DIGIO_ULTRASONIC			rc_dig_in06
#define DIGIO_ULTRASONIC_PORT		digital_io_06

#define SENS_IR_GO					rc_dig_in15
#define SENS_IR_RIGHT				rc_dig_in16
#define SENS_IR_STOP					rc_dig_in17
#define SENS_IR_LEFT90				rc_dig_in18

//**************  END of FRC CONNECTIONS *****************


//**************  OPERATOR INTERFACE CONNECTIONS *****************

#define OI_XBOX_LEFT			p2_y
#define OI_XBOX_RIGHT			p2_x
#define OI_XBOX_A				p2_sw_trig
#define OI_XBOX_B				p2_sw_top
#define OI_XBOX_X				p2_sw_aux1
#define OI_XBOX_Y				p2_sw_aux2
#define OI_XBOX_LT				((p2_wheel & 0x80) == 0)
#define OI_XBOX_RT				((p2_wheel & 0x40) == 0)
#define OI_XBOX_LB				((p2_wheel & 0x20) == 0)
#define OI_XBOX_RB				((p2_wheel & 0x10) == 0)
#define OI_XBOX_BACK			((p2_aux & 0x80) == 0)
#define OI_XBOX_START			((p2_aux & 0x40) == 0)
#define OI_XBOX_CENTERX		((p2_aux & 0x20) == 0)
#define OI_XBOX_DUP			((p2_aux & 0x10) == 0)
#define OI_XBOX_CONNECTED	(p2_wheel!=127 || p2_aux!=127 )

#define OI_PANEL_A			p1_sw_trig
#define OI_PANEL_B			p1_sw_top
#define OI_PANEL_C			p1_sw_aux1
#define OI_PANEL_D			p1_sw_aux2
#define OI_USERPOT1			p3_x
#define OI_USERPOT2			p3_y
#define OI_USERPOT3			p3_wheel

#define OI_JOYFWD			OI_XBOX_LEFT
#define OI_JOYTURN			OI_XBOX_RIGHT

#define OI_SHIFTHI			OI_XBOX_A
#define OI_SHIFTLO			OI_XBOX_B
//#define OI_SHIFTHI			OI_XBOX_B
//#define OI_SHIFTLO			OI_XBOX_A

#define OI_META				OI_XBOX_CENTERX

//EEPROM buttons.  OI_META must be pressed simultaneously. See SetUserOptions()
#define OI_EPROM_DRIVEMETHOD	(OI_META && OI_XBOX_RB)
#define OI_EPROM_SERVODRIVE		(OI_META && OI_XBOX_LB)
#define OI_EPROM_SERVICEMODE	(OI_META && OI_XBOX_RB && OI_XBOX_LB)


#define OI_AUTON_INIT OI_PANEL_A
#define OI_AUTON_STRAIGHT OI_PANEL_B
#define OI_AUTON_LATERAL OI_PANEL_C
#define OI_AUTON_SET OI_PANEL_D
//End of EEPROM button definitions



#define OI_REVERSE		OI_XBOX_DUP
#define OI_BRAKE			OI_XBOX_START
#define OI_BRAKE_LEFT			OI_XBOX_BACK
#define OI_BRAKE_RIGHT			OI_XBOX_START

#define OI_PRECISION	OI_XBOX_RB

#define OI_SERVODRIVE_GAIN_POT	OI_USERPOT1

#define OI_LIFT_UP		(OI_XBOX_Y || p3_sw_trig)
#define OI_LIFT_DOWN	(OI_XBOX_X || p3_sw_top)
#define OI_FORK_UP		p3_sw_aux1
#define OI_FORK_DOWN	p3_sw_aux2

#define OI_AUTOMODE	(!p4_sw_aux1)

#define OI_LIFT_UP_AUTO			(OI_AUTOMODE && OI_LIFT_UP)
#define OI_LIFT_DOWN_AUTO		(OI_AUTOMODE && OI_LIFT_DOWN)
#define OI_FORK_UP_AUTO			(OI_AUTOMODE && OI_FORK_UP)
#define OI_FORK_DOWN_AUTO		(OI_AUTOMODE && OI_FORK_DOWN)
#define OI_LIFT_UP_MANUAL		(!OI_AUTOMODE && OI_LIFT_UP)
#define OI_LIFT_DOWN_MANUAL		(!OI_AUTOMODE && OI_LIFT_DOWN)
#define OI_FORK_UP_MANUAL		(!OI_AUTOMODE && OI_FORK_UP)
#define OI_FORK_DOWN_MANUAL		(!OI_AUTOMODE && OI_FORK_DOWN)

#define OI_HAVEBALL				OI_PANEL_C
#define OI_DONTHAVEBALL			OI_PANEL_D

#define OI_VACUUM_OFF p4_sw_aux2
#define OI_LIFT_SETBASE		(OI_META && OI_LIFT_UP && OI_LIFT_DOWN)
#define OI_FORK_SETBASE		(OI_META && OI_FORK_UP && OI_FORK_DOWN)

#ifdef TURN_KNOB
#define mTurnPot1		p4_y
#define mTurnPot2		p3_y
#endif

//**************  END of OPERATOR INTERFACE CONNECTIONS *****************


#endif //connections_h_	NO CODE BELOW THIS LINE
