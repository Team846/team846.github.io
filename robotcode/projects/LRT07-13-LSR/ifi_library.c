/*******************************************************************************
* FILE NAME: ifi_library.c
*
* DESCRIPTION:
*  This file contains the routines for high priority interrupt handling,
*  SPI communications, and bootloader communications.  
*
* USAGE:
*  This file should not be modified at all by the user.
*******************************************************************************/

#include <spi.h>
#include "ifi_default.h"
#include "ifi_aliases.h"

#define RESET_MODE    1
#define RUN_MODE      2

/* local copy of SSPCON1bits so that user cannot see it in ifi_picdefs.h */
extern volatile near struct {
  unsigned SSPM0:1;
  unsigned SSPM1:1;
  unsigned SSPM2:1;
  unsigned SSPM3:1;
  unsigned CKP:1;
  unsigned SSPEN:1;
  unsigned SSPOV:1;
  unsigned WCOL:1;
} SSPCON1bits;


extern packed_struct statusflag;
extern void UpdateLocalPWMDirection(unsigned char pwm_mask);
extern void GetDataFromMaster(unsigned char *ptr);
extern void SendDataToMaster(unsigned char *ptr);

#ifdef _WPI_LIBRARY
extern void Fast_Interrupt_Callback(void);
#endif

#if defined (_SNOOP_ON_COM1) | defined (_SNOOP_ON_COM2)
extern void CheckUartInts(void);
extern void Init_Serial_Snoop(void);
extern void Connect_SPI_Pointers(unsigned char *spi_tx_ptr, unsigned char *spi_rx_ptr);
extern void Send_PWM_To_PC(unsigned char idx);
extern unsigned char PC_Controls_Pwms;
static unsigned char idx;
#endif
void Prep_SPI_4_First_Byte(void);
void Check_4_Violations(tx_data_ptr ptr);
void Handle_Spi_Int (void);

extern volatile unsigned char gTX_BUFF0[];     /*in Assembly file*/
extern volatile unsigned char gTX_BUFF1[];     /*in Assembly file*/
extern volatile unsigned char gRX_BUFF0[];     /*in Assembly file*/
extern volatile unsigned char gRX_BUFF1[];     /*in Assembly file*/
extern unsigned char PWMdisableMask;  /*in Assembly file*/

static unsigned char *xferbufr;
static volatile unsigned char *txferbufr;
static volatile unsigned char *rxferbufr;
static unsigned char I;
static volatile unsigned char count;
static unsigned char temp;
static unsigned char *rxPtr;
static unsigned char *txPtr;

/*******************************************************************************
* FUNCTION NAME: InterruptVectorHigh
* PURPOSE:       This vector jumps to the appropriate high pri. interrupt handler.
* CALLED FROM:   High priority interrupt vector
* ARGUMENTS:     none
* RETURNS:       void
*******************************************************************************/
#pragma code InterruptVectorHigh = HIGH_INT_VECTOR
void InterruptVectorHigh (void)
{
  _asm
    goto InterruptHandlerHigh  /*jump to interrupt routine*/
  _endasm
}


/*******************************************************************************
* FUNCTION NAME: InterruptHandlerHigh
* PURPOSE:       High priority interrupt handler
* This section handles the high priority interrupts, which include only SPI
* communications from the master microprocessor and interrupts from the
* serial line which tell it that the IFI Loader running on the PC is calling it.
* CALLED FROM:   this file, InterruptVectorHigh routine
* ARGUMENTS:     none
* RETURNS:       void
*******************************************************************************/
#pragma code
#pragma interruptlow InterruptHandlerHigh

void InterruptHandlerHigh ()
{
  if (INTCONbits.INT0IF)       /*SPI master says data is ready to send me*/
  { 
    Prep_SPI_4_First_Byte(); 
  }
  else if (PIR1bits.SSPIF)     /*SPI data is being transferred one byte at a time*/
  {   
    Handle_Spi_Int();
  }
#ifdef _WPI_INTERFACE
  else
    Fast_Interrupt_Callback();
#endif

}


