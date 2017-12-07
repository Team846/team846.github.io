/*******************************************************************************
*
*	TITLE:		camera_menu.c
*
*	VERSION:	0.1 (Beta)                           
*
*	DATE:		04-Oct-2005
*
*	AUTHOR:		R. Kevin Watson
*				kevinw@jpl.nasa.gov
*
*	COMMENTS:	This code will only work with the "bells and whistles" version
*				of camera.c and camera.h.
*
*				You are free to use this source code for any non-commercial
*				use. Please do not make copies of this source code, modified
*				or un-modified, publicly available on the internet or elsewhere
*				without permission. Thanks.
*
*				Copyright ©2005-2006 R. Kevin Watson. All rights are reserved.
*
********************************************************************************
*
*	CHANGE LOG:
*
*	DATE         REV  DESCRIPTION
*	-----------  ---  ----------------------------------------------------------
*	04-Oct-2005  0.1  RKW - Original code.
*
*******************************************************************************/
#include <stdio.h>
#include "ifi_default.h"
#include "ifi_aliases.h"
#include "serial_ports.h"
#include "eeprom.h"
#include "camera.h"
#include "camera_menu.h"
#include "tracking.h"


// temporary local copy of the camera configuration
// structure that can be safely edited without affecting
// the master global copy of the camera configuration
// structure
Camera_Config_Data_Type Temp_Camera_Config_Data;

