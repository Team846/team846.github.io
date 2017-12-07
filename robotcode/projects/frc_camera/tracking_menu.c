/*******************************************************************************
*
*	TITLE:		tracking_menu.c
*
*	VERSION:	0.1 (Beta)                           
*
*	DATE:		18-Nov-2005
*
*	AUTHOR:		R. Kevin Watson
*				kevinw@jpl.nasa.gov
*
*	COMMENTS:	This code will only work with the "bells and whistles" version
*				of tracking.c and tracking.h.
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
*	18-Nov-2005  0.1  RKW - Original code.
*
*******************************************************************************/
#include <stdio.h>
#include "ifi_default.h"
#include "ifi_aliases.h"
#include "serial_ports.h"
#include "eeprom.h"
#include "camera.h"
#include "tracking.h"
#include "tracking_menu.h"

// temporary local copy of the tracking configuration
// structure that can be safely edited without affecting
// the master global copy of the tracking configuration
// structure
Tracking_Config_Data_Type Temp_Tracking_Config_Data;

/*******************************************************************************
*
*	FUNCTION:		Tracking_Menu()
*
*	PURPOSE:		This function manages the tracking menu functionality,
*					which is used to enter tracking parameters using a 
*					terminal attached to the programming port of your robot 
*					controller.		
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
unsigned char Tracking_Menu(void)
{
	// when equal to one, this variable indicates that the
	// tracking menu is active
	static unsigned char tracking_menu_active = 0;
	static unsigned char menu_state = TM_INITIALIZE;
	static unsigned char char_buffer[6];
	static unsigned char char_buffer_index;
	static unsigned char offset;
	static unsigned char selection;
	static unsigned char current_value;
	unsigned char terminal_char;
	unsigned char byte;
	static unsigned char i;
	int user_entered_value;
	int sign;

	switch(menu_state)
	{
		//
		// initialize the tracking menu state machine
		//
		case TM_INITIALIZE:
			tracking_menu_active = 1;
			// get a local copy of the tracking configuration
			for(i = 0; i < sizeof(Tracking_Config_Data_Type); i++)
			{
				byte = ((unsigned char *)(&Tracking_Config_Data))[i];
				((unsigned char *)(&Temp_Tracking_Config_Data))[i] = byte;
			}
			menu_state = TM_MENU_PRINT;
			break;

		//
		// print main menu
		//
		case TM_MENU_PRINT:
			Tracking_Menu_Print();
			menu_state = TM_MENU_HANDLER;
			break;

		//
		// handle user input for the main menu
		//
		case TM_MENU_HANDLER:
			terminal_char = Read_Terminal_Serial_Port();
			if(terminal_char >= 'a' && terminal_char <= 'q')
			{
				// prompt user for input
				menu_state = TM_USER_PROMPT;
				// save selection for use in error checking
				selection = terminal_char;
				// calculate offset into the configuration structure to change
				offset = terminal_char - 'a' + 2;
			}
			else if(terminal_char == 'u')
			{
				// display PWM menu
				PWM_Menu_Print();
				// goto interactive PWM menu
				menu_state = TM_PWM_MENU_HANDLER;
			}
			else if(terminal_char == 'v')
			{
				menu_state = TM_GET_DEFAULT_VALUES;
			}
			else if(terminal_char == 'w')
			{
				// save configuration to EEPROM
				menu_state = TM_SAVE_TO_EEPROM;
			}
			else if(terminal_char == 'x')
			{
				// reinitialize when/if Tracking_Menu() is called again
				menu_state = TM_INITIALIZE;
				tracking_menu_active = 0;
			}
			break;

		//
		// prompt user for a new value
		//
		case TM_USER_PROMPT:
			// reset buffer index
			char_buffer_index = 0;
			// flush user input queue
			while(Read_Terminal_Serial_Port() != 0);
			// prompt user for input
			printf("\r\nEnter new value: ");
			// get and process input
			menu_state = TM_GETTING_USER_INPUT;
			break;

		//
		// get user input for the new value
		//
		case TM_GETTING_USER_INPUT:
			// read a byte from the terminal serial port
			terminal_char = Read_Terminal_Serial_Port();
			// make sure it's a decimal number or +/- sign and we don't overflow the buffer
			if(((terminal_char >= '0' && terminal_char <= '9') || 
				terminal_char == '+' || terminal_char == '-') && 
				char_buffer_index < sizeof(char_buffer))
			{
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
				// did the user enter a negative number?
				if(char_buffer[0] == '-')
				{
					// remember the sign and convert to a positive number
					sign = -1;
					char_buffer_index--;
					//left shift our number
					for(i=0; i<sizeof(char_buffer); i++)
					{
						char_buffer[i] = char_buffer[i+1];
					}
				}
				else if(char_buffer[0] == '+')
				{
					// number is already positive
					sign = 1;
					char_buffer_index--;
					//left shift our number
					for(i=0; i<sizeof(char_buffer); i++)
					{
						char_buffer[i] = char_buffer[i+1];
					}
				}

				else
				{
					// if there's no sign, we'll assume it's a positive number
					sign = 1;
				}
				
				// consider checking to make sure user didn't enter a +/- sign elsewhere

				if(char_buffer_index > 3) // too many digits entered?
				{
					user_entered_value = 256; // make sure valid number logic (below) handles the situation
				}					
				else if(char_buffer_index == 3) // three decimal digits entered?
				{
					user_entered_value = 100*(unsigned int)(char_buffer[0]-'0');
					user_entered_value += 10*(unsigned int)(char_buffer[1]-'0'); 
					user_entered_value += (unsigned int)(char_buffer[2]-'0');
					user_entered_value *= sign;
				}
				else if(char_buffer_index == 2) // two decimal digits entered?
				{
					user_entered_value = 10*(char_buffer[0]-'0'); 
					user_entered_value += (char_buffer[1]-'0');
					user_entered_value *= sign;
				}
				else if(char_buffer_index == 1) // one decimal digit entered?
				{
					user_entered_value = char_buffer[0] - '0';
					user_entered_value *= sign;
				}
				else if(char_buffer_index == 0) // no decimal digits entered (i.e., user just entered <return> or '-')?
				{
					// use the current value
					user_entered_value = ((unsigned char *)(&Temp_Tracking_Config_Data))[offset];
				}

				// make sure the user entered value is in range for the selected option
				if(selection == 'f' || selection == 'n')
				{
					// the pan/tilt rotation signs can only be 1 or -1
					if(user_entered_value == 1 || user_entered_value == -1)
					{
						// save the value using the offset calculated in TM_MENU_HANDLER, above
						((char *)(&Temp_Tracking_Config_Data))[offset] = (char)user_entered_value;
						// back to the main menu
						menu_state = TM_MENU_PRINT;
					}
					else
					{
						// bad value entered; ask again
						menu_state = TM_USER_INPUT_ERROR;
					}
				}
				else
				{
					// everything else has a range of 0 to 255
					if(user_entered_value >= 0 && user_entered_value <= 255)
					{
						// save the value using the offset calculated in TM_MENU_HANDLER, above
						((unsigned char *)(&Temp_Tracking_Config_Data))[offset] = (unsigned char)user_entered_value;
						// back to the main menu
						menu_state = TM_MENU_PRINT;
					}
					else
					{
						// bad value entered; ask again
						menu_state = TM_USER_INPUT_ERROR;
					}
				}
			}
			break;

		//
		// inform user that the value entered is bogus
		//
		case TM_USER_INPUT_ERROR:
			// DOH!
			printf("\r\nError: Number out of range.\r\n");
			// ask again for input
			menu_state = TM_USER_PROMPT;
			break;

		//
		// handle user input for the PWM menu
		//
		case TM_PWM_MENU_HANDLER:
			terminal_char = Read_Terminal_Serial_Port();
			if(terminal_char == 'a')
			{
				// pan left one step
				PAN_SERVO -= 1;
			}
			if(terminal_char == 'b')
			{
				// pan left ten steps
				PAN_SERVO -= 10;
			}
			else if(terminal_char == 'c')
			{
				// pan right one step
				PAN_SERVO += 1;
			}
			else if(terminal_char == 'd')
			{
				// pan right ten steps
				PAN_SERVO += 10;
			}
			else if(terminal_char == 'e')
			{
				// capture pan min PWM value
				Temp_Tracking_Config_Data.Pan_Min_PWM = PAN_SERVO;
			}
			else if(terminal_char == 'f')
			{
				// capture pan center PWM value
				Temp_Tracking_Config_Data.Pan_Center_PWM = PAN_SERVO;
			}
			else if(terminal_char == 'g')
			{
				// capture pan max PWM value
				Temp_Tracking_Config_Data.Pan_Max_PWM = PAN_SERVO;
			}
			else if(terminal_char == 'h')
			{
				// tilt up one step
				TILT_SERVO += 1;
			}
			else if(terminal_char == 'i')
			{
				// tilt up ten steps
				TILT_SERVO += 10;
			}
			else if(terminal_char == 'j')
			{
				// tilt down one step
				TILT_SERVO -= 1;
			}
			else if(terminal_char == 'k')
			{
				// tilt down ten steps
				TILT_SERVO -= 10;
			}
			else if(terminal_char == 'l')
			{
				// capture tilt min PWM value
				Temp_Tracking_Config_Data.Tilt_Min_PWM = TILT_SERVO;
			}
			else if(terminal_char == 'm')
			{
				// capture tilt center PWM value
				Temp_Tracking_Config_Data.Tilt_Center_PWM = TILT_SERVO;
			}
			else if(terminal_char == 'n')
			{
				// capture tilt max PWM value
				Temp_Tracking_Config_Data.Tilt_Max_PWM = TILT_SERVO;
			}
			else if(terminal_char == 'o')
			{
				printf("\r\n");
				printf("Pan = %u\r\n", (unsigned int)PAN_SERVO);
				printf("Tilt = %u", (unsigned int)TILT_SERVO);
				PWM_Menu_Print();
			}
			else if(terminal_char == 'p')
			{
				menu_state = TM_MENU_PRINT;
			}
			break;

		//
		// load default values from tracking.h for editing
		//
		case TM_GET_DEFAULT_VALUES:
			Temp_Tracking_Config_Data.Pan_Min_PWM = PAN_MIN_PWM_DEFAULT;
			Temp_Tracking_Config_Data.Pan_Center_PWM = PAN_CENTER_PWM_DEFAULT;
			Temp_Tracking_Config_Data.Pan_Max_PWM = PAN_MAX_PWM_DEFAULT;
			Temp_Tracking_Config_Data.Pan_Gain = PAN_GAIN_DEFAULT;
			Temp_Tracking_Config_Data.Pan_Allowable_Error = PAN_ALLOWABLE_ERROR_DEFAULT;
			Temp_Tracking_Config_Data.Pan_Rotation_Sign = PAN_ROTATION_SIGN_DEFAULT;
			Temp_Tracking_Config_Data.Pan_Search_Step_Size = PAN_SEARCH_STEP_SIZE_DEFAULT;
			Temp_Tracking_Config_Data.Pan_Target_Pixel = PAN_TARGET_PIXEL_DEFAULT;
			Temp_Tracking_Config_Data.Tilt_Min_PWM = TILT_MIN_PWM_DEFAULT;
			Temp_Tracking_Config_Data.Tilt_Center_PWM = TILT_CENTER_PWM_DEFAULT;
			Temp_Tracking_Config_Data.Tilt_Max_PWM = TILT_MAX_PWM_DEFAULT;
			Temp_Tracking_Config_Data.Tilt_Gain = TILT_GAIN_DEFAULT;
			Temp_Tracking_Config_Data.Tilt_Allowable_Error = TILT_ALLOWABLE_ERROR_DEFAULT;
			Temp_Tracking_Config_Data.Tilt_Rotation_Sign = TILT_ROTATION_SIGN_DEFAULT;
			Temp_Tracking_Config_Data.Tilt_Search_Step_Size = TILT_SEARCH_STEP_SIZE_DEFAULT;
			Temp_Tracking_Config_Data.Tilt_Target_Pixel = TILT_TARGET_PIXEL_DEFAULT;
			Temp_Tracking_Config_Data.Search_Delay = SEARCH_DELAY_DEFAULT;

			menu_state = TM_MENU_PRINT;
			break;

		//
		// update global tracking configuration structure and save it to EEPROM
		//
		case TM_SAVE_TO_EEPROM:
			// move local copy of the tracking configuration to the global copy
			for(i = 0; i < sizeof(Tracking_Config_Data_Type); i++)
			{
				byte = ((unsigned char *)(&Temp_Tracking_Config_Data))[i];
				((unsigned char *)(&Tracking_Config_Data))[i] = byte;
			}
			// write configuration to EEPROM
			Save_Tracking_Configuration(TRACKING_CONFIG_EEPROM_ADDRESS);

			printf("\r\nSaving to EEPROM...");

			// next state provides a EEPROM write delay 
			menu_state = TM_EEPROM_WRITE_DELAY;
			break;

		// allow EEPROM_Write_Handler() to write all of 
		// the data before returning to the main menu
		case TM_EEPROM_WRITE_DELAY:

			// wait here until all the EEPROM data is written
			if(EEPROM_Queue_Free_Space() == EEPROM_QUEUE_SIZE)
			{
				printf("Done\r\n");

				// back to the main menu...
				menu_state = TM_MENU_PRINT;
			}	
			break;
	}
	return(tracking_menu_active);
}

/*******************************************************************************
*
*	FUNCTION:		Tracking_Menu_Print()
*
*	PURPOSE:		
*
*	CALLED FROM:
*
*	PARAMETERS:		none
*
*	RETURNS:		nothing
*
*	COMMENTS:
*
*******************************************************************************/
void Tracking_Menu_Print(void)
{
	printf("\r\n");
	printf("\r\n");
	printf("[a] Change Pan Min PWM = %u\r\n",Temp_Tracking_Config_Data.Pan_Min_PWM);
	printf("[b] Change Pan Center PWM = %u\r\n",Temp_Tracking_Config_Data.Pan_Center_PWM);
	printf("[c] Change Pan Max PWM = %u\r\n",Temp_Tracking_Config_Data.Pan_Max_PWM);
	printf("[d] Change Pan Gain = %u\r\n",Temp_Tracking_Config_Data.Pan_Gain);
	printf("[e] Change Pan Allowable Error = %u\r\n",Temp_Tracking_Config_Data.Pan_Allowable_Error);
	printf("[f] Change Pan Rotation Sign = %+d\r\n",Temp_Tracking_Config_Data.Pan_Rotation_Sign);
	printf("[g] Change Pan Search Step Size = %u\r\n",Temp_Tracking_Config_Data.Pan_Search_Step_Size);
	printf("[h] Change Pan Target Pixel = %u\r\n",Temp_Tracking_Config_Data.Pan_Target_Pixel);
	printf("[i] Change Tilt Min PWM = %u\r\n",Temp_Tracking_Config_Data.Tilt_Min_PWM);
	printf("[j] Change Tilt Center PWM = %u\r\n",Temp_Tracking_Config_Data.Tilt_Center_PWM);
	printf("[k] Change Tilt Max PWM = %u\r\n",Temp_Tracking_Config_Data.Tilt_Max_PWM);
	printf("[l] Change Tilt Gain = %u\r\n",Temp_Tracking_Config_Data.Tilt_Gain);
	printf("[m] Change Tilt Allowable Error = %u\r\n",Temp_Tracking_Config_Data.Tilt_Allowable_Error);
	printf("[n] Change Tilt Rotation Sign = %+d\r\n",Temp_Tracking_Config_Data.Tilt_Rotation_Sign);
	printf("[o] Change Tilt Search Step Size = %u\r\n",Temp_Tracking_Config_Data.Tilt_Search_Step_Size);
	printf("[p] Change Tilt Target Pixel = %u\r\n",Temp_Tracking_Config_Data.Tilt_Target_Pixel);
	printf("[q] Change Search Delay = %u\r\n",Temp_Tracking_Config_Data.Search_Delay);
	printf("\r\n");
	printf("[u] Interactive PWM Adjustment Menu\r\n");
	printf("[v] Load default values\r\n");
	printf("[w] Save changes\r\n");
	printf("[x] Exit\r\n");
	printf("\r\n");
}

