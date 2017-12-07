/********************************************************************************
* FILE NAME: ifi_aliases.h	
*
* DESCRIPTION: selects which header file based on project macro _FRC_BOARD
*
********************************************************************************/
#ifndef __ifi_aliases_h_


#ifdef _FRC_BOARD
	#include "ifi_aliasesFRC.h"
#else
	#include "ifi_aliasesEDU.h"
	#undef __ifi_aliases_h_
	
	//patch up inconsistencies
	#undef rc_dig_in01
	#undef rc_dig_in02
	#undef rc_dig_in03
	#undef rc_dig_in04
	#undef rc_dig_in05
	#undef rc_dig_in06
	#undef rc_dig_in07
	#undef rc_dig_in08
	#undef rc_dig_in09
	#undef rc_dig_in10
	#undef rc_dig_in11
	#undef rc_dig_in12
	#undef rc_dig_in13
	#undef rc_dig_in14
	#undef rc_dig_in15
	#undef rc_dig_in17
	#undef rc_dig_in18
	
	#undef battery_voltage
	#include "ifi_aliasesEDU.h"


#endif



#endif	//__ifi_aliases_h_
