#include "common.h"

encoder gEncoders[2];

long encoder_sum(void)
{
	return gEncoders[LEFT].position + gEncoders[RIGHT].position;
}
//seems trivial, but defines direction
long encoder_diff_absolute(void)
{
	return gEncoders[LEFT].position - gEncoders[RIGHT].position;
}

// Measure by reading the encoder difference after pushing robot
// in a straight line. e.g. bearing = -24, dist = 1106+1130, so
// ENC_TRIM = -24, ENC_TRIM_DIST = 1106+1130
#define ENC_TRIM		(-24L)
#define ENC_TRIM_DIST	(1106L + 1130L)
long encoder_diff_corrected(void)
{
	long trim = encoder_sum() * ENC_TRIM / ENC_TRIM_DIST;
	return encoder_diff_absolute() - trim;
}

//************************************************
#define kThreshhold 30
void checkEncodersWorking(void)
{
	char sign;
	int speed;

	if (disabled_mode)	//Don't have a way to detect an error when disabled.
		return;
	
	sign = gMotorSpeed.left < 0; //Match FORWARD & REVERSE definitions
	speed = sign ? -gMotorSpeed.left : gMotorSpeed.left;
	if (speed > kThreshhold)
		if (!gEncoders[LEFT].newdata || sign != gEncoders[LEFT].direction)
			if (gEncoders[LEFT].cumulativeDirectionErrors<255)//don't roll over
				gEncoders[LEFT].cumulativeDirectionErrors++;

	sign = gMotorSpeed.right < 0; //Match FORWARD & REVERSE definitions
	speed = sign ? -gMotorSpeed.right : gMotorSpeed.right;
	if (speed > kThreshhold)
		if (!gEncoders[RIGHT].newdata || sign != gEncoders[RIGHT].direction)
			if (gEncoders[RIGHT].cumulativeDirectionErrors<255)//don't roll over
				gEncoders[RIGHT].cumulativeDirectionErrors++;

	//Clear errors on disabled->!disabled transition
	if (gLoop.enabledCount==1 || gLoop.autonomousCount==1)//transition from disabled
	{
		gEncoders[LEFT].cumulativeDirectionErrors=0;
		gEncoders[RIGHT].cumulativeDirectionErrors=0;
	}
	//else let error count decay. (Abrupt decelleration will generate false errors.)
	else if ((gLoop.count & 0x3) == 0) //every eigth cycle.
	{
		if (0<gEncoders[LEFT].cumulativeDirectionErrors)
			gEncoders[LEFT].cumulativeDirectionErrors--;
		if (0<gEncoders[RIGHT].cumulativeDirectionErrors)
			gEncoders[RIGHT].cumulativeDirectionErrors--;
	}
}
//********************************************************************************
void LCD_DisplayAnyEncoderError(void)
{
	char n=0;	// No. of char's sent to LCD, if any.
	if (gEncoders[RIGHT].cumulativeDirectionErrors)
		n=printfLCD(LCD_Error, "Enc. Err Right %d", gEncoders[RIGHT].cumulativeDirectionErrors);
	if (gEncoders[LEFT].cumulativeDirectionErrors)
		n=printfLCD(LCD_Error, "Enc. Err Left %d", gEncoders[LEFT].cumulativeDirectionErrors);
	if (n)	//An error got sent to the LCD
		LCD_Blink(LCD_Error, 1);
}