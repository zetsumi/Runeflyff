#ifndef __character_obj_h__
#define __character_obj_h__

#include "character_base.h"

class tplayer;
class character_obj;

class objtimerguard;
class objtimerguard;
class objaction
{
protected:
	friend class objtimercont;
	friend class objtimerguard;
	character_obj *owner;
	long long t;
	std::list<objtimerguard*> guards;
	void destroy();
public:
	virtual void setowner(character_obj *o)
	{
		owner=o;
	}
	objaction():t(0){};
	virtual ~objaction()
	{
		destroy();
	};
	virtual void operator()(character_obj *o, tplayer *p)=0;
	virtual void timer(){};
};

class objtimerguard
{
	objaction *a;
	std::list<objtimerguard*>::iterator guardremover;
	~objtimerguard()
	{
		if(a!=0)a->guards.erase(guardremover);
	}
	int nref;
public:
	objtimerguard(objaction *o):a(o),nref(1)
	{
		a->guards.push_front(this);
		guardremover=a->guards.begin();
	}
	void call()
	{
		if(a!=0)a->timer();
	}
	void clear()
	{
		a=0;
	}
	void inc()
	{
		nref++;
	}
	void dec()
	{
		nref--;
		if(nref<=0)delete this;
	}
};

class objtimercont
{
	long long t;
	objtimerguard *g;
public:
	void call()
	{
		g->call();
	}
	long long gettime() const
	{
		return t;
	}
	objtimercont(objaction *p):t(p->t)
	{
		g=new objtimerguard(p);
	};
	~objtimercont()
	{
		g->dec();
	}
	objtimercont(const objtimercont &b):g(b.g),t(b.t)
	{
		g->inc();
	};
	objtimercont& operator=(const objtimercont &b)
	{
		g=b.g;
		g->inc();
		t=b.t;
		return *this;
	}

	bool operator==(const objtimercont &b) const
	{
		return t==b.t;
	}
	bool operator<(const objtimercont &b) const
	{
		return t>b.t;
	}
};

class spawner;
class character_obj:public character_base
{
	friend class spawner;
	friend class flarisdroom;
	friend class objpsywall;
	void setmodeltype(int a)
	{
		modeltype=a;
	}
	void removethis();
	objaction *oa;
	spawner *os;
	int tag;
public:
	character_obj(int t, cluster *c, int m, float x1, float y1, float z1, int d, int s, objaction *oa1=0, spawner *os=0);
	virtual ~character_obj()
	{
		removethis();
	}
	void action(tplayer *p)
	{
		if(oa!=0)(*oa)(this, p);
	}
};

#endif