/*******************************************************************************
* FUNCTION NAME: Setup_Spi_Slave
* PURPOSE:       Initialize this user processor as a SPI communications slave.
* CALLED FROM:   this file, Initialize_Registers routine
* ARGUMENTS:     none
* RETURNS:       void
*******************************************************************************/
void Setup_Spi_Slave(void)
{
  OpenSPI(SLV_SSOFF,MODE_01,SMPMID);
}


/*******************************************************************************
* FUNCTION NAME: Initialize_Registers
* PURPOSE:       Configure necessary registers.
* CALLED FROM:   this file, IFI_Initialization routine
* ARGUMENTS:     none
* RETURNS:       void
*******************************************************************************/
void Initialize_Registers (void)
{
  CMCON = 0x07;          /*Turn comparators off*/
  PORTA = PORTB = PORTC = PORTD = PORTE = PORTF = PORTG = PORTH = PORTJ = 0;
  LATA = LATB = LATC = LATD = LATE = LATF = LATG = LATH = LATJ = 0;
  TRISA = 0x2F;
  TRISB = 0xFF;
  TRISC = 0x9C;
  TRISD = 0x00;
  TRISE = 0x00;           /*FRC BOARD*/
  TRISF = 0xFF;
#ifdef _FRC_NC1
  TRISG = 0x0D;
  TRISJ = 0x00;
#else
  TRISG = 0x04;           /* make sure RX2 is an input */
  TRISJ = 0x0E;
#endif
  TRISH = 0xFF;
  ADCON1 = 0x00;          /*all analog inputs; no digital*/
  INTCON2bits.NOT_RBPU = 0;  /*enable PORTB pull-ups (for bootloader interrupt)*/
  PSPCONbits.PSPMODE = 0;
  MEMCONbits.EBDIS = 1;   /*use PORTD for I/Os, not for external memory interface*/

  IPR1 = IPR2 = 0;
  PIE1 = PIE2 = 0;
  IPR3 = PIE3 = 0;        /*the 18F6520 has two extra Interrupt registers*/  
  Setup_Spi_Slave();
  
  IPR1bits.SSPIP = 1;     /*make SPI interrupt high priority*/
  RCONbits.IPEN = 1;
  PIR1bits.SSPIF = 0;
  PIE1bits.SSPIE = 1;
  TRISBbits.TRISB1 = 1;   /*make INT1 (RB1) an input*/

  INTCON3 = 0;
  INTCONbits.INT0IE = 1;  /*SPI Master Interrupt Line*/
  INTCON3bits.INT2IP = 0; /*Set INT2 to low priority*/
  INTCON3bits.INT2IE = 0; /*not used*/
  INTCON2bits.INT3P = 0;  /*Set INT3 to low priority*/
  INTCON3bits.INT3IE = 0; /*not used*/
  INTCON2 = 0;            /*INT0 / INT1 / INT2 / INT3 on falling edge*/
  INTCONbits.PEIE = 1;    /*Enable peripheral interrupt*/
  INTCONbits.GIE  = 1;    /*Enable global interrupts*/
}


/*******************************************************************************
* FUNCTION NAME: IFI_Initialization
* PURPOSE:       Configure registers and initializes the SPI RX/TX buffers.
* CALLED FROM:   main.c
* ARGUMENTS:     none
* RETURNS:       void
*******************************************************************************/
void IFI_Initialization (void)
{
  unsigned char I;
  
  Initialize_Registers();
  
  statusflag.TX_BUFFSELECT = 0;
  statusflag.RX_BUFFSELECT = 0;
  txPtr = (unsigned char *) &txdata.rc_pwm01;
  for (I=0;I<16;I++)
  {
    txPtr[I] = 0x7f;
  }
  txdata.packetnum = 0;
  txdata.current_mode = RESET_MODE;  
  txdata.control = 0xC2;     /*Master Must see a 0xC2*/
  statusflag.FIRST_TIME = 1;
  rxdata.oi_swA_byte.allbits = 0;
  rxdata.oi_swB_byte.allbits = 0;
  txdata.LED_byte1.data = 0;
  txdata.user_byte1.allbits = 0;
  txdata.user_byte2.allbits = 0;


#ifndef _DONT_USE_TMR0
    T0CON = 0x01; /* 1:4 Prescale */
    TMR0H = 0x32; /* Pre-load TMR1 to overflow after 21ms */
    TMR0L = 0xCF;
    T0CONbits.TMR0ON = 0; /* Turn timer off */
#endif

  Putdata(&txdata);             /*Need for initialization*/
  statusflag.TX_UPDATED = 0;    /*reset these to update latest data later*/
  Putdata(&txdata);             /*put data in both buffers*/
  statusflag.TX_UPDATED = 0;    /*reset these to update latest data later*/
}

