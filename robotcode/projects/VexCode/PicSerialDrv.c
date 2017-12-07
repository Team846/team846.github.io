/*******************************************************************************
* FILE NAME: PicSerialDrv.c
*
* DESCRIPTION:
*  This file contains an unsupported serial device driver for the 18F8520 
*  micro.  It was designed to be used with the Dynamic Debug Tool (DDT).  The 
*  DDT is now apart of the IFI Loader (ver 1.8 or later) under the 'Options' menu.
*  The DDT can be used to directly read and modify memory locations and registers 
*  of the user processor dynamically.
*
*  Once the driver is initialized (Serial_Driver_Initialize) it handles data
*  from both serial ports.  When a serial interrupt occurs, the interrupt service 
*  routine (InterruptHandlerLow) makes a call to CheckUartInts.  This routine
*  determines if it was a transmit or receive interrupt.  If transmit, then it will
*  work off the transmit buffer a byte at a time.  If receive, then it will call 
*  the Handle_Debug_Input state machine until a full packet has been received (from
*  the PC).  You can put your own callback routine inside CheckUartInts.  This will
*  allow you to parse data from another device in real time.    
*  
*
* USAGE:
*  This driver may be modified to suit the needs of the user.  
*******************************************************************************/
#include <string.h>
#include <usart.h>
#include <string.h>

#include "PicSerialdrv.h"
#include "ifi_default.h"
#include "delays.h"
#include "printf_lib.h"

#define PRIORITY            0           // Always set to Low (0)

#define READ_CMD            0           // Dynamic Debug commands           
#define READ_PWMs           1           // Reads PWM data
#define READ_rxSPI          2           // Used for reading rxdata
#define READ_txSPI          3           // Used for reading txdata
#define UPDATE_PWM_MASK     7           // Used for updating the PWM Mask
#define READ_EE_DATA        8           // Used for reading EE data

#define WRITE_CMD           20
#define WRITE_PWMs          21          // Updates PWM data
#define WRITE_rxSPI         22          // Used for updating rxdata
#define WRITE_txSPI         23          // Used for updating txdata
#define WRITE_EE_DATA       25          // Used for writing EE data

/*******************************************************************************
* PcBits - This bit field stucture is used to access bits from PC_Control_Byte.
*******************************************************************************/
typedef struct
{
  unsigned int  PCInControl:1;        /* PC has control of PWM data */
  unsigned int  AutoSend:1;           /* Automatically transmit PWM data to PC */
  unsigned int  bit2:1;    
  unsigned int  bit3:1;
  unsigned int  bit4:1;
  unsigned int  bit5:1;
  unsigned int  bit6:1;
  unsigned int  bit7:1;
} PcBits;
typedef PcBits *PcBitsPtr;

unsigned char PC_Control_Byte;        /* The PC will modify this control byte */
PcBitsPtr PC_Status;                  /* to access bits in the former byte */
unsigned char Current_PWM_Index;

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

extern tx_data_record txdata;         /* data to OI */
extern rx_data_record rxdata;         /* data from OI */

static char rom2ramBufr[MAX_MDM_BUFR_SIZE];  
static DYNO_OUTPUT_RECORD dataOut;    /* data output buffer (to PC) */
static DYNO_INPUT_RECORD  dataIn;     /* data input buffer (from PC) */
static unsigned char debugIntState;   /* used for incomming data from PC */

