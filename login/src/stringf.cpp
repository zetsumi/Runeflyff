#include "stringf.h"
#include <config.h>

#include <stdio.h>
char ptable[]="0123456789ABCDEF";
std::string toHex(uint32_t a)
{
	std::string str;
	char p[10];
	p[9]=0;
	int b=8;
	do{
		p[b--]=ptable[a&15];
		a>>=4;
	}while((b>=0)&&(a!=0));
	str=&p[b+1];
	return str;
}

std::string toHex(uint64_t a)
{
	std::string str;
	char p[20];
	unsigned int c;
	p[19]=0;
	int b=18;
	do{
		c=a&15;
		p[b--]=ptable[c];
		a>>=4;
	}while((b>=0)&&(a!=0));
	str=&p[b+1];
	return str;
}

std::string toString(uint64_t a)
{
	std::string str;
	char p[22];
	p[21]=0;
	int b=20;
	do
	{
		p[b--]=a%10+'0';
		a/=10;
	}while(a!=0);
	str=&p[b+1];
	return str;
}

std::string toString(int32_t a)
{
	char puffer[24];
	snprintf(&puffer[0], 24, "%d", a);
	return std::string(&puffer[0]);
}

std::string toString(int64_t a)
{
	std::string str;
	char p[22];
	p[21]=0;
	int b=20;
	bool e=(a<0);
	if(e)a*=-1;
	do
	{
		p[b--]=a%10+'0';
		a/=10;
	}while(a!=0);
	if(e)p[b--]='-';
	str=&p[b+1];
	return str;
}


std::string operator+(const char *s, const std::string& s2)
{
	return std::string(s)+s2;
}

std::string toString(float a)
{
	char puffer[64];
	snprintf(&puffer[0], 64, "%f", a);
	return std::string(&puffer[0]);
}

int toInt(std::string str)
{
	int a=0;
	sscanf(str.c_str(), "%d", &a);
	return a;
}

float toFloat(std::string str)
{
	float a=0;
	sscanf(str.c_str(), "%f", &a);
	return a;
}

int toInt(char *p)
{
	int a=0;
	sscanf(p, "%d", &a);
	return a;
}

float toFloat(char *p)
{
	float a=0;
	sscanf(p, "%f", &a);
	return a;
}

std::string& tostring(std::string &str)
{
	return str;
}

std::string toString(char *p)
{
	std::string str=p;
	return str;
}

int strlen(char *p)
{
	int a=0;
	if(p!=0)for(;p[a]!=0;a++);
	return a;
}

std::string idez(const char *p)
{
	std::string r;
	r=(char)34;
	r+=p;
	r+=(char)34;
	return r;
}

std::string idez(const std::string p)
{
	std::string r;
	r=(char)34;
	r+=p;
	r+=(char)34;
	return r;
}
