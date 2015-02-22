#include "reciever.h"
#include <winsock2.h>
#include "logger.h"
#include <stdexcept>
reciever::reciever()
{
	clear();
}

void reciever::clear()
{
	header.clear();
	header.resize(9);
	puffer.clear();
	rhead=0;
	rpuffer=0;
	torecieve=0;
	readingpos=0;
}

void reciever::init(int sck1)
{
	clear();
	sck=sck1;
}

int reciever::recieve()
{
	int a;
	if(rhead<9)
	{
    	a=recv(sck, (char*)&header[rhead], 9-rhead, 0);
    	if(a>0)rhead+=a;
		if(a==0)return 1;
    	if(((rhead>0)&&(header[0]!=0x5e))||(a<0))return 2;
    	if(rhead>=9)
    	{
            torecieve=*(int*)(&header[5])+4;
			if((torecieve<0)||(torecieve>=reciever::toobig))
			{
				throw std::range_error(std::string("Reciever invalid packet size: ")+toString(torecieve));
			}
			puffer.resize(torecieve+5);
            rpuffer=0;
        }
	}else
	{
    	a=recv(sck, (char*)&puffer[rpuffer], torecieve-rpuffer, 0);
    	if(a==0)return 1;
        if(a>0)rpuffer+=a;
		if(a<0)return 2;
	    if(rpuffer==torecieve)return 3;
	}
    return 0;
}

char reciever::get1char()
{
	char ch;
	if(readingpos+1<=rpuffer)
	{
		ch=puffer[readingpos];
		readingpos++;
	}else
	{
//		dump();
		std::range_error("Reciever overflow get1char "+toString(1)+" "+toString(readingpos)+" "+toString(rpuffer));
	}
	return ch;
}

int reciever::getpstr(std::string &p)
{
	int n;
	get(n);
	rangecheck(n);
	char ch;
	ch=puffer.at(readingpos+n);
	puffer.at(readingpos+n)=0;
	p=puffer.at(readingpos);
	puffer.at(readingpos+n)=ch;
	readingpos+=n;
	return n;
}

int reciever::getpstr(char *p, int f)
{
	int n;
	get(n);
	if(n+1>f)
	{
		back(sizeof(int));
		return -1;
	}else
	{
		getnchar(p, n);
		p[n]=0;
	}
	return n;
}

void reciever::getnchar(char *p, int n)
{
	if((n<0)||(n>rpuffer-readingpos))std::range_error("Reciever overflow getnchar "+toString(n)+" "+toString(readingpos)+" "+toString(rpuffer));
	memcpy(p, &puffer[readingpos], n);
	readingpos+=n;
}

int reciever::getlast4()
{
	int a;
	a=*(int*)(&puffer[rpuffer-4]);
	return a;
}

reciever& reciever::operator=(const reciever& a)
{
	sck=a.sck;
	rhead=a.rhead;
	rpuffer=a.rpuffer;
	torecieve=a.torecieve;
	readingpos=a.readingpos;
	header=a.header;
	puffer=a.puffer;
	return *this;
}

void reciever::dump()
{
	int a,b;
	logger.log("Torecieve: %d\n", this->torecieve);
	logger.log("Head(%d):\n", rhead);
	for(a=0,b=0;a<this->rhead;a++)
	{
		logger.log("%02x ", (int)(unsigned char)this->header[a]);
		if(++b==26){logger.log("\n");b=0;}
	}
	logger.log("\nMsg(%d):\n", rpuffer);
	for(a=0,b=0;a<this->rpuffer;a++)
	{
		logger.log("%02x ", (int)(unsigned char)this->puffer[a]);
		if(++b==26){logger.log("\n");b=0;}
	}
	logger.log("\n");
}
