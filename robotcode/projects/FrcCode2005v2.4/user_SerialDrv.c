/*******************************************************************************
* FILE NAME: user_serialdrv.c
*
* DESCRIPTION:
*  This file contains an unsupported serial device driver for the 18F8520 
*  micro.  
*
*  Once the driver is initialized (Serial_Driver_Initialize), it handles data
*  from both serial ports.  When a serial interrupt occurs, the interrupt service 
*  routine (InterruptHandlerLow) makes a call to CheckUartInts.  This routine
*  determines if it was a transmit or receive interrupt.  If transmit, then it will
*  work off the transmit buffer a byte at a time.  If receive, then it will call 
*  the Handle_Panel_Data state machine until a full packet has been received (from
*  the Breaker Panel).  You can put your own callback routine inside CheckUartInts.  
*  This will allow you to parse data from another device in real time.  
*
* ASSUMPTION AND LIMITATIONS:
*  A printf call now can be buffered by setting the USE_BUFFERED_PRINTF to one.  This
*  enables the user to write non-blocking application code.  However, if two consecutive 
*  printfs are invoked, the second will be blocked until the first is complete. This
*  can be resolved by adding a circular queue to the device status structure.
*  
*
* USAGE:
*  This driver may be modified to suit the needs of the user.  
*******************************************************************************/
#include <string.h>
#include <usart.h>
#include <string.h>
#include <stdio.h>

#include "user_Serialdrv.h"
#include "ifi_default.h"
#include "delays.h"

#define PRIORITY            0           // Always set to Low (0)

/*******************************************************************************
* DeviceStatus - This stucture is used as a device control block (dcb) and 
* provides status information for the driver as well.
*******************************************************************************/
typedef struct 
{
  unsigned char portId;
  unsigned char wtCount;
  unsigned char *wtbufr;
  unsigned char wtBufr[MAX_WT_BUFR_SIZE];
  unsigned char wtBufrIdx;
  volatile near unsigned char *myRCREG;
  volatile near unsigned char *myTXREG;
} DeviceStatus;

typedef DeviceStatus *DeviceStatusPtr;

static char rom2ramBufr[MAX_RD_BUFR_SIZE];  
static PANEL_INPUT_RECORD  dataIn;     /* data input buffer (from Breaker Panel) */
static unsigned char IntState;    

static DeviceStatus dcb[TOTAL_SERIAL_PORTS];  /* device control block */
unsigned char aBreakerWasTripped;

/*******************************************************************************
* FUNCTION NAME: Reset_Control_Block
* PURPOSE:       Handles incomming data from PROGRAM_PORT and/or TTL_PORT.  It
*                also transmits data from an output buffer.
* CALLED FROM:   Initialize_uart
* ARGUMENTS:     1
*     Argument       Type             IO   Description
*     --------       -------------    --   -----------
*     dcbPtr         DeviceStatusPtr  I    pointer to a dcb
*******************************************************************************/

static void Reset_Control_Block(DeviceStatusPtr dcbPtr)
{
  INTCONbits.PEIE = 0;          /* Disable peripheral interrupt */
  dcbPtr->wtCount = 0;
  dcbPtr->wtBufrIdx = 0;
  INTCONbits.PEIE = 1;          /* Enable peripheral interrupts */
}

/*******************************************************************************
* FUNCTION NAME: Initialize_uart
* PURPOSE:       Initializes the device control block and opens both serial ports.
* CALLED FROM:   Serial_Driver_Initialize
* ARGUMENTS:     none
*******************************************************************************/

