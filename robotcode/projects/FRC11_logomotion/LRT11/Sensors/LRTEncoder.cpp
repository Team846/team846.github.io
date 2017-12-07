#include "LRTEncoder.h"

LRTEncoder::LRTEncoder(UINT8 sourceA, UINT8 sourceB, float trim)
    : Encoder(sourceA, sourceB, false, CounterBase::k1X)
    , trim(trim)
//    , useless(15 + count, , false, CounterBase::k1X)
{

}

LRTEncoder::~LRTEncoder()
{
    // nothing
}

double LRTEncoder::GetRate()
{
    // WPILib's GetRate() returns NaN if the rate is 0
    // Karthik Viswanathan and Brian Axelrod, January 29th, 2011
    double rate = Encoder::GetRate();
    if(rate != rate) // test if rate is NaN because NaN != NaN
        return 0.0;
    return rate;
}

INT32 LRTEncoder::Get()
{
    return (INT32)(Encoder::Get() * trim);
}
