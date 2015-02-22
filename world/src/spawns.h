#ifndef __spawns_h__
#define __spawns_h__

#include <list>
#include "scheduler.h"

class character_base;
class cluster;
class spawner
{
	void removethis();
	schedulerl<spawner*>::sremover remover;
protected:
	cluster *cl;
public:
	spawner(cluster *c);
	virtual ~spawner()
	{
		removethis();
	}
	virtual void timer()=0;
	virtual void unregister(character_base *o)=0;
};

class mobspawn:public spawner
{
	friend class cluster;
	friend class character_mob;

	int type;
	int normal;
	int agro;
	float x;
	float y;
	float z;
	int rx;
	int ry;
	int mapid;
	int respawnrate;
	long long lastrespawn;

	int nagro;
	int nnormal;
public:
	mobspawn(cluster *c, int type1, int normal1, int agro1, float x1, float y1, float z1, int rx1, int ry1, int mapid1, int respawnrate1)
		:spawner(c), type(type1),normal(normal1), agro(agro1), x(x1), y(y1), z(z1), rx(rx1/2), ry(ry1/2), mapid(mapid1), respawnrate(respawnrate1*1000)
		,nnormal(0),nagro(0),lastrespawn(0)
	{

	}
	virtual ~mobspawn(){}
	void unregister(character_base *);
	void timer();
};


class itemspawn:public spawner
{
	int type;
	int num;
	float x;
	float y;
	float z;
	int rx;
	int ry;
	int mapid;
	int respawnrate;
	long long lastrespawn;

	int n;
public:
	itemspawn(cluster *c, int type1, int normal, float x1, float y1, float z1, int rx1, int ry1, int mapid1, int respawnrate1)
		:spawner(c), type(type1),num(normal), x(x1), y(y1), z(z1), rx(rx1/2), ry(ry1/2), mapid(mapid1), respawnrate(respawnrate1*1000)
		,n(0),lastrespawn(0)
	{}
	virtual ~itemspawn(){}
	void unregister(character_base *)
	{
		n--;
	}
	void timer();
};

class objaction;
class objactionfactory
{
public:
	virtual objaction* operator()()=0;
};

class gobjspawner:public spawner
{
	int id, mapid,n, resp, rx, ry;
	float x,y,z;
	int torespawn;
	objactionfactory &oaf;
public:
	gobjspawner(cluster *c, int id1, int mapid1, float x1, float y1, float z1, int n1, int r, int rx1, int ry1, objactionfactory &oaf1)
		:spawner(c),id(id1),mapid(mapid1),x(x1),y(y1),z(z1),n(n1),resp(r),rx(ry1/2),ry(ry1/2),torespawn(n1),oaf(oaf1)
	{}
	virtual ~gobjspawner(){};
	void timer();
	void unregister(character_base *o)
	{
		torespawn++;
	}
};

class flarisdroom:public spawner
{
	struct tfdp
	{
		float x;
		float y;
		float z;
	};
	static tfdp fdp[7];
	std::list<int> torespawn;
public:
	flarisdroom(cluster *c):spawner(c)
	{
		for(int a=0;a<7;a++)torespawn.push_front(a);
	}
	virtual ~flarisdroom(){}
	void timer();
	void unregister(character_base *o);
};

#endif