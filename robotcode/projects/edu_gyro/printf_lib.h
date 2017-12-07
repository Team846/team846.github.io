/*******************************************************************************
* FILE NAME: printf_lib.h
*
* DESCRIPTION: 
*  This is the include file which corresponds to printf_lib.c
*
* USAGE:
*  If you add your own routines to that file, this is a good place
*  to add function prototypes.
*******************************************************************************/

#ifndef __printf_lib_h_
#define __printf_lib_h_

int printf(rom const char *format, ...);

void printid(int data,int crtOn);
void printb(unsigned char data,int crtOn);
void printd(unsigned char data,int crtOn);
void printix(int data,int crtOn);
void printx(unsigned char data,int crtOn);
void debug_print(char *bufr,int data);
void debug_printb(char *bufr,unsigned int data);
void debug_println(char *bufr);

#endif


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
