
#ifndef LRT_ENCODER_H_
#define LRT_ENCODER_H_

#include "..\General.h"

/*
 * LRTEncoder patches bugs in WPILib's Encoder class.
 */
class LRTEncoder : public Encoder
{
private:
//  static int count = 0;
//    Encoder useless;
    float trim;

public:
    LRTEncoder(UINT8 sourceA, UINT8 sourceB, float trim = 1.0);
    ~LRTEncoder();

    INT32 Get();
    double GetRate();

};

#endif
