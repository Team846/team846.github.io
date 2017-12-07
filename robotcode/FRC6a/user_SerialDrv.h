/*******************************************************************************
* FILE NAME: user_serialdrv.h
*
* DESCRIPTION:
*  This is the header file for the serial device driver.   
*
* USAGE:
*  This file may be modified to suit the needs of the user.  
*******************************************************************************/
#ifndef __serialdrv_h_
#define __serialdrv_h_

typedef char  sword8;
typedef short sword16;
typedef long  sword32;

typedef unsigned char  uword8;
typedef unsigned short uword16;
typedef unsigned long  uword32;
typedef unsigned int   uint32;

#define TOTAL_SERIAL_PORTS  2

// Set these equal to 1 if you want to use these features, to 0 if not.
#define PROGRAM_PORT        0
#define TTL_PORT            1

// ASSIGN SERIAL PORTS
// Set which USART (serial port) you want to use for printfs 
// and which you want to use for the Dynamic Debug Tool.
#define PRINTF_PORT         PROGRAM_PORT
#define BREAKER_PANEL_PORT  PROGRAM_PORT

#define MAX_RD_BUFR_SIZE    32    
#define MAX_WT_BUFR_SIZE    80    

#define RXINTF              PIR1bits.RCIF
#define RXINTE              PIE1bits.RCIE
#define TXINTF              PIR1bits.TXIF 
#define TXINTE              PIE1bits.TXIE

#define RXINTF2             PIR3bits.RC2IF
#define RXINTE2             PIE3bits.RC2IE
#define TXINTF2             PIR3bits.TX2IF 
#define TXINTE2             PIE3bits.TX2IE

#define TXSTA               TXSTA1
#define TXREG               TXREG1
#define RCSTA               RCSTA1
#define RCREG               RCREG1

/*******************************************************************************
                             MACRO DEFINITIONS
*******************************************************************************/

typedef enum
{
  baud_19 = 128, // @40Hhz
  baud_38 =  64,
  baud_56 =  42,
  baud_115 = 21  // @40Hhz
} SERIAL_SPEED;

typedef struct
{
  unsigned int  bit0:1;
  unsigned int  bit1:1;
  unsigned int  bit2:1;
  unsigned int  bit3:1;
  unsigned int  bit4:1;
  unsigned int  validChecksum:1;
  unsigned int  bp2006:1;
  unsigned int  tripped:1;
} panel_bitid;

/*******************************************************************************
* This stucture is used to pass data from the PC to the device driver.
*******************************************************************************/

typedef struct
{ 
  unsigned char  RX_SYNC1;    //Always 0xC9
  unsigned char  RX_SYNC2;    //Always 0x17 or 0x18
  union
  { 
    panel_bitid bitselect;
    unsigned char allbits;    
  } data1Byte;
  unsigned char  data2;
  unsigned char  data3;
  unsigned char  data4;
} PANEL_INPUT_RECORD;


/*******************************************************************************
                           FUNCTION PROTOTYPES
*******************************************************************************/
/* These routines reside in PicPerialDrv.c */
void CheckUartInts(void);
void Serial_Driver_Initialize(void);
void Serial_Write_Char(int port,int ch_out);
void Serial_Display_Char(unsigned char data);
unsigned char Serial_Write(int port,rom const char *obufr,sword16 len);
void Serial_Char_Callback(unsigned char data);
unsigned char Breaker_Tripped(unsigned char id);
unsigned char NewPanel(void);


#endif


