
#include "DBSDrive.h"
#include <cmath>

const static float kTurnInPlaceThreshold = (1./128); // maybe this should be 0; joystick deadband handling doesn't belong here
const static float kBrakeTurnDeadband = (50./128);

//***********************************************************************
DBSDrive::DBSDrive(
	SpeedController &leftDrive, SpeedController &rightDrive,
	Brake &leftBrake, Brake &rightBrake, int maxBraking
)
	: DriveMethod(leftDrive, rightDrive),
	  m_leftBrake(leftBrake), m_rightBrake(rightBrake),
	  m_maxBraking(maxBraking)
{
}

DriveOutput DBSDrive::DoArcadeDrive(float forward, float turn) {
	enum {CW,  CCW}   turnDir;
	enum {LEFT,RIGHT} inboardSide;
	int brakeAmt;
	DriveOutput out;
	
	if (fabs(forward) < kTurnInPlaceThreshold) // Normal turn-in-place
	{
		out.left  = -turn;
		out.right = +turn;
	}
	else	//Use DitherBraking
	{
		out.left  = forward - turn;
		out.right = forward + turn;
		
		if (turn >= 0)
		{
			turnDir = CCW;
			inboardSide = (forward >= 0 ? LEFT : RIGHT);
		}
		else
		{
			turnDir = CW;
			inboardSide = (forward >= 0 ? RIGHT : LEFT);
		}

		// If we're turning within the deadband, then we only scale
		// down the power on one side. If we exceed the deadband,
		// then we apply successively greater braking to that side.
		
		float absTurn = fabs(turn);
		if (absTurn < kBrakeTurnDeadband)
		{
			float inboardSidePower = forward - ( forward * absTurn / kBrakeTurnDeadband );
			if (inboardSide == LEFT)
				out.left  = inboardSidePower;
			else
				out.right = inboardSidePower;
		}
		else	//Use Dithered braking
		{
			
			brakeAmt = (int)(
				(absTurn - kBrakeTurnDeadband)
				* (m_maxBraking+1) / (1 - kBrakeTurnDeadband)
			); // FIXME: make sure this int cast is okay.
			// brakeAmt on range {0, maxBraking}
			

			if (inboardSide == LEFT) {
				out.left  = 0;
				m_leftBrake.Set(brakeAmt);
			}
			else {
				out.right = 0;
				m_rightBrake.Set(brakeAmt);
			}
			
			//printf("brkAmt %d/%d => L%d R%d\r", brakeAmt, maxBraking, drive->brakeLeft, drive->brakeRight);
		}
	}
	
	return out;
}
