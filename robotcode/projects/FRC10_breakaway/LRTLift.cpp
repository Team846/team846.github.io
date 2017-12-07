// Author: Brandon Liu and Karthik Viswanathan (2010)

//pot low value = 222
//pot range = 315
// middle val = 400

#include "WPILib.h"
#include "LRTLift.h"
#include "LRTDriverStationLCD.h"
#include "AsynchronousPrinter.h"
#include "LRTConsole.h"
using namespace std;

LRTLift::LRTLift( SpeedController& m_armEsc)
: m_liftRelay( LRTConnections::kRelayLiftRaiser )
, m_liftPot(LRTConnections::kAnalogRotPotLift)
, m_liftState ( kLiftIdle )
, m_armEsc( m_armEsc )
, m_armPot( LRTConnections::kAnalogRotPotArm )
, m_armState ( kArmIdle )
, m_testMode(false)
, m_config( LRTConfig::GetInstance() )
, m_console(LRTConsole::GetInstance())
, m_prefix( "LRTLift." )
{
	ApplyConfig();
}

void LRTLift::ApplyConfig()
{	
	m_liftStallDetector.ApplyConfig();
	
	m_liftMinPosition = m_config.Get<int> (m_prefix + "liftMinPosition", 50);
	m_liftMaxPosition = m_config.Get<int> (m_prefix + "liftMaxPosition", 950);
	m_liftSafetyZone = m_config.Get<int> (m_prefix + "liftSafetyZone", 50);
	m_armMinPosition = m_config.Get<int> (m_prefix + "armMinPosition", 50);
	m_armMaxPosition = m_config.Get<int> (m_prefix + "armMaxPosition", 950);
	
	m_canExtendLiftThreshold = m_config.Get<int>( m_prefix + "canExtendLiftThreshold" );
	m_canLowerArmThreshold = m_config.Get<int>( m_prefix + "canLowerArmThreshold");
}

void LRTLift::SetTestMode(bool on) {
	m_testMode = on;
}

void LRTLift::LiftExtend() {
//	AsynchronousPrinter::Printf("LRTLift::LiftExtend\n");
	if (m_liftState != kLiftExtending) {
		m_liftStallDetector.Disable(); //Reset the detector
	}
	m_liftState = kLiftExtending;
}

void LRTLift::LiftRetract() {
//	AsynchronousPrinter::Printf("LRTLift::LiftRetract\n");
	if (m_liftState != kLiftRetracting) {
		m_liftStallDetector.Disable(); //Reset the detector
	}
	m_liftState = kLiftRetracting;
}

int LRTLift::GetLiftPosition() {
	return m_liftPot.GetValue();
}
int LRTLift::GetLiftRaw() {
	return m_liftPot.GetValue();
}

void LRTLift::ArmShiftUp() {
	AsynchronousPrinter::Printf("LRTLift::RightingShiftUp\n");
	m_armState = kArmRaising;
}

void LRTLift::ArmShiftDown() {
	AsynchronousPrinter::Printf("LRTLift::RightingShiftDown\n");
	m_armState = kArmLowering;
}

int LRTLift::GetArmPosition() {
	return m_armPot.GetValue();
}

int LRTLift::GetArmRaw() {
	return m_armPot.GetValue();
}

bool LRTLift::IsLiftInTopSafetyZone() {
	return GetLiftPosition() > m_liftMaxPosition - m_liftSafetyZone;
}
bool LRTLift::IsLiftInBottomSafetyZone() {
	return GetLiftPosition() < m_liftMinPosition + m_liftSafetyZone;
}

//Arm must be above a certain value before the lift can go up.
//Only activate this interlock if in the automatic targeting mode.
bool LRTLift::CanExtendLift() {
//	return true;
	return GetArmPosition() > m_canExtendLiftThreshold ;
}

//Only use this interlock in presets.
bool LRTLift::CanLowerArm() {
//	return true;
	return GetLiftPosition() < m_canLowerArmThreshold;
}

void LRTLift::AbortArm() {
	AsynchronousPrinter::Printf("ABORTING ARM\n");
	m_armState = kArmIdle;
}

void LRTLift::AbortLift() {
	AsynchronousPrinter::Printf("ABORTING LIFT\n");
	m_liftState = kLiftIdle;
}

void LRTLift::RunLiftUp() {
	if (m_liftPot.GetAverageValue() > 950) {
		m_console.PrintEverySecond("Lift Pot near MAX physical limits; disabling lift\n");
		return;
	}
	if (GetLiftPosition() > m_liftMaxPosition) {
		m_console.PrintEverySecond("Lift near MAX physical limits; disabling lift\n");
		return;
	}
	m_liftRelay.Set(kLiftUp);
//	AsynchronousPrinter::Printf("Running lift up\n");
}

void LRTLift::RunLiftDown() {
	if (m_liftPot.GetAverageValue() < 50) {
		m_console.PrintEverySecond("Lift Pot near MIN physical limits; disabling lift\n");
		return;
	}
	if (GetLiftPosition() < m_liftMinPosition) {
		m_console.PrintEverySecond("Lift near MIN physical limits; disabling lift\n");
		return;
	}
	
	m_liftRelay.Set(kLiftDown);
//	AsynchronousPrinter::Printf("Running lift down\n");
}

