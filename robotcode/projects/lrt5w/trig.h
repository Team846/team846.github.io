#ifndef __trig_h
#define __trig_h


//sine128() & cosine128() return values on {-128,128}
//  period is 256.  Casting to a char  arg implicitly converts
// angles greater (positive or negative) onto  the correct range;
//D.Giandomenico

int sine128(char angle256);
int cosine128(char angle256);


#endif	//__trig_h
