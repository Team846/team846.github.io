/*******************************************************************************
* FILE NAME: printf_lib.c
*
* DESCRIPTION:
*  This file contains generic routines that work like the standard C I/O library.
*  These unsupported routines have been modified to work with the Microchip 
*  C compiler.  The printf routine is limited as follows:
*
*  1.  Only the %s, %lx, %d, %x, %u, %X directives are parsed.  An additional %b 
*      directive is parsed for those who like to view data in binary (base 2).
*
*      Examples:   
*
*            rom const char *StrPtr = "Hello world!";
*            int x = 15;
*            int y = 0x50;
*            long z = 0xdeadface;
*
*            printf("%s\n",StrPtr);                         will display 'Hello world!'
*                                  
*            printf("X = %d, Y = %x, Z = %lx\n",x,y,z);     will display 'X = 15, Y = 0x50, Z = deadface'
*                                                      
*            printf("X = %b (base 2)\n",x);                 will display 'X = 1111 (base 2)'
*                   
*            printf("X = %16b\n",x);                        will display 'X = 0000000000001111'
*                           
*            printf("X = %04x\n",x);                        will display 'X = 000f'
*                           
*  2.  All bytes (8 bits) or bit references (rc_dig_in01) must be type-cast to a  
*      16 bit word.  The %c directive is unsupported.
*
*      Examples:   
*
*            unsigned char byte1 = 0xfa;
*            char byte2 = 25;
*
*            printf("1st byte = %x, 2nd byte = %d\n",(int)byte1,(int)byte2);
*                                                           will display '1st byte = fa, 2nd byte = 25
*            if (rc_dig_in01)
*              printf("On %d\n",(int)rc_dig_in01);          will display 'On 1'
*            else
*              printf("Off %d\n",(int)rc_dig_in01);         will display 'Off 0'
*
*  3.  The %s directive only supports a string of 40 bytes and the format string
*      may not exceed 80 bytes.  The sprintf routine is not included.  Keep in 
*      mind that the stack size has a 256 byte limit.
*
* USAGE:
*  These library routines  may be modified to suit the needs of the user.
*******************************************************************************/

#include "ifi_default.h"
#include "printf_lib.h"
#include "ifi_utilities.h"
#include "string.h"

/* the following should be enough for a 32 bit word */
#define PRINT_BUF_LEN 20

extern char ifi_printfBufr[]; /* declared in ifi_utilities.c */
rom char *nullStr = "(null)";

static char print_buf[PRINT_BUF_LEN];
static char scr[2];
static int  k;

/*******************************************************************************
* SUBROUTINE NAME: Write_Byte_To_Uart
* PURPOSE:       Writes a byte to the UART.
*     Argument       Type           IO   Description
*     --------       -----------    --   -----------
*         data       int            I    data to transmit to the UART
* RETURNS:       void
*******************************************************************************/

static void Write_Byte_To_Uart(int data)
{
#if USE_BUFFERED_PRINTF       // Set in PicSerialdrv.h 
  Serial_Display_Char((char) data);
#else
  TXREG = data;
  Wait4TXEmpty();
#endif
}

#define PAD_RIGHT 1
#define PAD_ZERO  2

/*******************************************************************************
* FUNCTION NAME: prints
* PURPOSE:       Pads the output stream.
* RETURNS:       void
*******************************************************************************/

static int prints(char *string, int width, int pad)
{
	register int pc = 0, padchar = ' ';

	if (width > 0) {
		register int len = 0;
		register char *ptr;
		for (ptr = string; *ptr; ++ptr) ++len;
		if (len >= width) width = 0;
		else width -= len;
		if (pad & PAD_ZERO) padchar = '0';
	}
	if (!(pad & PAD_RIGHT)) {
		for ( ; width > 0; --width) {
			Write_Byte_To_Uart (padchar);
			++pc;
		}
	}
	for ( ; *string ; ++string) {
		Write_Byte_To_Uart (*string);
		++pc;
	}
	for ( ; width > 0; --width) {
		Write_Byte_To_Uart (padchar);
		++pc;
	}

	return pc;
}

