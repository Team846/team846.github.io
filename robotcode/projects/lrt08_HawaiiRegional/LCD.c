#include "common.h"
#include "LCD.h"
#include "_serial_ports.h"
#include <stdio.h>
#include <string.h>
#include "delays.h"

struct {
	unsigned blinkOn:1; //flag used to mask-blink any line.
	unsigned char blinkFlag[(LCD_MAX_LINE_CNT+7)/8]; //which line to blink
	int curLineIdx;
	int topLineIdx;
	int scrollWrap;
}s = {0};

char LCDScrnBuf[LCD_MAX_LINE_CNT][LCD_LINE_LEN];


#define mSetBit(array,n)    (array)[(n)>>3] |= 1<<((n)&7)
#define mClearBit(array,n) (array)[(n)>>3] &= ~(1<<((n)&7))
#define mIsSetBit(array,n)  (0!= ((array)[(n)>>3] & (1<<((n)&7))))

static void LCD_WriteStrn(char *s, char length);
static char blinkShow(char lineNo);
void SetLCDBaud(void);
/* --------------------------------------------------

 Usage:

	Base LCD handler in user_routine.c

	. put updateLCD()
		recommended in any place that is called frequently

	How to print from application
	
	. printfLCD(linenum, "format", args) same syntax with printf
		but the first argument, which decides the line that the
		output is displayed at.

---------------------------------------------------- */

void InitLCD(void)
{
	//Sequence to tell LCD controller that LCD is 20x4
	//See Spark Fun Manual
	//Needs to be done once; causes trouble in 'real time'
	//Write_Serial_Port_Two(124);
	//Write_Serial_Port_Two(3);
	//Write_Serial_Port_Two(124);
	//Write_Serial_Port_Two(5);

//	SetLCDBaud();

	//set baud rate - exec once, then comment out.  Set rate in serial_port to match.
	//Write_Serial_Port_Two(124);
	//Write_Serial_Port_Two(16); //16d = ^p (38400Baud)
	//Write_Serial_Port_Two(18); //18d = ^r (9600Baud during splash screen)


	ClearLCD();
}
//*************************************************************
void SetLCDBaud(void)
{
	RCSTA2bits.SPEN = 0;  	// Serial Port Enable [249]
	SPBRG2 = BAUD_9600;		// baud rate generator register [251]
	RCSTA2bits.SPEN = 1;  	// Serial Port Enable [249]
//	Delay10KTCYx(10); // 1=1ms
	
//	Write_Serial_Port_Two(18); //18d = ^r (9600Baud during splash screen)
	Write_Serial_Port_Two(124);
	Write_Serial_Port_Two(16); //16d = ^p (38400Baud)

	Delay10KTCYx(10); // 1=1ms
	
	RCSTA2bits.SPEN = 0;  	// Serial Port Enable [249]
	SPBRG2 = BAUD_38400;		// baud rate generator register [251]
	RCSTA2bits.SPEN = 1;  	// Serial Port Enable [249]
//	Delay10KTCYx(10); // 1=1ms

}

void ClearLCD(void)
{
	Write_Serial_Port_Two(254);
	Write_Serial_Port_Two(1);

	// for debug need to delete later
	Write_Serial_Port_Two(254);
	Write_Serial_Port_Two(128);
	
}


void WriteLCD(char *str,unsigned int nc)
{
	int i;
	for(i=0;i<nc;++i)
		Write_Serial_Port_Two(str[i]);

}


/************************************************************
 ScrollUpdateLCD: handles scroll input and updates LCD display
	This needs to be called in user routine loop

 Argument
	none
 Return Val:
	none	

*************************************************************/
void ScrollUpdateLCD(void)
{
	int scroll;

	// LCD display code. needs to be in main loop
	scroll = readToggleLCD();
	if ( scroll == LCD_SCROLL_UP)
	{
		scrollUpLCD();
		displayLCD();
	}
	else if (scroll == LCD_SCROLL_DOWN)
	{
		scrollDownLCD();
		displayLCD();
	}
}

/************************************************************
 printfLCD: prints string on the LCD screen

 Argument
	lineIdx: 0 based index of line that the string will be printed
	buf: string buffer, which length should be at least LCD_LINE_LEN
	fmt: printf equivalent format specifier

 Return Val:
	if ret >= 0, number of characters printed
	if ret < 0, error
	
*************************************************************/


int printfLCD(int lineIdx, const rom char *f, ...)
{
  int n;
  va_list ap;
  char tempbuf[32], *buf = tempbuf;
	
//	return 0;

  if (setTopLineLCD(lineIdx) < 0)	// something wrong with line index
	return -1;

  va_start (ap, f);
  n = vfprintf ((FILE *) & buf, f, ap);
  va_end (ap);
  *buf = '\0';

  strncpy(&LCDScrnBuf[lineIdx][0], tempbuf, LCD_LINE_LEN);
#if _DEBUG_LCD
  printf("[LCD_DBG] Ln(%d) ChCnt(%d)\n", lineIdx, n);
#endif
  return n;
}

