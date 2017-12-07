//Author: Brandon Liu and David Liu (2010)

#include "WPILib.h"
#include "LRTIterativeRobot.h"
#include "LRTConnections.h"
#include "LRTDriverStationLCD.h"
#include "LRTDriverControls.h"
#include "DBSDrive.h"
#include "ArcadeDrive.h"
#include "LRTKicker.h"
#include "ClosedLoopSpeedController.h"
#include "LRTDriveEncoders.h"
#include "LRTKicker.h"
#include "SafeCANJaguar.h"
#include "LRTConfig.h"
#include "LRTGearBox.h"
#include "LRTWinch.h"
#include "LRTLift.h"
#include "LRTConsole.h"
#include "DigitalOutputBrake.h"
#include "CANJaguarBrake.h"
#include "Profiler.h"
#include <string>
#include "LRTRoller.h"
#include "AsynchronousPrinter.h"
#include "LRTDriveTrain.h"
#include "CANBusController.h"
#include "ProxiedCANJaguar.h"
#include <iostream>
#include <fstream>
#include <Timer.h>
#include "LRTAuton.h"

using namespace std;

class LRTBot10 : public LRTIterativeRobot
{
	LRTConfig &m_config;
	DriverStation &m_ds;
	LrtLcd &m_dsLcd;
	
	CANBusController &m_canBusController;
	
	LRTDriverControls m_controls;
	
	// Drive Train
	LRTDriveEncoders m_driveEncoders;
	
//	CANJaguar m_escLeftRaw, m_escRightRaw;
//	Jaguar m_escLeftRaw, m_escRightRaw;
//	ClosedLoopSpeedController m_escLeft, m_escRight;
	
	ProxiedCANJaguar m_escLeft, m_escRight;
	ProxiedCANJaguar m_escWinch, m_escArm, m_escRoller;
	
//	DigitalOutputBrake m_brakeLeft, m_brakeRight;
	CANJaguarBrake m_brakeLeft, m_brakeRight;
	
	LRTGearBox m_gearboxLeft, m_gearboxRight;
	
//	SimpleArcadeDrive m_robotDrive;
	DBSDrive m_robotDrive;
//	LRTDriveTrain m_robotDrive;
	
	LRTBallDetector m_detector;
	LRTKicker m_kicker;
	LRTWinch m_winch;
	LRTLift m_lift;
	LRTRoller m_roller;
//	LRTAuton m_auton;
	
	LRTConsole& m_console;
	
	//Used for service mode - direct access to spikes/jags
	Relay m_relayKicker;
	ProxiedCANJaguar m_jaguarWinch;
	
	bool m_isServiceMode, m_isTestMode;
	
	enum GameState {
		kNothing,
		kHasEnteredAutonomous,
		kHasEnteredAutonomousFollowedByTeleop
	};
	struct {
		GameState state;
		double teleopStartTime;
	} m_gameState;
	// heuristic for determining if we were on the field or not
	
	const char* m_loadArray;
	
public:
	LRTBot10()
	: m_config(LRTConfig::GetInstance())
	, m_ds(*DriverStation::GetInstance())
	, m_dsLcd(LrtLcd::GetInstance())
	, m_canBusController(CANBusController::GetInstance())
	