static void Initialize_uart(void)
{
  uword8 flush;
 
  /* Setup the device control block (dcb) */
  dcb[PROGRAM_PORT].myRCREG = &RCREG1;
  dcb[PROGRAM_PORT].myTXREG = &TXREG1;
  dcb[PROGRAM_PORT].portId = 1;         /* setup unique ID for PROGRAM_PORT */
  Reset_Control_Block(&dcb[PROGRAM_PORT]);

  dcb[TTL_PORT].myRCREG = &RCREG2;
  dcb[TTL_PORT].myTXREG = &TXREG2;
  dcb[TTL_PORT].portId = 2;             /* setup unique ID for TTL_PORT */
  Reset_Control_Block(&dcb[TTL_PORT]);

  /* Initialize the PROGRAM_PORT port */
#if USE_BUFFERED_PRINTF
  Open1USART(USART_TX_INT_ON & USART_RX_INT_ON & USART_ASYNCH_MODE &
    USART_EIGHT_BIT & USART_CONT_RX & USART_BRGH_HIGH, baud_115); 
#else
  Open1USART(USART_TX_INT_OFF & USART_RX_INT_ON & USART_ASYNCH_MODE &
    USART_EIGHT_BIT & USART_CONT_RX & USART_BRGH_HIGH, baud_115); 
#endif
  
  IPR1bits.TXIP = PRIORITY;      
  IPR1bits.RCIP = PRIORITY;     
  TRISCbits.TRISC6  = 0;        /*TX1 output*/
  TRISCbits.TRISC7 = 1;         /*RX1 input */
 
  flush = RCREG1;               /* flush data */
  flush = RCREG1;               /* flush data */
  RXINTF = 0;

  /* Initialize the TTL_PORT port */

  Open2USART(USART_TX_INT_OFF & USART_RX_INT_ON & USART_ASYNCH_MODE &
    USART_EIGHT_BIT & USART_CONT_RX & USART_BRGH_HIGH, baud_115); 
  
  IPR3bits.RC2IP = PRIORITY;     
  IPR3bits.TX2IP = PRIORITY;     
  TRISGbits.TRISG1 = 0;         /*TX2 output*/
  TRISGbits.TRISG2 = 1;         /*RX2 input */

  flush = RCREG2;               /* flush data */
  flush = RCREG2;               /* flush data */
  RXINTF2 = 0;
  aBreakerWasTripped = FALSE;
}

/*******************************************************************************
* FUNCTION NAME: EnableXmitInts
* PURPOSE:       Enables transmit interrupt
* CALLED FROM:   CheckTXIntFlag
* ARGUMENTS:     1
*     Argument       Type             IO   Description
*     --------       -------------    --   -----------
*     dcbPtr         DeviceStatusPtr  I    pointer to a dcb
*******************************************************************************/

static void EnableXmitInts(DeviceStatusPtr dcbPtr)
{
  if (dcbPtr->portId == 1)
    TXINTE = 1;
  else
    TXINTE2 = 1;
}

/*******************************************************************************
* FUNCTION NAME: CheckTXIntFlag
* PURPOSE:       Check transmit interrupt flag
* CALLED FROM:   Serial_Write_Bufr, Serial_Display_Char
* ARGUMENTS:     1
*     Argument       Type             IO   Description
*     --------       -------------    --   -----------
*     port           Int              I    TTL_PORT or PROGRAM_PORT
*******************************************************************************/

static void CheckTXIntFlag(int port)
{
  if (port == PROGRAM_PORT)
  {
    if (TXINTF)
      EnableXmitInts(&dcb[PROGRAM_PORT]); 
  }
  else
  {
    if (TXINTF2)
      EnableXmitInts(&dcb[TTL_PORT]); 
  }
}

/*******************************************************************************
* FUNCTION NAME: Serial_Resume_Port
* PURPOSE:       Used to reenable a port that has been terminated due to an error.
* CALLED FROM:   N/A
* ARGUMENTS:     1
*     Argument       Type             IO   Description
*     --------       -------------    --   -----------
*     port           Int              I    TTL_PORT or PROGRAM_PORT
*******************************************************************************/

void Serial_Resume_Port(int port)
{
  if (port == PROGRAM_PORT)
  {
    if (RCSTA1bits.OERR)
      RCSTA1bits.CREN = 0;
    RCSTA1bits.CREN = 1;
    RXINTE = 1;
  }
  else
  {
    if (RCSTA2bits.OERR)
      RCSTA2bits.CREN = 0;
    RCSTA2bits.CREN = 1;
    RXINTE2 = 1;
  }
}

/*******************************************************************************
***********************  Interrupt Routines  ***********************************
*******************************************************************************/

/*******************************************************************************
* FUNCTION NAME: DisableXmitInts
* PURPOSE:       Disables a transmit interrupt
* CALLED FROM:   Process_TX
* ARGUMENTS:     1
*     Argument       Type             IO   Description
*     --------       -------------    --   -----------
*     dcbPtr         DeviceStatusPtr  I    pointer to a dcb
*******************************************************************************/

void DisableXmitInts(DeviceStatusPtr dcbPtr)
{
  if (dcbPtr->portId == 1)
  {
    TXINTF = 0;
    TXINTE = 0;
  }
  else
  {
    TXINTF2 = 0;
    TXINTE2 = 0;
  }
}

/*******************************************************************************
* FUNCTION NAME: Process_TX
* PURPOSE:       Transmits an output buffer to the serial port.
* CALLED FROM:   CheckUartInts
* ARGUMENTS:     1
*     Argument       Type             IO   Description
*     --------       -------------    --   -----------
*     dcbPtr         DeviceStatusPtr  I    pointer to a dcb
*******************************************************************************/

