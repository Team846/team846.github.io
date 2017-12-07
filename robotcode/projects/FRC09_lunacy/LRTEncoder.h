#ifndef LRTENCODER_H_
#define LRTENCODER_H_

#include "Encoder.h"
#include "util.h"
#include "LRTConnections.h"

class LRTEncoder: public Encoder {
#if defined(LRT_2008_ROBOT)
	const static float kMaxEncoderRate = 170;
	//high gear, good library (3.0/r1718)
	// 12.6 V motors not running, 11.6 V running full speed on stands
	// approximately the same with 12.8 V/12.0 V under load
	
//	const static float kMaxEncoderRate = 335; //high gear, bad library (3.1)
//	const static float kMaxEncoderRate = 130; //low  gear
#elif defined(LRT_2009_ROBOT)
	const static float kMaxEncoderRate = 130;
//	const static float kMinDutyCycle = 20.0/128; // in order to start moving
#endif

public:
	LRTEncoder(UINT32 aSlot, UINT32 aChannel, UINT32 bSlot, UINT32 _bChannel, bool reverseDirection=false, EncodingType encodingType = k4X)
		: Encoder(aSlot, aChannel, bSlot, _bChannel, reverseDirection, encodingType)
	{
	}

	double GetRate() {
		return LRTUtil::set0ifInf(Encoder::GetRate());
	}
	
	double GetNormalizedRate() {
		return GetRate()/kMaxEncoderRate;
	}
};
#endif /*LRTENCODER_H_*/
