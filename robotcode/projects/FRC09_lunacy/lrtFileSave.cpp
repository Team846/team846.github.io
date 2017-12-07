#include "lrtFileSave.h"
#include <cstring>
#include <cstdio>

const char *FileSave::m_filename = "/LRT846/lrt_Data.txt";

FileSave::FileSave() {
	
}

FileSave::~FileSave() {
	
}


void FileSave::Read(void)
{
	FILE *fp = fopen(m_filename,"r");
	if(fp==NULL){
		printf("Error:  Couldn't open %s for reading\n",m_filename);
		return;
	}
	
	
	char theLine[128];
	
	for (int n=0; NULL != fgets(theLine, sizeof(theLine), fp); n++) {
		printf(theLine);
		if (theLine[0]== '#')
			continue;
		if (1==sscanf(theLine, "Gain: %f", &gain)) {
			printf("Gain = %f\n", gain);
			continue;
		}
		if (1==sscanf(theLine, "Time: %f", &time)) {
			printf("Time= %f\n", time);
			continue;
		}
		printf("Couldn't parse line %d: %s\n", n, theLine);
	}
	
	fclose(fp);
	
	
	
	
}



void FileSave::Write(void)
{
	FILE *fp = fopen(m_filename,"w");
	if(fp==NULL){
		printf("Error:  Couldn't open %s for writing\n",m_filename);
		return;
	}
	

	fprintf(fp,"Gain: %f\n", gain);
	fprintf(fp,"Time: %f\n", time);
		
	fclose(fp);
	
	
	
	
}


void FileSave::ClearVariables(void){
	gain = time = -1.;
}

void FileSave::PrintVariables(void){
	printf("PrintVariables:\n");
	printf("Gain = %f\n", gain);
	printf("Time = %f\n",time);
}


//Dear Software group,
//
//Mr W brought up the idea of reading data from a file, and I agree this may be a very effective way to make quick data changes without restarting the cRio.
//
//
//Here are a few considerations:
//*) if the file is binary, we can read and write it easily from the program, but we can't modify externally with a text editor.  This does not meet our needs.
//
//*) if the file is plain text, it is both human readable, and we can edit it with a plain text.  We can parse it simply using sscanf (see below)
//
//*) we want the values to be effective without restarting the cRio.  To do that, we can read the file whenever AutonomousInit() or TeleOpInit() are called.
//
//
//char *filename = "lrtData";
//FILE fp = fopen(filename , "r");
//if (fp == NULL) {
// printf("Error: Can't open %s\n", filename);
//    return;
//}
//
//
//char theLine[128];
//
//for (int n=0; EOF != fgets(theLine); n++) {
//	if (theLine[0]== '#')
//		continue;
//  if (1==sscanf(theLine, "Gain: %f", &gain)) {
//    printf("Gain = %f", " gain)
//    continue;
//    }
//  if (1==sscanf("Time: %f", &time)) {
//    printf("Time= %f", ", time);
//    continue;
//    }
//  printf("Couldn't parse line %d: %s\n", n, theLine);
//}
//fclose(fp);
//
//
//
//If our list of variables grow, we can extend it further by creating a list of variables and format strings, e.g.:
//
//struct parseStrings {
//    char *format[32];
//    float *f;
//
//} parseStrings[] = {
//    {"Gain: %f", &gain},
//    {"Time: %f", &time} };
//int nParseStrings = sizeof (parseStrings) / sizeof (parseStrings[0]);
//
//Then we can use
//    for (i=0; i< nParseStrings; i++) {
//        if (1 ==  sscanf(parseString[i].format, parseString[i].f)) {
//            printf(parseString[i].format, *(parseString[i].f));
//            break;
//       }
//   }
//
