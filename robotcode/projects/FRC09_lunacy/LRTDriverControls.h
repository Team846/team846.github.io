/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2008. All Rights Reserved.							  */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in $(WIND_BASE)/WPILib.  */
/*----------------------------------------------------------------------------*/

#ifndef LRT_DRIVER_CONTROLS_H_
#define LRT_DRIVER_CONTROLS_H_

#include "CurrentSensor.h"
#include "SpeedController.h"
#include "Timer.h"
#include "Joystick.h"
#include "DriverStation.h"
#include "DebouncedJoystick.h"

class LRTDriverControls {
protected:
	DebouncedJoystick operatorStick, driverStick;
	
	const static int kNumButtons = 12;
	
public:

	enum AutonomousOptions { kNoButton, kGoStraight, kGoCornerLeft, kGoCornerRight, kGoAllLeft, kGoAllRight };
	
	LRTDriverControls()
		: operatorStick(1, kNumButtons), driverStick(2, kNumButtons) {
	}
	
	void Update() {
		driverStick.Update();
		operatorStick.Update();
	}
	
	
	DebouncedJoystick* GetDriverStick() { return &driverStick; }
	DebouncedJoystick* GetOperatorStick() { return &operatorStick; }
	

/*********************************************************************************************************************************************/

	
	// Driver and Operator Stick Button Definitions

	/******************************
	 * RawButton(1) Definitions   *
	 ******************************/
//	float GetShooterSpeed() {
//		if (driverStick.GetRawButton(1))
//			return (1. + GetDriverThrottle())/2.;
//		else
//			return 0;
//	}
	/** End of Button 1 Def's **/

	
	/******************************
	 * RawButton(2) Definitions   *
	 ******************************/
	
	
//	bool ResetShooterCounter() {
//		return(driverStick.GetRawButton(2) || operatorStick.GetRawButton(2));
//	}
	
	bool GetPreloadButton() {
		return driverStick.GetRawButton(2)||operatorStick.GetRawButton(2);
	}
	/** End of Button 2 Def's **/
	
	
	/******************************
	 * RawButton(3) Definitions   *
	 ******************************/
	bool GetShootLowSpeedButton() {
		return driverStick.GetRawButton(3)||operatorStick.GetRawButton(3);
	}
	/** End of Button 3 Def's **/

	
	/******************************
	 * RawButton(4) Definitions   *
	 ******************************/
//	bool GetDriveModeButton() {
//			return false;
//	//		return driverStick.GetRawButton(4);
//	}
	
	bool GetShootWaterfallButton() {
		return driverStick.GetRawButton(4)||operatorStick.GetRawButton(4);
	}
	/** End of Button 4 Def's **/
	
	
	/******************************
	 * RawButton(5) Definitions   *
	 ******************************/
	bool GetShootHighSpeedButton() {
		return driverStick.GetRawButton(5)||operatorStick.GetRawButton(5);
	}
	/** End of Button 5 Def's **/
	
	
	/******************************
	 * RawButton(6) Definitions   *
	 ******************************/
	bool GetLoggingButton() {
		return driverStick.GetRawButton(6);
	}
	
	bool GetGyroResetTestButton() {
		return operatorStick.GetRawButton(6);
	}
	/** End of Button 6 Def's **/
	
	
	/******************************
	 * RawButton(7) Definitions   *
	 ******************************/
	bool GetPickupOnButton() {
		return driverStick.GetRawButton(7);
	}
	/** End of Button 7 Def's **/
	
	
	/******************************
	 * RawButton(8) Definitions   *
	 ******************************/
	bool GetLiftOnButton() {
		return driverStick.GetRawButton(8);
	}	
	/** End of Button 8 Def's **/
	
	
	/******************************
	 * RawButton(9) Definitions   *
	 ******************************/
	bool GetPickupOffButton() {
		return driverStick.GetRawButton(9);
	}
	/** End of Button 9 Def's **/
	
	
	/******************************
	 * RawButton(10) Definitions  *
	 ******************************/
	bool GetLiftOffButton() {
		return driverStick.GetRawButton(10);
	}
	/** End of Button 10 Def's **/
	
	
	/******************************
	 * RawButton(11) Definitions  *
	 ******************************/
	bool GetPickupRevButton() {
		return driverStick.GetRawButton(11);
	}
	
