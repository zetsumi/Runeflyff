#ifndef __buffer_h__
#define __buffer_h__

#include <list>
#include "logger.h"

class tplayer;
class buffer
{
	buffer& operator=(const buffer& a);
public:
	static const int nsize=256;
protected:
	class bpcont
	{
		int n;
		bpcont(const bpcont &a);
		void operator=(const bpcont &a);
	public:
		bool haspointer;
		int pos;
		char puffer[nsize];
		bpcont():pos(0),n(1),haspointer(false){}
		void inc()
		{
			n++;
		}
		void dec()
		{
			n--;
			if(n<=0)delete this;
		}
		int getn()
		{
			return n;
		}
	};
	std::list<bpcont*> plist;
	bpcont *top;
	void raise()
	{
		top=new bpcont();
		plist.push_back(top);
	}
	void checknRaise(int s)
	{
		if(top==0)raise();
		else if(s>(nsize-top->pos)||(top->getn()>1))raise();
	}
	int len, commandnumber;
public:
	tplayer *forbidden;
	buffer():len(0),commandnumber(0),forbidden(0),top(0){};

	buffer::buffer(const buffer& a)
	:forbidden(a.forbidden),len(a.len),
	commandnumber(a.commandnumber),top(a.top)
	{
		for(std::list<bpcont * >::const_iterator i=a.plist.begin();i!=a.plist.end();++i)
		{
			plist.push_back(*i);
			(*i)->inc();
		}
	}
	~buffer()
	{
		while(!plist.empty())
		{
			plist.front()->dec();
			plist.pop_front();
		}
	}
	void copy(const buffer &a)
	{
		for(std::list<bpcont*>::const_iterator i=a.plist.begin();i!=a.plist.end();++i)
		{
			plist.push_back(new bpcont());
			plist.back()->pos=(*i)->pos;
			memcpy(plist.back()->puffer, (*i)->puffer, (*i)->pos);
		}
		len+=a.len;
		if(plist.empty())top=0;
		else top=plist.back();
		commandnumber+=a.commandnumber;
	}
	void add(const buffer &a)
	{
		for(std::list<bpcont*>::const_iterator i=a.plist.begin();i!=a.plist.end();++i)
		{
			plist.push_back(*i);
			(*i)->inc();
		}
		len+=a.len;
		if(plist.empty())top=0;
		else top=plist.back();
		commandnumber+=a.commandnumber;
	}
	int length()
	{
		return len;
	}
	void dump();
	buffer& cmd(int id, unsigned char c)
	{
		*this << id << c;
		commandnumber++;
		return *this;
	}
	int getcommandnumber()
	{
		return commandnumber;
	}

	void inc()
	{
		commandnumber++;
	}

	template <class T>
	buffer& operator <<(T a)
	{
//#ifdef _DEBUG
//		if(sizeof(T)>nsize)throw std::range_error("Data is too big");
//#endif
		len+=sizeof(T);
		checknRaise(sizeof(T));
		*(T*)(&top->puffer[top->pos])=a;
		top->pos+=sizeof(T);
		return *this;
	}

	template <class T>
	T* insert(T a)
	{
		len+=sizeof(T);
		checknRaise(sizeof(T));
		T *p=(T*)(&top->puffer[top->pos]);
		*p=a;
		top->pos+=sizeof(T);
		top->haspointer=true;
		return p;
	}
	void buffer::sndpstr(const std::string &p)
	{
		int a=p.length();
		*this << a;
		sndnchar(p.c_str(), a);
	}
private:
	void buffer::sndnchar(const char *p, int a)
	{
		for(int b=0;b<a;b++)*this << p[b];
		return;

		if(a<1)return;
		int d=0;
		checknRaise(8);/*1*/
		len+=a;
		if(a>nsize-top->pos)
		{
			d=nsize-top->pos;
			memcpy(&top->puffer[top->pos], p, d);
			top->pos=nsize;
			raise();
			a-=d;
		}
		for(;a>nsize;d+=nsize)
		{
			memcpy(top->puffer, &p[d], nsize);
			top->pos=nsize;
			raise();
			a-=nsize;
		}
		if(a>0)
		{
			memcpy(&top->puffer[top->pos], &p[d], a);
			top->pos+=a;
		}
	}

};

class sendablebuffer:public buffer
{
	int sck, insendpos;
	std::list<bpcont*>::iterator insend;
public:
	sendablebuffer();
	sendablebuffer(const buffer& a);
	void initflush(int sck);
	int continueflush(char *q);
};
#endif
