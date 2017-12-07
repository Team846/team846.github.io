//Author: Brandon Liu and David Liu (2010)

#include "CANJaguarBrake.h"


// Note: Should this also contain the SpeedController object?
// And force it to 0 output when brakelevel > 0
CANJaguarBrake::CANJaguarBrake(ProxiedCANJaguar& jaggie)
	: m_jaguar(jaggie), m_cycleCount(0), m_amount(0)
{
}

void CANJaguarBrake::Set(int val8) {
	m_amount = val8;
}

void CANJaguarBrake::UpdateOutput() {
	// OLD WAY: Calculate dither pattern by reversing bits of binary 0-7
//	static const char ditherPattern[] = {0,4,2,6,1,5,3,7}; // OLD WAY
	//	if ((ditherPattern[m_cycleCount]) < m_amount) { // OLD WAY
	
	// 1-byte bitfields corresponding to value. See below for calculation.
	static const UINT8 ditherPattern[] = {0x00, 0x01, 0x11, 0x25, 0x55, 0xD5, 0xEE, 0xFE, 0xFF};
	
	++m_cycleCount;
	if (m_cycleCount >= 8)
		m_cycleCount = 0;
	
	if ( ditherPattern[m_amount] & ( 1 << m_cycleCount ) ) {
		m_jaguar.ConfigNeutralMode(CANJaguar::kNeutralMode_Brake);
//		printf("Brake\n");
	} else {
		m_jaguar.ConfigNeutralMode(CANJaguar::kNeutralMode_Coast);
	}
}

/*
 * Easier to understand way:
 * Each integer, corresponding to value, is a bitfield of 8 cycles
 * Pattern N has N bits out of 8 set to true.
 * 0: 0000 0000 = 0x00
 * 1: 0000 0001 = 0x01
 * 2: 0001 0001 = 0x11
 * 3: 0010 0101 = 0x25
 * 4: 0101 0101 = 0x55
 * 5: 1101 0101 = 0xD5
 * 6: 1110 1110 = 0xEE
 * 7: 1111 1110 = 0xFE
 * 8: 1111 1111 = 0xFF
 * 
 * Old way:
 * 001
 * 010
 * 011
 * 100
 * 101
 * 110
 * 111
 */