void LRTLift::StopLift()
{
	m_liftRelay.Set(Relay::kOff);
//	AsynchronousPrinter::Printf("Running lift STOP\n");
}

void LRTLift::RunArm(float speed) {
	if (speed > 0) {
		if (m_armPot.GetAverageValue() > 950) {
			m_console.PrintEverySecond("Arm Pot near MAX physical limits; disabling lift\n");
			return;
		}
		if (GetArmPosition() > m_armMaxPosition) {
			m_console.PrintEverySecond("Arm near MAX physical limits; disabling lift\n");
			return;
		}
	}
	else if (speed < 0) {
		if (m_armPot.GetAverageValue() < 50) {
			m_console.PrintEverySecond("Arm Pot near MIN physical limits; disabling lift\n");
			return;
		}
		if (GetArmPosition() < m_armMinPosition) {
			m_console.PrintEverySecond("Arm near MIN physical limits; disabling lift\n");
			return;
		}
		
	}
	m_armEsc.Set(speed);
}

void LRTLift::StopArm() {
	m_armEsc.Set(0);
}

void LRTLift::ApplyOutput() {
	ApplyArmOutput();
	ApplyLiftOutput();
}

void LRTLift::ApplyArmOutput() {
	double autoPower = m_testMode ? 0.35 : 1;
//	double autoPower = m_testMode ? kManualArmSpeed : 1;
	// power to use when automatically moving the arm.
	
	//Go slower when manually controlled, and faster when automatically controlled
	switch(m_armState) {
	case kArmRaising:
		RunArm(0.35); //raise up lift
		m_armState = kArmIdle; //Force driver to hold down button
		break;
	case kArmLowering:
		RunArm(-0.35); //lower lift
		m_armState = kArmIdle; //force driver to hold down button
		break;
	case kArmTargeting:
		int error = m_armTargetPosition - GetArmPosition();
		m_console.PrintMultipleTimesPerSecond(4, "ArmTargeting target = %d, error = %d\n", m_armTargetPosition, error);
		if (error > 0 && m_armTargetDirection == kUp) {
			RunArm(autoPower);
		}
		else if (error < 0 && m_armTargetDirection == kDown) {
			if (CanLowerArm()) {
				RunArm(-autoPower);
			}
			else {
				m_console.PrintMultipleTimesPerSecond(0.5, "CANNOT LOWER ARM\n");
			}
		}
		else {
			StopArm();
			m_armState = kArmIdle;
		}
		break;
	case kArmIdle:
		StopArm();
		break;
	}
}

void LRTLift::ApplyLiftOutput() {
	static LiftState m_previousLiftState = kLiftIdle;
	static int m_numberOfCyclesPowered = 0;
	static int m_numberOfCyclesResting = 0;
	static enum {kPulsePower, kPulseRest} m_pulseState = kPulsePower;
	static const int m_pulsePowerLength = 10; // out of 50/sec
	static const int m_pulseRestLength = 10; // out of 50/sec
	
	
	switch(m_liftState) {
	case kLiftExtending:
		RunLiftUp();
		m_liftStallDetector.EnableIfDisabled();
		m_liftTargetDirection = kUp; // used for safety-zone pulsing code
//		AsynchronousPrinter::Printf("Applying extension\n");
		break;
	case kLiftRetracting:
		RunLiftDown();
		m_liftStallDetector.EnableIfDisabled();
		m_liftTargetDirection = kDown; // used for safety-zone pulsing code
//		AsynchronousPrinter::Printf("Applying retraction\n");
		break;
	case kLiftTargeting:
//		AsynchronousPrinter::Printf("Applying stop\n");
		int error = m_liftTargetPosition - GetLiftPosition();
		m_console.PrintMultipleTimesPerSecond(4, "LiftTargeting target = %d, error = %d\n", m_liftTargetPosition, error);
		if (error > 0 && m_liftTargetDirection == kUp) {
			if (CanExtendLift()) {
				m_liftStallDetector.EnableIfDisabled();
				RunLiftUp();
			}
			else {
				m_liftStallDetector.Disable();
				m_console.PrintMultipleTimesPerSecond(0.5, "CANNOT EXTEND LIFT\n");
			}
		}
		else if (error < 0 && m_liftTargetDirection == kDown) {
			RunLiftDown();
		}
		else {
			StopLift();
			m_liftState = kLiftIdle;
		}
		break;
	case kLiftIdle:
		m_liftStallDetector.Disable();
		StopLift();
		break;
	}

	// Pulse in safety zones
	if (m_liftState != kLiftIdle && (
			IsLiftInTopSafetyZone() && m_liftTargetDirection == kUp
			|| IsLiftInBottomSafetyZone() && m_liftTargetDirection == kDown
		))
	{
		if (m_pulseState == kPulsePower) {
			++m_numberOfCyclesPowered;
			if (m_numberOfCyclesPowered > m_pulsePowerLength) {
				m_numberOfCyclesPowered = 0;
				m_pulseState = kPulseRest;
			}
		} else {
			++m_numberOfCyclesResting;
			StopLift(); // FIXME this belongs below the state handling code
			if (m_numberOfCyclesResting > m_pulseRestLength) {
				m_numberOfCyclesResting = 0;
				m_pulseState = kPulsePower;
			}
		}
	} else {
		// not in a pulsing mode; reset the pulse counters
		m_numberOfCyclesPowered = 0;
		m_numberOfCyclesResting = 0;
		m_pulseState = kPulsePower;
	}
	m_previousLiftState = m_liftState;

	//Force driver to hold down button if extending or retracting
	if (m_liftState == kLiftExtending || m_liftState == kLiftRetracting) {
		m_liftState = kLiftIdle;
	}
	
	//if the lift becomes stalled, stop the motor.
	if (m_liftStallDetector.IsLiftStalled(GetLiftPosition())) {
		AsynchronousPrinter::Printf("LIFT STALLED\n");
//		StopLift();
		m_liftState = kLiftIdle;
	}
}

