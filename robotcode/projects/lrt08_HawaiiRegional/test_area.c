//#include "common.h"

#ifndef __LARGE__
#error Not Compiled with -ml Large (>64K) data option
#endif

void dontwarn(void) {;}

#ifdef _SIMULATOR   //_SIMULATOR Must be defined to exec testArea()
#include <string.h>
#include <stdio.h>
#include "lcd.h"
#include <limits.h>
#include "Ultrasonic.h"
#include "common.h"
void Lift_TestArea(void);
void testArea()
{
	//printf("Hello world\r");
	//Lift_TestArea();
	void LCD_Print_Gyro_Angle(char lcdLineNo);
	char str[8];
	char str2[32];
	char cnt;
	int i;
	
	sprintf(str, "%d", 100);
	sprintf(str, "%d", 999);
	sprintf(str, "%d", 1000);
	sprintf(str, "%ld", 100L);
	sprintf(str, "%ld", 100000L);
	
	for (i = 999; i < 1023; i++) {
		sprintf(str, "%d", i);
	}
	return;

/*
#define kUltraTicksPerInch ((unsigned int)(2/(12*1127.0 * 400E-9)))	//about 370
	unsigned int ticks;
	unsigned char temp;
	long ttt;
	
	char rolling;
//	if (++rolling == gLoop.count38)
	
	gLoop.f.allFlags=0;
	gLoop.f.allFlags=~0;
	gLoop.f.allFlags=0;
	
return;
	ticks = 47000;
	temp = ticks /kUltraTicksPerInch;
	ttt = ticks * (long)((1L<<24)/kUltraTicksPerInch);
	temp = ((unsigned char *)&ttt)[3];	//much faster than shift 24

	ttt = 0x12345678;
	ttt = mDivideBy256(ttt);
	ttt = 0x12345678;
	ttt = mDivideBy1024(ttt);
	

	LCD_Print_Gyro_Angle(LCD_Gyro);
#define THETA "\xF2"
#define DEGREES "\xDF"
	sprintf(str,"Gryo %c: %02d",(int)242, 3);
	cnt = strlen(str);
	
	cnt = sprintf(str,"%02d", 4);
	str[cnt+1] = NULL;
	str[cnt] = str[cnt-1];
	str[cnt-1] = '.';
//	printfLCD(LCD_Angle,"%s%c",str,(int)223);
//	printfLCD(LCD_Angle,"Gyro " THETA ": "%s" DEGREES,str);
	sprintf(str2,"Gyro " THETA ": %s" DEGREES,str);*/
}


#endif
 //_SIMULATOR Must be defined to exec testArea()