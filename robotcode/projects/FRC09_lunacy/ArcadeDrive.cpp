
#include "ArcadeDrive.h"
#include <cmath>

DriveOutput ArcadeDrive::DoArcadeDrive(float forward, float turn) {
	DriveOutput out;

	forward = LRTUtil::clamp(forward, -1, 1);
	turn = LRTUtil::clamp(turn, -1, 1);
	
	if (m_squaredInputs) {
		// square the inputs (while preserving the sign) to increase fine control while permitting full power
		if (forward >= 0.0) {
			forward = (forward * forward);
		} else {
			forward = -(forward * forward);
		}
		if (turn >= 0.0) {
			turn = (turn * turn);
		} else {
			turn = -(turn * turn);
		}
	}

	if (forward > 0.0) {
		if (turn > 0.0) {
			out.left = forward - turn;
			out.right = max(forward, turn);
		} else {
			out.left = max(forward, -turn);
			out.right = forward + turn;
		}
	} else {
		if (turn > 0.0) {
			out.left = -max(-forward, turn);
			out.right = forward + turn;
		} else {
			out.left = forward - turn;
			out.right = -max(-forward, -turn);
		}
	}
	
	return out;
}
