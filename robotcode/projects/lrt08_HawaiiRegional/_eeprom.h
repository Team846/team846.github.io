#ifndef __EEPROM_H
#define __EEPROM_H
//***************************************************************************

unsigned char EEPROM_ReadByte(unsigned int address);
void EEPROM_WriteByte(unsigned int address, unsigned char c);

int EEPROM_ReadInt(unsigned int address);
void EEPROM_WriteInt(unsigned int address, int value);

enum eeprom_addresses {
	EEPROM_SERVO_DRIVE=0,
	EEPROM_DRIVE_METHOD,
	EEPROM_PRINT_OPTIONS,
	
	EEPROM_GYRO_BIAS,
	EEPROM_GYRO_BIAS1, // FIXME: this is an error-prone way to store ints
	
	EEPROM_LIFT_BASEPOS,
	EEPROM_LIFT_BASEPOS1,
	
	EEPROM_FORK_BASEPOS,
	EEPROM_FORK_BASEPOS1,
	
	EEPROM_HYBRID_DIST_INITIAL,
	EEPROM_HYBRID_DIST_STRAIGHT,
	EEPROM_HYBRID_DIST_LATERAL,
	
	//
	EEPROM_N
};


//this is not currently used; experimental [DG]
typedef struct
{
	char servo_drive;
	char drive_method;
	char print_options;
	unsigned int gyrobias;
	unsigned int lift_basepos;
	unsigned int fork_basepos;
}eprom;
#define mOffsetEPROM(field) ((int)( ((char*)&((eprom*)0)->field) - ((char*)(eprom*)0) ))
#define mSizeofEPROM(field) sizeof( (eprom*)0)->field )




#endif //__EEPROM_H