/*******************************************************************************
*
*	FUNCTION:		Camera_Menu()
*
*	PURPOSE:		This function manages the camera menu functionality,
*					which is used to enter camera initialization and
*					tracking parameters using a terminal attached to the
*					programming port of your robot controller.		
*
*	CALLED FROM:	user_routines.c/Process_Data_From_Master_uP()
*
*	PARAMETERS:		none
*
*	RETURNS:		0: the user is done and has exited.
*					1: the user is still working with the menu.
*
*	COMMENTS:
*
*******************************************************************************/
unsigned char Camera_Menu(void)
{
	// when equal to one, this variable indicates the camera menu is active
	static unsigned char camera_menu_active = 0;
	static unsigned char menu_state = CM_INITIALIZE;
	static unsigned char char_buffer[6];
	static unsigned char char_buffer_index;
	static unsigned char offset;
	static unsigned char current_value;
	unsigned int user_entered_value;
	unsigned char terminal_char;
	unsigned char byte;
	static unsigned char i;

	switch(menu_state)
	{
		// initialize the camera menu state machine
		case CM_INITIALIZE:
			camera_menu_active = 1;
			// get a local copy of the camera configuration
			for(i = 0; i < sizeof(Camera_Config_Data_Type); i++)
			{
				byte = ((unsigned char *)(&Camera_Config_Data))[i];
				((unsigned char *)(&Temp_Camera_Config_Data))[i] = byte;
			}
			menu_state = CM_MENU_PRINT;
			break;

		// print main menu
		case CM_MENU_PRINT:
			Menu_Print();
			menu_state = CM_MENU_HANDLER;
			break;

		// handle user input for the main menu
		case CM_MENU_HANDLER:
			terminal_char = Read_Terminal_Serial_Port();
			if(terminal_char >= 'a' && terminal_char <= 's')
			{
				// prompt user for input
				menu_state = CM_USER_PROMPT;
				// calculate offset into the configuration structure to change
				offset = terminal_char - 'a' + 2;
			}
			else if(terminal_char == 'u')
			{
				// restart the camera
				menu_state = CM_RESTART_CAMERA;
			}
			else if(terminal_char == 'v')
			{
				// load default values from camera.h
				menu_state = CM_GET_DEFAULT_VALUES;
			}
			else if(terminal_char == 'w')
			{
				// save configuration to EEPROM
				menu_state = CM_SAVE_TO_EEPROM;
			}
			else if(terminal_char == 'x')
			{
				// reinitialize when/if Camera_Menu() is called again
				menu_state = CM_INITIALIZE;
				camera_menu_active = 0;
			}
			break;

		// prompt user for a new value
		case CM_USER_PROMPT:
			// reset buffer index
			char_buffer_index = 0;
			// flush user input queue
			while(Read_Terminal_Serial_Port() != 0);
			// prompt user for input
			printf("\r\nEnter new value (preceed hexadecimal numbers with 0x): ");
			// get and process input
			menu_state = CM_GETTING_USER_INPUT;
			break;

		// get user input for the new value
		case CM_GETTING_USER_INPUT:
			// read a byte from the terminal serial port
			terminal_char = Read_Terminal_Serial_Port();
			// make sure it's a decimal number or hexadecimal number 
			// or 'x'/'X' and we don't overflow the buffer
			if(((terminal_char >= '0' && terminal_char <= '9') ||
				(terminal_char >= 'a' && terminal_char <= 'f') ||
				(terminal_char >= 'A' && terminal_char <= 'F') || 
				(terminal_char == 'x' || terminal_char == 'X')) && 
				char_buffer_index < sizeof(char_buffer))
			{
				// if necessary, convert to upper case letter
				if((terminal_char >= 'a' && terminal_char <= 'f') || terminal_char == 'x')
				{
					// 'A' - 'a' is the fixed number (==32) that is subtracted from
					// a lower-case letter to convert it to an upper-case letter
					terminal_char -= 'a' - 'A';
				}
				// echo the character back to the terminal
				Write_Terminal_Serial_Port(terminal_char);
				// add the new character to the buffer
				char_buffer[char_buffer_index] = terminal_char;
				// increment buffer index
				char_buffer_index++;			
			}
			// did the user hit the return key?
			else if(terminal_char == '\r')
			{
				// did the user enter a hexadecimal number?
				if(char_buffer[0] == '0' && char_buffer[1] == 'X')
				{
					if(char_buffer_index > 4)
					{
						user_entered_value = 256; // make sure valid number logic (below) handles the situation
					}
					else if(char_buffer_index == 4) // two hexadecimal digits entered?
					{
						// calculate the value of the most significant hexadecimal digit
						if(char_buffer[2] >= '0' && char_buffer[2] <= '9')
						{
							user_entered_value = 16 * (char_buffer[2] - '0'); 
						}
						else if(char_buffer[2] >= 'A' && char_buffer[2] <= 'F')
						{
							user_entered_value = 16 * ((char_buffer[2] - 'A') + 10);
						}
						else if(char_buffer[2] == 'X') // user entered 'x' or 'X'
						{
							user_entered_value = 256; // make sure valid number logic (below) handles the situation
						}

						// calculate the value of the least significant hexadecimal digit
						if(char_buffer[3] >= '0' && char_buffer[3] <= '9')
						{
							user_entered_value += char_buffer[3] - '0'; 
						}
						else if(char_buffer[3] >= 'A' && char_buffer[3] <= 'F')
						{
							user_entered_value += char_buffer[3] - 'A' + 10;
						}
						else if(char_buffer[3] == 'X') // user entered 'x' or 'X'
						{
							user_entered_value += 256; // make sure valid number logic (below) handles the situation
						}

					}
					else if(char_buffer_index == 3) // one hexadecimal digit entered?
					{
						// calculate the value of the hexadecimal digit
						if(char_buffer[2] >= '0' && char_buffer[2] <= '9')
						{
							user_entered_value = char_buffer[2] - '0'; 
						}
						else if(char_buffer[2] >= 'A' && char_buffer[2] <= 'F')
						{
							user_entered_value = char_buffer[2] - 'A' + 10;
						}
						else if(char_buffer[2] == 'X') // user entered 'x' or 'X'
						{
							user_entered_value += 256; // make sure valid number logic (below) handles the situation
						}
					}
					else // no hexadecimal digits entered (i.e., user just entered 0x<return>)?
					{
						user_entered_value = 256; // make sure valid number logic (below) handles the situation
					}
				}
				else // a decimal number was entered
				{
					// consider additional error checking for letters A-F here

					if(char_buffer_index > 3) // too many digits entered?
					{
						user_entered_value = 256; // make sure valid number logic (below) handles the situation
					}					
					else if(char_buffer_index == 3) // three decimal digits entered?
					{
						user_entered_value = 100*(unsigned int)(char_buffer[0]-'0');
						user_entered_value += 10*(unsigned int)(char_buffer[1]-'0');
						user_entered_value += (unsigned int)(char_buffer[2]-'0');
					}
					else if(char_buffer_index == 2) // two decimal digits entered?
					{
						user_entered_value = 10*(char_buffer[0]-'0');
						user_entered_value += (char_buffer[1]-'0');
					}
					else if(char_buffer_index == 1) // one decimal digit entered?
					{
						user_entered_value = char_buffer[0] - '0';
					}
					else if(char_buffer_index == 0) // no decimal digits entered (i.e., user just entered <return>)?
					{
						// use the current value
						user_entered_value = ((unsigned char *)(&Temp_Camera_Config_Data))[offset];
					}
				}

				// make sure the user entered a valid number
				if(user_entered_value <= 255)
				{
					// save the value using the offset calculated in CM_MENU_HANDLER, above
					((unsigned char *)(&Temp_Camera_Config_Data))[offset] = (unsigned char)user_entered_value;
					// back to the main menu
					menu_state = CM_MENU_PRINT;
				}
				else
				{
					// bad value entered; ask again
					menu_state = CM_USER_INPUT_ERROR;
				}
			}
			break;

		// inform user that the value entered is bogus
		case CM_USER_INPUT_ERROR:
			// DOH!
			printf("\r\nError: Number out of range.\r\n");
			// ask again for input
			menu_state = CM_USER_PROMPT;
			break;

		// load default values from camera.h for editing
		case CM_GET_DEFAULT_VALUES:
			Temp_Camera_Config_Data.R_Min = R_MIN_DEFAULT;
			Temp_Camera_Config_Data.R_Max = R_MAX_DEFAULT;
			Temp_Camera_Config_Data.G_Min = G_MIN_DEFAULT;
			Temp_Camera_Config_Data.G_Max = G_MAX_DEFAULT;
			Temp_Camera_Config_Data.B_Min = B_MIN_DEFAULT;
			Temp_Camera_Config_Data.B_Max = B_MAX_DEFAULT;
			Temp_Camera_Config_Data.NF = NF_DEFAULT;
			Temp_Camera_Config_Data.AGC = AGC_DEFAULT;
			Temp_Camera_Config_Data.BLU = BLU_DEFAULT;
			Temp_Camera_Config_Data.RED = RED_DEFAULT;
			Temp_Camera_Config_Data.SAT = SAT_DEFAULT;
			Temp_Camera_Config_Data.BRT = BRT_DEFAULT;
			Temp_Camera_Config_Data.AEC = AEC_DEFAULT;
			Temp_Camera_Config_Data.COMA = COMA_DEFAULT;
			Temp_Camera_Config_Data.COMB = COMB_DEFAULT;
			Temp_Camera_Config_Data.COMI = COMI_DEFAULT;
			Temp_Camera_Config_Data.EHSH = EHSH_DEFAULT;
			Temp_Camera_Config_Data.EHSL = EHSL_DEFAULT;
			Temp_Camera_Config_Data.COMJ = COMJ_DEFAULT;

			menu_state = CM_MENU_PRINT;
			break;

		// update global camera configuration structure and save it to EEPROM
		case CM_SAVE_TO_EEPROM:
			// move local copy of the camera configuration to the global copy
			for(i = 0; i < sizeof(Camera_Config_Data_Type); i++)
			{
				byte = ((unsigned char *)(&Temp_Camera_Config_Data))[i];
				((unsigned char *)(&Camera_Config_Data))[i] = byte;
			}
			// write configuration to EEPROM
			Save_Camera_Configuration(CAMERA_CONFIG_EEPROM_ADDRESS);

			printf("\r\nSaving to EEPROM...");

			// next state provides a EEPROM write delay 
			menu_state = CM_EEPROM_WRITE_DELAY;
			break;

		// allow EEPROM_Write_Handler() to write all of 
		// the data before returning to the main menu
		case CM_EEPROM_WRITE_DELAY:

			// wait here until all the EEPROM data is written
			if(EEPROM_Queue_Free_Space() == EEPROM_QUEUE_SIZE)
			{
				printf("Done\r\n");

				// back to the main menu...
				menu_state = CM_MENU_PRINT;
			}	
			break;

		// ask Camera_Handler() to restart the camera and...
		case CM_RESTART_CAMERA:
			printf("\r\nRestarting Camera...\r\n");
			// ask Camera_Handler() to restart the camera
			Restart_Camera();
			// allow camera to restart before going back to main menu
			menu_state = CM_RESTART_CAMERA_DELAY;
			break;

		// ...wait for the camera to finish restarting before printing the main menu
		case CM_RESTART_CAMERA_DELAY:
			// wait here until camera is running again
			if(Get_Camera_State() == 1)
			{
				// back to the main menu...
				menu_state = CM_MENU_PRINT;
			}
			break;
	}

	return(camera_menu_active);
}

