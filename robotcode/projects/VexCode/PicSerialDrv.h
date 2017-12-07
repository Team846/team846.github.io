/*******************************************************************************
* FILE NAME: PicSerialDrv.h
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
typedef unsigned int uint32;

// Set these equal to 1 if you want to use these features, to 0 if not.
#define USE_DYNAMIC_DEBUG   1
#define USE_BUFFERED_PRINTF 1

#define PROGRAM_PORT        0
#define TTL_PORT            1

// ASSIGN SERIAL PORTS
// Set which USART (serial port) you want to use for printfs 
// and which you want to use for the Dynamic Debug Tool.
#define PRINTF_PORT         TTL_PORT  

#ifdef _DIAGNOSTICS         //Only defined for a standalone diagnostics module
#define DEBUG_PORT          TTL_PORT
#else
#define DEBUG_PORT          PROGRAM_PORT
#endif

#define MAX_MDM_BUFR_SIZE   32    
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
  baud_96 = 64,   // Low speed
  baud_19 = 128,  // @40Hhz
  baud_38 =  64,
  baud_56 =  42,
  baud_115 = 21   // @40Hhz
} SERIAL_SPEED;

/*******************************************************************************
* This stucture is used to pass data from the PC to the device driver.
*******************************************************************************/

typedef struct
{ 
  unsigned char  RX_SYNC1;    //Always 0xAA
  unsigned char  RX_SYNC2;    //Always 0x55
  unsigned char  RX_INDEX;     
  unsigned char  RX_DATAH;
  unsigned char  RX_DATAL;
  unsigned char  RX_CMD;
  unsigned char  RX_FSRH;
  unsigned char  RX_FSRL;
} DYNO_INPUT_RECORD;

/*******************************************************************************
* This stucture is used to pass data from the device driver to the PC.
*******************************************************************************/

typedef struct
{ 
  unsigned char  TX_SYNC1;    //Always 0xAA
  unsigned char  TX_SYNC2;    //Always 0x55
  unsigned char  TX_INDEX;     
  unsigned char  TX_DATAH;
  unsigned char  TX_DATAL;
  unsigned char  TX_CMD;
} DYNO_OUTPUT_RECORD;

/*******************************************************************************
                           FUNCTION PROTOTYPES
*******************************************************************************/
/* These routines reside in PicPerialDrv.c */
void CheckUartInts(void);
void Serial_Driver_Initialize(void);
void Serial_Write_Char(int port,int ch_out);
void Serial_Display_Char(unsigned char data);
void Serial_Write_Bufr(int port,unsigned char *ch_out, sword16 len); 
unsigned char Serial_Write(int port,rom const char *obufr,sword16 len);
void Serial_Char_Callback(unsigned char data);
void Serial_Refresh_PC_PWM_Panel(void);
void Process_Debug_Stream(void);
unsigned char PC_Is_Not_In_Control(void);

/* These routines reside in asm_lib.asm */
void ReadEE(unsigned char addr, unsigned char gByteCnt, unsigned char *bufr);
unsigned char GetEEData(unsigned char addr);
void WriteEE(unsigned char addr, unsigned char gByteCnt, unsigned char *bufr);
void write_to_address(uword8 FSR0H,uword8 FSR0L,uword8 data);
uword8 read_from_address(uword8 FSR0H,uword8 FSR0L);


#endif