/************************************************************
 displayLCD: display buffer onto LCD screen

 Argument

 Return Val:
	if ret >= 0, number of characters printed
	if ret < 0, error
	
*************************************************************/

int displayLCD(void)
{
	const char svdStdoutSerialPort = stdout_serial_port;
	
//	return 0;
	
	stdout_serial_port = SERIAL_PORT_TWO;

	s.blinkOn = !s.blinkOn;	//flag used to mask-blink any line.
#if _DEBUG_LCD
	int i;
	
	for (i=0; i < 10; i++)
	{
		printf("[%d] {%s}\r\n", i, LCDScrnBuf[i]);
	}
	printf("Idx: [%d]\r\n", s.topLineIdx);
#endif


  ClearLCD();
 
 // s.curLineIdx = 0;		// for debug
  
  // printf first line
	if (blinkShow(s.curLineIdx))
		LCD_WriteStrn(LCDScrnBuf[s.curLineIdx], LCD_LINE_LEN);
  //printf("%s",LCDScrnBuf[s.curLineIdx]);

  // move to line 2
	if (blinkShow(s.curLineIdx+1))
	{
		Write_Serial_Port_Two(254);
		Write_Serial_Port_Two(128L+64);
		LCD_WriteStrn(LCDScrnBuf[s.curLineIdx+1], LCD_LINE_LEN);
	}
//  if (s.curLineIdx +1 <= s.topLineIdx)
//	printf("%s",LCDScrnBuf[s.curLineIdx+1]);
//  else if (s.scrollWrap)
//	LCD_WriteStrn(LCDScrnBuf[s.curLineIdx], LCD_LINE_LEN);
//	//printf("%s",LCDScrnBuf[0]);

  if (LCD_PHY_LINE > 2)	//then we assume 4 line display
	{
	// third line
	if (blinkShow(s.curLineIdx+2))
	{
		Write_Serial_Port_Two(254);
		Write_Serial_Port_Two(128L+20);
		LCD_WriteStrn(LCDScrnBuf[s.curLineIdx+2], LCD_LINE_LEN);
	}

//  	if (s.curLineIdx +2 <= s.topLineIdx)
//		printf("%s",LCDScrnBuf[s.curLineIdx+2]);
//  	else if (s.scrollWrap)
//    	printf("%s",LCDScrnBuf[s.curLineIdx+2-(s.topLineIdx+1)]);

	// fourth line
	if (blinkShow(s.curLineIdx+3))
	{
		Write_Serial_Port_Two(254);
		Write_Serial_Port_Two(128L+84);
		LCD_WriteStrn(LCDScrnBuf[s.curLineIdx+3], LCD_LINE_LEN);
	}
 
//	if (s.curLineIdx +3 <= s.topLineIdx)
//		printf("%s",LCDScrnBuf[s.curLineIdx+3]);
//  	else if (s.scrollWrap)
//    	printf("%s",LCDScrnBuf[s.curLineIdx+3-(s.topLineIdx+1)]);
	}

	stdout_serial_port = svdStdoutSerialPort;
	return 0;
}

/************************************************************
 scrollDownLCD: Scroll down LCD screen by 1 line

 Argument

 Return Val:
	
*************************************************************/
/* we are going to scroll down, and higher line number will show */
int scrollDownLCD()
{
  s.curLineIdx++;
  
  if (s.curLineIdx > s.topLineIdx)	// we reached to max
	if (s.scrollWrap)
	  s.curLineIdx = 0;	// wrap
	else
	  s.curLineIdx--;		// undo increment. We don't want to scroll down

  return 0;
}

/************************************************************
 scrollUpLCD: Scroll up LCD screen by 1 line

 Argument

 Return Val:
	
*************************************************************/
int scrollUpLCD()
{
  s.curLineIdx--;
  
  if (s.curLineIdx < 0)	// we are already beginning
	if (s.scrollWrap)
	  s.curLineIdx = s.topLineIdx;	// wrap
	else
	  s.curLineIdx++;		// undo decrement. We don't want to scroll up

  return 0;
}

/************************************************************ 

 setTopLineLCD: set the beginning line that will be displayed on LCD

 Argument:

 Return Val:

*************************************************************/

int setTopLineLCD(int lineIdx)
{
	if (lineIdx + 1 >= LCD_MAX_LINE_CNT)
		return -1;
	
	if (lineIdx > s.topLineIdx)
		s.topLineIdx = lineIdx;

	return LCD_MAX_LINE_CNT-1; //s.topLineIdx;
}

/************************************************************
 getScrollUpInput: detects active input from robot controller or master

 Argument

 Return Val:
	
*************************************************************/
int getScrollUpInput(void)
{
	int ret = LCD_SCROLL_INACTIVE;

#if _DEBUG_LCD
	printf("master up [%d]\r\n", (int) LCD_SCROLL_UP_INPUT_MASTER );
#endif

	if ((int) LCD_SCROLL_UP_INPUT_MASTER == 1 /*LCD_SCROLL_ACTIVE*/)
		ret = LCD_SCROLL_ACTIVE;
	if ((int) LCD_SCROLL_UP_INPUT == LCD_SCROLL_ACTIVE)
		ret = LCD_SCROLL_ACTIVE;

	return ret;
}