static void Process_TX(DeviceStatusPtr dcbPtr)
{
  if (dcbPtr->wtCount == 0)     /* if all data is transmitted, then disable TX */
  {
    DisableXmitInts(dcbPtr);
    return;
  }
  *dcbPtr->myTXREG = *dcbPtr->wtbufr++;  /* Load transmit register with data */
  dcbPtr->wtCount--;                     /* advance bufr counter */
}

/*******************************************************************************
* FUNCTION NAME: Handle_Panel_Data
* PURPOSE:       State machine for incomming Breaker Panel data.  
*
*
* NOTE:          Normally device specific code should reside in another file but
*                for the purpose of simplification we chose to put it here. 
*
* CALLED FROM:   CheckUartInts
* ARGUMENTS:     1
*     Argument       Type             IO   Description
*     --------       -------------    --   -----------
*     data           unsigned char    I    incoming data byte
* LIMITATIONS:   The data from the breaker panel does not contain a checksum.
*				 See the breaker-panel-packet-definitions.pdf file for more info.
*******************************************************************************/

void Handle_Panel_Data(unsigned char data)  
{
  switch (IntState)
  {
    case 0:  // 1st 0xC9
      IntState = 0;
      if (data == 0xC9) IntState = 1;
      break;
    case 1 : // 2nd 0x17
      IntState = 0;
      if (data == 0x17) IntState = 2;
      break;
    case 2 : //get DATA1
      dataIn.data1Byte.allbits = data;
      txdata.user_byte3 = data;
      aBreakerWasTripped = (int) dataIn.data1Byte.bitselect.tripped;
      IntState = 3;
      break;
    case 3 : //get DATA2
      dataIn.data2 = data;
      txdata.user_byte4 = data;
      IntState = 4;
      break;
    case 4 : //get DATA3
      dataIn.data3 = data;
      txdata.user_byte5 = data;
      IntState = 5;
      break;
    case 5 : //get DATA4
      dataIn.data4 = data;
      txdata.user_byte6 = data;
      IntState = 0;
      break;
    default:
      IntState= 0;
      break;
  }
}

/*******************************************************************************
* FUNCTION NAME: CheckUartInts
* PURPOSE:       Handles incoming data from PROGRAM_PORT and/or TTL_PORT.  It
*                also transmits data from an output buffer.
* CALLED FROM:   InterruptHandlerLow
* ARGUMENTS:     none
*******************************************************************************/

void CheckUartInts ()
{
  if (RXINTF)       
  { 
    RXINTF = 0;
    Handle_Panel_Data(*dcb[BREAKER_PANEL_PORT].myRCREG);
    return;
  }
  if (TXINTF)       //data is being transferred one byte at a time
    Process_TX(&dcb[PROGRAM_PORT]);

  if (RXINTF2)       
  { 
    RXINTF2 = 0;
    /* If you want to handle RX data on the program port, you could provide a
       callback routine to parse data.

       Example :

       Serial_Char_Callback(*dcb[TTL_PORT].myRCREG);

       You must supply the body of this routine in a file of your choice.  
    */
    Serial_Char_Callback(*dcb[TTL_PORT].myRCREG);
    return;
  }
  if (TXINTF2)      //data is being transferred one byte at a time
    Process_TX(&dcb[TTL_PORT]);

}

/*******************************************************************************
*****************************  Service Routines  *******************************
*******************************************************************************/

/*******************************************************************************
* FUNCTION NAME: Serial_Driver_Initialize
* PURPOSE:       Initializes the device driver
* CALLED FROM:   User application layer
* ARGUMENTS:     none
*******************************************************************************/

void Serial_Driver_Initialize(void)
{
  Initialize_uart();  
}

/*******************************************************************************
* FUNCTION NAME: Serial_Write_Bufr
* PURPOSE:       Writes a buffer to the specified serial port.  
* CALLED FROM:   Serial_Write or any application layer routine
* ARGUMENTS:     3
*     Argument       Type             IO   Description
*     --------       -------------    --   -----------
*     port           int              I    TTL_PORT or PROGRAM_PORT
*     ch_out         unsigned char *  I    output buffer
*     len            sword16          I    number of bytes to transmit
*******************************************************************************/

void Serial_Write_Bufr(int port,unsigned char *ch_out, sword16 len) 
{
  static DeviceStatusPtr dcbPtr;

  dcbPtr = &dcb[port];

  INTCONbits.PEIE = 0;    //Disable peripheral interrupt
  dcbPtr->wtbufr = ch_out;
  dcbPtr->wtCount = (unsigned char) len;
  INTCONbits.PEIE = 1;    //Enable peripheral interrupt
  CheckTXIntFlag(port);
}

