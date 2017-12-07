/*******************************************************************************
*
*	TITLE		adc.c 
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

#include "adc.h"
#include "ifi_aliases.h"
#include "ifi_default.h"


unsigned int adc_sample_rate;
unsigned int adc_samples_per_update;
unsigned char num_adc_channels;
unsigned char adc_result_divisor;

volatile unsigned long accum[NUM_ADC_CHANNELS]; // sample accumulator
volatile unsigned int adc_result[NUM_ADC_CHANNELS]; // ADC recults
volatile unsigned int samples; // current number of samples accumulated
volatile unsigned char channel; // current ADC channel
volatile unsigned char adc_update_count = 0; // ADC update flag


/*******************************************************************************
*
*	FUNCTION:		Initialize_ADC()
*
*	PURPOSE:		Initializes the Analog to Digital Conversion (ADC) hardware.
*
*	CALLED FROM:	user_routines.c/User_Initialization()
*
*	PARAMETERS:		None
*
*	RETURNS:		Nothing
*
*	COMMENTS:		Place "#include "adc.h" in the includes section
*					of user_routines.c then call Initialize_ADC() in
*					user_routines.c/User_Initialization().
*
*******************************************************************************/
void Initialize_ADC(void)  
{
	unsigned char i;

	// Initialize ADC related global variables using default values from adc.h.
	// Another way to do this would be to pass the values in, read them from
	// values stored in EEPROM or perhaps set them via a menu-based terminal
	// interface.
	adc_sample_rate = ADC_SAMPLE_RATE;
	num_adc_channels = NUM_ADC_CHANNELS;
	adc_result_divisor = ADC_RESULT_DIVISOR;
	adc_samples_per_update = ADC_SAMPLES_PER_UPDATE;

	// reset the sample accumulator(s) to zero
	for(i=0; i < num_adc_channels; i++)
	{
		accum[i] = 0L;
	}

	// start a new sample set
	samples = 0;

	// start at ADC channel zero
	channel = 0;

	// enable the ADC hardware and select ADC channel 0
	ADCON0 = 0b00000001;

	#ifdef _FRC_BOARD
	// If this is being built for the FRC-RC, enable all sixteen analog 
	// inputs. This shouldn't be changed because the FRC-RC has sixteen 
	// dedicated analog inputs. The VCFG0 and VCFG1 bits are also set to 
	// select Vdd and Vss as the reference voltages
	ADCON1 = 0;

	#else
	// If this is being built for the EDU-RC, enable NUM_ADC_CHANNELS 
	// analog inputs (NUM_ADC_CHANNELS is documented in adc.h). This must 
	// match the default value used in user_routines.c/User_Initialization()/
	// Set_Number_of_Analog_Channels(). The VCFG0 and VCFG1 bits are also 
	// set to select Vdd and Vss as the reference voltages

	if(num_adc_channels >= 1 && num_adc_channels <= 14)
	{
		ADCON1 = 15 - num_adc_channels;
	}
 	else
	{
		ADCON1 = 0; // sixteen channnels or out of range
	}

	#endif

	// select Fosc/64 as the ADC Conversion Clock and right justify data
	// in the conversion result register
	ADCON2 = 0b10000110;

	// ADC interrupt is low priority
	IPR1bits.ADIP = 0;

	// to prevent a spurious interrupt, make sure the interrupt flag is reset
	PIR1bits.ADIF = 0;

	// enable the ADC interrupt
	PIE1bits.ADIE = 1;

	// initialize and start timer 2, which is responsible for starting
	// analog to digital conversions
	Initialize_Timer_2(adc_sample_rate);
}

/*******************************************************************************
*
*	FUNCTION:		Disable_ADC()
*
*	PURPOSE:		Disables the Analog to Digital Conversion (ADC) hardware.
*
*	CALLED FROM:
*
*	PARAMETERS:		None
*
*	RETURNS:		Nothing
*
*	COMMENTS:
*
*******************************************************************************/
void Disable_ADC(void)  
{
	unsigned char i;

	// disable timer 2
	Disable_Timer_2();

	// disable the ADC interrupt
	PIE1bits.ADIE = 0;

	// disable the ADC hardware and select ADC channel 0
	ADCON0 = 0b00000000;
}

