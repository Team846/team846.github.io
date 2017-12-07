#ifndef _DEBUG_H
#define _DEBUG_H

void ReadTerminalForOptions(void);
void doPrintOptions(void);
void ControlPrinting(void);
void ReadPrintOptionsFromEPROM(void);
void printRelays(void);
void CheckForSerialPortErrors(void);
void Tracking_Info_Terminal(void);

typedef struct {
	unsigned print:1;	//if false, don't print anything
	unsigned printOnA:1;
	unsigned printOnB:1;
	unsigned printOnC:1;
	unsigned printOnD:1;
	unsigned printOnS:1;	//stats controls 'onSecondLast'
	unsigned printCamera:1;
//	unsigned pausePrintingForMenu:1;
} printOptions;
extern printOptions gPrintOptions;

#endif	// _DEBUG_H
