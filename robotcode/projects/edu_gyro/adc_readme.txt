****************************************************************

You are free to use this source code for any non-commercial
use. Please do not make copies of this source code, modified
or un-modified, publicly available on the internet or
elsewhere without permission. Thanks.

Copyright ©2005 R. Kevin Watson. All rights are reserved.

****************************************************************
The source code in adc.c/.h contains software to setup and run
your robot controllers analog-to-digital converter in the 
background. A few advantages of using this software include 
parallel, non-blocking operation of the ADC hardware, precise 
control over the sampling period, which is important for signal 
processing and control applications and the ability to make more 
precise measurements using oversampling. Number of channels, 
sample rate and oversampling parameters can be changed by 
editing adc.h and re-compiling.

This software makes the assumption that when it's running, it 
solely controls the analog to digital conversion hardware.

WARNING: The higher sampling rates can place quite a load on
the robot controllers CPU. If you get the red-light-of-death
or notice any general wackiness in the operation of your
robot, first try a lower sampling rate to determine if this
is the problem. In general, you should select the lowest
sampling rate that meets your performance criteria.

This source code will work with the Robovation (A/K/A EDU-RC) 
robot controller and the FIRST Robotics robot controller.

The included project files were built with MPLAB version 7.20.
If your version of MPLAB complains about the project version, 
the best thing to do is just create a new project with MPLAB's 
project wizard. Include every file except: FRC_alltimers.lib 
and ifi_alltimers.lib and you should be able to build the code.

****************************************************************

Eight things must be done before this software will work 
correctly with your robot controller:

1) A #include statement for the adc.h header file must be 
included at the beginning of each source file that calls the 
functions in adc.c. The statement should look like this: 
#include "adc.h".

2) Initialize_ADC() must be called from user_routines.c/
User_Initialization().

3) On the EDU-RC, all analog inputs must be configured as 
INPUTs in user_routines.c/User_Initialization().

4) The call to Set_Number_of_Analog_Channels() must be removed
or commented out in user_routines.c/User_Initialization().

5) The timer 2 interrupt handler, Timer_2_Int_Handler(), and
ADC interrupt handler, ADC_Int_Handler(), must be installed in
User_Routines_Fast.c/InterruptHandlerLow(). See the included 
copy of User_Routines_Fast.c to see how this is done.

6) Define the number of analog channels that you'd like this
software to track by opening adc.h and following the embedded
instructions above #define NUM_ADC_CHANNELS.

7) For advanced users, analog channels can be oversampled to
decrease noise and gain resolution in your analog measurements.
The oversampling ratio can be changed from the default x16
by commenting-out the line #define ADC_SAMPLES_PER_UPDATE_16
found in adc.h and then removing the // from in front of one
of the other options. Measurement range and resolution can be 
determined from this table:

ADC Samples  Effective
 Averaged     Bits of    Measurement   Voltage
Per Update   Resolution     Range      Per Bit
___________  __________  ___________  _________
      1          10        0-1023      4.88 mV
      2          10        0-1023      4.88 mV
      4          11        0-2047      2.44 mV
      8          11        0-2047      2.44 mV
     16          12        0-4095      1.22 mV
     32          12        0-4095      1.22 mV
     64          13        0-8191       610 uV
    128          13        0-8191       610 uV
    256          14        0-16383      305 uV


8) Finally, pick the master sample rate by selecting one
of the available rates found in adc.h. The update rate can
be determined using this formula:

Update Rate = 
    Sample Rate / (Samples Per Update * Number Of Channels)


****************************************************************


Here's a description of the functions in adc.c:

Get_ADC_Result_Count()

This function returns an unsigned char containing the number 
of ADC updates generated since Reset_ADC_Result_Count() or 
Initialize_ADC() were called. Use this function to determine
when fresh analog to digital conversion data is ready.


Reset_ADC_Result_Count()

This function can be called to reset the ADC update counter
to zero.


Get_ADC_Result()

This function can be called to retreive analog to digital
conversion results. Call this function with an unsigned char
containing the ADC channel you'd like data from. This function
will return an unsigned int with the last conversion result.

Convert_ADC_to_mV()

This function converts the value generated by the ADC to
millivolts.


Initialize_ADC()

This function initializes the ADC hardware and software. It 
should be called from user_routines.c/User_Initialization().
Once called, analog to digital conversions will automatically
take place in the background.


Disable_ADC()

This function disables the ADC hardware and software. To
minimize the load on the microcontroller, this function should 
be called when the ADC functionality is no longer needed.

 
Initialize_Timer_2()

This function initializes and starts timer 2, which initiates
a new analog to digital conversion each time the timer 2
interrupt service routine is called. This is called by
Initialize_ADC() above and shouldn't be called directly.


Disable_Timer_2()


This function is called from Disable_ADC() to disable timer 2 
and shouldn't be called directly.


Timer_2_Int_Handler()

This function is automatically called when timer 2 causes an
interrupt and shouldn't be called directly. New analog to
digital conversions are initiated from this function.


ADC_Int_Handler()

This function is automatically called when new analog to
digital conversion data is ready and shouldn't be called
directly.


Kevin Watson
kevinw@jpl.nasa.gov
