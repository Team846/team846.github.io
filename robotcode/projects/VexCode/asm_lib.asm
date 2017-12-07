;*****************************************************************************
;* FILE NAME: Asm_lib.asm
;*
;* DESCRIPTION: 
;* This file contains routines needed by the serial driver.
;*
;*****************************************************************************

#include    p18f8520.inc

    GLOBAL  GetEEData,ReadEE,WriteEE,write_to_address,read_from_address
	GLOBAL  gVar1

ASM_VAR	            UDATA_ACS	0x0	; Force address to zero, since MATH_DATA 
gVar1				RES		.20
FSR0L_TEMP	        RES		1		; normally gets this address 
FSR0H_TEMP	        RES		1
gByteCnt            RES     1
grtnByte			RES		1


ASM_LIB		CODE

;*****************************************************************************
;* FUNCTION NAME: write_to_address
;* PURPOSE:       writes a byte to a specified memory address
;*****************************************************************************
write_to_address	;(uword8 FSR0H,uword8 FSR0L,uword8 data)
    MOVFF   FSR0H,FSR0H_TEMP
    MOVFF   FSR0L,FSR0L_TEMP

	MOVLW	0xff		    ;two's complement representation of -1
	MOVFF	PLUSW1,FSR0H	;get upper address from stack    
	MOVLW	0xfe		    ;two's complement representation of -2
	MOVFF	PLUSW1,FSR0L	;get lower address from stack 	  
	MOVLW	0xfd		    ;two's complement representation of -3
	MOVFF	PLUSW1,INDF0	;get data from stack     

    MOVFF   FSR0H_TEMP,FSR0H
    MOVFF   FSR0L_TEMP,FSR0L
	return

;*****************************************************************************
;* FUNCTION NAME: read_from_address
;* PURPOSE:       read a byte from a specified memory address
;*****************************************************************************
read_from_address	;uword8 (uword8 FSR0H,uword8 FSR0L)
    MOVFF   FSR0H,FSR0H_TEMP
    MOVFF   FSR0L,FSR0L_TEMP

	MOVLW	0xff		    ;two's complement representation of -1
	MOVFF	PLUSW1,FSR0H	;get upper address from stack     
	MOVLW	0xfe		    ;two's complement representation of -2
	MOVFF	PLUSW1,FSR0L	;get lower address from stack     

	MOVF	INDF0,w			;return byte in accumulator
    MOVFF   FSR0H_TEMP,FSR0H
    MOVFF   FSR0L_TEMP,FSR0L
	return

;*****************************************************************************
;* FUNCTION NAME: ReadEE
;* PURPOSE:       read a byte from a specified EEprom location
;*****************************************************************************
ReadEE                      ; (uword8 addr, uword8 gByteCnt, uword8 *bufr)
    MOVFF   FSR0H,FSR0H_TEMP
    MOVFF   FSR0L,FSR0L_TEMP

	MOVLW	0xff		    ;two's complement representation of -1
	MOVFF	PLUSW1,EEADR	;get location from stack    
	MOVLW	0xfe		    ;two's complement representation of -2
	MOVFF	PLUSW1,gByteCnt	;get count from stack    
	MOVLW	0xfd		    ;two's complement representation of -3
	MOVFF	PLUSW1,FSR0H	;get upper address from stack  
	MOVLW	0xfc		    ;two's complement representation of -4
	MOVFF	PLUSW1,FSR0L	;get lower address from stack  
    clrf    EEADRH

ReadEE_loop
    clrf    EECON1 
    bsf     EECON1, RD      ; Read the data
    movff   EEDATA, POSTINC0
    infsnz  EEADR, F        ; Adjust EEDATA pointer
    incf    EEADRH, F
    decfsz  gByteCnt,F
    bra     ReadEE_loop     ; Not finished then repeat

    MOVFF   FSR0H_TEMP,FSR0H
    MOVFF   FSR0L_TEMP,FSR0L
    return

;*****************************************************************************
;* FUNCTION NAME: GetEEData
;* PURPOSE:       returns a byte from a specified EEprom location
;*****************************************************************************
GetEEData                   ; (uword8 addr)
	MOVLW	0xff		    ;two's complement representation of -1
	MOVFF	PLUSW1,EEADR	    
    clrf    EEADRH

    clrf    EECON1 
    bsf     EECON1, RD      ; Read the data
    movff   EEDATA, grtnByte
	movf	grtnByte,w
    return

;*****************************************************************************
;* FUNCTION NAME: WriteEE
;* PURPOSE:       write a byte to a specified EEprom location
;*****************************************************************************
WriteEE                     ;(uword8 addr, uword8 gByteCnt, uword8 *bufr)
    MOVFF   FSR0H,FSR0H_TEMP
    MOVFF   FSR0L,FSR0L_TEMP

	MOVLW	0xff		    ;two's complement representation of -1
	MOVFF	PLUSW1,EEADR	;get location from stack     
	MOVLW	0xfe		    ;two's complement representation of -2
	MOVFF	PLUSW1,gByteCnt	;get count from stack    
	MOVLW	0xfd		    ;two's complement representation of -3
	MOVFF	PLUSW1,FSR0H	;get upper address from stack 
	MOVLW	0xfc		    ;two's complement representation of -4
	MOVFF	PLUSW1,FSR0L	;get lower address from stack  
    clrf    EEADRH

WriteEE_loop
    movff   POSTINC0, EEDATA
    movlw   b'00000100'     ; Setup for EEData
    movwf   EECON1
    rcall   StartWrite    
WriteWait  
    btfsc   EECON1, WR      ; Write and wait
    bra     WriteWait
    infsnz  EEADR, F        ; Adjust EEDATA pointer
    incf    EEADRH, F
    decfsz  gByteCnt,F      
    bra     WriteEE_loop    ; Not finished then repeat

    MOVFF   FSR0H_TEMP,FSR0H
    MOVFF   FSR0L_TEMP,FSR0L
    return

;*****************************************************************************
;* FUNCTION NAME: StartWrite
;* PURPOSE:       Unlock and start the write or erase sequence.
;*****************************************************************************
StartWrite
    clrwdt
    movlw   0x55            ; Unlock
    movwf   EECON2
    movlw   0xAA
    movwf   EECON2
    bsf     EECON1, WR      ; Start the write
    nop
    return
	
    END

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
