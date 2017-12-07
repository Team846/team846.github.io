#ifndef _DEBUG_H
#define _DEBUG_H

void ReadTerminalForOptions(void);
void ControlPrinting(void);
void ReadPrintOptionsFromEPROM(void);
void printRelays(void);
void Tracking_Info_Terminal(void);

void Log_LoopStart(void);
void Log(const char *type, const char *fmt, ...);

#define LOG_EVENT(...)	Log(1, "*EV", __VA_ARGS__)
#define LOG_LIFT(...)	do { if (gLoop.printA) Log("LFT", __VA_ARGS__) } while (0)
#define LOG_DRIVE(...)	do { if (gLoop.printC) Log("DRV", __VA_ARGS__) } while (0)
#define LOG_HYBRID(...)	do { if (gLoop.printA) Log("HYB", __VA_ARGS__) } while (0)
#define LOG_PARAM(...)	do { if (gLoop.printA) Log("PRM", __VA_ARGS__) } while (0)
#define LOG_STAT(...)	do { if (gLoop.printStats) Log("STT", __VA_ARGS__) } while (0)

typedef struct {
	unsigned print:1;	//if false, don't print anything
	unsigned printA:1;
	unsigned printB:1;
	unsigned printC:1;
	unsigned printD:1;
	unsigned printStats:1;	//stats controls 'onSecondLast'
} PrintOptions;
extern PrintOptions gPrintOptions;

#endif	// _DEBUG_H
