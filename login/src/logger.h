#ifndef __logger_h__
#define __logger_h__


#include <stdio.h>
//#include "pmutex.h"
#include <string>

class tlogger
{
	static const int nbuffersize=2*1024*1024;
	char buffer[nbuffersize];
//	pmutex mutex;
	int nerrors;
	FILE *f1;
	std::string prefix;
public:
	tlogger(const std::string& prefix, const std::string& fname = NULL);
	~tlogger();
	void log(const char *s, ...);
	void elog(const char *s, ...);
	int gete();

	tlogger& operator << (char a){log("%c", a);return *this;}
	tlogger& operator << (int a){log("%d", a);return *this;}
	tlogger& operator << (float a){log("%f", a);return *this;}
	tlogger& operator << (char* a){log("%s", a);return *this;}
	tlogger& operator << (std::string &a){log("%s", a.c_str());return *this;}
/*	tlogger& operator << (long long a)
	{
		string str=toString(a);
		log("%s", str.c_str());
		return *this;
	}
*/
	class endl
	{
	};
	tlogger& operator << (const endl &a){log("\n");return *this;}
};

extern tlogger logger;

#endif
