#ifndef SCROLL_H
#define SCROLL_H
#include "WPILib.h"

class Scroll
{
public:
	Scroll(UINT32,UINT32,UINT32);
	virtual ~Scroll();
	int ScrollDirection(void);
	void ScrollCheck(void);
	
private:
	
	const static int kLCD_SCRL_RPT_CNT = 2;
	DigitalInput *m_upPin;
	DigitalInput *m_dnPin;
	int m_ScrollDirection;
	
	
};

#endif
