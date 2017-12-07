/*******************************************************************************
* FILE NAME: main.c <FRC VERSION>
*
* DESCRIPTION:
*  This file contains the main program loop.
*
* USAGE:
*  You should not need to modify this file.
*  Note the different loop speed for the two routines:
*     Process_Data_From_Master_uP
*     Process_Data_From_Local_IO
*******************************************************************************/
#include "common.h"

tx_data_record txdata;          /* DO NOT CHANGE! */
rx_data_record rxdata;          /* DO NOT CHANGE! */
packed_struct statusflag;       /* DO NOT CHANGE! */

/*******************************************************************************
* FUNCTION NAME: main
* PURPOSE:       Main program function.
* CALLED FROM:   ifi_startup.c
* ARGUMENTS:     none
* RETURNS:       void
* DO NOT DELETE THIS FUNCTION 
*******************************************************************************/
void main (void)
{
#ifdef UNCHANGEABLE_DEFINITION_AREA
	IFI_Initialization ();        /* DO NOT CHANGE! */
#endif

	User_Initialization();        /* You edit this in user_routines.c */

	statusflag.NEW_SPI_DATA = 0;  /* DO NOT CHANGE! */ 

	while (1)   /* This loop will repeat indefinitely. */
	{
		#ifdef _SIMULATOR
			//statusflag.NEW_SPI_DATA = 1;
			//Using Timer 0 to simulate NEW_SPI_DATA
		#endif
	
		if (statusflag.NEW_SPI_DATA)      /* 26.2ms loop area */
		{
			Process_Data_From_Master_uP(); 
			gCPULoad.idleCycles = 0;	//reset counter
		}
		else
			gCPULoad.idleCycles++;	//count cycles with no new data;
	
		Process_Data_From_Local_IO();     /* I'm fast!  I execute during every loop.*/
	}
}  /* END of Main */


/******************************************************************************/
