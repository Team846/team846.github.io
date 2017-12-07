#include "LineSensor.h"
#include "..\Config\RobotConfig.h"
#include "..\Util\Util.h"
#include "..\Util\AsynchronousPrinter.h"
#include "..\Util\Profiler.h"
#include <fstream>
#include <iomanip>

LineSensor::LineSensor()
    : adc(RobotConfig::ANALOG::LINE_SENSOR_ADC)
    , clock(RobotConfig::DIGITAL_IO::LINE_SENSOR_CLOCK)
    , si(RobotConfig::DIGITAL_IO::LINE_SENSOR_SI)
    , lastLinePos(0)
    , firstRun(true)
{
    Configure(); // get lineThreshold

    si.Set(0);
    clock.Set(0);

    ResetPixelData(); // set all pixel values to 0
}

LineSensor::~LineSensor()
{
    // nothing
}

void LineSensor::Configure()
{
    lineThreshold = Config::GetInstance().Get<int>("LineSensor" , "lineThreshold", 275);
}

void LineSensor::ResetPixelData()
{
    for(int i = 0; i < NUM_PIXELS; i++)
        pixels[i] = 0;
}

bool LineSensor::Read(int exposure_us)
{
    int clockCycle = 1;

    si.Set(1); // starts sensor reset for next 18 cycles and schedules integrator after
    clock.Set(1); // sensor reset starts here
    si.Set(0);
    clock.Set(0);

    UINT32 expireTime;

    // clock out to the 18th cycle
    while(++clockCycle <= 128)
    {
        clock.Set(1);
        clock.Set(0);

        // camera exposure (integration) starts at the 18th cycle
        if(clockCycle == 18)
            // microseconds
            expireTime = GetFPGATime() + exposure_us;
    }

    // 129th clock cycle triggers tristate and ends cycle
    clock.Set(1);
    clock.Set(0);

    // 129th clock cycle requires a minimum 20 microsecond delay
    Delay(50 * 20);

    // unwanted data is clocked out; now wait for exposure to complete
    while(GetFPGATime() < expireTime)
        ; // wait

    bool isGoodReading = (GetFPGATime() < expireTime + exposure_us / 10);
    clockCycle = 1; // reset to read data

    si.Set(1); // starts sensor reset for next 18 cycles and schedules integrator after
    clock.Set(1); // sensor reset starts here
    si.Set(0);
    clock.Set(0);
    pixels[clockCycle] = adc.GetValue();

    while(++clockCycle <= 128)
    {
        clock.Set(1);
        clock.Set(0);
        pixels[clockCycle] = adc.GetValue();
    }

    // on the 129th clock cycle, the output will go to tristate
    clock.Set(1);
    clock.Set(0);

    return isGoodReading;
}

void LineSensor::ResetFirstRun()
{
    firstRun = true;
    lastLinePos = 0;
}

int LineSensor::GetLinePosition()
{
    unsigned int intensitySum = 0, weightedSum = 0;
    unsigned int pixelValue, maxPixel = 0;

    lineDetected = false;

    {
        // read line sensor
        ProfiledSection pf("Reading line sensor");
        Read(4000);
    }

// cut off 4 pixels that tend to hold high, bogus values 3/12/11 -KV
#define START_PIXEL 4

    for(int i = START_PIXEL; i < NUM_PIXELS; i++)
    {
        pixelValue = Util::Max<int>(pixels[i] - lineThreshold, 0);
        if(pixels[maxPixel] < pixelValue)
            maxPixel = i;

        // check if the line has been detected
        lineDetected = lineDetected || (pixelValue > 0);

        // weighted average based on pixel position
        weightedSum += pixelValue * i;
        intensitySum += pixelValue;
    }


    // log pixel data for debugging
#ifdef USE_DASHBOARD
//    SmartDashboard::Log((int)maxPixel, "Max Line Sensor Pixel Value");
//    SmartDashboard::Log((int)pixels[maxPixel], "Max Line Sensor Value");
    SmartDashboard::Log((int)intensitySum, "Line Sensor Intensity Sum");
#endif

    // 25000 empirically determined to be a cutoff intensity sum
    // for the end of the line in room 612 on 3/12/11 -KV
    if(intensitySum > 25000)
        return END_OF_LINE;

    // 0 pixels is clockwise

    int linePosition = LINE_NOT_DETECTED; // assume no line detected
    if(lineDetected)
        // similar to a center of gravity calculation
        linePosition = weightedSum / intensitySum;

//    static int cycleCount = 0;
//    if(cycleCount++ % 25 == 0)   // update every quarter second
//    {
//        ofstream out("/lineout.csv", ios::app);
//
//        for(int i = START_PIXEL; i < NUM_PIXELS; i += 2)
//        {
//            pixelValue = pixels[i];
//            out << setw(3) << pixelValue << ",";
//        }
//
//        out << endl;
//        out.close();
//    }

#undef START_PIXEL

#ifdef USE_DASHBOARD
    SmartDashboard::Log(linePosition, "Line Position");
#endif

    return linePosition;
}

bool LineSensor::IsLineDetected()
{
    return lineDetected;
}

void LineSensor::DelayMilliseconds(UINT32 ms)
{
    // parameter to taskDelay is in ms
    taskDelay(ms);
}

void LineSensor::Delay(UINT32 ticks)
{
    while(ticks > 0)
        ticks--;
}
