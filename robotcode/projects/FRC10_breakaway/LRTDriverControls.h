//Author: Brandon Liu and David Liu (2009,2010)

#ifndef LRT_DRIVER_CONTROLS_H_
#define LRT_DRIVER_CONTROLS_H_

#include "SpeedController.h"
#include "Timer.h"
#include "Joystick.h"
#include "DriverStation.h"
#include "DebouncedJoystick.h"
#include "LRTUtil.h"
#include <algorithm>

class LRTDriverControls {
protected:
	DebouncedJoystick driverStick, operatorStick;
	
	const static int kNumButtons = 12;
	const static int kNumAxes = 6;
public:

	LRTDriverControls()
	: driverStick(1, kNumButtons, kNumAxes)
	, operatorStick(2, kNumButtons, kNumAxes)
	{
		
	}
	
	void Update() {
		driverStick.Update();
		operatorStick.Update();
	}
	
	DebouncedJoystick& GetDriverStick() { return driverStick; }
	DebouncedJoystick& GetOperatorStick() { return operatorStick; }


//	bool GetShiftLowGear() {
//		return !driverStick.IsButtonDown(1);
//	}
	bool GetShiftHighGear() {
//		return driverStick.IsButtonDown(3);
		return false;
	}
	bool GetAbortButton() {
		return driverStick.IsButtonDown(5);
	}
	bool GetKickerRelease() {
		return driverStick.IsButtonDown(2);
	}
	//old braking ocer bump
	bool GetBrake() {
		return driverStick.IsButtonDown(11) || driverStick.IsButtonDown(12);
	}
	
	bool GetRightPivot(){
		return driverStick.IsButtonDown(12);
	}
	
	bool GetLeftPivot(){
		return driverStick.IsButtonDown(11);
	}
	
	bool GetLCDScrollLeft() {
		return IsOperatorShift() && operatorStick.IsButtonJustPressed(5);
		// can't detect hat LEFT, only UPLEFT and DOWNLEFT
//		return driverStick.IsHatSwitchJustPressed(5, -1);
	}
	bool GetLCDScrollRight() {
		return IsOperatorShift() && operatorStick.IsButtonJustPressed(6);
//		return driverStick.IsHatSwitchJustPressed(5, 1);
	}
	bool GetLCDScrollUp() {
		return operatorStick.IsHatSwitchJustPressed(6, -1);
	}
	bool GetLCDScrollDown() {
		return operatorStick.IsHatSwitchJustPressed(6, 1);
	}
	
	bool GetRetractWinch(){
		return driverStick.IsButtonDown(7) || operatorStick.IsButtonDown(2);
	}
	bool GetReleaseWinch(){
		return driverStick.IsButtonDown(8);
	}

	bool IsOperatorShift() {
		return operatorStick.IsButtonDown(1);
	}
	
	bool GetApplyConfigButton() {
		return IsOperatorShift() && operatorStick.IsButtonJustPressed(7);
	}
	bool GetReloadConfigButton() {
		return IsOperatorShift() && operatorStick.IsButtonJustPressed(9);
	}
	bool GetSaveConfigButton() {
		return IsOperatorShift() && operatorStick.IsButtonJustPressed(11);
	}
	
	bool GetArmVertical() {
		return !IsOperatorShift() && operatorStick.IsButtonJustPressed(7);
	}
	bool GetArmVerticalThenLiftExtend() {
		return operatorStick.IsButtonJustPressed(3);
	}
	bool GetLiftHomeThenArmMiddle() {
		return operatorStick.IsButtonJustPressed(6);
	}
	bool GetLiftHomeThenArmHome() {
		return operatorStick.IsButtonJustPressed(4);
	}
	bool GetLiftExtend() {
		return operatorStick.IsButtonJustPressed(8);
	}
	bool GetLiftUp() {
		return driverStick.IsButtonDown(10)
			|| operatorStick.IsButtonDown(10);
	}
	bool GetLiftDown() {
		return operatorStick.IsButtonDown(12);
	}
	bool GetLiftHome() {
		return operatorStick.IsButtonJustPressed(5);
	}
	
	const static float minArmY = 0.5;
	bool GetArmUp() {
		return !IsOperatorShift() && operatorStick.IsButtonDown(9);
	}
	bool GetArmDown() {
		return !IsOperatorShift() && operatorStick.IsButtonDown(11);
	}
	float GetArmSpeed() {
		// need negative value to ensure correct direction
		// (if joystick pushed forward, arm up)
		return -LRTUtil::addDeadband<float>( operatorStick.GetY(), minArmY ) * 0.6;
	}
	bool GetArmSpeedControlled() {
		return LRTUtil::abs<float>(operatorStick.GetY()) > minArmY;
	}
	

