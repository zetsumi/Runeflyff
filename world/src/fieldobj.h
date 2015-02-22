#ifndef __fieldobj_h__
#define __fieldobj_h__

#include "vector.h"
#include <list>

class cluster;
class tgridcell;
class character_base;
class fieldobj
{
protected:
	cluster *cl;
	tgridcell *gridcell;
	vector3d<> pos;
	int mapid;
	void destroy();
	std::list<fieldobj*>::iterator remover;
public:
	fieldobj(cluster *c, int m, const vector3d<> &p);
	virtual ~fieldobj()
	{
		destroy();
	};
	virtual bool check(const vector3d<> &s, const vector3d<> &e, character_base *p)=0;
};

class tplayer;
struct tskilldata;
class f_psywall:public fieldobj
{
	vector3d<> normal;
	float range;
	tplayer *owner;
	tskilldata *s;
public:
	f_psywall(cluster *c, int m, const vector3d<> &p, const vector3d<> &n, float r, tplayer *o);
	virtual ~f_psywall()
	{
	};
	bool check(const vector3d<> &s, const vector3d<> &e, character_base *p);
};

class f_telc:public fieldobj
{
	float range;
	vector3d<> dest;
	int destmap;
public:
	f_telc(cluster *c, int m, const vector3d<> &p, float r, const vector3d<> &d, int dm)
		:fieldobj(c,m,p),range(r*r),dest(d), destmap(dm){}
	virtual ~f_telc()
	{
	};
	bool check(const vector3d<> &s, const vector3d<> &e, character_base *p);
};

#endif
