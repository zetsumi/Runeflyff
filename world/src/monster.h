#ifndef __monster_h__
#define __monster_h__

#include "buff.h"
#include "character_base.h"
#include "spawns.h"
#include "scheduler.h"

extern const float md;

class tattacker
{
protected:
	float nar;
	int na;
public:
	tattacker():na(0x1d),nar(md){}
	virtual void operator()(character_buffable *attacker, character_buffable *target)
	{
		attacker->attack(na);
	}
	float nextattackrange()
	{
		return nar;
	}
};

class tcwattack:public tattacker
{
public:
	tcwattack():tattacker(){}
	virtual void operator()(character_buffable *attacker, character_buffable *target)
	{
		switch(na)
		{
		case 0x1d:
			attacker->attack(na);
			break;
		}

	}
};

extern tattacker basic_attack;

class tgridcell;
class character_mob : public character_buffable
{
	friend class mobspawn;
	tattacker &attacker;
	bool agro;
	void removethis();
	void drop(character_buffable *koer, bool fc, bool gb);
	enum tmobstate {mswait, msattack, mskoed, msdropped} actstate;
	long long kotime;
	mobspawn *mobs;
	bool registeredAttack;
	int endval;
	void onHpChanged();
	long long lastattack;
	int attacktrate;
	bool flying;
	bool onwayback;

protected:
	int *bsstate, *bsstate2;
	virtual void onKo(character_buffable *p);
	long long staytime;
	void dropitem(item &targy, vector3d<> p, tplayer *k, bool i=false);
public:
	schedulerl<character_mob*>::sremover mobremover;
	schedulerl<character_mob*>::sremover agromobremover;
	schedulerl<character_mob*>::sremover mobattackremover;
	void arrived();
	bool agrocheck();
	void addstat1(int stat, int val, bool forcesub=false);
//	int koerid,koerdmg;
	character_mob(int t, cluster *cl, int m, float x1, float y1, float z1, bool a, float s=1.0f, mobspawn* ms=0, tattacker &at=basic_attack);
	virtual ~character_mob()
	{
		removethis();
	}
	virtual bool act();
	virtual bool mobattack();
	buffer &spawn();
	void adddelayed(buffer *b, long long t)
	{
		multicast(*b);
		delete b;
	}
};
/*
class tguild;
class cw_caged:public character_mob
{
	long long begintime;
	tguild *guild;
	void teleportout();
public:
	cw_caged(int t, cluster *cl, int m, float x1, float y1, float z1, bool a, mobspawn* ms, tplayer *gl);
	virtual ~cw_caged()
	{
		teleportout();
	}
	void onKo();
};
*/
#endif
