#ifndef _LCD_h
#define _LCD_h

enum {
	LCD_Error,
	LCD_Encoder,
	LCD_Encoder_Corrected,
	LCD_Joy_And_Motors,
	LCD_OI_Pots,
	LCD_Lift,
	LCD_Fork,
	LCD_AutomationState,	// for lift&fork
	LCD_UltraSonic,
	LCD_temp,
	
	LCD_Angle,
	LCD_Gyro,
	LCD_Digital_IO_on_FRC,
	
	LCD_End
};
// LCD dimension
#define LCD_LINE_LEN		20
#define LCD_PHY_LINE		4

// LCD buffer line count
#define LCD_MAX_LINE_CNT	12	//LCD_LINE_LEN * LCD_MAX_LINE_CNT must be <=256 bytes

// auto scroll delay in number of user loop count
#define LCD_SCRL_RPT_CNT	26

// scroll input definitions in Connections.h
#define LCD_SCROLL_DN_INPUT_MASTER 0 // p1_sw_trig
#define LCD_SCROLL_UP_INPUT_MASTER 0 // p1_sw_top

#define LCD_SCROLL_INACTIVE	1		
#define LCD_SCROLL_ACTIVE	0

// scroll related constants
#define LCD_SCROLL_UP		1
#define LCD_SCROLL_DOWN		2

// debug control
#define _DEBUG_LCD			0		

void InitLCD(void);
void ClearLCD(void);
void WriteLCD(char *,unsigned int);
int printfLCD(int, const rom char *, ...);
int displayLCD(void);
void putcharLCD(int row, int col, char c);
void putstrLCD(int row, int col, char *str);
int scrollDownLCD(void);
int scrollUpLCD(void);
int setTopLineLCD(int idxLine);
int readToggleLCD(void);
void updateLCD(void);
void LCD_Blink(char lineNo, char yes);

//Characters on the LCD display
#define THETA "\xF2"
#define DEGREES "\xDF"

#endif  //_LCD_h
