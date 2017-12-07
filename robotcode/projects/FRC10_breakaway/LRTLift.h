// Author: Brandon Liu and Karthik Viswanathan (2010)

#ifndef LRT_LIFT_H_
#define LRT_LIFT_H_

#include "LRTConnections.h"
#include "LRTUtil.h"
#include "PIDController.h"
#include "LRTConfig.h"
#include "SafeCANJaguar.h"
#include "LRTConsole.h"
#include <string>
#include "CANJaguar/CANJaguar.h"
#include "Relay.h"

class LRTLiftStallDetector {
public:
	LRTLiftStallDetector() :
	m_cycleCount(0) 
	, m_isEnabled(false) 
	, m_isLiftStalled(false) {
	}
	
	void ApplyConfig() {
		LRTConfig &config = LRTConfig::GetInstance();
		std::string prefix = "LRTLiftStallDetector.";
		
		//the lift should travel about one tick per cycle, and we're monitoring over 25 cycles
		m_minLiftMovement = config.Get<int>(prefix + "minLiftMovement", 5);
	}
	
	void EnableIfDisabled() {
		if (!m_isEnabled) {
			m_isEnabled = true;
			m_isLiftStalled = false;
		}
	}
	
	void Disable() {
		m_isEnabled = false;
		m_cycleCount = 0;
	}
	
	bool IsLiftStalled(int liftPosition) {
		if (!m_isEnabled) {
			return false;
		}
		
		if (m_isLiftStalled) {
			return true;
		}
		
		m_prevPotValues[m_cycleCount % kNumValues] = liftPosition;
		m_cycleCount++;
		if (m_cycleCount >= kNumValues) {
			int diff = LRTUtil::abs<int>(m_prevPotValues[m_cycleCount % kNumValues] - liftPosition);
//			AsynchronousPrinter::Printf("Lift Movement = %d\n", diff);
//			AsynchronousPrinter::Printf("Lift Min Movement = %d\n", m_minLiftMovement);
			if (diff < m_minLiftMovement) {
				m_isLiftStalled = true;
				return true;
			}
			return false;
		}
		return false;
	}
	
private:
	const static int kNumValues = 25;
	int m_cycleCount;
	int m_prevPotValues[kNumValues];
	int m_minLiftMovement;
	bool m_isEnabled;
	bool m_isLiftStalled;
};

class LRTLift {
private:
	Relay m_liftRelay;
	AnalogChannel m_liftPot;
	LRTLiftStallDetector m_liftStallDetector;
	int m_liftTargetPosition;
	int m_liftMinPosition, m_liftMaxPosition; // hard limit positions of the mechanical system
	int m_liftSafetyZone; // can only pulse when within this many pot-units of the hard limit
	enum LiftState {kLiftExtending, kLiftRetracting, kLiftTargeting, kLiftIdle} m_liftState;
	enum Direction {kUp, kDown};
	Direction m_liftTargetDirection; // lets us know when we've passed our target
	
	SpeedController& m_armEsc;
	AnalogChannel m_armPot;
	int m_armTargetPosition;
	int m_armMinPosition, m_armMaxPosition;
	enum ArmState {kArmRaising, kArmLowering, kArmTargeting, kArmIdle} m_armState;
	Direction m_armTargetDirection; // lets us know when we've passed our target
	
	//Used for interlock
	int m_canExtendLiftThreshold; // arm must be above this value to extend lift  
	int m_canLowerArmThreshold; // lift must be below this value to lower arm
	
	bool m_testMode;
	
	LRTConfig &m_config; // member variable because it is used in multiple functions
	LRTConsole &m_console;
	string m_prefix; // member variable because it is used in multiple functions
	
	void ApplyLiftOutput();
	void ApplyArmOutput();
	
public:
	LRTLift( SpeedController& m_armEsc);
	static enum PresetState {kLiftExtended, kLiftHome, kArmVertical, kArmMiddle, kArmHome} m_presetState;
	
	void ApplyConfig();
	void SetTestMode(bool on);
	void LiftExtend();
	void LiftRetract();
	int GetLiftPosition();
	int GetLiftRaw();
	
	void ArmShiftUp();
	void ArmShiftDown();
	int GetArmPosition();
	int GetArmRaw();

	bool IsLiftInTopSafetyZone();
	bool IsLiftInBottomSafetyZone();
	
	bool CanExtendLift();
	bool CanLowerArm();
	
	void AbortLift();
	void AbortArm();
	
	void ApplyOutput();
	SafeCANJaguar& GetSpeedController();
	AnalogChannel& GetArmPot();
	Relay& GetLiftRelay();
	
	void SavePreset( PresetState ps );
	void SavePreset( PresetState ps, int value );
	int GetPreset( PresetState ps );
	void ActivatePreset( PresetState ps );
	
	void RunLiftUp();
	void RunLiftDown();
	void StopLift();
	
	void RunArm(float speed);
	void StopArm();
	
	const static Relay::Value kLiftUp = Relay::kReverse;
	const static Relay::Value kLiftDown = Relay::kForward;
	
//	const static float kManualArmSpeed = 0.35;
};


#endif /* LRT_LIFT_H_ */