/*******************************************************************************
* FUNCTION NAME: User_Proc_Is_Ready
* PURPOSE:       This routine informs the master processor that all user 
*                initalization has been complete.  The call to this routine must 
*                always be the last line of User_Initialization.  
* CALLED FROM:   user_routines.c
* ARGUMENTS:     none
* RETURNS:       void 
*******************************************************************************/
void User_Proc_Is_Ready (void)
{
  /* DNLD_ACTIVE (PortA4): is pulled high since the master has pullups on PORTB.*/
  PORTCbits.RC1 = 1;        /*BOOT_ACTIVE:Tell Master I'm up*/
  PORTAbits.RA4 = 1;  
  TRISCbits.TRISC1 = 0;     /*Output*/
  TRISAbits.TRISA4 = 0;     /*Output*/
}

/*******************************************************************************
* FUNCTION NAME: Prep_SPI_4_First_Byte
* PURPOSE:       Prepare for first byte of SPI data.
* CALLED FROM:   this file, InterruptHandlerHigh routine
* ARGUMENTS:     none
* RETURNS:       void
*******************************************************************************/
void Prep_SPI_4_First_Byte(void)
{
  INTCONbits.INT0IF = 0;
  count = SPI_XFER_SIZE;
  
  IPR1bits.SSPIP = 1;    /*force SPI interrupt high priority*/

  /*we're double buffering so we want to select the opposite buffer from the Getdata routine*/
  if (statusflag.RX_BUFFSELECT)
    rxferbufr = gRX_BUFF1;
  else
    rxferbufr = gRX_BUFF0;

  if (statusflag.NEW_SPI_DATA == 0)  
    statusflag.RX_BUFFSELECT ^= 1;  /* Only advance if RX buffer is read */

  /*we're double buffering so we want to select the opposite buffer from the Putdata routine*/
  if (statusflag.TX_BUFFSELECT)
    txferbufr = gTX_BUFF0;
  else
    txferbufr = gTX_BUFF1;
  
  temp = SSPBUF;          /*clear buffer out*/
  SSPBUF = *txferbufr++;  /*prep first byte*/

} /*end Prep_SPI_4_First_Byte*/


/*******************************************************************************
* FUNCTION NAME: Handle_Spi_Int
* PURPOSE:       Get data from the master microprocessor over the SPI bus.
* CALLED FROM:   this file, InterruptHandlerHigh routine
* ARGUMENTS:     none
* RETURNS:       void
*******************************************************************************/
void Handle_Spi_Int (void)
{
  PIR1bits.SSPIF = 0;
  temp = SSPBUF;
  if (count > 0)
  {
    *rxferbufr++ = temp;
    temp = *txferbufr++;
    SSPBUF = temp;
    count--;
    if (count == 0) 
    {
      txdata.current_mode = RUN_MODE;    /*Now in Run Mode */
      statusflag.TX_UPDATED = 0;         /*Indicate it's time to update tx buffer */
      statusflag.NEW_SPI_DATA = 1;
#ifndef _DONT_USE_TMR0
    TMR0H = 0x32; /* Pre-load TMR1 to overflow after 21ms */
    TMR0L = 0xCF;
      INTCONbits.TMR0IF = 0;
      T0CONbits.TMR0ON = 1; /* Turn timer on */
#endif
    }
  }
  else
    SSPBUF = 0;
}  /*end Handle_Spi_Int*/


/*******************************************************************************
* FUNCTION NAME: Getdata
* PURPOSE:       Retrieve data from the SPI receive buffer sent by the master 
*                microprocessor.  This routine takes 14.8 us to complete.
* CALLED FROM:   user_routines(_fast).c
* ARGUMENTS:     
*     Argument       Type           IO   Description
*     --------       -----------    --   -----------
*          ptr       rx_data_ptr    I    pointer to the receive buffer
* RETURNS:       void
*******************************************************************************/
/* We are double buffering to protect the data.  
 * Similar to the SERIN command in PBASIC.
 */