/*******************************************************************************
* FUNCTION NAME: printi
* PURPOSE:       Converts the output stream to the proper width and base.
* RETURNS:       void
*******************************************************************************/

static int printi(int i, int b, int sg, int width, int pad, int letbase)
{
	register char *s;
	register int t, neg = 0, pc = 0;
	register unsigned int u = i;

   print_buf[0] = 0xBE;
   print_buf[PRINT_BUF_LEN] = 0xEF;
	if (i == 0) {
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return prints (print_buf, width, pad);
	}

	if (sg && b == 10 && i < 0) {
		neg = 1;
		u = -i;
	}

	s = print_buf + PRINT_BUF_LEN-1;
	*s = '\0';

	while (u) {
		t = u % b;
		if( t >= 10 )
			t += letbase - '0' - 10;
		*--s = t + '0';
		u /= b;
	}

	if (neg) {
		if( width && (pad & PAD_ZERO) ) {
			Write_Byte_To_Uart ('-');
			++pc;
			--width;
		}
		else {
			*--s = '-';
		}
	}

	return pc + prints (s, width, pad);
}

/*******************************************************************************
* FUNCTION NAME: print
* PURPOSE:       Parses the output stream.
* RETURNS:       void
*******************************************************************************/

static int print(char *format, int *varg)
{
  char tmpBufr[40];
	register int width, pad;
	register int pc = 0;

	for (; *format != 0; ++format) {
		if (*format == '%') {
			++format;
			width = pad = 0;
			if (*format == '\0') break;
			if (*format == '%') goto out;
			if (*format == '-') {
				++format;
				pad = PAD_RIGHT;
			}
			while (*format == '0') {
				++format;
				pad |= PAD_ZERO;
			}
			for ( ; *format >= '0' && *format <= '9'; ++format) {
				width *= 10;
				width += *format - '0';
			}
			if( *format == 's' ) 
      {
        strcpypgm2ram(tmpBufr,(rom char *) *varg--);
        if (tmpBufr[0] == 0)
          strcpypgm2ram(tmpBufr,nullStr);
				pc += prints (tmpBufr, width, pad);
				continue;
			}
			if( *format == 'd' ) {
				pc += printi (*varg--, 10, 1, width, pad, 'a');
				continue;
			}
			if( *format == 'x' ) {
				pc += printi (*varg--, 16, 0, width, pad, 'a');
				continue;
			}
			if( *format == 'X' ) {
				pc += printi (*varg--, 16, 0, width, pad, 'A');
				continue;
			}
			if( *format == 'u' ) {
				pc += printi (*varg--, 10, 0, width, pad, 'a');
				continue;
			}
			if( *format == 'l' ) {    /* assumes lx */
				pc += printi (*varg--, 16, 0, width, pad, 'a');
				pc += printi (*varg--, 16, 0, width, pad, 'a');
				format++;               /* skip over x*/
				continue;
			}
			if( *format == 'b' ) {
				pc += printi (*varg--, 2, 0, width, 2, 'a');
				continue;
			}
		}
		else {
		out:
      if (*format == '\n') *format = '\r';  /* replace line feed with cr */
			Write_Byte_To_Uart (*format);
			++pc;
		}
	}
	return pc;
}

/*******************************************************************************
* FUNCTION NAME: printf
* PURPOSE:       Formats an output stream.
* RETURNS:       void
*******************************************************************************/

int printf(rom const char *format, ...)
{
    register int *varg = (int *)(&format);
    /* 
       Since constant strings are kept in program (flash) memory, the strcpypgm2ram
       routine copies the string from flash to ram. 
    */
    strcpypgm2ram(ifi_printfBufr,(rom char *) format);

    varg--; /* adjust stack for Microchip C Compiler */
    return print(ifi_printfBufr, varg);
}

