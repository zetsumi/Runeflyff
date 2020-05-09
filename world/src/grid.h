#ifndef __grid_h__
#define __grid_h__

#include "vector2.h"
#include <list>
#include <map>
#include "logger.h"
#include "character_base.h"
#include "monster.h"
#include "character_npc.h"
#include "character_obj.h"
#include "fieldobj.h"
#include "listremover.h"

extern int cellsize;

template <int o>
constexpr int mod(int a)
{
    return a % o < 0 ?
           (a % o) + o :
           (a % o);
}

constexpr int mod(int a, int o)
{
    return a % o < 0 ?
           (a % o) + o :
           (a % o);
}


class tplayer;
class character_item;
class tgrid;
class tworld;
class buffer;
class character_buffable;
class character_mob;
class tgridcell
{
	friend class fieldobj;
	friend class tworld;
	friend class character_buffable;
	friend class character_mob;
	unsigned char mark;
//	bool setmarked;
//	std::list<tgridcell*>::iterator tbr;
	tgridcell *neighbour[9];
	tworld *owner;
	int x,z;
	void init();
	void deinit();
	int nelements,nplayers, nplayersaround;
	bool isneighbour(int x, int z)
	{
		x=this->x-x;
		z=this->z-z;
		return ((x>=-1)&&(x<=1)&&(z>=-1)&&(z<=1));
	}
	bool marked;
	void markbc();
	buffer *bs;
	std::list<buffer> bslist;
	void domulticast(tplayer *p=0);
	void removeallfrom(tplayer *p);
	void createallfor(tplayer *p);
	void addbs(buffer &bs);
	void incplayers()
	{
//		int a;
//		nplayers++;
//		for(a=0;a<9;a++)if(neighbour[a]!=0)neighbour[a]->nplayersaround++;
		fixplayers();
	}
	void decplayers()
	{
//		int a;
//		nplayers--;
//		for(a=0;a<9;a++)if(neighbour[a]!=0)neighbour[a]->nplayersaround--;
		fixplayers();
	}
	void fixplayers()
	{
		int a,b;
		b=players.size()-nplayers;
		nplayers+=b;
		if(b!=0)for(a=0;a<9;a++)if(neighbour[a]!=0)neighbour[a]->nplayersaround+=b;
		
	}
	std::list<character_mob*>::iterator *mobiterator;
	std::list<tplayer*>::iterator *playeriterator;
	listremover<tgridcell*> broadcast_remover;
public:
	void setmobiterator(std::list<character_mob*>::iterator *i)
	{
		mobiterator=i;
	}
	void clearmobiterator()
	{
		mobiterator=0;
	}
	void setplayeriterator(std::list<tplayer*>::iterator *i)
	{
		playeriterator=i;
	}
	void clearplayeriterator()
	{
		playeriterator=0;
	}
	std::list<tplayer*> players;
	std::list<character_obj*> objs;
	std::list<character_mob*> mobs;
	std::list<character_npc*> npcs;
	std::list<character_item*> items;
	std::list<fieldobj*> fieldobjs;
	tgridcell* getsz(int a)
	{
		return neighbour[a];
	}
	bool checkmove(const vector3d<> &s, const vector3d<> &e, character_base *p)
	{
		if(!fieldobjs.empty())
		{
			for(std::list<fieldobj*>::iterator i=fieldobjs.begin();i!=fieldobjs.end();++i)
			{
				if((*i)->check(s, e, p))return true;
			}
		}
		return false;
	}

	int playersaround()
	{
		return nplayersaround;
	}
	void multicast2(buffer &a);
	void getmulticast(tplayer *p);
	tgridcell(tworld *o, int x1, int z1);
	~tgridcell();

	std::list<character_npc*>::iterator addcharacter(character_npc *p);
	void removecharacter(character_npc *p);
	std::list<character_mob*>::iterator addcharacter(character_mob *p);
	void removecharacter(character_mob *p);
	std::list<character_obj*>::iterator addcharacter(character_obj *p);
	void removecharacter(character_obj *p);
	std::list<character_item*>::iterator addcharacter(character_item *p);
	void removecharacter(character_item *p);
	std::list<tplayer*>::iterator addcharacter(tplayer *p);
	void removecharacter(tplayer *p);

	void removecharacter(character_base *p)
	{
		switch(p->type)
		{
		case character_base::ttplayer:
			removecharacter((tplayer*)p);
			break;
		case character_base::ttmob:
			removecharacter((character_mob*)p);
			break;
		case character_base::ttnpc:
			removecharacter((character_npc*)p);
			break;
		case character_base::ttobject:
			removecharacter((character_obj*)p);
			break;
		case character_base::ttitem:
			removecharacter((character_item*)p);
			break;
		default:
			throw "hai";
		};
	}

	tgridcell* update(character_base* p, int x, int z);
	tgridcell* update(character_base* p, int xn, int zn, buffer &bs1);
	int getmapid();
	int getworldid();
};