/*******************************************************************************
*
*	FUNCTION:		Menu_Print()
*
*	PURPOSE:		Sends the menu of camera operating parameters to
*					the terminal screen. 
*
*	CALLED FROM:	Camera_Menu(), above
*
*	PARAMETERS:		none
*
*	RETURNS:		nothing
*
*	COMMENTS:
*
*******************************************************************************/
void Menu_Print(void)
{
	printf("\r\n");
	printf("\r\nCalibration parameters:\r\n");
	printf("[a] Change R_Min = %u (0x%02X)\r\n",(unsigned int)Temp_Camera_Config_Data.R_Min,
												(unsigned int)Temp_Camera_Config_Data.R_Min);
	printf("[b] Change R_Max = %u (0x%02X)\r\n", (unsigned int)Temp_Camera_Config_Data.R_Max,
												(unsigned int)Temp_Camera_Config_Data.R_Max);
	printf("[c] Change G_Min = %u (0x%02X)\r\n", (unsigned int)Temp_Camera_Config_Data.G_Min,
												(unsigned int)Temp_Camera_Config_Data.G_Min);
	printf("[d] Change G_Max = %u (0x%02X)\r\n", (unsigned int)Temp_Camera_Config_Data.G_Max,
												(unsigned int)Temp_Camera_Config_Data.G_Max);
	printf("[e] Change B_Min = %u (0x%02X)\r\n", (unsigned int)Temp_Camera_Config_Data.B_Min,
												(unsigned int)Temp_Camera_Config_Data.B_Min);
	printf("[f] Change B_Max = %u (0x%02X)\r\n", (unsigned int)Temp_Camera_Config_Data.B_Max,
												(unsigned int)Temp_Camera_Config_Data.B_Max);
	printf("[g] Change NF = %u (0x%02X)\r\n", (unsigned int)Temp_Camera_Config_Data.NF,
												(unsigned int)Temp_Camera_Config_Data.NF);
	printf("\r\nAdvanced options:\r\n");

	printf("[h] Change AGC = %u (0x%02X)\r\n", (unsigned int)Temp_Camera_Config_Data.AGC,
												(unsigned int)Temp_Camera_Config_Data.AGC);
	printf("[i] Change BLU = %u (0x%02X)\r\n", (unsigned int)Temp_Camera_Config_Data.BLU,
												(unsigned int)Temp_Camera_Config_Data.BLU);
	printf("[j] Change RED = %u (0x%02X)\r\n", (unsigned int)Temp_Camera_Config_Data.RED,
												(unsigned int)Temp_Camera_Config_Data.RED);
	printf("[k] Change SAT = %u (0x%02X)\r\n", (unsigned int)Temp_Camera_Config_Data.SAT,
												(unsigned int)Temp_Camera_Config_Data.SAT);
	printf("[l] Change BRT = %u (0x%02X)\r\n", (unsigned int)Temp_Camera_Config_Data.BRT,
												(unsigned int)Temp_Camera_Config_Data.BRT);
	printf("[m] Change AEC = %u (0x%02X)\r\n", (unsigned int)Temp_Camera_Config_Data.AEC,
												(unsigned int)Temp_Camera_Config_Data.AEC);
	printf("[n] Change COMA = %u (0x%02X)\r\n", (unsigned int)Temp_Camera_Config_Data.COMA,
												(unsigned int)Temp_Camera_Config_Data.COMA);
	printf("[o] Change COMB = %u (0x%02X)\r\n", (unsigned int)Temp_Camera_Config_Data.COMB,
												(unsigned int)Temp_Camera_Config_Data.COMB);
	printf("[p] Change COMI = %u (0x%02X)\r\n", (unsigned int)Temp_Camera_Config_Data.COMI,
												(unsigned int)Temp_Camera_Config_Data.COMI);
	printf("[q] Change EHSH = %u (0x%02X)\r\n", (unsigned int)Temp_Camera_Config_Data.EHSH,
												(unsigned int)Temp_Camera_Config_Data.EHSH);
	printf("[r] Change EHSL = %u (0x%02X)\r\n", (unsigned int)Temp_Camera_Config_Data.EHSL,
												(unsigned int)Temp_Camera_Config_Data.EHSL);
	printf("[s] Change COMJ = %u (0x%02X)\r\n", (unsigned int)Temp_Camera_Config_Data.COMJ,
												(unsigned int)Temp_Camera_Config_Data.COMJ);
	printf("\r\n");
	printf("[u] Restart camera\r\n");
	printf("[v] Load default values\r\n");
	printf("[w] Save changes\r\n");
	printf("[x] Exit\r\n");
	printf("\r\n");
}