/******************************************************************************
* 
* The following routines are examples how to use the printf library routines
* to create your own output modules:
*  
*******************************************************************************/

/*******************************************************************************
* FUNCTION NAME: printid
* PURPOSE:       Prints a 16bit word (base 10) w/o a carriage return.
* RETURNS:       void
*******************************************************************************/

void printid(int data,int crtOn)
{
  printi (data, 10, 1, 4, 2, 'a');
  if (crtOn)
    Write_Byte_To_Uart('\r');
}

/*******************************************************************************
* FUNCTION NAME: printd
* PURPOSE:       Prints an 8bit word (base 10) w/o a carriage return.
* RETURNS:       void
*******************************************************************************/

void printd(unsigned char data,int crtOn)
{
  printi ((int) data, 10, 1, 3, 2, 'a');
  if (crtOn)
    Write_Byte_To_Uart('\r');
}

/*******************************************************************************
* FUNCTION NAME: printib
* PURPOSE:       Prints a 16bit binary word (base 2) w/o a carriage return.
* RETURNS:       void
*******************************************************************************/

void printib(unsigned int data,int crtOn)
{
  printi (data, 2, 0, 16, 2, 'a');
  if (crtOn)
    Write_Byte_To_Uart('\r');
}

/*******************************************************************************
* FUNCTION NAME: printb
* PURPOSE:       Prints an 8bit binary word (base 2) w/o a carriage return.
* RETURNS:       void
*******************************************************************************/

void printb(unsigned char data,int crtOn)
{
  printi ((int) data, 2, 0, 8, 2, 'a');
  if (crtOn)
    Write_Byte_To_Uart('\r');
}

/*******************************************************************************
* FUNCTION NAME: printix
* PURPOSE:       Prints a 16bit hex word (base 16) w/o a carriage return.
* RETURNS:       void
*******************************************************************************/

void printix(int data,int crtOn)
{
  printi (data, 16, 0, 2, 0, 'a');
  if (crtOn)
    Write_Byte_To_Uart('\r');
}

/*******************************************************************************
* FUNCTION NAME: printx
* PURPOSE:       Prints an 8bit hex word (base 16) w/o a carriage return.
* RETURNS:       void
*******************************************************************************/

void printx(unsigned char data,int crtOn)
{
  printi ((int) data, 16, 0, 2, 2, 'a');
  if (crtOn)
    Write_Byte_To_Uart('\r');
}

/*******************************************************************************
* FUNCTION NAME: debug_print
* PURPOSE:       Prints a header and a 16bit hex word (base 16) with a carriage 
*                return.
* RETURNS:       void
*******************************************************************************/

void debug_print(char *bufr,int data)
{
  strcpypgm2ram (ifi_printfBufr,(rom char *) bufr);
  for (k=0;k<strlen(ifi_printfBufr);k++)
    Write_Byte_To_Uart(ifi_printfBufr[k]);
  printix(data,1);
}

/*******************************************************************************
* FUNCTION NAME: debug_printb
* PURPOSE:       Prints a header and an 8bit binary word (base 2) with a carriage 
*                return.
* RETURNS:       void
*******************************************************************************/

void debug_printb(char *bufr,unsigned int data)
{
  strcpypgm2ram (ifi_printfBufr,(rom char *) bufr);
  for (k=0;k<strlen(ifi_printfBufr);k++)
    Write_Byte_To_Uart(ifi_printfBufr[k]);
  printib(data,1);
}

/*******************************************************************************
* FUNCTION NAME: debug_println
* PURPOSE:       Prints a header (assumming a carriage return is supplied).
* RETURNS:       void
*******************************************************************************/

void debug_println(char *bufr)
{
  strcpypgm2ram (ifi_printfBufr,(rom char *) bufr);
  for (k=0;k<strlen(ifi_printfBufr);k++)
    Write_Byte_To_Uart(ifi_printfBufr[k]);
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