/*******************************************************************************
*
*	FUNCTION:		PWM_Menu_Print()
*
*	PURPOSE:		
*
*	CALLED FROM:
*
*	PARAMETERS:		none
*
*	RETURNS:		nothing
*
*	COMMENTS:
*
*******************************************************************************/
void PWM_Menu_Print(void)
{
	printf("\r\n");
	printf("\r\n");
	printf("[a] Pan Left One Step\r\n");
	printf("[b] Pan Left Ten Steps\r\n");
	printf("[c] Pan Right One Step\r\n");
	printf("[d] Pan Right Ten Steps\r\n");
	printf("[e] Set Pan Min PWM Value\r\n");
	printf("[f] Set Pan Center PWM Value\r\n");
	printf("[g] Set Pan Max PWM Value\r\n");
	printf("[h] Tilt Up One Step\r\n");
	printf("[i] Tilt Up Ten Steps\r\n");
	printf("[j] Tilt Down One Step\r\n");
	printf("[k] Tilt Down Ten Steps\r\n");
	printf("[l] Set Tilt Min PWM Value\r\n");
	printf("[m] Set Tilt Center PWM Value\r\n");
	printf("[n] Set Tilt Max PWM Value\r\n");
	printf("[o] Display Current PWM Values\r\n");
	printf("[p] Return To Main Tracking Menu\r\n");
	printf("\r\n");
}

