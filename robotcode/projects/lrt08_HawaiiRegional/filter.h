#ifndef __FILTER_h
#define __FILTER_h
#if 1
typedef struct {
	unsigned char order;
	float *in;
	float *a;	//coef a[0] to a[n]
	char index_in;
	
	float *out;
	float *b; //coef b[1] to b[n]; Set remaining terms to 0 if an FIR filter
	char index_out;
} FilterParameters;

int digitalFilter (int input, FilterParameters *f);

#else //FLOAT


typedef struct {
	unsigned char order;
	int *in;
	int *a;	//coef a[0] to a[n]
	unsigned char scale_a_pwr2;
	char index_in;
	
	int *out;
	int *b; //coef b[1] to b[n]; Set remaining terms to 0 if an FIR filter
	unsigned char scale_b_pwr2;
	char index_out;
} FilterParameters;

int filterIIR (int input, FilterParameters *f);
#endif


#endif	// __FILTER_h