	, m_controls()
	, m_driveEncoders()
	
//	, m_escLeftRaw(LRTConnections::kPwmDriveLeft), m_escRightRaw(LRTConnections::kPwmDriveRight)
//	, m_escLeftRaw(LRTConnections::kCanDriveLeft), m_escRightRaw(LRTConnections::kCanDriveRight)
//	, m_escLeft(m_escLeftRaw, m_driveEncoders.GetLeftEncoder(), LRTDriveEncoders::kMaxEncoderRate, false, 0, 0, 0)	// currently closed-loop is disabled
//	, m_escRight(m_escRightRaw, m_driveEncoders.GetRightEncoder(), LRTDriveEncoders::kMaxEncoderRate, false, 0, 0, 0)
	
//	, m_escLeft(LRTConnections::kPwmDriveLeft), m_escRight(LRTConnections::kPwmDriveRight)
	, m_escLeft(LRTConnections::kCanDriveLeft), m_escRight(LRTConnections::kCanDriveRight)
	, m_escWinch(LRTConnections::kCanWinch), m_escArm(LRTConnections::kCanArm)
	, m_escRoller(LRTConnections::kCanRoller)
	
//	, m_brakeLeft(LRTConnections::kDioBrakeLeft), m_brakeRight(LRTConnections::kDioBrakeLeft)
//	, m_brakeLeft(m_escLeftRaw), m_brakeRight(m_escRightRaw)
	, m_brakeLeft(m_escLeft), m_brakeRight(m_escRight)
//	, m_gearboxLeft(string("Left"), m_escLeftRaw, LRTConnections::kPwmShifterLeft)
//	, m_gearboxRight(string("Right"), m_escRightRaw, LRTConnections::kPwmShifterRight)
	, m_gearboxLeft(string("Left"), m_escLeft, LRTConnections::kPwmShifterLeft)
	, m_gearboxRight(string("Right"), m_escRight, LRTConnections::kPwmShifterRight)
//	, m_brakeLeft(m_escLeft), m_brakeRight(m_escRight)
//	, m_robotDrive(m_escLeft, m_escRight, false)
	, m_robotDrive(m_escLeft, m_escRight, m_brakeLeft, m_brakeRight, LRTConnections::kMaxBraking, false)
//	, m_robotDrive(m_escLeft, m_escRight, m_brakeLeft, m_brakeRight, LRTConnections::kMaxBraking, &m_driveEncoders, false)
	, m_detector()
	, m_kicker(m_detector)
	, m_winch(m_escWinch)
	, m_lift(m_escArm)
	, m_roller(m_escRoller, m_driveEncoders, m_detector)
//	, m_auton(m_escLeft, m_escRight, m_kicker)
	, m_console(LRTConsole::GetInstance())
	, m_relayKicker(LRTConnections::kRelayKicker)
	, m_jaguarWinch(LRTConnections::kCanWinch)
	, m_isServiceMode(false)
	, m_isTestMode(false)
	, m_loadArray("\\|/-")
	{
		AsynchronousPrinter::Printf("LRTBot10 Constructor Started\n");
		
//		m_console = LRTConsole::GetInstance();
		
		RobotInit();
		AsynchronousPrinter::Printf("LRTBot10 Constructor Completed\n");
	}
	
	
	/********************************** Init Routines *************************************/

	void RobotInit() {
		// Actions which would be performed once (and only once) upon initialization of the
		// robot would be put here.
		
		AsynchronousPrinter::Printf("RobotInit() completed.\n");
		m_gameState.state = kNothing;
		
//		m_escLeft.ConfigNeutralMode(CANJaguar::kNeutralMode_Jumper);
//		m_escRight.ConfigNeutralMode(CANJaguar::kNeutralMode_Jumper);
	}
	
	void LogPostGameData() {
		ofstream log("/gamelog.txt", ios::app);
		double stamp = Timer::GetFPGATimestamp();
		
		log << "\n";
		
		log << "["<<stamp << "] missed " << packetsMissedInLifetime;
		log << " over " << stamp-m_gameState.teleopStartTime << "\n";
		
		log << "Min Cycle Time: " << m_minCycleTime << "\n";
		log << "Max Cycle Time: " << m_maxCycleTime << "\n";
		
		log.close();
	}
	
	void DisabledInit() {
//		m_auton.StopAuton();
		AsynchronousPrinter::Printf("Rechecking CANJaguars.\n");
		m_canBusController.Enumerate();
		
		m_lift.StopLift();
		
		if (m_gameState.state == kHasEnteredAutonomousFollowedByTeleop) {
			LogPostGameData();
			m_gameState.state = kNothing;
		}
	}

	void AutonomousInit() {
		m_roller.SetEnabled(true);
		m_gameState.state = kHasEnteredAutonomous;
		
		packetsMissedInLifetime = 0;
//		m_auton.StartAuton();
	}

