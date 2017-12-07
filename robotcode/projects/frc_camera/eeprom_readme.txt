****************************************************************

You are free to use this source code for any non-commercial
use. Please do not make copies of this source code, modified
or un-modified, publicly available on the internet or
elsewhere without permission. Thanks.

Copyright ©2005-2006 R. Kevin Watson. All rights are reserved.

****************************************************************
The source code in eeprom.c/.h contains software to read from
and write to the Electrically Erasable Programmable Read-only 
Memory (EEPROM) contained within your robot controller's 
processor. Storing information in EEPROM has the advantage of 
being permanent, unlike Random Access Memory (RAM), which
gets reinitialized each time you reset or restart your robot
controller.

See the code in Process_Data_From_Master_uP() for an example
application of this software.

This source code will work with the PIC18F8520-based FRC robot
controller from 2004/2005 and the newer PIC18F8722-based FRC
robot controller. It will also work with the nifty EDU robot 
controller.

The included project files were built with MPLAB version 7.20.
If your version of MPLAB complains about the project version, 
the best thing to do is just create a new project with MPLAB's 
project wizard. Include every file except: FRC_alltimers.lib 
and ifi_alltimers.lib and you should be able to build the code.

****************************************************************

Three things must be done before this software will work 
correctly with your robot controller:

1) You need to add eeprom.c and eeprom.h to your project.
Do this by copying the two files to your project directory
and then right clicking on "Source Files" in the project
tree, selecting "Add Files...", if necessary, navigate to
the project directory and then double click on eeprom.c.
Repeat the above procedure for eeprom.h under "Header Files".

2) A #include statement for the eeprom.h header file must be 
included at the beginning of each source file that calls the 
functions in eeprom.c. The statement should look like this: 
#include "eeprom.h".

3) The function EEPROM_Write_Handler() function must be called
from Process_Data_From_Master_uP() each time it executes.

****************************************************************

Here's a description of the functions in eeprom.c:


EEPROM_Read()

Reads data from EEPROM.


EEPROM_Write()

Places new write data on the EEPROM write queue. This function
returns a value of one if there was an available slot on the
queue, zero if there wasn't. EEPROM_Queue_Free_Space() should
be called to determine if enough free space is available on
the queue before calling EEPROM_Write(). By default, there
are sixteen slots available. The number of usable slots is
defined in eeprom.h.


EEPROM_Write_Handler()

If buffered EEPROM write data is present (i.e., EEPROM_Write()
has been called), this function will write one byte to EEPROM 
each time it is called. A call to this function should be placed
in and called each time Process_Data_From_Master_uP() is called.
If, on average, you need to write more than one byte of data to
EEPROM each time Process_Data_From_Master_uP() is called, you can 
call it multiple times each loop. If you start experiencing wacky
behavior when your 'bot runs or get the red-light-of-death, you
might be attempting to write too much data too quickly. Because
it takes 2 milliseconds to write each byte of data to EEPROM.


EEPROM_Queue_Free_Space()

Returns the number of free slots available on the EEPROM write
queue. This function should be called to determine if enough 
free space is available on the write queue before calling 
EEPROM_Write().


Kevin Watson
kevinw@jpl.nasa.gov