/*******************************************************************************
*
*	FUNCTION:		Save_Tracking_Configuration()
*
*	PURPOSE:		Writes the current tracking configuration, as stored in
*					the Tracking_Config_Data structure, to EEPROM
*
*	CALLED FROM:	Tracking_Menu(), above.
*
*	PARAMETERS:		EEPROM address to save to.
*
*	RETURNS:		Nothing
*
*	COMMENTS:
*
*******************************************************************************/
void Save_Tracking_Configuration(unsigned int eeprom_address)
{
	unsigned int i;
	unsigned char byte;
	unsigned int checksum;

	checksum = 0;

	// make sure the identification bytes are present in the structure
	Tracking_Config_Data.Letter_G = 'G';
	Tracking_Config_Data.Letter_K = 'K';

	// except for the checksum, read each byte of the tracking 
	// configuration structure and write it to EEPROM
	for(i = 0; i < sizeof(Tracking_Config_Data) - 1; i++)
	{
		// this ugly code allows the tracking configuration data
		// structure to be addressed as an unsigned char array
		byte = ((unsigned char *)(&Tracking_Config_Data))[i];

		// write the EEPROM
		EEPROM_Write(eeprom_address + i, byte);

		checksum += (unsigned int)byte;
	}

	// finish by writing the checksum
	EEPROM_Write(eeprom_address + sizeof(Tracking_Config_Data) - 1, (unsigned char)checksum);
}