	bool GetDisableRoller() {
		return driverStick.IsButtonDown(9);
//		return false;
	}
	bool GetReverseRoller() {
		return driverStick.IsButtonDown(1);
	}
	bool GetToggleRoller() {
		return driverStick.IsButtonJustPressed(9);
	}
	
	/*************** SERVICE MODE **************/
	bool GetTestMode() {
		return GetOperatorThrottle() > 0.8 ^ GetDriverThrottle() > 0.8; 
	}
	
	bool GetServiceMode() {
		return GetOperatorThrottle() > 0.8 && GetDriverThrottle() > 0.8; 
	}
	
	/*** DRIVER STICK ***/
	bool GetServiceKickerGo() {
		return driverStick.IsButtonDown(2);
	}
	bool GetServiceKickerPulseReverse() {
		return driverStick.IsButtonJustPressed(3);
	}
	bool GetServiceRetractWinch() {
		return driverStick.IsButtonDown(7);
	}
	bool GetServiceReleaseWinch() {
		return driverStick.IsButtonDown(8);
	}
	bool GetServiceArmUp() {
		return driverStick.IsButtonDown(9);
	}
	bool GetServiceArmDown() {
		return driverStick.IsButtonDown(11);
	}
	bool GetServiceLiftUp() {
		return driverStick.IsButtonDown(10);
	}
	bool GetServiceLiftDown() {
		return driverStick.IsButtonDown(12);
	}
	
	/*** OPERATOR STICK ***/

	bool GetServiceChangeAutonZone() {
		return IsOperatorShift() && operatorStick.IsButtonJustPressed(4);
	}
	
	bool GetServiceSetArmVertical() {
		return IsOperatorShift() && operatorStick.IsButtonJustPressed(9);
	}
	
	bool GetServiceSetArmMiddle() {
		return IsOperatorShift() && operatorStick.IsButtonJustPressed(7);
	}

	bool GetServiceSetArmHome() {
		return IsOperatorShift() && operatorStick.IsButtonJustPressed(11);
	}

	bool GetServiceSetLiftExtended() {
		return IsOperatorShift() && operatorStick.IsButtonJustPressed(10);
	}

	bool GetServiceSetLiftHome() {
		return IsOperatorShift() && operatorStick.IsButtonJustPressed(12);
	}

	
/*****************************************************************************************************/
	
	// Driver and Operator Stick Axis Definitions

	//normal controls
	float GetForward() {
		return LRTUtil::addDeadband<float>( -driverStick.GetY(), 0.15 );
	}
	
	//backwards controls
//	float GetForward() {
//		return LRTUtil::addDeadband<float>( driverStick.GetY(), 0.15 );
//	}
	
	//normal turn
	float GetTurn() {
//		return LRTUtil::addDeadband<float>( -driverStick.GetX(), 0.15 );
//		return LRTUtil::addDeadband<float>( -driverStick.GetRawAxis(3), 0.15 ); // twist axis
		return LRTUtil::addDeadband<float>( -driverStick.GetRawAxis(3), 0.2 ); // twist axis
		
//		Using both axes:
//		float twistAxis = LRTUtil::addDeadband<float>( -driverStick.GetRawAxis(3), 0.2 );
//		float xAxis = LRTUtil::addDeadband<float>( -driverStick.GetX(), 0.15 );
//		
//		if( LRTUtil::abs( xAxis ) < 0.15 )
//			return LRTUtil::absMax( twistAxis, xAxis );
//		else
//			return xAxis;
	}
	
	//backwards turn
//	float GetTurn() {
//	//		return LRTUtil::addDeadband<float>( -driverStick.GetX(), 0.15 );
//	//		return LRTUtil::addDeadband<float>( -driverStick.GetRawAxis(3), 0.15 ); // twist axis
//			return LRTUtil::addDeadband<float>( driverStick.GetRawAxis(3), 0.2 ); // twist axis
//			
//	//		Using both axes:
//	//		float twistAxis = LRTUtil::addDeadband<float>( -driverStick.GetRawAxis(3), 0.2 );
//	//		float xAxis = LRTUtil::addDeadband<float>( -driverStick.GetX(), 0.15 );
//	//		
//	//		if( LRTUtil::abs( xAxis ) < 0.15 )
//	//			return LRTUtil::absMax( twistAxis, xAxis );
//	//		else
//	//			return xAxis;
//		}
	
	float GetOperatorThrottle() {
		return -operatorStick.GetRawAxis(4);
	}
	
	float GetDriverThrottle() {
		return -driverStick.GetRawAxis(4);
	}
	
	
	

/*********************************************************************************************************************************************/

};

#endif /* LRT_DRIVER_CONTROLS_H_ */
