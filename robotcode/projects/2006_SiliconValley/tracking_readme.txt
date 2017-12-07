****************************************************************

You are free to use this source code for any non-commercial
use. Please do not make copies of this source code, modified
or un-modified, publicly available on the internet or
elsewhere without permission. Thanks.

Copyright ©2005-2006 R. Kevin Watson. All rights are reserved.

****************************************************************

    *** "Streamlined" version of the tracking code ***

The source code in tracking.c/.h contains software to track a
colored game object, keeping the center of the tracked object 
in the center of the camera's image using two servos that drive 
a pan/tilt platform. When the center of the colored object is
centered on the camera's imager, you can use the current
commanded pan and tilt PWM values to calculate a heading and
range to the colored object.

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

Five things must be done before this software will work 
correctly with your robot controller:

1) Install the camera.c/.h software using the instructions 
found in camera_readme.txt. This software can be downloaded 
from http://kevin.org/frc.

2) You need to add tracking.c and tracking.h to your project.
Do this by copying the two files to your project directory
and then right clicking on "Source Files" in the project
tree, selecting "Add Files...", if necessary, navigate to
the project directory and then double click on tracking.c.
Repeat the above procedure for tracking.h under "Header Files".

3) A #include statement for the tracking.h header file must
be included at the beginning of each source file that calls
the functions in tracking.c. The statement should look like
this: #include "tracking.h".

4) The function Servo_Track() function must be called from
Process_Data_From_Master_uP() each time it executes.

5) Select the PWM outputs that your pan and tilt servos are
connected to by opening the tracking.h file and following the
embedded instructions. By default, the pan servo is assumed
to use pwm01 and the tilt servo is assumed to use pwm02.

****************************************************************

Here's a description of the single function in tracking.c:

Servo_Track()
This function reads data placed in the T_Packet_Data structure
by the Camera_Handler() function and if new tracking data is 
available, attempts to keep the center of the tracked object 
in the center of the camera's image using two servos that drive 
a pan/tilt platform. If the camera doesn't have the object 
within it's field of view, this function will execute a very
basic search algorithm in an attempt to find the object.		


Kevin Watson
kevinw@jpl.nasa.gov
