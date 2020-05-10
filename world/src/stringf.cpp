#include "stdafx.h"
#include "stringf.h"

#include <stdio.h>
#include <config.h>

char ptable[]="0123456789ABCDEF";


unsigned long long toUnsignedLongLong(const std::string &str)
{
	unsigned long long r=0;
	int b=0;
	const char *p=str.c_str();
	while((p[b]!=0)&&(p[b]<33))b++;
	while((p[b]>='0')&&(p[b]<='9'))
	{
		r*=10;
		r+=p[b]-'0';
		b++;
	}
	return r;
}

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


std::string toString(int32_t a)
{
	char puffer[24];
	snprintf(&puffer[0], 24, "%d", a);
	return std::string(&puffer[0]);
}

std::string toString(uint32_t a)
{
	char puffer[24];
	snprintf(&puffer[0], 24, "%u", a);
	return std::string(&puffer[0]);
}

std::string toString(float a)
{
	char puffer[64];
	snprintf(&puffer[0], 64, "%f", a);
	return std::string(&puffer[0]);
}

int toInt(const std::string &str)
{
	int a=0;
	sscanf_s(str.c_str(), "%d", &a);
	return a;
}

float toFloat(const std::string &str)
{
	float a=0;
	sscanf_s(str.c_str(), "%f", &a);
	return a;
}

int toInt(const char *p)
{
	int a=0;
	sscanf_s(p, "%d", &a);
	return a;
}

float toFloat(const char *p)
{
	float a=0;
	sscanf_s(p, "%f", &a);
	return a;
}

std::string& tostring(std::string &str)
{
	return str;
}

std::string toString(const char *p)
{
	std::string str=p;
	return str;
}
/*
int strlen(const char *p)
{
	int a=0;
	if(p!=0)for(;p[a]!=0;a++);
	return a;
}
*/
std::string catclaws(const std::string &p)
{
	std::string s;
	s=(char)34;
	s+=p;
	s+=(char)34;
	return s;
}
