#include "_eeprom.h"
#include <p18cxxx.h>

//********************************************************************************************
// Code based on sections 7.3 and 7.4 of the PIC18F6520/8520/6620/8620/6720/8720 manual
// D.Giandomenico
//********************************************************************************************
unsigned char EEPROM_ReadByte(unsigned int address)
{
	while (EECON1bits.WR)
		;	//wait here until prior write is finished.

	EEADRH = ((char *) &address)[1];	//high bits of Data Memory address
	EEADR =  ((char *) &address)[0];	//low  bits of Data Memory address
    EECON1bits.EEPGD = 0; 
    EECON1bits.CFGS = 0; 
    EECON1bits.RD = 1; 
	return EEDATA;
}
//********************************************************************************************
//EEPROM_write() may take several milliseconds to complete.
void EEPROM_WriteByte(unsigned int address, unsigned char c)
{
	while (EECON1bits.WR)
		;	//wait here until prior write is finished.

	EEADRH = ((char *) &address)[1];	//high bits of Data Memory address
	EEADR  = ((char *) &address)[0];	//low  bits of Data Memory address
	EEDATA=c;

	EECON1bits.EEPGD=0;	//Data EEPROM (not Program EEPROM)
	EECON1bits.CFGS=0;	//access EEPROM

	EECON1bits.WREN=1;	//Enable writes
	INTCONbits.GIE=0;	//Disable Interrupts
	EECON2=0x55;		//Required Sequence
	EECON2=0xAA;		//Required Sequence
	EECON1bits.WR=1;	//Required Sequence (begin write)
	INTCONbits.GIE=1;	//Reenable Interrupts

	EECON1bits.WREN=0;	//Disable writes
}
//********************************************************************************************

int EEPROM_ReadInt(unsigned int address)
{
	int value;
	((char*) &value)[0] = EEPROM_ReadByte(address+0);
	((char*) &value)[1] = EEPROM_ReadByte(address+1);
	return value;
}
void EEPROM_WriteInt(unsigned int address, int value)
{
	EEPROM_WriteByte(address+0, ((char*) &value)[0]);
	EEPROM_WriteByte(address+1, ((char*) &value)[1]);
}