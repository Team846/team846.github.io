****************************************************************

You are free to use this source code for any non-commercial
use. Please do not make copies of this source code, modified
or un-modified, publicly available on the internet or
elsewhere without permission. Thanks.

Copyright ©2005-2006 R. Kevin Watson. All rights are reserved.

****************************************************************

The source code in camera_menu.c/.h contains optional software
that can be used to interactively change, on-the-fly, the
operating parameters of the camera software. This software
will only work with the "Bells and Whistles" version of the
camera software.

By default, the camera menu is invoked by entering the letter
'c' into the "To Port ->" window of the IFI loader's terminal
window and hitting the enter key.

Support can be had by visiting the Chief Delphi forums and 
posting a question in the programming forum. You are encouraged
to report software and documentation bugs there too. The URL is
http://www.chiefdelphi.com

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

Four things must be done before this software will work 
correctly with your robot controller:

1) Install the camera.c/.h software using the instructions 
found in camera_readme.txt. This software can be downloaded
from http://kevin.org/frc.

2) You need to add camera_menu.c and camera_menu.h to your
project. Do this by copying the two files to your project 
directory and then right clicking on "Source Files" in the 
project tree, selecting "Add Files...", if necessary, 
navigate to the project directory and then double click on 
camera_menu.c. Repeat the above procedure for camera_menu.h 
under "Header Files".

3) A #include statement for the camera_menu.h header file 
must be included at the beginning of each source file that 
calls the functions in camera_menu.c. The statement should 
look like this: #include "camera_menu.h".

4) Code to detect the camera menu invocation "hotkey" must
execute within the Process_Data_From_Master_uP() function.
See the included copy of user_routines.c for example code
that performs this function.

****************************************************************

Here's a description of the functions in camera_menu.c:

Camera_Menu()
This function manages the camera menu functionality, which is
used to enter camera initialization and	calibration parameters
using a terminal attached to your robot controller.


Menu_Print()
This function is responsible for sending the camera menu text
to the terminal screen.


Save_Camera_Configuration()
This function builds a camera configuration structure and
saves it to EEPROM, where it can be retrieved by the camera
software the next time the camera is initialized.


Kevin Watson
kevinw@jpl.nasa.gov