/*******************************************************************************
*
*	FUNCTION:		Initialize_Timer_2()
*
*	PURPOSE:		Initializes the timer 2 hardware, which is responsible for
*					periodically starting analog to digital conversions.
*
*	CALLED FROM:	adc.c/Initialize_ADC()
*
*	PARAMETERS:		Unsigned integer containing the sample rate expressed in Hz
*
*	RETURNS:		Nothing
*
*	COMMENTS:		The only supported sample rates are 200Hz, 400Hz, 800Hz,
*					1600Hz, 3200Hz and 6400Hz.
*
*******************************************************************************/
void Initialize_Timer_2(unsigned int adc_sample_rate)  
{
	switch(adc_sample_rate)
	{
		case 200:
			// use these parameters for a 200Hz ADC sample rate

			// use a 1:16 prescaler and 1:14 postscaler
			T2CON = 0b01101010;

			// Count to 221 before rolling over and generating 
			// an interrupt (223.21 - 2 is ideal)
			PR2	= 221;
			break;

		case 400:
			// use these parameters for a 400Hz ADC sample rate

			// use a 1:16 prescaler and 1:11 postscaler
			T2CON = 0b01010010;

			// Count to 140 before rolling over and generating 
			// an interrupt (142.05 - 2 is ideal)
			PR2	= 140; 
			break;

		case 800:
			// use these parameters for a 800Hz ADC sample rate

			// use a 1:16 prescaler and 1:11 postscaler
			T2CON = 0b01010010;

			// Count to 69 before rolling over and generating 
			// an interrupt (71.02 - 2 is ideal)
			PR2	= 69; 
			break;

		case 1600:
			// use these parameters for a 1600Hz ADC sample rate

			// use a 1:4 prescaler and 1:11 postscaler
			T2CON = 0b01010001;

			// Count to 140 before rolling over and generating 
			// an interrupt (142.05 - 2 is ideal)
			PR2	= 140; 
			break;

		case 3200:
			// use these parameters for a 3200Hz ADC sample rate

			// use a 1:1 prescaler and 1:11 postscaler
			T2CON = 0b01010001;

			// Count to 69 before rolling over and generating 
			// an interrupt (71.02 - 2 is ideal)
			PR2	= 69;
			break;
	
		case 6400:
			// use these parameters for a 6400Hz ADC sample rate

			// use a 1:4 prescaler and 1:11 postscaler
			T2CON = 0b01010000;

			// Count to 140 before rolling over and generating 
			// an interrupt (142.05 - 2 is ideal)
			PR2	= 140; 
			break;

		default:
			// if a non-supported rate is specified, 
			// default to a 200Hz ADC sample rate

			// use a 1:16 prescaler and 1:14 postscaler
			T2CON = 0b01101010;

			// Count to 221 before rolling over and generating 
			// an interrupt (223.21 - 2 is ideal)
			PR2	= 221; 
			break;
	}

	// make sure the timer 2 register starts at zero
	TMR2 = 0x00;

	// timer 2 interrupt is low priority
	IPR1bits.TMR2IP = 0;

	// to prevent a spurious interrupt, make sure the interrupt flag is reset
	PIR1bits.TMR2IF = 0;

	// enable the timer 2 interrupt
	PIE1bits.TMR2IE = 1;

	// enable timer 2
	T2CONbits.TMR2ON = 1;
}

/*******************************************************************************
*
*	FUNCTION:		Disable_Timer_2()
*
*	PURPOSE:		Disables the timer 2 hardware.
*
*	CALLED FROM:
*
*	PARAMETERS:		None
*
*	RETURNS:		Nothing
*
*	COMMENTS:
*
*******************************************************************************/
void Disable_Timer_2(void)
{
	// disable the timer 2 interrupt
	PIE1bits.TMR2IE = 0;

	// disable timer 2
	T2CONbits.TMR2ON = 0;
}

/*******************************************************************************
*
*	FUNCTION:		Get_ADC_Result()
*
*	PURPOSE:		Given the ADC channel number, returns the last ADC result
*					expressed in "data number" units.
*
*	CALLED FROM:
*
*	PARAMETERS:		ADC channel number
*
*	RETURNS:		Nothing
*
*	COMMENTS:
*
*******************************************************************************/
unsigned int Get_ADC_Result(unsigned char channel)
{
	unsigned int temp_adc_result;

	if(channel <= num_adc_channels)
	{
		// disable the ADC interrupt
		PIE1bits.ADIE = 0;

		// one is subtracted because analog input 1 maps to adc_result[0], 
		// input 2 maps to adc_result[1],..., input 16 maps to adc_result[15]
		temp_adc_result = adc_result[channel - 1];

		// enable the ADC interrupt
		PIE1bits.ADIE = 1;
	}
	else
	{
		// bad channel number; return zero
		temp_adc_result = 0;
	}

	return(temp_adc_result);
}