/*******************************************************************************
*
*	FUNCTION:		Save_Camera_Configuration()
*
*	PURPOSE:		Writes the current camera configuration, as stored in
*					the Camera_Config_Data structure, to EEPROM.
*
*	CALLED FROM:	Camera_Menu(), above
*
*	PARAMETERS:		EEPROM address to save to.
*
*	RETURNS:		Nothing
*
*	COMMENTS:
*
*******************************************************************************/
void Save_Camera_Configuration(unsigned int eeprom_address)
{
	unsigned int i;
	unsigned char byte;
	unsigned int checksum;

	checksum = 0;

	// make sure the identification bytes are present in the structure
	Camera_Config_Data.Letter_B = 'B';
	Camera_Config_Data.Letter_P = 'P';

	// except for the checksum, read each byte of the camera 
	// configuration structure and write it to EEPROM
	for(i = 0; i < sizeof(Camera_Config_Data) - 1; i++)
	{
		// this ugly code allows the camera configuration data
		// structure to be addressed as an unsigned char array
		byte = ((unsigned char *)(&Camera_Config_Data))[i];

		// write the EEPROM
		EEPROM_Write(eeprom_address + i, byte);

		checksum += (unsigned int)byte;
	}

	// finish by writing the checksum
	EEPROM_Write(eeprom_address + sizeof(Camera_Config_Data) - 1, (unsigned char)checksum);
}
