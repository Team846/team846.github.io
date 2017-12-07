****************************************************************

You are free to use this source code for any non-commercial
use. Please do not make copies of this source code, modified
or un-modified, publicly available on the internet or
elsewhere without permission. Thanks.

Copyright ©2005-2006 R. Kevin Watson. All rights are reserved.

****************************************************************

     *** "Streamlined" version of the camera code ***

The source code in camera.c/.h contains software to configure
and parse the data stream of your CMUcam2 vision sensor.

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

Six things must be done before this software will work 
correctly with your robot controller:

1) Install the serial_ports.c/.h software using the
instructions found in serial_ports_readme.txt. This software
can be downloaded from http://kevin.org/frc.

2) You need to add camera.c and camera.h to your project.
Do this by copying the two files to your project directory
and then right clicking on "Source Files" in the project
tree, selecting "Add Files...", if necessary, navigate to
the project directory and then double click on camera.c.
Repeat the above procedure for camera.h under "Header Files".

3) A #include statement for the camera.h header file must be 
included at the beginning of each source file that calls the 
functions in camera.c. The statement should look like this: 
#include "camera.h".

4) The function Camera_Handler() function must be called
from Process_Data_From_Master_uP() each time it executes.

5) The global variable "stdout_serial_port" must be set to
point to your terminal serial port. This can automatically
be done by inserting this code into User_Initialization():

#ifdef TERMINAL_SERIAL_PORT_1    
  stdout_serial_port = SERIAL_PORT_ONE;
#endif

#ifdef TERMINAL_SERIAL_PORT_2    
  stdout_serial_port = SERIAL_PORT_TWO;
#endif

6) Select the robot controller serial port that your camera
will communicate through by opening the camera.h file and
following the embedded instructions. By default, serial port
two (the TTL-level port) is used.

****************************************************************

Here's a description of the functions in camera.c:

Camera_Handler()
This function is responsable for camera initialization
and camera serial data interpretation. Once the camera
is initialized and starts sending tracking data, this 
function will continuously update the T_Packet_Data data
structure with the received tracking information.


Camera_State_Machine()
This function parses the camera serial data stream looking 
for data packets, ACKs and NCKS. When packets are complete 
the individual packet counter variable is incremented and, 
in the case of packets, the global data structure is
updated with the new data.


Initialize_Camera()
This function is responsable for initializing the camera.


Track_Color()
This function properly formats and sends a "Track Color"
command to the camera.


Camera_Idle()
If the camera is currently streaming data, this function
will stop the streaming and prepare it to receive commands.


Restart_Camera()
This function forces a camera reinitialization.


Get_Camera_State()
This function returns the operating state of the camera.
The two possibilities are initializing and initialized
and sending data packets. 


Raw_Mode()
This function properly formats and sends a "Raw Mode"
command	to the camera.


Noise_Filter()
This function properly formats and sends a "Noise Filter" 
command to the camera.


Write_Camera_Module_Register()
This function properly formats and sends a "Camera Register"
command to the camera.


Camera_Serial_Port_Byte_Count()
This function returns the number of bytes in the camera 
serial port's received data queue.


Read_Camera_Serial_Port()
This function reads a byte of data from the camera serial
port.


Write_Camera_Serial_Port()
This function sends a byte of data to the camera serial 
port.


Terminal_Serial_Port_Byte_Count()
This function returns the number of bytes in the terminal 
serial port's received data queue.


Read_Terminal_Serial_Port()
This function reads a byte of data from the terminal serial
port.


Write_Terminal_Serial_Port()
This function sends a byte of data to the terminal serial
port.


Kevin Watson
kevinw@jpl.nasa.gov
