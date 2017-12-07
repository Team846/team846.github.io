#ifndef __EEPROM_H
#define __EEPROM_H
//***************************************************************************

//***************************************************************************
unsigned char EEPROM_read(unsigned int address);
void EEPROM_write(unsigned int address, unsigned char c);
//***************************************************************************

enum eeprom_addresses {
	kEPROMAdr_drive_type=0,
	kEPROMAdr_drive_method,
	kEPROMAdr_PrintOptions,
	kEPROMAdr_CameraPrint,
	kEPROMAdr_AutonomousMode,
	kEPROMAdr_LiftL0,
	kEPROMAdr_LiftL0_2,
	kEPROMAdr_LiftL1,
	kEPROMAdr_LiftL1_2,
	kEPROMAdr_LiftL2,
	kEPROMAdr_LiftL2_2,
	kEPROMAdr_LiftL3,
	kEPROMAdr_LiftL3_2,
	kEPROMAdr_AutonomousDist_Side,
	kEPROMAdr_AutonomousDist_Ctr,
	
	//more values here

	kEPROM_N
};

#endif //__EEPROM_H -- !!Nothing below this line!!
