***************************************************************

You are free to use this source code for any non-commercial
use. Please do not make copies of this source code, modified
or un-modified, publicly available on the internet or
elsewhere without permission. Thanks.

Copyright ©2004-2006 R. Kevin Watson. All rights are reserved.

***************************************************************

The source code in serial_ports.c/.h contains a software 
implementation of a fully buffered, interrupt-driven, full-
duplex serial port driver that can be used with either or both 
on-board serial ports. This software is also specifically
designed to work with the output stream functions included 
with Microchip's C18 C compiler version 2.4  

This source code will work with the PIC18F8520-based FIRST 
Robotics robot controller, the PIC18F8722-based FIRST Robotics
robot controller, and the Robovation (A/K/A EDU-RC) robot 
controller.

Because you can now easily receive data from another computer, 
you can interact with your nifty IFI robot controller in real-
time to change operating parameters on-the-fly using common 
terminal emulation software, or send real telemetry to custom 
applications written with Visual Basic, Visual C++, MATLAB, 
etc... Don't want to drag a long serial cable behind your 'bot? 
Well, check-out the nifty SMiRF radio modem from SparkFun 
Electronics (http://www.sparkfun.com). Would the coolness 
factor of your 'bot be elevated if you had a LCD mounted on 
board to display diagnostics (yes, this is a rhetorical 
question)? How about using one of the serial LCDs that can be 
found on the 'net? I've had success using Scott Edward's 
Electronics (http://www.seetron.com) serial LCDs. The TRM-425L 
will work with the TTL-level serial port two and also includes 
a keypad interface. I've been mostly using the BPP-420L on 
serial port one. To use the above devices you'll need to build 
a simple three or four conductor cable. Disclaimer: Other than 
being a satisfied customer, I have no interest (financially, or 
otherwise) in the companies mentioned above.

The included project files were built with MPLAB version 7.20.
If your version of MPLAB complains about the project version, 
the best thing to do is just create a new project with MPLAB's 
project wizard. Include every file except: FRC_alltimers.lib, 
ifi_alltimers.lib, and you should be able to build the code.

By default, serial port one will operate at 115200 baud, which
is compatible with InnovationFIRST's terminal program, and 
serial port two will operate at 9600 baud, which will work with 
the above mentioned peripheral devices. These values can be 
easily changed by modifying the serial port initialization 
functions mentioned below.

For an example of how to use this software, see the code
in Process_Data_From_Master_uP(), which demonstrates how to
properly use this new functionality.

***************************************************************

Here's a description of the functions in serial_ports.c:


Init_Serial_Port_One()
Init_Serial_Port_Two()

These functions initialize the serial ports. This is where 
you will set operating parameters like the baud rate. One 
or both of these functions must be called before any serial 
port operations can take place.


Serial_Port_One_Byte_Count()
Serial_Port_Two_Byte_Count()

These functions will return the number of bytes present in
their respective received data queues. Because there might
not be any data in the queues, these functions must be called 
before you can read any data from a serial port.


Read_Serial_Port_One()
Read_Serial_Port_Two()

These functions will return the next byte from the received
data queue. If no data is present in the queue, the function 
will return the number zero, which could cause problems if
your incoming data can also contain a zero. This is why the
Serial_Port_xxx_Byte_Count() functions must be called first.


Write_Serial_Port_One()
Write_Serial_Port_Two()

These functions put a byte of data on the serial port transmit 
queue. If the queue is full, the function will make you wait
until a storage slot becomes available before allowing your
code to execute again.


Rx_1_Int_Handler()
Rx_2_Int_Handler()

When a new byte of data is received by the serial port, the 
microcontroller will automatically call these functions to
get the new data and place it in the received data queue for
you. You shouldn't have to call these functions yourself.


Tx_1_Int_Handler()
Tx_2_Int_Handler()

When the serial port is ready to start sending a new byte of 
data, the microcontroller will automatically call these 
functions to get the next byte of data from the transmission 
queue and give it to the serial port for transmission. You 
shouldn't have to call these functions yourself.


_user_putc()

This function is the "glue" that interfaces the C18 output
stream functions to this serial port driver. If the global
variable stdout  is set to "_H_USER", which is defined in 
stdio.h, the C18 output stream functions will call this
function to send data to a serial port rather than the
library function putc(). Stdout is set to _H_USER within the
serial port initialization functions Init_Serial_Port_One()
and Init_Serial_Port_Two() described above.

***************************************************************

Nine things must be done before this software will work 
correctly:

  1a) FRC-RC: As this software is intended to replace IFI's
  serial port driver, the call to Serial_Driver_Initialize()
  in user_routines.c / User_Initialization() should be 
  removed or commented out. In addition, all references to
  "user_Serialdrv.c" and "user_Serialdrv.h" must be removed 
  from the project and all project source files.

  1b) EDU-RC: As this software is intended to replace IFI's
  serial port driver, the call to Initialize_Serial_Comms()
  in user_routines.c / User_Initialization() should be 
  removed or commented out.	In addition, all references to
  "printf_lib.c" and "printf_lib.h" must be removed from 
  the project and all project source files.

  2) You must add the serial_ports.c/.h source files to
  your MPLAB project.

  3) A #include statement for the serial_ports.h header
  file must be included at the beginning of each source
  file that uses the serial ports. The statement should
  look like this: #include "serial_ports.h".

  4) If you intend to use the C18 output stream functions,
  a #include statement for the stdio.h header file must be 
  included at the beginning of each source file that calls
  any of these functions. The statement should look like 
  this: #include <serial_ports.h>.

  5) Init_Serial_Port_One() and/or Init_Serial_Port_Two()
  must be called from the User_Initialization() function 
  located in the user_routines.c source file.

  6) The interrupt handler(s) must be installed in the
  InterruptHandlerLow() function located in the 
  user_routines_fast.c source file. See the accompanying
  copy of user_routines_fast.c to see how this is done.

  7) Decide what functionality you need and comment out the
  #define ENABLE_SERIAL_PORT_xxx_yy entries in serial_ports.h
  as necessary. As an example, if you only need to send data
  using serial port one and would like to reclaim the resources
  used by serial port two and serial port one's receiver
  source code, the top of the serial_ports.h file would look
  like this:

  // comment out the next line to disable all serial port one
  // receive functionality
  // #define ENABLE_SERIAL_PORT_ONE_RX

  // comment out the next line to disable all serial port one
  // transmit functionality
  #define ENABLE_SERIAL_PORT_ONE_TX

  // comment out the next line to disable all serial port two
  // receive functionality
  // #define ENABLE_SERIAL_PORT_TWO_RX

  // comment out the next line to disable all serial port two
  // transmit functionality
  // #define ENABLE_SERIAL_PORT_TWO_TX

  By default, both serial ports and their respective receive
  and transmit sections are enabled.

  8) As the default output device for C18's output stream 
  functions is the null device, you'll presumably want to 
  change the value of stdout_serial_port to "SERIAL_PORT_ONE" 
  or "SERIAL_PORT_TWO" if you want to see printf()'s output.
  User_Initialization() is a good place to do this.

  9) To support terminal emulation software, \r\n should 
  be used instead of just \n in the printf() format string.


This serial port driver can send output stream data to either 
of the serial ports by setting the value of the global variable
stdout_serial_port before calling output stream	functions like
printf(). Setting the value to "SERIAL_PORT_ONE" will send the 
output to serial port one. Likewise, setting the value to 
"SERIAL_PORT_TWO" will send the output to serial port two. 
Setting the value to "NUL" will send the output to the null 
device, meaning that the output is sent	nowhere. These values 
are #define'd in serial_ports.h. As an example,

  stdout_serial_port = SERIAL_PORT_ONE;
  printf("Kernighan");
  stdout_serial_port = NUL;
  printf("and");
  stdout_serial_port = SERIAL_PORT_TWO;
  printf("Ritchie");

will send the text "Kernighan" to the peripheral device attached
to serial port one, the text "Ritchie" to the device attached to 
serial port two and the text "and" won't be sent anywhere.
By default, output is sent to the null device, which is the only
output device guaranteed to be present.


Kevin Watson
kevinw@jpl.nasa.gov
