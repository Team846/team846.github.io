//Author: David Liu (2009)

#ifndef DRIVEMETHOD_H
#define DRIVEMETHOD_H
#include "WPILib.h"
#include "LRTUtil.h"

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
	
	virtual DriveOutput ComputeTankDrive(float left, float right) {
		float forward = (left+right)/2;
		float turn = (right-left)/2;
		return ComputeArcadeDrive(forward, turn);
	}
	virtual DriveOutput ComputeArcadeDrive(float forward, float turn) {
		float left = forward - turn;
		float right = forward + turn;
		return ComputeTankDrive(left, right);
	}
	
	virtual void SetDriveOutputs(DriveOutput speeds) {
		speeds.left = LRTUtil::clamp<float>(speeds.left, -1, 1);
		speeds.right = LRTUtil::clamp<float>(speeds.right, -1, 1);
		m_leftDrive.Set(speeds.left);
		m_rightDrive.Set(speeds.right);
	}
	
public:
	
	void TankDrive(float left, float right) {
		left = LRTUtil::clamp<float>(left, -1, 1);
		right = LRTUtil::clamp<float>(right, -1, 1);
		SetDriveOutputs(ComputeTankDrive(left, right));
	}
	void ArcadeDrive(float forward, float turn) {
		forward = LRTUtil::clamp<float>(forward, -1, 1);
		turn = LRTUtil::clamp<float>(turn, -1, 1);
		SetDriveOutputs(ComputeArcadeDrive(forward, turn));
	}
	
};

#endif
