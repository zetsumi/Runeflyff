#ifndef __objactions_h__
#define __objactions_h__

#include "character_obj.h"
#include "buffer.h"
#include "player.h"
#include "fieldobj.h"


class objopendoor:public objaction
{
	int id;
	bool open, toremove;
public:
	objopendoor(int id1, bool r=false):id(id1),open(false),toremove(r){};
	void openit()
	{
		if(!open)
		{
				open=true;
				buffer bs;
				bs.cmd(owner->getId(), 0x7e) << (char)0;
				bs.cmd(owner->getId(), 0x7e) << (char)1;
				owner->gridcell.multicast2(bs);
				t=owner->cl->ido+10000;
				owner->cl->addobjtimer(objtimercont(this));
		}
	}
	void operator()(character_obj *o, tplayer *p)
	{
		if(!open)
			if(p->inv.contains(id))
			{
				openit();
				if(toremove)p->inv.removeitemid(id, 1);
			}
	}
	void timer()
	{
		if(open)
		{
			open=false;
			buffer bs;
			bs.cmd(owner->getId(), 0x7e) << (char)0;
			bs.cmd(owner->getId(), 0x7e) << (char)1;
			owner->gridcell.multicast2(bs);
		}
	}
};


class objdooropener:public objaction
{
	objopendoor *d;
public:
	objdooropener(objopendoor *d1):d(d1){}
	void operator()(character_obj *o, tplayer *p)
	{
		buffer bs;
		bs.cmd(owner->getId(), 0x7e) << (char)0;
		bs.cmd(owner->getId(), 0x7e) << (char)1;
		owner->gridcell.multicast2(bs);
		d->openit();
	}
};

class objteleportdmine:public objaction
{
	int mapid,itemid;
	float x,y,z;
public:
	objteleportdmine(int m, float x1, float y1, float z1, int i)
		:mapid(m),x(x1),y(y1),z(z1),itemid(i){};
	void operator()(character_obj *o, tplayer *p)
	{
		if(p->level>=70)
			if(p->inv.contains(itemid))
				p->changemap(mapid,x,y,z);
	}
};

class objteleportkebaras:public objaction
{
	int itemid;
public:
	objteleportkebaras(int i)
		:itemid(i){};
	void operator()(character_obj *o, tplayer *p)
	{
		if(p->inv.contains(itemid))
		{
			p->inv.removeitemid(itemid, 1);
			p->townblink();
		}
	}
};

class objteleport:public objaction
{
	int mapid;
	float x,y,z;
public:
	objteleport(int m, float x1, float y1, float z1):mapid(m),x(x1),y(y1),z(z1){};
	void operator()(character_obj *o, tplayer *p)
	{
		p->changemap(mapid,x,y,z);
	}
};

//26
class openanddrop:public objaction
{
	int id;
	bool opened;
public:
	openanddrop(int i):id(i),opened(false){};
	void operator()(character_obj *o, tplayer *p)
	{
		if(opened)return;
		buffer bs;
		bs.cmd(o->getId(), 0x7e) << (char)0;
		bs.cmd(o->getId(), 0x7e) << (char)1;
		o->gridcell.multicast2(bs);

		item targy(id);
		new character_item(targy, o->cl, o->getmapid(), o->getx(), o->gety(), o->getz(), p);
		opened=true;
		t=owner->cl->ido+5000;
		owner->cl->addobjtimer(objtimercont(this));
	}
	void timer()
	{
		delete owner;
	}
};

class objpsywall:public objaction
{
	tplayer *plowner;
	std::list<objpsywall*>::iterator remover;
	f_psywall *w;
public:
	objpsywall(tplayer *o):plowner(o),w(0)
	{
		character_base *f=plowner->getfocus();
		if(f!=0)
		{
			vector3d<> n=f->getpos()-o->getpos();
			if(n.sqrlength()!=0)
			{
				n/=n.length();
			}else
			{
				n.set(0,0,-1);
			}
			w=new f_psywall(o->cl, o->getmapid(), o->getpos(), n, 3, plowner);
			remover=o->addpsywall(this);
		}else plowner=0;
	}
	virtual ~objpsywall()
	{
	}
	void setowner(character_obj *o)
	{
		owner=o;
//		owner->setmodeltype(0);
		t=owner->cl->ido+5000;
		owner->cl->addobjtimer(objtimercont(this));
	}
	void operator()(character_obj *o, tplayer *p)
	{
	};
	void timer()
	{
		clear();
		delete this;
	}
	void clear()
	{
		if(plowner!=0)
		{
			plowner->removepsywall(remover);
			plowner=0;
		}
		if(owner!=0)
		{
			owner->oa=0;
			delete owner;
			owner=0;
		}
		if(w!=0)
		{
			delete w;
			w=0;
		}
	}
};

class openandspawn:public objaction
{
	int mob, n;
	bool agro;
	bool opened;
public:
	openandspawn(int m, int n1, bool a):mob(m),n(n1),agro(a){}
	void operator()(character_obj *o, tplayer *p)
	{
		if(opened)return;
		buffer bs;
		bs.cmd(o->getId(), 0x7e) << (char)0;
		bs.cmd(o->getId(), 0x7e) << (char)1;
		o->gridcell.multicast2(bs);

		for(int a=0;a<n;a++)
			new character_mob(mob, o->cl, o->getmapid(), o->getx(), o->gety(), o->getz(), agro);

		opened=true;
		t=owner->cl->ido+5000;
		owner->cl->addobjtimer(objtimercont(this));
	}
	void timer()
	{
		delete owner;
	}
};

class openandspawn_ifcontains:public objaction
{
	int mob, n;
	bool agro;
	bool opened;
	int id;
public:
	openandspawn_ifcontains(int m, int n1, bool a, int i):mob(m),n(n1),agro(a),id(i){}
	void operator()(character_obj *o, tplayer *p)
	{
		if(opened)return;
		if(p->inv.contains(id))
		{
			buffer bs;
			bs.cmd(o->getId(), 0x7e) << (char)0;
			bs.cmd(o->getId(), 0x7e) << (char)1;
			o->gridcell.multicast2(bs);

			for(int a=0;a<n;a++)
				new character_mob(mob, o->cl, o->getmapid(), o->getx(), o->gety(), o->getz(), agro);

			opened=true;
			t=owner->cl->ido+5000;
			owner->cl->addobjtimer(objtimercont(this));
		}
	}
	void timer()
	{
		delete owner;
	}
};



class mobspawner_oaf:public objactionfactory
{
	int id,n;
public:
	mobspawner_oaf(int id1, int n1):id(id1),n(n1){}
	objaction* operator()()
	{
		return new openandspawn(id,n,true);
	}
};

class mobspawner_q_oaf:public objactionfactory
{
	int id,n,itemid;
public:
	mobspawner_q_oaf(int id1, int n1, int i):id(id1),n(n1),itemid(i){}
	objaction* operator()()
	{
		return new openandspawn_ifcontains(id,n,true, itemid);
	}
};

#endif