	void TeleopInit() {
//		m_auton.StopAuton();
		m_gearboxLeft.ShiftTo(LRTGearBox::kHighGear);
		m_gearboxRight.ShiftTo(LRTGearBox::kHighGear);
		
		if (m_gameState.state == kHasEnteredAutonomous) {
			m_gameState.state = kHasEnteredAutonomousFollowedByTeleop;
			m_gameState.teleopStartTime = Timer::GetFPGATimestamp();
		}
	}
	
	void TestModeInit() {
		m_lift.SetTestMode(true);
	}
	void TestModeExit() {
		m_lift.SetTestMode(false);
	}
	
	void ServiceModeInit() {
		m_kicker.Disable();
		m_gearboxLeft.SetServiceMode(true);
		m_gearboxRight.SetServiceMode(true);
	}
	void ServiceModeExit() {
		m_kicker.Enable();
		m_gearboxLeft.SetServiceMode(false);
		m_gearboxRight.SetServiceMode(false);
	}

	/********************************** Periodic Routines *************************************/
	
	void ApplyConfig() {
		m_gearboxLeft.ApplyConfig();
		m_gearboxRight.ApplyConfig();
		m_kicker.ApplyConfig();
		m_lift.ApplyConfig();
		m_roller.ApplyConfig();
//		m_auton.ApplyConfig();
		//TODO put everything that needs to be called in here.
	}
	
	void TestCode() {
//		float target = m_controls.GetDriverThrottle();
////		m_roller.SetRollerTargetSpeed(target);
//		m_rollerJaguar.Set(target);
//		m_dsLcd.print(LrtLcd::kRollerLine, "RollerTarget: %.3f", target);
		

		m_console.PrintMultipleTimesPerSecond(0.25,
				"ShiftL:%.3f R:%.3f\n", m_gearboxLeft.m_shifterServo.Get()
				, m_gearboxRight.m_shifterServo.Get());
		
		m_console.PrintMultipleTimesPerSecond(0.25, "PulseCountLeft: %d  Right: %d\n",
				m_gearboxLeft.GetPulseCount(), m_gearboxRight.GetPulseCount());
		m_console.PrintMultipleTimesPerSecond(0.25, "Gearbox state Left: %d, Right: %d\n",
				m_gearboxLeft.GetState(), m_gearboxRight.GetState());
	}
	
	void OutputLcdStatuses() {
		char beat = m_loadArray[(m_commonLoops / 12) % 4];
		string mode = "Normal Mode";
		if( m_isServiceMode )
			mode = "**SERVICE MODE**";
		else if( m_isTestMode )
			mode = "**TEST MODE**";
		m_dsLcd.print( LrtLcd::kHeartbeatLine, "%c %s", beat, mode.c_str() );
		

		m_console.PrintMultipleTimesPerSecond(0.25,
			"Arm=%d, Lift=%d\n",
			m_lift.GetArmRaw(), m_lift.GetLiftRaw()
		);

		LrtLcd::GetInstance().print(LrtLcd::kEncoderLine,
				"EncL=%.1f,R=%.1f,fwd=%.1f",
				m_driveEncoders.GetLeftSpeed(),
				m_driveEncoders.GetRightSpeed(),
				m_driveEncoders.GetForwardSpeed()
		);
		
		LrtLcd::GetInstance().print(LrtLcd::kLiftExtenderLine,
//					"Arm=%d,Lift=%d,K%d",
				"Arm=%d,Lift=%d,K%s",
				m_lift.GetArmRaw(), m_lift.GetLiftRaw(),
//					m_kicker.GetSenseRaw());
				m_kicker.GetSense() ? "!" : "-");
		
		LrtLcd::GetInstance().print(LrtLcd::kDriveLine,
				"Fw:%.3f Turn: %.3f",
				m_controls.GetForward(), m_controls.GetTurn());

		LrtLcd::GetInstance().print(LrtLcd::kBallDetectorLine, "%s: %d | %d",
				m_detector.IsBallClose() ? "!BALL!" : "noball",
				m_detector.GetValue(), m_detector.GetThreshold());
	}