/*******************************************************************************
* FUNCTION NAME: Serial_Write
* PURPOSE:       Prepares a data buffer to be written to a serial port. 
* CALLED FROM:   User application layer
* ARGUMENTS:     3
*     Argument       Type             IO   Description
*     --------       -------------    --   -----------
*     port           int              I    TTL_PORT or PROGRAM_PORT
*     obufr          rom const char   I    output buffer
*     len            sword16          I    number of bytes to transmit
*******************************************************************************/

unsigned char Serial_Write(int port,rom const char *obufr,sword16 len)
{
  while (dcb[port].wtCount > 0);  /* wait till done writing */

  if (len > MAX_WT_BUFR_SIZE) len = MAX_WT_BUFR_SIZE;
  strcpypgm2ram(rom2ramBufr,(rom char *) obufr);
  Serial_Write_Bufr(port,(unsigned char *)rom2ramBufr,len);
  return 0;
}

/*******************************************************************************
* FUNCTION NAME: Serial_Write_Char
* PURPOSE:       Write a byte to a specified serial port. 
* CALLED FROM:   User application layer
* ARGUMENTS:     2
*     Argument       Type             IO   Description
*     --------       -------------    --   -----------
*     port           int              I    TTL_PORT or PROGRAM_PORT
*     ch_out         int              I    output character
*******************************************************************************/

void Serial_Write_Char(int port,int ch_out) 
{
  static DeviceStatusPtr dcbPtr;

  dcbPtr = &dcb[port];

  *dcbPtr->myTXREG = (unsigned char) ch_out;  /* Load transmit register with data */

  if (port == PROGRAM_PORT)
    while (!TXINTF);
  else
    while (!TXINTF2);
}

/*******************************************************************************
* FUNCTION NAME: Serial_Display_Char
* PURPOSE:       Buffers data until a '\r' is seen or MAX_WT_BUFR_SIZE is reached.
*                Once either condition is met, the transmission is started.
* CALLED FROM:   User application layer (Write_Byte_To_Uart)
* ARGUMENTS:     1
*     Argument       Type             IO   Description
*     --------       -------------    --   -----------
*     data           unsigned char    I    data to be buffered
*******************************************************************************/

void Serial_Display_Char(unsigned char data)
{
  while (dcb[PRINTF_PORT].wtCount > 0);  /* wait till done writing */

  dcb[PRINTF_PORT].wtBufr[dcb[PRINTF_PORT].wtBufrIdx++] = data;
  if ((data == '\r') || (dcb[PRINTF_PORT].wtBufrIdx >= MAX_WT_BUFR_SIZE) )
  {
    INTCONbits.PEIE = 0;    //Disable peripheral interrupt
    dcb[PRINTF_PORT].wtbufr = &dcb[PRINTF_PORT].wtBufr[0];
    dcb[PRINTF_PORT].wtCount = dcb[PRINTF_PORT].wtBufrIdx;
    INTCONbits.PEIE = 1;    //Enable peripheral interrupt
    dcb[PRINTF_PORT].wtBufrIdx = 0;
    CheckTXIntFlag(PRINTF_PORT);
  }
}

/*******************************************************************************
* FUNCTION NAME: Breaker_Tripped
* PURPOSE:       This routine determines the tripped breaker(s) using the data
*                from the breaker panel. 
*
* NOTE:          Normally device specific code should reside in another file but
*                for the purpose of simplification we chose to put it here. 
*
* CALLED FROM:   CheckUartInts
* ARGUMENTS:     1
*     Argument       Type             IO   Description
*     --------       -------------    --   -----------
*     id             unsigned char    I    breaker id number
*******************************************************************************/
unsigned char Breaker_Tripped(unsigned char id)
{
  unsigned char bit2check;
  unsigned char retn;

  retn = FALSE;
  if (id < 9)         //Check breakers 1-8
  {
    bit2check = 1 << (id - 1);
    retn = dataIn.data4 & bit2check;
  }
  else if (id < 17)   //Check breakers 9-16
  {
    bit2check = 1 << (id - 9);
    retn = dataIn.data3 & bit2check;
  }
  else if (id < 25)   //Check breakers 17-24
  {
    bit2check = 1 << (id - 17);
    retn = dataIn.data2 & bit2check;
  }
  else                //Check breakers 25-28
  {
    bit2check = 1 << (id - 25);
    retn = dataIn.data1Byte.allbits & bit2check;
  }
  return retn;
}

