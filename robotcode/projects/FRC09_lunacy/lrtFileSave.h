#ifndef lrtFileSave_H_
#define lrtFileSave_H_
#include <cstdio>

class FileSave {
public:
//	FILE *fp;
	static const char *m_filename;
	
	
	float gain;
	float time;
	
	
	
	FileSave();
	~FileSave();
	
	void Read(void);
	void Write(void);
	void ClearVariables(void);
	void PrintVariables(void);
	
};

#endif /*lrtFileSave_H_*/