	bool GetBrakeOverride() {
		return operatorStick.GetRawButton(11);
	}
	/** End of Button 11 Def's **/
		
	
	/******************************
	 * RawButton(12) Definitions   *
	 ******************************/
	bool GetTractionOverride() {
		return operatorStick.GetRawButton(12);
	}
	
	bool GetLiftRevButton() {
		return driverStick.GetRawButton(12);
	}
	
	//	bool GetTurn45Button() { //only for testing autonomous
	//		return driverStick.GetRawButton(12);
	//	}
	/** End of Button 12 Def's **/
		

	/*************************************
	 * Simultaneous Button Definitions   *
	 *************************************/
	
	/*************************************
	 *  Buttons 7 & 8 simulataneously   *
	 *************************************/
	float GetMaxAccel(){
		float maxaccel = -1.;	// set to a negative value unless we have valid data (which can only be positive)
		if(driverStick.GetRawButton(7)&&driverStick.GetRawButton(8)){
			maxaccel = (GetDriverThrottle()+1)/2.;
			maxaccel = maxaccel*0.3 + 0.0;
		}
		return maxaccel;
	}
	
	
	/*************************************
	 *  Buttons 9 & 10 simulataneously   *
	 *************************************/
	float GetMaxBrake(){
		float maxbrake = -1.;	// set to a negative value unless we have valid data (which can only be positive)
		if(driverStick.GetRawButton(9)&&driverStick.GetRawButton(10)){
			maxbrake = (GetDriverThrottle()+1)/2.;
			maxbrake = maxbrake*0.5 + 0.0;
		}
		return maxbrake;
	}
	
	/*************************************
	 *  Buttons 11 & 12 simulataneously   *
	 *************************************/
	float GetGyroPGain(){
		float pgain = -1.;	// set to a negative value unless we have valid data (which can only be positive)
		if(driverStick.GetRawButton(11)&&driverStick.GetRawButton(12)){
			pgain = (GetDriverThrottle()+1)/2.;
			pgain *= 0.5;
		}
		return pgain;
	}

	

	/*************************************
	 *  Buttons 7 - 12 simulataneously   *
	 *************************************/
	bool GetEnterServiceMode() {
		// buttons 7-12 on Operator Stick must be held down
		for (int i = 7; i <= 12; ++i) {
			if (!operatorStick.GetRawButton(i))
				return false;
		}
		return true;
	}
	bool GetExitServiceMode() {
		// buttons 7-12 on Driver Stick must be held down
		for (int i = 7; i <= 12; ++i) {
			if (!driverStick.GetRawButton(i))
				return false;
		}
		return true;
	}
	/** End of Simultaneous Button Def's **/

	
/*********************************************************************************************************************************************/
	
	
	// Driver and Operator Stick Axis Definitions

	/******************************
	 * RawButton(1) Definitions   *
	 ******************************/
	float GetForward() {
			return GetY();
	}
	
	float GetTurn() {
		return GetTwist();
	}
	
	float GetX() {
		return -operatorStick.GetX();
	}
	
	float GetY() {
		//return -stickLeft.GetY();
		return -driverStick.GetY();
	}
	
	float GetTwist(){
		return -driverStick.GetRawAxis(3);
	}
	
	float GetOperatorThrottle() {
		return -operatorStick.GetRawAxis(4);
	}
	
	float GetDriverThrottle() {
			return -driverStick.GetRawAxis(4);
		}
	
	
	//returns 1.00 for up, -1.00 for down
	float GetHatScroll() {
		return -driverStick.GetRawAxis(6);
	}
	
	bool GetHatScrollUp() {
		return GetHatScroll() > 0.50;
	}
	
	bool GetHatScrollDown() {
		return GetHatScroll() < -0.50;
	}	
	
//	This only works for the KOP joystick
//	float GetSlewAdjust() {
//		return (operatorStick.GetThrottle() + 1)/2;
//	}
		
//	UINT8 GetAirTextButton(){
//		if(operatorStick.GetRawButton(2)){
//			if(operatorStick.GetRawAxis(5)>0.)
//				return(1);
//			if(operatorStick.GetRawAxis(5)<0.)
//							return(3);
//			if(operatorStick.GetRawAxis(6)>0.)
//							return(2);
//			if(operatorStick.GetRawAxis(6)<0.)
//							return(4);
//			if((int)operatorStick.GetRawAxis(5)== 0 && (int)operatorStick.GetRawAxis(6) == 0)
//				return(20);
//		}
//		return 0;
//	}
/*********************************************************************************************************************************************/

	
	

};

#endif
