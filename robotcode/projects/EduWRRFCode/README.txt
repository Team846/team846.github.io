This directory contains the default code for the Innovation First 2004 EDU Robot Controller.

To compile this code you must have Microchip's MPLAB IDE with their C18 compiler installed on your system.

If you wish to restore your EDU Robot Controller to the default functionality as shipped from the factory, use the IFI Loader application to download the EDU_default.hex file to your Robot Controller.

CAUTION:  Do not relocate this EduCode directory very deep in your directory tree.  This is because the Microchip compiler has a 64 character path/filename limit.

NOTE:  The EDU Default Code will be continuously evolving to improve the user experience in preparation for the FIRST Competition Kick-off.  Check back frequently for newer versions.

www.InnovationFirst.com


2/16/2004
- ifi_aliases.h
  Changed aliases for CCP pins to point to correct ports.
- 18f8520user.lkr
    Removed debug section of program memory.  Now User code can go all the way up to 0x7FFF.
- ifi_utilities.h
  Corrected baud_19 macro to equal 128.
- ifi_picdefs.h
  Removed comments on EEIE and EEIF bits.  They are not reserved bits.
- ifi_default.h
  File made common with FRC code.  No changes that affect EDU programming.

12/15/2003
- user_routines_fast.c
  Changed line 54 to fix occasional interrupt caused glitches.

12/8/2003
- ifi_aliases.h
  Corrected alias for usart2_RX.

12/4/2003
- user_routines.c
  Added EIGHTH User_Initialization option: Setup_PWM_Output_Type(...)
- ifi_aliases.h
  Added macros for PWM Type Definitions used in Setup_PWM_Output_Type(...)
  Added aliases for CCP and USART2 pins.
- ifi_default.h
  Added function prototype for Setup_PWM_Output_Type(...)
- ifi_library.lib
  Added Setup_PWM_Output_Type(...) function.
  Modified Generate_Pwms(...) function so that it will do nothing if the SPI interrupt is imminent.  Previously many people had trouble synchronizing this function and saw twitchy motors/servos when using it.  Now they should see better results without having to synchronize it themselves.  Note that now Timer0 is unavailable for use.  If you do not want this feature (and want to use Timer0 yourself) then you should use ifi_alltimers.lib instead.
- ifi_picdefs.h
  Added comments reserving Timer0 resources in the case where you use ifi_library.lib.
- various files - Modified comments for clarity.

10/28/2003
- Changed project structure slightly to make it simpler for the user.  Now the user should ONLY modify the user_routines.c, user_routines.h, and user_routines_fast.c files.
- ifi_utilities.c
  Increased delay time in Get_Analog_Value() to improve reliability.
- ifi_library.h 
  Removed file by moving function prototypes to ifi_default.h
- Numerous cosmetic changes to improve simplicity.

10/22/2003
- user_routines.c
  Changed printf example.
- ifi_default.h
  Removed FIRST_RX bit definition.
- user_routines_analogex.c
  Removed file to avoid confusion.  It was only an example file.
- ifi_library.h
  Changed comment in header to refer to ifi_library.lib rather than ifi_library.c
- ifi_library.lib
  Optimized code.  Not user accessible.

10/16/2003
- ifi_picdefs.h 
  Comments modified to indicate that user can utilize EEPROM.

10/15/2003
- Baseline default code released.

