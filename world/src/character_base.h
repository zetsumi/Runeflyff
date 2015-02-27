#ifndef __character_base_h__
#define __character_base_h__

#include "buffer.h"
#include "item.h"
#include "vector.h"
#include "funkciok.h"
#include <list>
#include "adat.h"
#include "pmutex.h"
#include "scheduler.h"
#include "gridremover.h"

int iabs(int a);



class cluster;
class tgridcell;
class character_base;
class f_telc;
bool moverfunkc(character_base *p);
class tgridcellremover;
class character_base
{
	friend class f_telc;
	friend bool moverfunkc(character_base *p);
	vector3d<> start, irany;
	float movelength;
	long long movestart, lastmove;
	bool moveing;
	char movetype;
	buffer *spb;
	character_base *focus;
	character_base *followed;
	std::list<character_base*> focuslist;
	std::list<character_base*>::iterator focusremover;
	std::list<character_base*>::iterator addfocus(character_base *f);
//	std::list<character_base*>::iterator moverremover;
	schedulerl<character_base*>::sremover moverremover;

	std::list<std::pair<character_base*, float> > followers;
	std::list<std::pair<character_base*,float> >::iterator fr;
	float followdistance,followdistance2;

	void followersonstop();
	void checkmove();
protected:
	void setmoveing(bool f)
	{
		moveing = f;
	}
	vector3d<> cel;
	buffer *sp9;
	long long lsp9;
	void removefocuses();

//	tgridcell *gridcell;
	float *bsx,*bsy,*bsz;
	short *bsdir;
	buffer *rs,*sp;
	int modeltype;
	vector3d<> lastpos;
	int mapid,dir;
	int state,state2, size;
	float speed,basespeed;
	int id;
	void setandcheckpos(float fx,float fy,float fz);
public:
	tgridcellremover gridcell;
	int modelid;
	vector3d<> pos;
	const vector3d<>& getlastpos(){return lastpos;}
	void clearfollowers()
	{
		while(!followers.empty())
		{
			followers.front().first->setfollowed(0, 0);
		}
	}
	character_base* getfollowed()
	{
		return followed;
	}
	void setfollowed(character_base *p, float range)
	{
		if(((followed!=p)||(range!=followdistance2))&&(p!=this))
		{
			if(followed!=0)followed->followers.erase(fr);
			if((p!=0)&&(p->followed==this))p=0;
			followed=p;
			followdistance=range*range;
			followdistance2=range;
			if(followed!=0)
			{
				followed->followers.push_front(std::pair<character_base*, float>(this, range));
				fr=followed->followers.begin();
				movefollow();
				buffer bs;
				gotoid(&bs, id, p->getId(), range);
				multicast(bs);
			}else stopmove();
		}
	}
	const vector3d<>& getpos()
	{
		return pos;
	}
	int getdir(const vector3d<> &v2);
	void setlastpos()
	{
		lastpos=pos;
	}
	float getspeed(){return speed;}
	float getbasespeed(){return basespeed;}
	bool domove();
	void processfollowers(bool stm);
	int abletomove;
	int getmodelid(){return modelid;}
	const vector3d<>& getcel(){return cel;}
	const vector3d<>& getstart(){return start;}
	cluster *cl;
	enum ttype {ttnone, ttplayer, ttmob, ttnpc, ttitem, ttobject} type;
	ttype getType()
	{
		return type;
	}

	int getmapid()
	{
		return mapid;
	}
	float getx()
	{
		return pos.x;
	}
	float gety()
	{
		return pos.y;
	}
	float getz()
	{
		return pos.z;
	}

