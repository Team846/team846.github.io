#ifndef DRIVEMETHOD_H
#define DRIVEMETHOD_H
#include "WPILib.h"
#include "util.h"

// Warning: You MUST override DoTankDrive or DoArcadeDrive, or infinite loop will occur.

struct DriveOutput {
	float left, right;
};

class DriveMethod {
protected:
	SpeedController &m_leftDrive, &m_rightDrive;
	DriveMethod(SpeedController &leftDrive, SpeedController &rightDrive)
		: m_leftDrive(leftDrive), m_rightDrive(rightDrive)
	{
		// Must be subclassed!
	}
	
	virtual DriveOutput DoTankDrive(float left, float right) {
		float forward = (left+right)/2;
		float turn = (right-left)/2;
		return DoArcadeDrive(forward, turn);
	}
	virtual DriveOutput DoArcadeDrive(float forward, float turn) {
		float left = forward - turn;
		float right = forward + turn;
		return DoTankDrive(left, right);
	}
	
	virtual void DoDrive(DriveOutput speeds) {
		m_leftDrive.Set(speeds.left);
		m_rightDrive.Set(speeds.right);
	}
	
public:
	
	void TankDrive(float left, float right) {
		left = LRTUtil::clamp(left, -1, 1);
		right = LRTUtil::clamp(right, -1, 1);
		DoDrive(DoTankDrive(left, right));
	}
	void ArcadeDrive(float forward, float turn) {
		forward = LRTUtil::clamp(forward, -1, 1);
		turn = LRTUtil::clamp(turn, -1, 1);
		DoDrive(DoArcadeDrive(forward, turn));
	}
	
};

#endif