static DeviceStatus dcb[TTL_PORT+1];  /* device control block */

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
  Open1USART(USART_TX_INT_ON & USART_RX_INT_ON & USART_ASYNCH_MODE &
    USART_EIGHT_BIT & USART_CONT_RX & USART_BRGH_HIGH, baud_115); 
  
  IPR1bits.TXIP = PRIORITY;      
  IPR1bits.RCIP = PRIORITY;     
  TRISCbits.TRISC6  = 0;        /*TX1 output*/
  TRISCbits.TRISC7 = 1;         /*RX1 input */
 
  flush = RCREG1;               /* flush data */
  flush = RCREG1;               /* flush data */
  RXINTF = 0;

  /* Initialize the TTL_PORT port */

  Open2USART(USART_TX_INT_ON & USART_RX_INT_ON & USART_ASYNCH_MODE &
    USART_EIGHT_BIT & USART_CONT_RX & USART_BRGH_HIGH, baud_115); 
  
  IPR3bits.RC2IP = PRIORITY;     
  IPR3bits.TX2IP = PRIORITY;     
  TRISGbits.TRISG1 = 0;         /*TX2 output*/
  TRISGbits.TRISG2 = 1;         /*RX2 input */

  flush = RCREG2;               /* flush data */
  flush = RCREG2;               /* flush data */
  RXINTF2 = 0;

  dataOut.TX_SYNC1 = 0xAA;
  dataOut.TX_SYNC2 = 0x55;

  PC_Status = (PcBitsPtr) &PC_Control_Byte;
  PC_Status->PCInControl = TRUE;
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
* CALLED FROM:   Serial_Write_Bufr, Serial_Display_Char, Debug_Send
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
* FUNCTION NAME: Handle_Debug_Input
* PURPOSE:       State machine for incomming PC data.  When the full packet has
*                been received, the state is set to 8 allowing Process_Debug_Stream
*                to handle the new packet.
* CALLED FROM:   CheckUartInts
* ARGUMENTS:     1
*     Argument       Type             IO   Description
*     --------       -------------    --   -----------
*     data           unsigned char    I    incoming data byte
*******************************************************************************/

