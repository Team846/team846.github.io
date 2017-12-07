#include "common.h"
#include <delays.h>

#ifdef SERIAL_LED

/* Need to provide clock, data ready, and data signal
 * Using chip STMIcroelectronics stp08cdc596
 * http://www.st.com/stonline/products/literature/ds/11420/stp08cdc596.pdf
 * D.Giandomenico 
 */

/* uses outputs
	mLEDClock rc_dig_in10
	mLEDDataOut rc_dig_in11
	mLEDDataReady rc_dig_in12
*/


void SendLEDSerial(unsigned int ledData)
{
	char i;
	for (i=8; i>0; i--)	//go forward on first byte
	{
		Delay10TCYx(1);
		mLEDClock = 0;
		mLEDDataOut = ledData & 0x1 ? 1 : 0;
		ledData >>= 1;	//next bit of data
		Delay10TCYx(1);
		mLEDClock = 1;
	}
	for (i=8; i>0; i--)	//go backward on second byte (board is wired that way -dg)
	{
		Delay10TCYx(1);
		mLEDClock = 0;
		mLEDDataOut = ledData & 0x80 ? 1 : 0;
		ledData <<= 1;	//next bit of data
		Delay10TCYx(1);
		mLEDClock = 1;
	}
	mLEDDataReady=1;	//start latch data pulse
	Delay10TCYx(1);	//need this delay on the DataReady pulse
	mLEDClock = mLEDDataOut = 0;	//probably not needed; just leave lines in predictable state.
	mLEDDataReady=0;	//end latch data pulse
}

#endif // SERIAL_LED