/************************************************************
 getScrollDownInput: detects active input from robot controller or master

 Argument

 Return Val:
	
*************************************************************/

int getScrollDownInput(void)
{
	int ret = LCD_SCROLL_INACTIVE;

#if _DEBUG_LCD
	printf("master down [%d]\r\n", (int) LCD_SCROLL_DN_INPUT_MASTER );
#endif

	if ((int) LCD_SCROLL_DN_INPUT_MASTER == 1 /* LCD_SCROLL_ACTIVE*/)
		ret = LCD_SCROLL_ACTIVE;
	if ((int) LCD_SCROLL_DN_INPUT == LCD_SCROLL_ACTIVE)
		ret = LCD_SCROLL_ACTIVE;

	return ret;
}

/************************************************************
 readToggleLCD: handles scroll input

 Argument

 Return Val:
	
*************************************************************/

int readToggleLCD()
{
	int ret = 0;
	static int prevStateUp = LCD_SCROLL_INACTIVE;
	static int prevStateDown = LCD_SCROLL_INACTIVE;
	static int scrollCnt=0;
	int curStateUp;
	int curStateDown;
	
	curStateDown = getScrollUpInput();
	curStateUp = getScrollDownInput();
	
	if (curStateDown != prevStateDown)
	{
		scrollCnt = 0;
#if _DEBUG_LCD
		// status change
		printf("dOWN CHANGE!\R\N");
#endif
		if (curStateDown == LCD_SCROLL_ACTIVE)
		{
			ret = LCD_SCROLL_DOWN;
			//printf("SCROLL DOWN\n");
			prevStateDown = LCD_SCROLL_ACTIVE;
		}
		else
			prevStateDown = LCD_SCROLL_INACTIVE;
	}
	else if (curStateUp != prevStateUp)
	{
		scrollCnt = 0;
#if _DEBUG_LCD
		// status change
		printf("Up CHANGE!\R\N");
#endif
		if (curStateUp == LCD_SCROLL_ACTIVE)
		{
			ret = LCD_SCROLL_UP;
			//printf("SCROLL Up\n");
			prevStateUp = LCD_SCROLL_ACTIVE;
		}
		else
			prevStateUp = LCD_SCROLL_INACTIVE;
		
	}
	else if (curStateDown == LCD_SCROLL_ACTIVE || curStateUp == LCD_SCROLL_ACTIVE)
	{
		if (scrollCnt < LCD_SCRL_RPT_CNT)
		{
			scrollCnt++;
#if _DEBUG_LCD
			printf("Waiting(%d)\r\n", scrollCnt);
#endif
		}
		else
		{
#if _DEBUG_LCD
			printf("Waitingdone\r\n");
#endif
			scrollCnt = 0;
			if (curStateDown == LCD_SCROLL_ACTIVE)
			{
				ret = LCD_SCROLL_DOWN;
				prevStateDown = LCD_SCROLL_ACTIVE;
			}
			else
			{
				ret = LCD_SCROLL_UP;
				prevStateUp = LCD_SCROLL_ACTIVE;
			}
		}
	}

	return ret;
}


/************************************************************
 LCDputchar: display a character at row & col

 Argument
	row
	col
	c

 Return Val:
	
*************************************************************/
void LCDputchar(int row, int col, char c)
{
#if _DEBUG_LCD
	printf("LCD row,col[%d,%d],[%c]\r\n", row, col, c);
#endif
	// nothing yet
}

/************************************************************
 LCDputstr: display a str at row & col

 Argument
	row
	col
	str

 Return Val:
	
*************************************************************/
void LCDputstr(int row, int col, char *str)
{
#if _DEBUG_LCD
	printf("LCD row,col[%d,%d],[%s]\r\n", row, col, str);
#endif
	// nothing yet
}


/************************************************************
 updateLCD: basic LCD manager, displays & manage scroll

 Argument
	row
	col
	str

 Return Val:
	
*************************************************************/

void updateLCD()
{
	if (gLoop.f.printLCD)
		displayLCD();		// update twice a second

	ScrollUpdateLCD();		// checks scroll button frequently
}

//************************************************************************
//Write out a char at a time, up to length or '\0', which ever is first
//don't send '\0'
void LCD_WriteStrn(char *s, char length)
{
	while (--length >= 0 && *s != 0)
		Write_Serial_Port_Two(*s++);
}
//************************************************************************
void LCD_Blink(char lineNo, char yes)
{
	if (yes)
		mSetBit(s.blinkFlag, lineNo);
	else
		mClearBit(s.blinkFlag, lineNo);
	//printf("set bit %d,%8lx\r", lineNo,*(long *)s.blinkFlag);
}
//***********************************************************************
static char blinkShow(char lineNo)
{
//	Returns true if line should be displayed.
	if (!mIsSetBit(s.blinkFlag,lineNo))
		return 1;	//don't blink this line
	return s.blinkOn;	//else we blink

//  returns false only if blink is true and the blinkFlag is true
//	return !(mIsSetBit(s.blinkFlag,lineNo) && s.blinkOn);
}
