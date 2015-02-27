#ifndef __reciever_h__
#define __reciever_h__

#include "vector2.h"
#include <string>
#include "stringf.h"
#include <stdexcept>

class reciever
{
	friend class cluster;
	std::vector2<unsigned char> header;
	std::vector2<unsigned char> puffer;
	int sck;
	int rhead;
	int rpuffer;
	int torecieve;
	int readingpos;
	static const int toobig=4096;
public:
	reciever();
	reciever& operator=(const reciever &a);
	void clear();
	void init(int sck1);
	int recieve();
	char get1char();

	int getlast4();
	void getnchar(char *p, int n);
	int getpstr(char *p, int r);
//	int getpstr(std::string &p);
	template <class T>
	void get(T &a)
	{
		if(readingpos+(int)sizeof(T)<=rpuffer)
		{
			a=*(T*)(&puffer.at(readingpos));
			readingpos+=sizeof(T);
		}else throw std::range_error("Reciever overflow get "+toString((int)sizeof(T))+" "+toString(readingpos)+" "+toString(rpuffer));
	}
	void rewind()
	{
		readingpos=0;
	}
	void seek(int a)
	{
		readingpos=a;
	}
	void forward(int a)
	{
		readingpos+=a;
	}
	void back(int a)
	{
		readingpos-=a;
	}
	int length()
	{
		return rpuffer;
	}
	int getposition()
	{
		return readingpos;
	}
	void rangecheck(int n)
	{
		if((n<0)||(n>rpuffer-readingpos))throw std::range_error(std::range_error("Reciever overflow rangecheck "+toString(readingpos)+" "+toString(rpuffer)));
	}
	void dump();
};

#endif