	void CommonPeriodic() {
		{
			ProfiledSection pf("CommonPeriodic");
			
//			AsynchronousPrinter::Printf("asdfklsa;dfjkldsf asdflksdfsdf\n");
			OutputLcdStatuses();

			{
				ProfiledSection pf("ConsoleOutput+Controls");
				
				m_console.NextCycle();
				
				m_controls.Update(); // need to update debouncing
			}

			{
				ProfiledSection pf("TestCode");
//				TestCode();
				
				m_console.PrintMultipleTimesPerSecond(0.25,"Left encoder ticks: %d  Right encoder ticks: %d\n", m_driveEncoders.GetLeftEncoder().Get(), m_driveEncoders.GetRightEncoder().Get());
//				m_console.PrintMultipleTimesPerSecond(4,"Forward Rate = %g\n", m_driveEncoders.GetForwardSpeed());
			}
//			if (m_commonLoops %24 == 0)
//			{
//				AsynchronousPrinter::Printf("left speed %f right speed %f\n", m_driveEncoders.GetLeftSpeed(), m_driveEncoders.GetRightSpeed());
//			}
			
			//update the LCD at 4Hz
			if (m_commonLoops % 12 == 0) {
				ProfiledSection pf("LCDUpdate");
				m_dsLcd.LCDUpdate();
			}
			
			HandleServiceModeEntryControls();
			HandleLCDScrollControls();
			if (m_isServiceMode) {
				ServiceMode();
			} else {
				{
					ProfiledSection pf("HandleConfig");
					HandleConfig();
				}
			}
		}
	}
	
	void EnabledPeriodic() {
		{
			ProfiledSection pf("ApplyingOutputs");
			
			if (!m_isServiceMode) {
				m_brakeLeft.UpdateOutput();
				m_brakeRight.UpdateOutput();
				
				m_winch.ApplyOutput();
				{
					ProfiledSection pf("ApplyingOutputs.Lift");
					m_lift.ApplyOutput();
				}
				{
					ProfiledSection pf("ApplyingOutputs.Roller");
					m_roller.ApplyOutput();
				}
			}
		}
	}
	
	void ServiceMode() {
		HandleServiceLiftControls();
		HandleServiceWinchControls();
		HandleServiceKickerControls();
		HandleServiceAutonControls();
	}
	
	void DisabledPeriodic()  {
	}

	void AutonomousPeriodic() {
	}
	
	void CANDiagnostics(const char* name, CANJaguar& esc) {
		m_console.PrintEveryHalfSecond("%s=> ", name);
		m_console.PrintEveryHalfSecond("Bus: %5.2f V ", esc.GetBusVoltage());
		m_console.PrintEveryHalfSecond("Out: %6.2f V ", esc.GetOutputVoltage());
		m_console.PrintEveryHalfSecond("Cur: %4.1f A ", esc.GetOutputCurrent());
		m_console.PrintEveryHalfSecond("Temp: %5.1f Deg C ", esc.GetTemperature());
		m_console.PrintEveryHalfSecond("LimSw: %s%s ", esc.GetForwardLimitOK() ? "F":"-",
				esc.GetReverseLimitOK() ? "R":"-");
		m_console.PrintEveryHalfSecond("PwrCyc: %d ", esc.GetPowerCycled() ? 1:0);
		m_console.PrintEveryHalfSecond("\n");
	}