class tworld
{
	friend class tgridcell;
	friend class tgrid;
	int mapid,worldid,hsize;
	tgrid *owner;
	bool running;
	std::vector2<std::list<tgridcell*> > M;
	std::list<tgridcell*> tomulticast;
//	void removemark(std::list<tgridcell*>::iterator &i)
//	{
//		(*i)->setmarked=false;
//		tomulticast.erase(i);
//	}
	void addbc(tgridcell *a, listremover<tgridcell*> &gcr)
	{
//		a->setmarked=true;
//		tomulticast.push_front(a);
//		a->tbr=tomulticast.begin();
		gcr.set(&tomulticast, a);
	}
	void domulticast()
	{
		while(!tomulticast.empty())
		{
			tomulticast.front()->broadcast_remover.clear();
//			tomulticast.front()->setmarked=false;
			tomulticast.front()->domulticast();
			tomulticast.pop_front();
		}
	}
	void removecell(tgridcell *r)
	{
		r->broadcast_remover.remove();
		if(r->marked)
		{
/*
			std::list<tgridcell*>::iterator i2;
			for(i2=tomulticast.begin();i2!=tomulticast.end();++i2)
			{
				if(*i2==r)
				{
					tomulticast.erase(i2);
					break;
				}
			}
*/
		}
		int m=(mod(r->x, hsize))+(mod(r->z, hsize))*hsize;
		std::list<tgridcell*>::iterator i;
		for(i=M.at(m).begin();i!=M.at(m).end();++i)
			if(*i==r)
			{
				M.at(m).erase(i);
//				logger.log("cell erased\n");
				break;
			}
				
	}
	tgridcell* createneighbour(int x, int z)
	{
		tgridcell *r;
		int m=(mod(x, hsize))+(mod(z, hsize))*hsize;
		r=new tgridcell(this, x, z);
		M.at(m).push_back(r);
		return r;
	}
	tgridcell* getneighbour(int x, int z)
	{
		tgridcell *r=0;
		int m=(mod(x, hsize))+(mod(z, hsize))*hsize;
		std::list<tgridcell*>::iterator i;
		for(i=M.at(m).begin();i!=M.at(m).end();++i)
			if(((*i)->x==x)&&((*i)->z==z))
			{
				r=*i;
				break;
			}
		return r;
	}
public:
	bool contains(int x, int z)
	{
		x/=cellsize;
		z/=cellsize;
		return (x>=0)&&(z>=0)&&(x<hsize)&&(z<hsize);
	}
	int getmapid(){return mapid;}
	int getworldid(){return worldid;}
	tworld(int mid, int wid, tgrid *o, int h)
		:mapid(mid),worldid(wid), owner(o), running(true),hsize(h)
	{
		M.resize(hsize*hsize);
	}
	~tworld()
	{
		int a;
		tgridcell *r;
		running=false;
		for(a=0;a<hsize*hsize;++a)
			while(!M.at(a).empty())
			{
				r=M.at(a).front();
				delete r;
				M.at(a).pop_front();
			}
		
	}
	tgridcell& get(int x, int z)
	{
		x/=cellsize;
		z/=cellsize;
		int m=(mod(x, hsize))+(mod(z, hsize))*hsize;
		tgridcell *r=0;
		std::list<tgridcell*>::iterator i;
		for(i=M.at(m).begin();i!=M.at(m).end();++i)
			if(((*i)->x==x)&&((*i)->z==z))
			{
				r=*i;
				break;
			}
		if(r==0)
		{
			r=new tgridcell(this, x, z);
			M.at(m).push_back(r);
		}
		return *r;
	}
};

class tgrid
{
	friend class tworld;
	std::map<int, tworld*> worlds;
	void releasemap(int wid)
	{
		std::map<int, tworld*>::iterator m=worlds.find(wid);
		if(m!=worlds.end())
		{
			delete m->second;
			worlds.erase(m);
		}
		worldlookup[maplookup[wid]]=1;
		maplookup[wid]=1;
	}
	std::vector2<int> maplookup;
	std::vector2<int> worldlookup;
public:
	int getmap(int a)
	{
		int r;
		if((a<1)||(a>(int)maplookup.size()))r=1;
		else r=maplookup[a];
		return r;
	}
	int getworld(int a)
	{
		int r;
		if((a<1)||(a>(int)worldlookup.size()))r=1;
		else r=worldlookup[a];
		return r;
	}
	void domulticast()
	{
		std::map<int, tworld*>::iterator i;
		for(i=worlds.begin();i!=worlds.end();++i)
			i->second->domulticast();
	}
	~tgrid()
	{
		std::map<int, tworld*>::iterator i;
		for(i=worlds.begin();i!=worlds.end();++i)delete i->second;
	}
	void addmap(int mapid, int worldid, int hsize)
	{
		if((int)maplookup.size()<worldid+1)maplookup.resize(worldid+1, 1);
		if((int)worldlookup.size()<mapid+1)worldlookup.resize(mapid+1, 1);

		worlds.insert(std::pair<int, tworld*>(worldid, new tworld(mapid, worldid, this, hsize)));
		maplookup[worldid]=mapid;
		worldlookup[mapid]=worldid;
	}
	bool validworld(int worldid)
	{
		std::map<int, tworld*>::iterator m=worlds.find(worldid);
		return m!=worlds.end();
	}
	bool contains(int worldid, int x, int z)
	{
		tworld *mm=0;
		std::map<int, tworld*>::iterator m=worlds.find(worldid);
		if(m!=worlds.end())mm=m->second;
		if(mm==0)return false;
		return mm->contains(x,z);
	}
	tgridcell* get(int worldid, int x, int z)
	{
		tworld *mm=0;
		tgridcell *retval=0;
		std::map<int, tworld*>::iterator m=worlds.find(worldid);
		if(m!=worlds.end())mm=m->second;
/*
		if(m==0)
		{
			std::map<int, tworld*>::iterator m=worlds.find(1);
			if(m!=worlds.end())mm=m->second;
		}
*/
		retval=&mm->get(x,z);
		return retval;
	}
};

#endif