void Handle_Debug_Input(unsigned char data)  
{
  switch (debugIntState)
  {
    case 0:  // 1st 0xAA
      if (data == 0xAA) debugIntState = 1;
      break;
    case 1 : // 2nd 0x55
      debugIntState = 0;
      if (data == 0x55) debugIntState = 2;
      break;
    case 2 : //get idx
      dataIn.RX_INDEX = data;
      debugIntState = 3;
      break;
    case 3 : //get dataH
      dataIn.RX_DATAH = data;
      debugIntState = 4;
      break;
    case 4 : //get dataL
      dataIn.RX_DATAL = data;
      debugIntState = 5;
      break;
    case 5 : //get cmd
      dataIn.RX_CMD = data;
      debugIntState = 6;
      break;
    case 6 : //get FSRH
      dataIn.RX_FSRH = data;
      debugIntState = 7;
      break;
    case 7 : //get FSRL
      dataIn.RX_FSRL = data;
      debugIntState = 8;
      break;
    case 8 : //still processing
      break;
    default:
      debugIntState= 0;
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
#ifdef _DIAGNOSTICS         //Only defined for a standalone diagnostics module
    Serial_Char_Callback(*dcb[PROGRAM_PORT].myRCREG);
#else
    Handle_Debug_Input(*dcb[PROGRAM_PORT].myRCREG);
#endif
    return;
  }
  if (TXINTF)       //data is being transferred one byte at a time
    Process_TX(&dcb[PROGRAM_PORT]);

  if (RXINTF2)       
  { 
    RXINTF2 = 0;
    Handle_Debug_Input(*dcb[TTL_PORT].myRCREG);

    /* If you're not using the DDT on the TTL port then you could remove the call
       to Handle_Debug_Input and add your own callback routine to parse data.

       Example :

       Serial_Char_Callback(*dcb[PROGRAM_PORT].myRCREG);

       You must supply the body of this routine in a file of your choice.  
    */
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
* FUNCTION NAME: Debug_Send
* PURPOSE:       Transmits a completed packet to the PC.
* CALLED FROM:   Process_Debug_Stream
* ARGUMENTS:     none
*******************************************************************************/

static void Debug_Send(void)
{
  static DeviceStatusPtr dcbPtr;

  dcbPtr = &dcb[DEBUG_PORT];

  INTCONbits.PEIE = 0;    //Disable peripheral interrupt
  dcbPtr->wtbufr = (unsigned char *)&dataOut;
  dcbPtr->wtCount = 6;
  INTCONbits.PEIE = 1;    //Enable peripheral interrupt
  CheckTXIntFlag(DEBUG_PORT);
}

/*******************************************************************************
* FUNCTION NAME: Process_Debug_Stream
* PURPOSE:       Transmits a completed packet to the PC.
* CALLED FROM:   User application layer (Process_Data_From_Local_IO)
* ARGUMENTS:     none
*******************************************************************************/

void Process_Debug_Stream(void)   /* Called from user_routines_fast.c */
{
  static unsigned char *ptr;

  if (debugIntState == 8)  /* If data is ready to be processed */
  {
    dataOut.TX_INDEX = dataIn.RX_INDEX;  /* echo object index */
    dataOut.TX_CMD = dataIn.RX_CMD;      /* echo command */

    switch (dataIn.RX_CMD)
    {
      case READ_CMD:
        dataOut.TX_DATAH = 0;
        dataOut.TX_DATAL = read_from_address(dataIn.RX_FSRH,dataIn.RX_FSRL);
        Debug_Send();
        break;
      case WRITE_CMD:
        write_to_address(dataIn.RX_FSRH,dataIn.RX_FSRL,dataIn.RX_DATAL);
        break;
      case READ_PWMs:
        ptr = (unsigned char *) &txdata.rc_pwm01;
        dataOut.TX_DATAH = 0;
        dataOut.TX_DATAL = ptr[dataIn.RX_INDEX];
        Debug_Send();
        break;
      case WRITE_PWMs:
        if (dataIn.RX_INDEX == 250) /* Special command to update PC_Control_Byte*/
        {
          PC_Control_Byte = dataIn.RX_DATAL;
        }
        else
        {
          txdata.user_cmd |= 0x02;       /* Tell master you want to be in auton mode. */
          ptr = (unsigned char *) &txdata.rc_pwm01;
          if (dataIn.RX_INDEX < 16)
            ptr[dataIn.RX_INDEX] = dataIn.RX_DATAL;
        }
        break;
      case UPDATE_PWM_MASK:            
        txdata.pwm_mask = dataIn.RX_DATAL;
       break;
      case READ_EE_DATA:
        dataOut.TX_DATAL = GetEEData(dataIn.RX_FSRL);
        Debug_Send();
        break;
      case WRITE_EE_DATA:
        WriteEE(dataIn.RX_FSRL, 1, (unsigned char *)&dataIn.RX_DATAL);
        break;
      case WRITE_rxSPI:
        ptr = (unsigned char *)&rxdata;
        ptr[dataIn.RX_FSRL] = dataIn.RX_DATAL;
        break;
      case READ_rxSPI:
        ptr = (unsigned char *)&rxdata;
        dataOut.TX_DATAL = ptr[dataIn.RX_FSRL];
        Debug_Send();
        break;
      case WRITE_txSPI:
        ptr = (unsigned char *)&txdata;
        ptr[dataIn.RX_FSRL] = dataIn.RX_DATAL;
        break;
      case READ_txSPI:
        ptr = (unsigned char *)&txdata;
        dataOut.TX_DATAL = ptr[dataIn.RX_FSRL];
        Debug_Send();
        break;
    }
    INTCONbits.PEIE = 0;    //Disable peripheral interrupt
    debugIntState = 0;
    INTCONbits.PEIE = 1;    //Enable peripheral interrupt
  }
}


/*******************************************************************************
* FUNCTION NAME: Serial_Refresh_PC_PWM_Panel
* PURPOSE:       Transmits PWM data (read from Master uP) to PC.
* CALLED FROM:   PC_Is_Not_In_Control
* ARGUMENTS:     none
*******************************************************************************/

void Serial_Refresh_PC_PWM_Panel(void)
{
  static unsigned char *ptr;

  ptr = (unsigned char *) &rxdata.oi_analog01;
  dataOut.TX_INDEX = Current_PWM_Index;
  dataOut.TX_CMD = READ_PWMs;  //Command to update PWMs on GUI
  dataOut.TX_DATAH = 0;
  dataOut.TX_DATAL = ptr[Current_PWM_Index++];
  if (Current_PWM_Index > 7) Current_PWM_Index = 0;
  Debug_Send();
}

/*******************************************************************************
* FUNCTION NAME: PC_Is_Not_In_Control
* PURPOSE:       Returns FALSE if PC is in control of PWMS else TRUE.
* CALLED FROM:   Process_Data_From_Master_uP
* ARGUMENTS:     none
*******************************************************************************/

unsigned char PC_Is_Not_In_Control(void)
{
  if (PC_Status->AutoSend)
    Serial_Refresh_PC_PWM_Panel();

  if (PC_Status->PCInControl)
    return FALSE;

  return TRUE;
}