	void TeleopPeriodic() {
		{
			ProfiledSection pf("TeleopPeriodic");
			if (m_isServiceMode) {
				return;
			}
			
			/*if (m_controls.GetBrake()) {
				m_robotDrive.ArcadeDrive(0,0);
				m_brakeLeft.SetBrake();
				m_brakeRight.SetBrake();*/
//			} else {
			{
			m_robotDrive.ArcadeDrive(m_controls.GetForward(), m_controls.GetTurn());
			}
			//if supposed to pivot apply on to be braking
			if (m_controls.GetRightPivot()){
				m_escRight.Set(0.0);
				m_brakeRight.SetBrake();
			}
			else if(m_controls.GetLeftPivot()){
				m_escLeft.Set(0.0);
				m_brakeLeft.SetBrake();
			}
			LrtLcd::GetInstance().print(LrtLcd::kDriveLine2, "L:%.3f R:%.3f", m_escLeft.Get(), m_escRight.Get());
			
			ProfilerHelper pfh;
			pfh.Start("Teleop.Kicker");
			HandleKickerControls();
			pfh.Finish(); pfh.Start("Teleop.Winch");
			HandleWinchControls();
			pfh.Finish(); pfh.Start("Teleop.Lift");
			HandleLiftControls();
			pfh.Finish(); pfh.Start("Teleop.Gearbox");
			HandleGearBoxControls(); //Uncomment when ready to test gearboxes
			pfh.Finish(); pfh.Start("Teleop.Roller");
			HandleRollerControls();
			pfh.Finish();
			
			if (m_controls.GetAbortButton()) {
				m_kicker.Stop();
				m_lift.AbortArm();
				m_lift.AbortLift();
			}
	//		CANDiagnostics("left", m_escLeftRaw);
	//		CANDiagnostics("left", m_escLeft);
	//		CANDiagnostics("winch", m_escWinch);
		}
	}
	
	/**********************  CONTROL ROUTINES **********************/
	void HandleLCDScrollControls() {
		//Driver Station LCD Scrolling
		if (m_controls.GetLCDScrollUp()) {
			m_dsLcd.ScrollLCD(0, -2);
		} else if (m_controls.GetLCDScrollDown()) {
			m_dsLcd.ScrollLCD(0, 2);
		} else if (m_controls.GetLCDScrollLeft()) {
			m_dsLcd.ScrollLCD(-7, 0);
		} else if (m_controls.GetLCDScrollRight()) {
			m_dsLcd.ScrollLCD(7, 0);
		}
	}
	
	void HandleConfig() {
		{
			ProfiledSection pf("HandleConfig.ButtonHandling");
			//Operator Button 2 and 11 (press 2 first)
			if (m_controls.GetSaveConfigButton()) { // N.B. must be first if save button = reload+apply
				AsynchronousPrinter::Printf("SAVE CONFIG\n");
				m_config.Save();
			} 
			//Operator Button 2 and 9 (press 2 first)
			else if (m_controls.GetReloadConfigButton()) {
				ProfilerHelper pf;
				AsynchronousPrinter::Printf("RELOAD CONFIG\n");
				pf.Start("HandleConfig.ButtonHandling.load");
				m_config.Load();
				pf.Finish();
				pf.Start("HandleConfig.ButtonHandling.Apply");
				ApplyConfig();
				pf.Finish();
				pf.Start("HandleConfig.ButtonHandling.Save");
				m_config.Save(); // cause any missing variables to be automatically regenerated.
				pf.Finish();
			} 
			//Operator Button 2 and 7 (press 2 first)
			else if (m_controls.GetApplyConfigButton()) {
				AsynchronousPrinter::Printf("APPLY CONFIG\n");
				ApplyConfig();
			}
		}
		
		float assignableAnalogs[4];
		{
//			ProfiledSection pf("HandleConfig.GetAnalogs");
			for (int i = 0; i < 4; ++i) {
				assignableAnalogs[i] = m_ds.GetAnalogIn(i+1);
			}
		}
		{
			ProfiledSection pf("HandleConfig.UpdateAssignable");
			m_config.UpdateAssignableControls(assignableAnalogs);
		}
	}
	
	void HandleKickerControls() {
		if (m_controls.GetKickerRelease()) {
			m_kicker.Release();
		}
	}
	
	void HandleRollerControls(){
		
//		if (m_controls.GetToggleRoller()) {
//			m_roller.SetEnabled(!m_roller.GetEnabled());
//		}
//		
		if(m_controls.GetDisableRoller()) {
			m_roller.SetEnabled(false);
		}
		else {
			m_roller.SetEnabled(true);
		}
		
		if (m_controls.GetReverseRoller()) {
			m_console.PrintEveryHalfSecond("Reversing Roller\n");
			m_roller.SetReverse(true);
		}
	}
	
