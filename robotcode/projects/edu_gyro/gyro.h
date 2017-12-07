/*******************************************************************************
*
*	TITLE		gyro.h 
*
*	VERSION:	0.5 (Beta)                           
*
*	DATE:		04-Sep-2005
*
*	AUTHOR:		R. Kevin Watson
*				kevinw@jpl.nasa.gov
*
*	COMMENTS:
*
********************************************************************************
*
*	CHANGE LOG:
*
*	DATE         REV  DESCRIPTION
*	-----------  ---  ----------------------------------------------------------
*	21-Nov-2004  0.1  RKW - Original code.
*	12-Jan-2005  0.2  RKW - Altered Get_Gyro_Rate() and Get_Gyro_Angle() to use
*	                  long integers for internal calculations, allowing larger
*	                  numerators and denominators in the GYRO_RATE_SCALE_FACTOR
*	                  and GYRO_ANGLE_SCALE_FACTOR #defines.
*	12-Jan-2005  0.2  RKW - GYRO_RATE_SCALE_FACTOR and GYRO_ANGLE_SCALE_FACTOR
*	                  #defines added for Analog Devices' ADXRS401, ADXRS150 and
*	                  ADXRS300 gyros.
*	16-Jan-2005  0.3  RKW - Using preprocessor directives, added the ability 
*	                  to select the gyro type, angular units, sample rate and
*	                  number of averaged samples per update.
*	21-Jan-2005  0.3  RKW - Added scaling factors for the BEI GyroChip.
*	30-Jan-2005  0.4  RKW - Revised the way bias calculations are done.
*	                  Instead of using only one data set as a bias, multiple
*	                  sample sets can now be averaged over a much longer period
*	                  of time to derive the gyro bias. Updated documentation.
*	04-Sep-2005  0.5  RKW - Significant overhaul of gyro code to strip-out ADC-
*	                  specific code and use new adc.c/.h interface. Added
*	                  deadband option.
*
*******************************************************************************/

#ifndef _gyro_h
#define _gyro_h

// Analog channel on the robot controller that's hooked-up to your gyro
#define GYRO_CHANNEL 1


// Pick the angular unit by removing the // from one of these two lines.
#define MILLIRADIANS
// #define TENTHS_OF_A_DEGREE


// Measured angular rates will only be used if their absolute value is 
// greater than this value. This will help filter out measurement noise
// at the expense of measuring small angular rates. To use this feature,
// start with a value of eight, re-compile and test. If your angular
// drift is now zero, iterate the value down to the lowest value that
// still gives you zero drift over a one or two minute period. If your
// angular drift didn't go to zero with a value of eight, iterate this
// this value up until the angular drift rate is zero for a minute or
// two.
#define GYRO_DEADBAND 8


// Pick your gyro by removing the // from one of the six lines below.
// #define GYROCHIP_64	// BEI GyroChip AQRS-00064-xxx
// #define GYROCHIP_75	// BEI GyroChip AQRS-00075-xxx
// #define CRS03		// Silicon Sensing Systems' CRS03-02
// #define ADXRS401	 	// Analog Devices' ADXRS401
#define ADXRS150		// Analog Devices' ADXRS150
// #define ADXRS300		// Analog Devices' ADXRS300


// For optimum performance, you'll need to calibrate the scaling factor 
// to match that of your gyro's. One way to calibrate your gyro is to 
// mount it very securely to a hefty, square or rectangular object. 
// Mounting the gyro to a hefty object will help dampen higher frequency 
// vibrations that can adversly effect your measurements. Place the 
// mounted gyro against another square object and start the included 
// demonstration software. To get good results, the mount must be 
// absolutely still when the "Calibrating Gyro Bias..." message appears 
// on the terminal screen. After a few seconds, the gyro angular rate 
// and angle will be sent to the terminal screen. If the angle drifts 
// rapidly while the mounted gyro is motonless, you need to restart the 
// software to acquire a new gyro bias measurement. Again, gyros are 
// very sensitive and must be still while the bias is calculated. Once 
// the gyro is running with little drift, rotate the mount 180 degrees 
// and note the reported angle. If the angular units are set to tenths 
// of a degree, the ideal reported angle is 1800. If set to milliradians, 
// the ideal angle 1s 3142 (Pi times a thousand). For every tenth of a 
// percent that the angle is high, decrease the GYRO_CAL_FACTOR numerator 
// by one. Conversly, for every tenth of a percent low, increase the 
// numerator by one. Repeat until you're satisfied with the accuracy.
#define GYRO_CAL_FACTOR 1000/1000