/*******************************************************************************
*
*	FUNCTION:		Convert_ADC_to_mV()
*
*	PURPOSE:		Converts the raw output of the ADC to millivolts.
*
*	CALLED FROM:
*
*	PARAMETERS:		ADC output value to convert
*
*	RETURNS:		Millivolts
*
*	COMMENTS:
*
*******************************************************************************/
unsigned int Convert_ADC_to_mV(unsigned int adc)
{
	return((unsigned int)(((long)adc * (VREF_POS_MV - VREF_NEG_MV)) / ADC_RANGE));
}

/*******************************************************************************
*
*	FUNCTION:		Get_ADC_Result_Count()
*
*	PURPOSE:
*
*	CALLED FROM:
*
*	PARAMETERS:		None
*
*	RETURNS:		Nothing
*
*	COMMENTS:
*
*******************************************************************************/
unsigned char Get_ADC_Result_Count()
{
	unsigned char temp_adc_update_count;

	// disable the ADC interrupt
	PIE1bits.ADIE = 0;

	temp_adc_update_count = adc_update_count;

	// enable the ADC interrupt
	PIE1bits.ADIE = 1;

	return(temp_adc_update_count);
}

/*******************************************************************************
*
*	FUNCTION:		Reset_ADC_Result_Count()
*
*	PURPOSE:		Resets the ADC update counter to zero
*
*	CALLED FROM:
*
*	PARAMETERS:		None
*
*	RETURNS:		Nothing
*
*	COMMENTS:
*
*******************************************************************************/
void Reset_ADC_Result_Count()
{
	// disable the ADC interrupt
	PIE1bits.ADIE = 0;

	adc_update_count = 0;

	// enable the ADC interrupt
	PIE1bits.ADIE = 1;
}

/*******************************************************************************
*
*	FUNCTION:		Timer_2_Int_Handler()
*
*	PURPOSE:		Timer 2 interrupt service routine
*
*	CALLED FROM:	user_routines_fast.c/InterruptHandlerLow()
*
*	PARAMETERS:		None
*
*	RETURNS:		Nothing
*
*	COMMENTS:
*
*******************************************************************************/
void Timer_2_Int_Handler(void)
{
	// start a new analog to digital conversion
	ADCON0bits.GO = 1;
}

/*******************************************************************************
*
*	FUNCTION:		ADC_Int_Handler()
*
*	PURPOSE:		ADC interrupt service routine
*
*	CALLED FROM:	user_routines_fast.c/InterruptHandlerLow()
*
*	PARAMETERS:		None
*
*	RETURNS:		Nothing
*
*	COMMENTS:
*
*******************************************************************************/
void ADC_Int_Handler(void)
{
	unsigned int adc;
	unsigned char adcon0_temp;
	int i;

	// get conversion results
	adc = ADRESH;
	adc <<= 8;
	adc += ADRESL;

	// add the ADC data to the appropriate accumulator
	accum[channel] += (long)adc;
	
	// increment the ADC channel index
	channel++;

	// do we need to wrap around to zero?
	if(channel >= num_adc_channels)
	{
		channel = 0;
		samples++;
	}

	// Select next ADC channel. This also starts the process whereby the ADC 
	// sample and hold capacitor is allowed to start charging, which must be
	// completed before the next analog to digital conversion can be started.
	adcon0_temp = channel;
	adcon0_temp <<= 2;
	adcon0_temp |= 0b00000001;
	ADCON0 = adcon0_temp;

	// check to see if we've got a full sample set
	if(samples >= adc_samples_per_update)
	{
		// update the ADC result array
		for(i=0; i < num_adc_channels; i++)
		{
			adc_result[i] = (long)(accum[i] >> adc_result_divisor);
		}
		// reset the sample accumulator(s) to zero
		for(i=0; i < num_adc_channels; i++)
		{
			accum[i] = 0L;
		}

		// signal that a fresh sample set is available
		adc_update_count++;

		// start a fresh sample set
		samples = 0;
	}	
}