	void HandleWinchControls() {
		if (m_controls.GetRetractWinch()) {
			m_winch.RetractWinch();
		} else if (m_controls.GetReleaseWinch()) {
			m_winch.ReleaseWinch();
		}
	}
	
	void HandleLiftControls() {	
		//Driver or Operator Button 3
		if (m_controls.GetArmVerticalThenLiftExtend()) {
			AsynchronousPrinter::Printf("lift and arm going to extend\n");
			m_lift.ActivatePreset( LRTLift::kArmVertical );
			m_lift.ActivatePreset( LRTLift::kLiftExtended );			
		} else if (m_controls.GetLiftExtend()) {
			AsynchronousPrinter::Printf("Fully extending lift\n");
			m_lift.ActivatePreset( LRTLift::kLiftExtended);
		} else if (m_controls.GetLiftUp()) {
			m_console.PrintEveryHalfSecond("extending lift\n");
			m_lift.LiftExtend();
		} else if (m_controls.GetLiftDown()) {
			m_console.PrintEveryHalfSecond("retracting lift\n");
			m_lift.LiftRetract();
		} else if (m_controls.GetLiftHome()) {
			AsynchronousPrinter::Printf("lift going to home\n");
			m_lift.ActivatePreset( LRTLift::kLiftHome );			
		} else if (m_controls.GetLiftHomeThenArmHome()) {
			AsynchronousPrinter::Printf("lift and arm going to home\n");
			m_lift.ActivatePreset( LRTLift::kLiftHome );			
			m_lift.ActivatePreset( LRTLift::kArmHome );
		} else if (m_controls.GetLiftHomeThenArmMiddle()) {
			AsynchronousPrinter::Printf("lift to home and arm to middle\n");
			m_lift.ActivatePreset( LRTLift::kLiftHome );			
			m_lift.ActivatePreset( LRTLift::kArmMiddle );			
		}
		
		if (m_controls.GetArmVertical()) {
			AsynchronousPrinter::Printf("Arm going vertical\n");
			m_lift.ActivatePreset(LRTLift::kArmVertical);
		} else if (m_controls.GetArmUp()) {
			m_console.PrintEveryHalfSecond("raising arm\n");
			m_lift.ArmShiftUp();
		} else if (m_controls.GetArmDown()) {
			m_console.PrintEveryHalfSecond("lowering arm\n");
			m_lift.ArmShiftDown();
		} else if (m_controls.GetArmSpeedControlled()) {
			m_lift.RunArm( m_controls.GetArmSpeed() );
		}
	}
	
	void HandleGearBoxControls() {
		if (m_controls.GetShiftHighGear()) {
			// momentary high gear
			m_gearboxLeft.ShiftTo(LRTGearBox::kHighGear);
			m_gearboxRight.ShiftTo(LRTGearBox::kHighGear);
		} else {
			// default low gear
			m_gearboxLeft.ShiftTo(LRTGearBox::kLowGear);
			m_gearboxRight.ShiftTo(LRTGearBox::kLowGear);
		}
	}
	
	/********************* SERVICE MODE CONTROLS ***************/
	
	void HandleServiceModeEntryControls() {
		if (m_isServiceMode) {
			if (!m_controls.GetServiceMode()) { // user flipped to non servicemode
				AsynchronousPrinter::Printf("Service Mode EXITED\n");
				m_isServiceMode = false;
				ServiceModeExit();
			}
		} else {
			if (m_controls.GetServiceMode()) { // user flipped to SERVICEMODE
				AsynchronousPrinter::Printf("Service Mode ENTERED\n");
				m_isServiceMode = true;
				ServiceModeInit();
			}
		}
		
		if (!m_isServiceMode) {
			if (m_isTestMode) {
				if (!m_controls.GetTestMode()) { // user flipped out of TESTMODE
					AsynchronousPrinter::Printf("Test Mode EXITED\n");
					m_isTestMode = false;
					TestModeExit();
				}
			} else {
				if (m_controls.GetTestMode()) { // user flipped to TESTMODE
					AsynchronousPrinter::Printf("Test Mode ENTERED\n");
					m_isTestMode = true;
					TestModeInit();
				}
			}
		}
	}
	