//
// If you modify stuff below this line, you'll break the software.
//

// These constants convert the gyro output voltage, expressed in volts,
// to an angular rate of change in either tenths of a degree per second
// or milliradians per second.

// BEI GyroChip AQRS-00064-xxx (sensitivity = 35.0mV/deg/sec)
#ifdef GYROCHIP_64
#define GYRO_SENSITIVITY_DEG 286L	// in units of tenths of a degree/sec/volt
#define GYRO_SENSITIVITY_RAD 499L	// in units of milliradians/sec/volt
#endif

// BEI GyroChip AQRS-00075-xxx (sensitivity = 30.0mV/deg/sec)
#ifdef GYROCHIP_75
#define GYRO_SENSITIVITY_DEG 333L	// in units of tenths of a degree/sec/volt
#define GYRO_SENSITIVITY_RAD 581L	// in units of milliradians/sec/volt
#endif

// Silicon Sensing Systems' CRS03 (sensitivity = 20mV/deg/sec)
#ifdef CRS03
#define GYRO_SENSITIVITY_DEG 500L	// in units of tenths of a degree/sec/volt
#define GYRO_SENSITIVITY_RAD 873L	// in units of milliradians/sec/volt
#endif

// Analog Devices' ADXRS401 (sensitivity = 15mV/deg/sec)
#ifdef ADXRS401
#define GYRO_SENSITIVITY_DEG 667L	// in units of tenths of a degree/sec/volt
#define GYRO_SENSITIVITY_RAD 1163L	// in units of milliradians/sec/volt
#endif

// Analog Devices' ADXRS150 (sensitivity = 12.5mV/deg/sec)
#ifdef ADXRS150
#define GYRO_SENSITIVITY_DEG 800L	// in units of tenths of a degree/sec/volt
#define GYRO_SENSITIVITY_RAD 1396L	// in units of milliradians/sec/volt
#endif

// Analog Devices' ADXRS300 (sensitivity = 5.0mV/deg/sec) 
#ifdef ADXRS300
#define GYRO_SENSITIVITY_DEG 2000L	// in units of tenths of a degree/sec/volt
#define GYRO_SENSITIVITY_RAD 3490L	// in units of milliradians/sec/volt
#endif

// Select the appropriate angular unit

#ifdef MILLIRADIANS
#define GYRO_SENSITIVITY GYRO_SENSITIVITY_RAD
#endif

#ifdef TENTHS_OF_A_DEGREE
#define GYRO_SENSITIVITY GYRO_SENSITIVITY_DEG
#endif

// Function prototypes

void Initialize_Gyro(void);			// initializes and starts the gyro software
int Get_Gyro_Rate(void);			// returns the current heading angular rate of change
long Get_Gyro_Angle(void);			// returns the current heading angle
void Start_Gyro_Bias_Calc(void);	// starts an ongoing gyro bias calculation
void Stop_Gyro_Bias_Calc(void);		// stops an ongoing gyro bias calculation
int Get_Gyro_Bias(void);			// returns the current calculated gyro bias
void Set_Gyro_Bias(int);			// manually sets the gyro bias
void Reset_Gyro_Angle(void);		// resets the heading angle to zero
void Process_Gyro_Data(void);		// processes gyro data when the ADC completes a measurement
	
#endif