void Getdata(rx_data_ptr ptr)
{
  if (statusflag.NEW_SPI_DATA == 0)  /* Only update buffer once per 26.2ms */
    return;

  GetDataFromMaster((unsigned char *)ptr);

  PIE1bits.SSPIE = 0;
  statusflag.NEW_SPI_DATA = 0;
  PIE1bits.SSPIE = 1;
}

/*******************************************************************************
* FUNCTION NAME: Putdata
* PURPOSE:       Fill the transmit buffer with data to send out to the master 
*                microprocessor. This routine takes 23 us to complete.
* CALLED FROM:   user_routines(_fast).c
* ARGUMENTS:     
*     Argument       Type           IO   Description
*     --------       -----------    --   -----------
*          ptr       tx_data_ptr    I    pointer to the transmit buffer
* RETURNS:       void
*******************************************************************************/
/* We are double buffering to protect the data.  
 * Similar to the SEROUT command in PBASIC.
 */
void Putdata(tx_data_ptr ptr)   
{
  if (statusflag.TX_UPDATED)    /* Only update buffer once per 26.2ms */
    return;

  if (ptr->current_mode == RUN_MODE) /* Only check in running mode */
    Check_4_Violations(ptr);  //takes 11us
  ptr->cmd_byte1 &= 0x7F;     /* Don't allow this byte to be 0xFF */
  SendDataToMaster((unsigned char *)ptr);

  PIE1bits.SSPIE = 0;
  statusflag.TX_BUFFSELECT ^= 1;
  statusflag.TX_UPDATED = 1;
  PIE1bits.SSPIE = 1;
  txdata.packetnum++;
}

/*******************************************************************************
* FUNCTION NAME: Clear_SPIdata_flag
* PURPOSE:       Clears the SPI data flag so that  
* CALLED FROM:   user_routines_fast.c
* ARGUMENTS:     none
* RETURNS:       void
*******************************************************************************/
void Clear_SPIdata_flag(void)
{
  PIE1bits.SSPIE = 0;
  statusflag.NEW_SPI_DATA = 0;
  PIE1bits.SSPIE = 1;
}

/*******************************************************************************
* FUNCTION NAME: Setup_PWM_Output_Type
* PURPOSE:       
* CALLED FROM:   user_routines.c
* ARGUMENTS:     
*     Argument       Type           IO   Description
*     --------       -----------    --   -----------
*     pwmSpec1       int            I    pwm13 type definition (IFI_PWM or USER_CCP)
*     pwmSpec2       int            I    pwm14 type definition (IFI_PWM or USER_CCP)
*     pwmSpec3       int            I    pwm15 type definition (IFI_PWM or USER_CCP)
*     pwmSpec4       int            I    pwm16 type definition (IFI_PWM or USER_CCP)
* RETURNS:       void
*******************************************************************************/
void Setup_PWM_Output_Type(int pwmSpec1,int pwmSpec2,int pwmSpec3,int pwmSpec4)
{
  PWMdisableMask = 0;                  /* Default to normal PWMs */
  if (pwmSpec1 == USER_CCP)            /* If USER_CCP then disable PWM bit0 */
    PWMdisableMask |= 0x01;   
  else
    CCP2CON = 0x00;
  if (pwmSpec2 == USER_CCP)            
    PWMdisableMask |= 0x02;
  else
    CCP3CON = 0x00;
  if (pwmSpec3 == USER_CCP)            
    PWMdisableMask |= 0x04;
  else
    CCP4CON = 0x00;
  if (pwmSpec4 == USER_CCP)            
    PWMdisableMask |= 0x08;
  else
    CCP5CON = 0x00;
}

