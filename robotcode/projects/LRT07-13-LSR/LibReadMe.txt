The following has been changed to the IFI_Library file:

From:

#pragma interrupt InterruptHandlerHigh


To:
#pragma interruptlow InterruptHandlerHigh


According to :

C Language Programming: If using the Microchip MPLAB® C18 C Compiler, define both high and 
low priority interrupt handler functions as “low priority” by using the pragma interruptlow 
directive. This directive instructs the compiler to not use the RETFIE FAST instruction. 
If the proper high priority interrupt bit is set in the IPRx register, then the interrupt 
is treated as high priority in spite of the pragma interruptlow directive. 
