Both these libraries fix a possible data corruption issue pertaining to the 18F8722 chip.  

*** Note : Please refer to the MicroChip PIC18F8722 errata sheet at 
*** http://ww1.microchip.com/downloads/en/DeviceDoc/80221b.pdf for more information regarding 
*** this anomaly.

To use a modified library, replace the one currently in your project (either "FRC_Library.lib"
or "FRC_alltimers.lib") with the appropriate one (either "FRC_Library_8722.lib" or 
"FRC_alltimers_8722.lib").  Once this is complete, recompile and download your code.


The following has been changed to the IFI_Library file:

From:

#pragma interrupt InterruptHandlerHigh


To:
#pragma interruptlow InterruptHandlerHigh


According to : (from the errata sheet)

C Language Programming: If using the Microchip MPLAB® C18 C Compiler, define both high and 
low priority interrupt handler functions as “low priority” by using the pragma interruptlow 
directive. This directive instructs the compiler to not use the RETFIE FAST instruction. 
If the proper high priority interrupt bit is set in the IPRx register, then the interrupt 
is treated as high priority in spite of the pragma interruptlow directive. 


