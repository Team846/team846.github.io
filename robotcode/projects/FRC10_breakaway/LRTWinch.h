//Author: Brandon Liu (2010)

#ifndef LRT_WINCH_H_
#define LRT_WINCH_H_

#include "SafeCANJaguar.h"
#include "ProxiedCANJaguar.h"

class LRTWinch {
public:
	LRTWinch(ProxiedCANJaguar& esc);
	
	void RetractWinch();
	void ReleaseWinch();
	void ApplyOutput();
	float GetCurrent();
	
private:
	ProxiedCANJaguar& m_esc;
	enum WinchState {kRetracting, kReleasing, kIdle} m_winchState;
};

#endif /* LRT_WINCH_H_ */
