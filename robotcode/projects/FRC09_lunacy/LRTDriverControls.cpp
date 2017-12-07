/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2008. All Rights Reserved.							  */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in $(WIND_BASE)/WPILib.  */
/*----------------------------------------------------------------------------*/

#include "LRTDriverControls.h"

//
//	LRTDriverControls::LRTDriverControls()
//		: operatorStick(1), driverStick(2) {
//	}
//	
//	bool LRTDriverControls::GetFireButton() {
//		return driverStick.GetRawButton(1);
//	}
//	bool LRTDriverControls::GetShooterLowSpeedButton() {
//		return driverStick.GetRawButton(2);
//	}
//	bool LRTDriverControls::GetShooterWaterfallButton() {
//		return driverStick.GetRawButton(3);
//	}
//	bool LRTDriverControls::GetShooterHighSpeedButton() {
//		return driverStick.GetRawButton(5);
//	}
//	bool LRTDriverControls::GetShooterOffButton() {
//		return driverStick.GetRawButton(4);
//	}
//
//	//LRTDriverControls::Joystick GetLeftStick() { return driverStick; }
//	//LRTDriverControls::Joystick GetRightStick() { return operatorStick; }
//
//	float LRTDriverControls::GetForward() {
//		return GetY();
//	}
//	float LRTDriverControls::GetTurn() {
//		return GetTwist();
//	}
//	
//	float LRTDriverControls::GetX() {
//		return -operatorStick.GetX();
//	}
//	float LRTDriverControls::GetY() {
//		//return -stickLeft.GetY();
//		return -driverStick.GetY();
//	}
//	float LRTDriverControls::GetTwist(){
//		return -driverStick.GetRawAxis(3);
//	}
//
//	bool LRTDriverControls::GetPickupOnButton() {
//		return driverStick.GetRawButton(7);
//	}
//	bool LRTDriverControls::GetPickupOffButton() {
//		return driverStick.GetRawButton(9);
//	}
//	bool LRTDriverControls::GetPickupRevButton() {
//		return driverStick.GetRawButton(11);
//	}
//	bool LRTDriverControls::GetLiftRevButton() {
//		return driverStick.GetRawButton(12);
//	}
//	bool LRTDriverControls::GetLiftOnButton() {
//		return driverStick.GetRawButton(8);
//	}
//	bool LRTDriverControls::GetLiftOffButton() {
//		return driverStick.GetRawButton(10);
//	}
//	
////	bool LRTDriverControls::GetPickupOnButton() {
////		//return stickRight.GetRawButton(6);
////		//return (stickRight.GetRawButton(7)| stickLeft.GetRawButton(6));
////		return (driverStick.GetRawButton(7)| operatorStick.GetRawButton(7));
////	}
////	bool LRTDriverControls::GetPickupOffButton() {
////		//return stickRight.GetRawButton(7);
////		//return (stickRight.GetRawButton(9)| stickLeft.GetRawButton(7));
////		return (driverStick.GetRawButton(9)| operatorStick.GetRawButton(9));
////	}
////	bool LRTDriverControls::GetPickupRevButton() {
////		//return stickRight.GetRawButton(8);
////		//return (stickRight.GetRawButton(11) | stickLeft.GetRawButton(8));
////		return (driverStick.GetRawButton(11) | operatorStick.GetRawButton(11));
////	}
////	bool LRTDriverControls::GetLiftButton() {
////		//return stickRight.GetTrigger();
////		//return (stickRight.GetRawButton(8) | stickLeft.GetRawButton(11));
////		return (driverStick.GetRawButton(8) | operatorStick.GetRawButton(8));
////	}
////	bool LRTDriverControls::GetLiftRevButton() {
////		//return stickLeft.GetTrigger();
////		//return (stickRight.GetRawButton(12) | stickLeft.GetRawButton(9));
////		return (driverStick.GetRawButton(12) | operatorStick.GetRawButton(12));
////	}
////	bool LRTDriverControls::GetLiftOnButton() {
////		//return stickRight.GetRawButton(11);
////		//return (stickRight.GetRawButton(8) | stickLeft.GetRawButton(11));
////		return (driverStick.GetRawButton(8) | operatorStick.GetRawButton(8));
////	}
////	bool LRTDriverControls::GetLiftOffButton() {
////		return (driverStick.GetRawButton(10) | operatorStick.GetRawButton(10));
////	}
//	
//	bool LRTDriverControls::ResetShooterCounter() {
//		return(driverStick.GetRawButton(2) | operatorStick.GetRawButton(2));
//	}
//	float LRTDriverControls::GetShooterSpeed() {
//		//return stickRight.GetThrottle();
//		//return stickRight.GetRawAxis(4);
//		//return (1. - stickLeft.GetThrottle())/2.;
//		if (driverStick.GetRawButton(1))
//			return (1. - driverStick.GetRawAxis(4))/2.;
//		else
//			return 0;
//	}
//	
//	float LRTDriverControls::GetOperatorThrottle() {
//			
//				return -operatorStick.GetRawAxis(4);
//		}
//	
//	//returns 1.00 for up, -1.00 for down
//	float LRTDriverControls::GetHatScroll() {
//		return -driverStick.GetRawAxis(6);
//	}
//	
//	bool LRTDriverControls::GetHatScrollUp() {
//		return GetHatScroll() > 0.50;
//	}
//	
//	bool LRTDriverControls::GetHatScrollDown() {
//		return GetHatScroll() < -0.50;
//	}
//	
//	float LRTDriverControls::GetSlewAdjust() {
//		return (operatorStick.GetThrottle() + 1)/2;
//	}
//
//	bool LRTDriverControls::GetDriveModeButton() {
//		return driverStick.GetRawButton(4);
//	}
//
//	bool LRTDriverControls::GetLoggingButton() {
//		return driverStick.GetRawButton(2);
//	}
//	
//	UINT8 LRTDriverControls::GetAirTextButton(){
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
//
//
//	bool LRTDriverControls::GetEnterServiceMode() {
//		// buttons 7-12 on Operator Stick must be held down
//		for (int i = 7; i <= 12; ++i) {
//			if (!operatorStick.GetRawButton(i))
//				return false;
//		}
//		return true;
//	}
//	bool LRTDriverControls::GetExitServiceMode() {
//		// buttons 7-12 on Driver Stick must be held down
//		for (int i = 7; i <= 12; ++i) {
//			if (!driverStick.GetRawButton(i))
//				return false;
//		}
//		return true;
//	}
//
//