	void HandleServiceLiftControls() {
		
		/****Controls for setting presets****/
		
		//Operator Button 9
		if (m_controls.GetServiceSetArmVertical()) {
			AsynchronousPrinter::Printf("Service: Setting Arm Vertical Preset\n");
			m_lift.SavePreset( LRTLift::kArmVertical );
		}
		
		// Operator button 7
		if (m_controls.GetServiceSetArmMiddle()) {
			AsynchronousPrinter::Printf("Service: Setting Arm Middle Preset\n");
			m_lift.SavePreset( LRTLift::kArmMiddle );
		}
		
		//Operator Button 10
		if (m_controls.GetServiceSetLiftExtended()) {
			AsynchronousPrinter::Printf("Service: Setting Lift Extended Preset\n");
			m_lift.SavePreset( LRTLift::kLiftExtended );
		}
		
		//Operator Button 11
		if (m_controls.GetServiceSetArmHome()) {
			AsynchronousPrinter::Printf("Service: Setting Arm Home Preset\n");
			m_lift.SavePreset( LRTLift::kArmHome );
		}
		
		//Operator Button 12
		if (m_controls.GetServiceSetLiftHome()) {
			AsynchronousPrinter::Printf("Service: Setting Lift Home Preset\n");
			m_lift.SavePreset( LRTLift::kLiftHome );
		}
		
		/****Manual Controls for Arm and Lift****/
		
		//Driver Button 10 - need to hold
		if (m_controls.GetServiceLiftUp()) {
			m_console.PrintEveryHalfSecond("Service: Lift up\n");
			m_lift.RunLiftUp();
		}
		//Driver Button 12 - need to hold
		else if (m_controls.GetServiceLiftDown()) {
			m_console.PrintEveryHalfSecond("Service: Lift down\n");
			m_lift.RunLiftDown();
		}
		else {
			m_lift.StopLift();
		}
		
		//Driver Button 9 - need to hold
		if (m_controls.GetServiceArmUp()) {
			m_console.PrintEveryHalfSecond("Service: Arm up\n");
//			m_lift.RunArm(LRTLift::kManualArmSpeed);
			m_lift.RunArm(0.35);
		}
		//Driver Button 11 - need to hold
		else if (m_controls.GetServiceArmDown()) {
			m_console.PrintEveryHalfSecond("Service: Arm down\n");
//			m_lift.RunArm(-LRTLift::kManualArmSpeed);
			m_lift.RunArm(-0.35);
		}
		else {
			m_lift.StopArm();
		}
	}
	
	void HandleServiceWinchControls() {
		if (m_controls.GetServiceRetractWinch()) {
			m_console.PrintEveryHalfSecond("Service: Retracting Winch\n");
			m_jaguarWinch.Set(1.0);
		}
		else if (m_controls.GetServiceReleaseWinch()) {
			m_console.PrintEveryHalfSecond("Service: Releasing Winch\n");
			m_jaguarWinch.Set(-1.0);
		}
		else {
			m_jaguarWinch.Set(0);
		}
	}
	
	void HandleServiceKickerControls() {
		if(m_controls.GetServiceKickerGo()) {
			m_console.PrintEveryHalfSecond("Service: Kicker Going\n");
			m_relayKicker.Set(LRTKicker::kKickerWindup);
		}
		else if (m_controls.GetServiceKickerPulseReverse()) {
			m_console.PrintEveryHalfSecond("Service: Kicker Reversing\n");
			m_relayKicker.Set(LRTKicker::kKickerReverse);
		}
		else {
			m_relayKicker.Set(Relay::kOff);
		}
	}
	
	void HandleServiceAutonControls() {
//		if( m_controls.GetServiceChangeAutonZone() )
//			m_auton.SetIsInCloseZone( !m_auton.GetIsInCloseZone() );
	}
};

START_ROBOT_CLASS(LRTBot10);