	int getId()
	{
		return id;
	}
	int isMoveing()
	{
		int a=0;
		if(moveing)a=movetype+1;
		return a;
	}
	bool movefollow();
	void movein(const vector3d<>& i);
	void moveto(float x1, float y1, float z1);
	virtual void stopmove();
	void startmove()
	{
		if(!followers.empty())
		{
			buffer bs;
			for(std::list<std::pair<character_base*, float> >::iterator i=followers.begin();i!=followers.end();++i)
			{
				if(!i->first->isMoveing())
				{
					i->first->movefollow();
					gotoid(&bs, i->first->getId(), id, i->second);
				}
			}
			if(bs.getcommandnumber()>0)multicast(bs);
		}
	};
	float distance(const character_base& a)
	{
		vector3d<> v=pos;
		v-=a.pos;
		return v*v;
	}
	float distanceny(character_base &p)
	{
		float x=pos.x-p.pos.x;
		float z=pos.z-p.pos.z;
		return x*x+z*z;
	}

	void chat(const char *t);
	character_base *getfocus()
	{
		return focus;
	}
	virtual void arrived(){};
	void setfocus(character_base *f);
	virtual void onChangePosition();

	virtual void multicast(buffer &bs);

	bool spmovepacket;
	character_base();
	void init()
	{
		rs=0;
		sp=0;
		spb=0;
		abletomove=0;
		moveing=false;
		size=100;
		state=1;
		state2=0;
		speed=6.0f;
		setfocus(0);
		dir=rnd(3600);
		sp9=0;
		lsp9=0;
		gridcell.remove();
	}
	void deinit()
	{
		gridcell.remove();
		setfocus(0);
		removefocuses();
		clearfollowers();
		setfollowed(0, 0);
		stopmove();
		delete rs;
		delete sp;
		delete spb;
		delete sp9;
		rs=0;
		sp=0;
		spb=0;
		sp9=0;
	}
	virtual ~character_base()
	{
		deinit();
	}
	void getmovepacket(buffer *bs)
	{
		if((isMoveing()==3)&&(followed!=0))gotoid(bs, id, followed->getId(), followdistance2);
		else gotoxyz(bs, id, cel.x, cel.y, cel.z);
	}
	void createremovepacket()
	{
		delete rs;
		rs=new buffer;
		rs->cmd(id, 0xf1);
		if(type==ttplayer)rs->forbidden=(tplayer*)this;
	}
	buffer& removepacket()
	{
		return *rs;
	}
	virtual buffer& spawn()
	{
		if(!moveing)
		{
			if(spmovepacket)
			{
				delete sp9;
				sp9=0;
			}
			return *sp;
		}else
		{
			if(!spmovepacket)
			{
				sp9=new buffer(*sp);
				getmovepacket(sp9);
			}
			return *sp9;
		}
	}

};

struct itemremover;
class cluster;
class spawner;
class character_item:public character_base
{
	friend struct itemremover;
	void removethis();
	long long spawntime, picktime;
	itemremover *ir;
	item targy;
	void nullir()
	{
		ir=0;
	}
	spawner *sw;
	void *owner, *owner2;
public:
	void setnum(int n);
	bool ableToPick(void *p);
	item getitem()
	{
		return targy;
	}
	long long getspawntime()
	{
		return spawntime;
	}
	character_item(item t, cluster *c, int m, float x1, float y1, float z1, void *owner=0, spawner *sw=0, void* owner2=0);
	virtual ~character_item()
	{
		removethis();
	}
	void pickedup();
};

struct itemremover
{
private:
	friend class cluster;
	character_item *i;
public:
	void zeroit()
	{
		if(i!=0)
		{
			i->nullir();
			i=0;
		}
	}
	itemremover(character_item *p):i(p){}
	itemremover(const itemremover& a):i(a.i){}
	~itemremover()
	{
		if(i!=0)
		{
			i->nullir();
			delete i;
		}
	}
	bool operator==(const itemremover& a) const
	{
		return i->getspawntime()==a.i->getspawntime();
	}
	bool operator<(const itemremover& a) const
	{
		return i->getspawntime()<a.i->getspawntime();
	}
};


#endif
