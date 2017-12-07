#include "CANJaguarBrake.h"

// Note: Should this also contain the SpeedController object?
// And force it to 0 output when brakelevel > 0
CANJaguarBrake::CANJaguarBrake(ProxiedCANJaguar& jaggie)
    : jaguar(jaggie), cycleCount(0), amount(0)
{
}

void CANJaguarBrake::SetBrake(int brakeAmount)
{
    amount = Util::Clamp<int>(brakeAmount, 0, 8);
}

// does nothing if setpoint is non-zero
void CANJaguarBrake::ApplyBrakes()
{
    // 1-byte bitfields corresponding to value. See below for calculation.
    static const UINT8 ditherPattern[] = {0x00, 0x01, 0x11, 0x25, 0x55, 0xD5, 0xEE, 0xFE, 0xFF};

    // cycleCount ranges from 0 to 8
    if(++cycleCount >= 8)
        cycleCount = 0;

    // select the cycleCount-th bit from the dither pattern; see below
    bool shouldBrakeThisCycle = ditherPattern[amount] & (1 << cycleCount);

    // ConfigNeutralMode sets whether the jaguar should brake or coast
    jaguar.ConfigNeutralMode(shouldBrakeThisCycle ? LRTCANJaguar::kNeutralMode_Brake : LRTCANJaguar::kNeutralMode_Coast);
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
