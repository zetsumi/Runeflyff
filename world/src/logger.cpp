#include "stdafx.h"
#include "logger.h"
#include <stdio.h>


	#include <conio.h>
	#include "mysql.h"
	#include <windows.h>

#define __LOG__NORMAL__
#define __LOG__ERRORS__

tlogger logger("", "logger.dat");

tlogger sqllogger("", "queries.dat", false);

tlogger::tlogger(const std::string& prefix, const std::string& fname, bool onscreen):f1(0),nerrors(0),prefix(prefix), os(onscreen)
{
	if(fname.c_str() != NULL)
        fopen(&f1, fname.c_str(), "a+b");
}

tlogger::~tlogger()
{
	int ne;
	{
		pmutex::unlocker mlock=mutex.lock();
		ne=nerrors;
	}

	log("Errors=%d\n", ne);

	{
		pmutex::unlocker mlock=mutex.lock();
		if(f1!=0)fclose(f1);
		f1=0;
	}

}

void tlogger::log(const char *s, ...)
{
#ifdef __LOG__NORMAL__
	va_list lista;
	if(s!=0)
	{
		pmutex::unlocker mlock=mutex.lock();

		va_start(lista, s);
		vsprintf(&buffer[0],  s, lista);
		va_end(lista);
		buffer[nbuffersize-1]=0;
		if(os)printf("%s%s", prefix.c_str(), &buffer[0]);
		if(f1!=0)
		{
			fprintf(f1, "%s", &buffer[0]);
//			fflush(f1);
		}
	}
#endif
}

void tlogger::elog(const char *s, ...)
{
#ifdef __LOG__ERRORS__
	va_list lista;
	if(s!=0)
	{
		pmutex::unlocker mlock=mutex.lock();

		nerrors++;
		va_start(lista, s);
		vsprintf(&buffer[0], s, lista);
		va_end(lista);
		buffer[nbuffersize-1]=0;
		fprintf(stderr, "ERROR: %s%s", prefix.c_str(), &buffer[0]);
		if(f1!=0)
		{
			fprintf(f1, "ERROR: %s", &buffer[0]);
			fflush(f1);
		}

	}
#endif
}

int tlogger::gete()
{
	int retval;
	{
		pmutex::unlocker mlock=mutex.lock();
		retval=nerrors;
	}
	return retval;
}
