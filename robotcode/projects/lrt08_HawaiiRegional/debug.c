// debug.c: Serial debug/diagnostics

#include "common.h"
#include "debug.h"

static void DoPrintOptions(void);

PrintOptions gPrintOptions = {
	0,			//all printing
	1,1,1,1,	//A-D
	0			//stats
};

char gFakeDisableSw    = 1;
char gFakeAutonomousSw = 0;

void ReadPrintOptionsFromEPROM(void)
{
	*(char *) &gPrintOptions = EEPROM_ReadByte( EEPROM_PRINT_OPTIONS );
	gPrintOptions.print = 0;	// all printing off by default.
}
//**********************************************************************************

void ReadTerminalForOptions(void)
{
	char input_chr, input2;
	static char programit = 0;
	
	while(0!=(input_chr=Read_Serial_Port_One()))
	{
		putc(input_chr,stdout);
		switch (input_chr)
		{
			case 'p':
				DoPrintOptions();
				break;

			case 'D':
				gFakeDisableSw=1;
				break;
			case 'd':
				gFakeDisableSw=0;
				break;
			case 'A':
				gFakeAutonomousSw=1;
				break;
			case 'a':
				gFakeAutonomousSw=0;
				break;

			case '/':
				DIGOUT_PRGM = 0;
//				while(1);	// freeze!!
				break;
				
			case '\r':
				break;
			default:
				puts("Unknown input\r");
		}
	}

	//Display what options are being printed.
	if (!mRolling()) return;
	if (!gPrintOptions.print)
	{
		puts("Enter 'p' to print\r");
		return;
	}
	printf("\rPrint Options: A=%d B=%d C=%d D=%d Stats=%d\r",
			(int)gPrintOptions.printA,
			(int)gPrintOptions.printB,
			(int)gPrintOptions.printC,
			(int)gPrintOptions.printD,
			(int)gPrintOptions.printStats);
}

//******************************************************************
//handle print options by supresssing 'printX' after they are set by UpdateTimers()
void ControlPrinting(void)
{
	if (!gPrintOptions.print || !gPrintOptions.printA) gLoop.f.printA=0;
	if (!gPrintOptions.print || !gPrintOptions.printB) gLoop.f.printB=0;
	if (!gPrintOptions.print || !gPrintOptions.printC) gLoop.f.printC=0;
	if (!gPrintOptions.print || !gPrintOptions.printD) gLoop.f.printD=0;
	if (!gPrintOptions.print || !gPrintOptions.printStats) gLoop.f.printStats=0;
}

//******************************************************************

static void DoPrintOptions(void)
{
	char input_chr = input_chr = Read_Serial_Port_One();
	char dirty = 1;	//assume option changed
	switch (input_chr)
	{
		case 0:
		case '\r':
			gPrintOptions.print = !gPrintOptions.print;
			break;
		case 'a':
			gPrintOptions.printA=!gPrintOptions.printA;
			break;
		case 'b':
			gPrintOptions.printB=!gPrintOptions.printB;
			break;
		case 'c':
			gPrintOptions.printC=!gPrintOptions.printC;
			break;
		case 'd':
			gPrintOptions.printD=!gPrintOptions.printD;
			break;
		case 's':	// stats
			gPrintOptions.printStats=!gPrintOptions.printStats;
			break;
		default:
			dirty = 0;
			puts("unknown option\r");
	}
	
	if (dirty)
		 EEPROM_WriteByte( EEPROM_PRINT_OPTIONS, *(char *) &gPrintOptions );
}

//********************************************************************************

void printRelays(void)
{
	printf("Relays: %d%d / %d%d / %d%d\r",
		(int)relay1_fwd,
		(int)relay1_rev,
		(int)relay2_fwd,
		(int)relay2_rev,
		(int)relay3_fwd,
		(int)relay3_rev
	);
}

//void Log_LoopStart(void) {
//	if (gLoop.f.onSecond)
//		printf("=LOOP=\r");
//}

#if 0
void Log(const char *type, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	printf("%s ", type);
	vprintf(fmt, args);
	printf("\n");
	va_end(args);
}
#endif