/*******************************************************************************
* FUNCTION NAME: Check_4_Violations
* PURPOSE:       
* CALLED FROM:   this file, Prep_SPI_4_First_Byte routine
* ARGUMENTS:     
*     Argument       Type           IO   Description
*     --------       -----------    --   -----------
* RETURNS:       void
*******************************************************************************/
void Check_4_Violations(tx_data_ptr ptr)
{
  ptr->error_code = 0x00;
  ptr->warning_code = 0x00;

  /* WARNINGS */
  if (SSPCON1bits.WCOL)   /* SPI write collision occured due to serial port activity */
  {
    SSPCON1bits.WCOL = 0;
    ptr->warning_code = 1;
  }
  else if ( (INTCON2bits.RBPU) && (INTCONbits.RBIE) )
  {                                    /* PORTB pullups disabled and PORTB interrupts enabled */
    ptr->warning_code = 2;
  }
  else if (INTCON3bits.INT1IE)         /* RB1/INT1 pin is the USR_TTL_CTS output */
  {                                    /* so its interrupt should never be enabled */
    LATHbits.LATH7 = 0;
    ptr->warning_code = 3;
  }

/* ERRORS */
  if ((SSPCON1 & 0x25) != 0x25)         /* SPI configuration */
  {
    ptr->error_code = 1;
    ptr->warning_code = SSPCON1;
  }
  else if (SSPCON2 != 0x00)             /* I2C configuration */
  {
    ptr->error_code = 2;
    ptr->warning_code = SSPCON2;
  }
  else if (RCONbits.IPEN != 1)          /* Interrupt Priority disabled */
  {
    ptr->error_code = 3;
    ptr->warning_code = RCON;
  }
  else if (PIE1bits.SSPIE != 1)         /* SPI Interrupt disabled */
  {
    ptr->error_code = 4;
    ptr->warning_code = PIE1;
  }
  else if (MEMCON != 0x00)              /* MEMCON configuration */
  {
    ptr->error_code = 5;
    ptr->warning_code = MEMCON;
  }
#if defined (_SNOOP_ON_COM1) | defined (_SNOOP_ON_COM2)
#if defined (_SNOOP_ON_COM1)
  else if (IPR1 != 0x38)               
#else
  else if (IPR1 != 0x8)                 /* only SPI Interrupt and */
#endif
  {                                     /* USART1 RX/TX high priority */
    ptr->error_code = 6;
    ptr->warning_code = IPR1;
  }
#else
  else if (IPR1 != 0x08)                /* only SPI Interrupt high priority */
  {
    ptr->error_code = 6;              /* IPR1 high priority Interrupt enabled */
    ptr->warning_code = IPR1;
  }
#endif
  else if (IPR2 != 0x00)                /* all low priority */
  {
    ptr->error_code = 7;              /* IPR2 high priority Interrupt enabled */
    ptr->warning_code = IPR2;
  }
#if defined (_SNOOP_ON_COM2)
  else if (IPR3 != 0x30)                /* all low priority */
  {
    ptr->error_code = 8;              /* IPR3 high priority Interrupt enabled */
    ptr->warning_code = IPR3;
  }
#else
  else if (IPR3 != 0x00)                /* all low priority */
  {
    ptr->error_code = 8;              /* IPR3 high priority Interrupt enabled */
    ptr->warning_code = IPR3;
  }
#endif
  else if ((INTCON & 0x10) != 0x10)     /* INT0 Enable bit */
  {
    ptr->error_code = 9;              /* INTCON configuration */
    ptr->warning_code = INTCON;
  }
  else if ((INTCON2 & 0x47) != 0x00)    /* INTCON2 configuration */
  {
    ptr->error_code = 10;
    ptr->warning_code = INTCON2;
  }
  else if ((INTCON3 & 0xC0) != 0x00)    /* INTCON3 configuration */
  {
    ptr->error_code = 11;
    ptr->warning_code = INTCON3;
  }
  else if (TRISAbits.TRISA4)            /* A4 should be an output */
  {
    ptr->error_code = 12;
    ptr->warning_code = TRISA;
  }
  else if (TRISBbits.TRISB0 != 1)       /* B0 should be an input */
  {
    ptr->error_code = 13;
    ptr->warning_code = TRISB;
 }
  else if ((TRISC & 0x3E) != 0x1C)      /* PORTC configuration */
  {
    ptr->error_code = 14;
    ptr->warning_code = TRISC;
  }
  else if (TRISFbits.TRISF7 != 1)       /* F7 should be an input */
  {
    ptr->error_code = 15;
    ptr->warning_code = TRISF;
  }

}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
