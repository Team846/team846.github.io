#include "Scroll.h"


Scroll::Scroll(UINT32 slot, UINT32 upInput,UINT32 dnInput)
{
	m_upPin = new DigitalInput(slot,upInput);
	m_dnPin = new DigitalInput(slot,dnInput);
	m_ScrollDirection = 0;
}

Scroll::~Scroll()
{
	delete m_upPin;
	delete m_dnPin;
	m_upPin = NULL;
	m_dnPin = NULL;
}

int Scroll::ScrollDirection(void)
{
	return m_ScrollDirection;
	
}

void Scroll::ScrollCheck(void)
{
	static int scroll_direction;
	static int prev_direction;
	static int cnt;
	
	
	if(m_upPin->Get()==0)
		scroll_direction = -1;
	else if(m_dnPin->Get()==0)
		scroll_direction = 1;
	else
		scroll_direction = 0;
	
	if(scroll_direction == prev_direction)
		cnt++;
	else
		cnt = 0;
	
	if(cnt==kLCD_SCRL_RPT_CNT)
	{
		m_ScrollDirection = scroll_direction;
	}
	else if(cnt>kLCD_SCRL_RPT_CNT)
	{
		cnt--;
		
		m_ScrollDirection = 0;
	}
		
	prev_direction = scroll_direction;
			
}