/******************** PRESET CODE ******************/

void LRTLift::SavePreset( PresetState ps )
{
	switch( ps ) {
		case kLiftHome:
		case kLiftExtended:
			m_liftState = kLiftIdle;
			m_liftTargetPosition = GetLiftPosition();
			
			SavePreset( ps, m_liftTargetPosition );
			break;
			
		case kArmHome:
		case kArmMiddle:
		case kArmVertical:
			m_armState = kArmIdle;
			m_armTargetPosition = GetArmPosition();
			
			SavePreset( ps, m_armTargetPosition );
			break;
	}
}

void LRTLift::SavePreset( PresetState ps, int value )
{
	switch( ps ) {
		case kLiftHome:		
			m_config.Set<int>( m_prefix + "liftPresetHome", value );
			AsynchronousPrinter::Printf( "LRTLift::StorePreset successfully stored %d as lift home preset\n", value );
			break;
		
		case kLiftExtended:
			m_config.Set<int>( m_prefix + "liftPresetExtended", value );
			AsynchronousPrinter::Printf( "LRTLift::StorePreset successfully stored %d as lift extended preset\n", value );
			break;
			
		case kArmHome:		
			m_config.Set<int>( m_prefix + "armPresetHome", value );
			AsynchronousPrinter::Printf( "LRTLift::StorePreset successfully stored %d as arm home preset\n", value );
			break;
			
		case kArmMiddle:
			m_config.Set<int>( m_prefix + "armPresetMiddle", value );
			AsynchronousPrinter::Printf( "LRTLift::StorePreset successfully stored %d as arm middle preset\n", value );
			break;
			
		case kArmVertical:		
			m_config.Set<int>( m_prefix + "armPresetVertical", value );
			AsynchronousPrinter::Printf( "LRTLift::StorePreset successfully stored %d as arm vertical preset\n", value );
			break;
	}
	
	m_config.Save();
}

int LRTLift::GetPreset( PresetState ps )
{
	switch( ps ) {
		case kLiftHome:		
			return m_config.Get<int>( m_prefix + "liftPresetHome" );
		
		case kLiftExtended:
			return m_config.Get<int>( m_prefix + "liftPresetExtended" );
			
		case kArmHome:		
			return m_config.Get<int>( m_prefix + "armPresetHome" );
		
		case kArmMiddle:
			return m_config.Get<int>( m_prefix + "armPresetMiddle" );
		
		case kArmVertical:		
			return m_config.Get<int>( m_prefix + "armPresetVertical" );
		
		default:
			return 0;
	}
}

void LRTLift::ActivatePreset( PresetState ps )
{
	int preset = GetPreset( ps );
	
	switch( ps ) {
		case kLiftHome:
		case kLiftExtended:
			m_liftState = kLiftTargeting;
			m_liftTargetPosition = preset;
			if (GetLiftPosition() < m_liftTargetPosition)
				m_liftTargetDirection = LRTLift::kUp;
			else
				m_liftTargetDirection = LRTLift::kDown;
			AsynchronousPrinter::Printf( "LRTLift::ActivatePreset activated lift preset with a value of %d\n", preset );
			break;
			
		case kArmHome:
		case kArmMiddle:
		case kArmVertical:
			m_armState = kArmTargeting;
			m_armTargetPosition = preset;
			if (GetArmPosition() < m_armTargetPosition)
				m_armTargetDirection = LRTLift::kUp;
			else
				m_armTargetDirection = LRTLift::kDown;
			AsynchronousPrinter::Printf( "LRTLift::ActivatePreset activated arm preset with a value of %d\n", preset );
			break;
	}
}

//CANJaguar& LRTLift::GetSpeedController()
//{
//	return m_armEsc;
//}

AnalogChannel& LRTLift::GetArmPot()
{
	return m_armPot;
}

Relay& LRTLift::GetLiftRelay()
{
	return m_liftRelay;
}
