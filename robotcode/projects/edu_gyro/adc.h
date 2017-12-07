/*******************************************************************************
*
*	TITLE		adc.h 
*
*	VERSION:	0.2 (Beta)                           
*
*	DATE:		17-Jul-2005
*
*	AUTHOR:		R. Kevin Watson
*				kevinw@jpl.nasa.gov
*
*	COMMENTS:	You are free to use this source code for any non-commercial
*				use. Please do not make copies of this source code, modified
*				or un-modified, publicly available on the internet or
*				elsewhere without permission. Thanks.
*
*				Copyright ©2005 R. Kevin Watson. All rights are reserved.
*
********************************************************************************
*
*	CHANGE LOG:
*
*	DATE         REV  DESCRIPTION
*	-----------  ---  ----------------------------------------------------------
*	10-Jul-2005  0.1  RKW - Original code.
*	17-Jul-2005  0.2  RKW - Added x128 and x256 oversampling options
*
*******************************************************************************/

#ifndef _adc_h
#define _adc_h

// Number of ADC channels to cycle through. This value must be a value 
// between one and fourteen or sixteen (fifteen is not an option). Make sure
// each of these analog channels is defined as an input in user_routines.c/
// User_Initialization() if you're using the EDU-RC.
#define NUM_ADC_CHANNELS 1


// Pick the slowest ADC sample rate that still meets your performace criteria.
// These #defines are used below to set the value of ADC_SAMPLE_RATE and to
// set the timer 2 update rate in adc.c/Initialize_Timer_2().
// #define ADC_SAMPLE_RATE_200HZ
// #define ADC_SAMPLE_RATE_400HZ
#define ADC_SAMPLE_RATE_800HZ
// #define ADC_SAMPLE_RATE_1600HZ
// #define ADC_SAMPLE_RATE_3200HZ
// #define ADC_SAMPLE_RATE_6400HZ


// Number of ADC samples that will be averaged for each update. More will
// generally give you more resolution and less noise (see chart below), but 
// your update rate will decrease proportionately.
//
// ADC Samples  Effective
//  Averaged     Bits of    Measurement   Voltage
// Per Update   Resolution     Range      Per Bit
// ___________  __________  ___________  _________
//       1          10        0-1023      4.88 mV
//       2          10        0-1023      4.88 mV
//       4          11        0-2047      2.44 mV
//       8          11        0-2047      2.44 mV
//      16          12        0-4095      1.22 mV
//      32          12        0-4095      1.22 mV
//      64          13        0-8191       610 uV
//     128          13        0-8191       610 uV
//     256          14        0-16383      305 uV
//
// #define ADC_SAMPLES_PER_UPDATE_1
// #define ADC_SAMPLES_PER_UPDATE_2
// #define ADC_SAMPLES_PER_UPDATE_4
// #define ADC_SAMPLES_PER_UPDATE_8
#define ADC_SAMPLES_PER_UPDATE_16
// #define ADC_SAMPLES_PER_UPDATE_32
// #define ADC_SAMPLES_PER_UPDATE_64
// #define ADC_SAMPLES_PER_UPDATE_128
// #define ADC_SAMPLES_PER_UPDATE_256

//
// If you modify stuff below this line, you'll break the software.
//
#ifdef ADC_SAMPLES_PER_UPDATE_1
#define ADC_SAMPLES_PER_UPDATE 1
#define ADC_RANGE 1024L
#define ADC_RESULT_DIVISOR 0 - 0 // 10-bit effective resolution
#endif

#ifdef ADC_SAMPLES_PER_UPDATE_2
#define ADC_SAMPLES_PER_UPDATE 2
#define ADC_RANGE 1024L
#define ADC_RESULT_DIVISOR 1 - 0 // 10-bit effective resolution
#endif

#ifdef ADC_SAMPLES_PER_UPDATE_4
#define ADC_SAMPLES_PER_UPDATE 4
#define ADC_RANGE 2048L
#define ADC_RESULT_DIVISOR 2 - 1 // 11-bit effective resolution
#endif

#ifdef ADC_SAMPLES_PER_UPDATE_8
#define ADC_SAMPLES_PER_UPDATE 8
#define ADC_RANGE 2048L
#define ADC_RESULT_DIVISOR 3 - 1 // 11-bit effective resolution
#endif

#ifdef ADC_SAMPLES_PER_UPDATE_16
#define ADC_SAMPLES_PER_UPDATE 16
#define ADC_RANGE 4096L
#define ADC_RESULT_DIVISOR 4 - 2 // 12-bit effective resolution
#endif

#ifdef ADC_SAMPLES_PER_UPDATE_32
#define ADC_SAMPLES_PER_UPDATE 32
#define ADC_RANGE 4096L
#define ADC_RESULT_DIVISOR 5 - 2 // 12-bit effective resolution
#endif

#ifdef ADC_SAMPLES_PER_UPDATE_64
#define ADC_SAMPLES_PER_UPDATE 64
#define ADC_RANGE 8192L
#define ADC_RESULT_DIVISOR 6 - 3 // 13-bit effective resolution
#endif

#ifdef ADC_SAMPLES_PER_UPDATE_128
#define ADC_SAMPLES_PER_UPDATE 128
#define ADC_RANGE 8192L
#define ADC_RESULT_DIVISOR 7 - 3 // 13-bit effective resolution
#endif

#ifdef ADC_SAMPLES_PER_UPDATE_256
#define ADC_SAMPLES_PER_UPDATE 256
#define ADC_RANGE 16384L
#define ADC_RESULT_DIVISOR 8 - 4 // 14-bit effective resolution
#endif

#ifdef ADC_SAMPLE_RATE_200HZ
#define ADC_SAMPLE_RATE 200
#endif

#ifdef ADC_SAMPLE_RATE_400HZ
#define ADC_SAMPLE_RATE 400
#endif

#ifdef ADC_SAMPLE_RATE_800HZ
#define ADC_SAMPLE_RATE 800
#endif

#ifdef ADC_SAMPLE_RATE_1600HZ
#define ADC_SAMPLE_RATE 1600
#endif

#ifdef ADC_SAMPLE_RATE_3200HZ
#define ADC_SAMPLE_RATE 3200
#endif

#ifdef ADC_SAMPLE_RATE_6400HZ
#define ADC_SAMPLE_RATE 6400
#endif

#define VREF_POS_MV 5000L	// ADC Vref+ expressed in millivolts
#define VREF_NEG_MV 0L		// ADC Vref- expressed in millivolts

// number of ADC sample sets generated per second
#define ADC_UPDATE_RATE ADC_SAMPLE_RATE/(ADC_SAMPLES_PER_UPDATE * NUM_ADC_CHANNELS)

// function prototypes
void Initialize_ADC(void);
void Disable_ADC(void);
void Initialize_Timer_2(unsigned int);
void Disable_Timer_2(void);
void Timer_2_Int_Handler(void);
void ADC_Int_Handler(void);
unsigned int Get_ADC_Result(unsigned char);
unsigned int Convert_ADC_to_mV(unsigned int);
unsigned char Get_ADC_Result_Count(void);
void Reset_ADC_Result_Count(void);
	
#endif
