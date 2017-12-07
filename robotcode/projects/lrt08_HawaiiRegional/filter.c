#include "utilities.h"
#include "filter.h"

#if 1	//Float
float InnerProductWithShift(char n, float k[], float x[], char index_x0);
//***********************************************************************************
//While I had first wrote this with integer mult (no floats), precision became
//difficult to maintain.  A 7th order IIR (8+7 coef) takes about 700us+ using floating point vs 350us using
//integer math.  However, truncation errors proved tricky to reduce because of the large range of coefs. [dg]
//***********************************************************************************

#undef Example
#ifdef Example
int myFilter(int input)
{
	enum { ORDER=7};
	static float a[ORDER+1] = {
		0.00009034894,
		0.0006324426,
		0.0018973278,
		0.003162213,
		0.003162213,
		0.0018973278,
		0.0006324426,
		0.00009034894,
};  	//coefficients of the filter
	static float b[ORDER] = {  //coefficients of the filter; b0 = 1
		-4.18233,
		7.8717194,
		-8.530942, 
		5.7099447,
		-2.3492472,
		0.5482648,
		-0.055844672,
};
	static float in[ORDER+1];
	static float out[ORDER];
	static FilterParameters filter = {
		ORDER,
		in, a, 0,
		out,b, 0
	};
	int v = digitalFilter(input, &filter);
	return v;
}
#endif
//***********************************************************************************
int digitalFilter (int input, FilterParameters *g)
{
	float sum_in, sum_out, output;

	FilterParameters fff, *f=&fff;
	*f = *g;

	//first, record the new input. Most recent with lowest indice.
	if (--f->index_in <0)
		f->index_in=f->order;	//increment and wrap
	f->in[f->index_in] = input;

	
	sum_in =  InnerProductWithShift(f->order+1,f->a,f->in, f->index_in);
	if (f->b == 0L)
		sum_out = 0;	//no b coef means we have a FIR filter.
	else
		sum_out = InnerProductWithShift(f->order,  f->b,f->out,f->index_out);
	output = sum_in - sum_out;

	//Record the output
	if (--f->index_out <0)
		f->index_out=f->order-1;	//increment and wrap; output holds one less than input.
	f->out[f->index_out] = output;
	
*g = fff;	//copy back
	return output;
}

//**********************************************************************
float InnerProductWithShift(char n, float k[], float x[], char index_x0)
{
	float theSum=0;
	char i,j = index_x0;
	for (i=0; i<n; i++)
	{
		theSum += (*k++) * x[j];
		if (++j >= n) j=0;	//increment and wrap
	}
	return theSum;
}









#else	//Float
long InnerProductWithShift(char n, int k[], int x[], char index_x0);
//***********************************************************************************
int myFilter(int input)
{
	enum { ORDER=7, SCALE_A_PWR2=15+8, SCALE_B_PWR2=15-4,
		SCALE_A=(1L << SCALE_A_PWR2), SCALE_B=(1L << SCALE_B_PWR2)}; //use enum to limit scope of constants

	static int a[ORDER+1] = {
		SCALE_A * 0.00009034894,
		SCALE_A * 0.0006324426,
		SCALE_A * 0.0018973278,
		SCALE_A * 0.003162213,	//scale chosen so that number is < 2^15.
		SCALE_A * 0.003162213,
		SCALE_A * 0.0018973278,
		SCALE_A * 0.0006324426,
		SCALE_A * 0.00009034894,
};  	//coefficients of the filter
	static int b[ORDER] = {  //coefficients of the filter; b0 = 1
		SCALE_B * -4.18233,
		SCALE_B * 7.8717194,
		SCALE_B * -8.530942, //scale chosen so that number is < 2^15.
		SCALE_B * 5.7099447,
		SCALE_B * -2.3492472,
		SCALE_B * 0.5482648,
		SCALE_B * -0.055844672,
};
	static int in[ORDER+1];
	static int out[ORDER];
	static FilterParameters filter = {
		ORDER,
		in, a, SCALE_A_PWR2, 0,
		out,b, SCALE_B_PWR2, 0
	};
	int v = digitalFilter(input, &filter);
	return v;
}
//***********************************************************************************
int digitalFilter (int input, FilterParameters *g)
{
	long sum_in, sum_out, output;

	FilterParameters fff, *f=&fff;
	*f = *g;

	//first, record the new input. Most recent with lowest indice.
	if (--f->index_in <0)
		f->index_in=f->order;	//increment and wrap
	f->in[f->index_in] = input;

	
	sum_in =  InnerProductWithShift(f->order+1,f->a,f->in, f->index_in);
	sum_in =  mDivideByPowerOf2(sum_in,f->scale_a_pwr2);

	sum_out = InnerProductWithShift(f->order,  f->b,f->out,f->index_out);
	sum_out = mDivideByPowerOf2(sum_out,f->scale_b_pwr2);

//	output = mDivideByPowerOf2(sum_in,f->scale_a_pwr2) + mDivideByPowerOf2(sum_out,f->scale_b_pwr2);
	output = sum_in - sum_out;

	//Record the output
	if (--f->index_out <0)
		f->index_out=f->order-1;	//increment and wrap; output holds one less than input.
	f->out[f->index_out] = output;
	
*g = fff;	//copy back
	return output;
}

//**********************************************************************
long InnerProductWithShift(char n, int k[], int x[], char index_x0)
{
	long theSum=0;
	char i,j = index_x0;
	for (i=0; i<n; i++)
	{
		theSum += (long)(*k++) * x[j];
		if (++j >= n) j=0;	//increment and wrap
	}
	return theSum;
}
#endif
