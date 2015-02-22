#ifndef __gridremover_h__
#define __gridremover_h__

class character_base;
class tplayer;
class character_mob;
class character_npc;
class character_item;
class character_obj;

class buffer;
class tgridcell;
#include "error.h"

class tgridcellremover
{
	friend class tgridcell;
	class pbase{};
	class tplayer_p:public pbase
	{
	public:
		std::list<tplayer*>::iterator i;
		tplayer_p(const std::list<tplayer*>::iterator &j):i(j){}
	};
	class character_mob_p:public pbase
	{
	public:
		std::list<character_mob*>::iterator i;
		character_mob_p(std::list<character_mob*>::iterator &j):i(j){}
	};
	class character_item_p:public pbase
	{
	public:
		std::list<character_item*>::iterator i;
		character_item_p(std::list<character_item*>::iterator &j):i(j){}
	};
	class character_obj_p:public pbase
	{
	public:
		std::list<character_obj*>::iterator i;
		character_obj_p(std::list<character_obj*>::iterator &j):i(j){}
	};
	class character_npc_p:public pbase
	{
	public:
		std::list<character_npc*>::iterator i;
		character_npc_p(std::list<character_npc*>::iterator &j):i(j){}
	};

	std::list<tplayer*>::iterator getplayeriterator()
	{
//#ifdef _DEBUG
		if(i==0)throw error("iterator was not set", "getplayeriterator");
//#else
//		if(i==0)
//		{
//			added=false;
//			add();
//		}
//#endif
		return ((tplayer_p*)i)->i;
	}

	std::list<character_mob*>::iterator getmobiterator()
	{
//#ifdef _DEBUG
		if(i==0)throw error("iterator was not set", "getmobiterator");
//#else
//		if(i==0)
//		{
//			added=false;
//			add();
//		}
//#endif
		return ((character_mob_p*)i)->i;
	}
	std::list<character_npc*>::iterator getnpciterator()
	{
//#ifdef _DEBUG
		if(i==0)throw error("iterator was not set", "getnpciterator");
//#else
//		if(i==0)
//		{
//			added=false;
//			add();
//		}
//#endif
		return ((character_npc_p*)i)->i;
	}
	std::list<character_obj*>::iterator getobjiterator()
	{
//#ifdef _DEBUG
		if(i==0)throw error("iterator was not set", "getobjiterator");
//#else
//		if(i==0)
//		{
//			added=false;
//			add();
//		}
//#endif

		return ((character_obj_p*)i)->i;
	}
	std::list<character_item*>::iterator getitemiterator()
	{
//#ifdef _DEBUG
		if(i==0)throw error("iterator was not set", "getitemiterator");
//#else
//		if(i==0)
//		{
//			added=false;
//			add();
//		}
//#endif
		return ((character_item_p*)i)->i;
	}

	void setandadd(tgridcell *g, std::list<tplayer*>::iterator &j)
	{
//#ifdef _DEBUG
		if(owner!=0)throw error("owner isnt null", "setanddd player");
//#else
//		if(owner!=0)remove();
//#endif
		owner=g;
		i=new tplayer_p(j);
		added=true;
	}
	void setandadd(tgridcell *g, std::list<character_mob*>::iterator &j)
	{
//#ifdef _DEBUG
		if(owner!=0)throw error("owner isnt null", "setandadd mob");
//#else
//		if(owner!=0)remove();
//#endif
		owner=g;
		i=new character_mob_p(j);
		added=true;
	}
	void setandadd(tgridcell *g, std::list<character_obj*>::iterator &j)
	{
//#ifdef _DEBUG
		if(owner!=0)throw error("owner isnt null", "setandadd obj");
//#else
//		if(owner!=0)remove();
//#endif
		owner=g;
		i=new character_obj_p(j);
		added=true;
	}
	void setandadd(tgridcell *g, std::list<character_npc*>::iterator &j)
	{
//#ifdef _DEBUG
		if(owner!=0)throw error("owner isnt null", "setandadd npc");
//#else
//		if(owner!=0)remove();
//#endif
		owner=g;
		i=new character_npc_p(j);
		added=true;
	}
	void setandadd(tgridcell *g, std::list<character_item*>::iterator &j)
	{
//#ifdef _DEBUG
		if(owner!=0)throw error("owner isnt null", "setandadd item");
//#else
//		if(owner!=0)remove();
//#endif
		owner=g;
		i=new character_item_p(j);
		added=true;
	}
	pbase *i;

	void cleariterator()
	{
		delete i;
		i=0;
		owner=0;
		added=false;
	}
	void cleargc()
	{
		purposely_added=false;
		cleariterator();
	}

	tgridcell *owner;
	character_base *p;

	void addcharacter();
	void removecharacter();
	void update();
	bool added, purposely_added;
	tgridcellremover(const tgridcellremover&);
	void operator=(const tgridcellremover&);
	static std::list<tplayer*> players1;
public:
	tgridcell* ggc()
	{
		return owner;
	}
	tgridcellremover(character_base *q):owner(0),p(q),i(0),added(false),purposely_added(false){}
	tgridcell* operator->()
	{
//		if((owner==0)&&(purposely_added))add();
		if(owner==0)throw error("Referenced null gridcell", "tgridremover::->");
		return owner;
	}
	int playersaround();
	void multicast2(buffer &bs1);
	tgridcell* getsz(int a);
	bool checkmove(const vector3d<> &s, const vector3d<> &e, character_base *p);
	bool valid()
	{
		return owner!=0;
	}
	bool add();
	void check();
	void check(buffer &bs);
	void remove()
	{
		if(owner!=0)
		{
			removecharacter();
			delete i;
			i=0;
		}
		purposely_added=false;
	}
	void half_remove()	//DO NOT USE THIS! (for debugging only)
	{
		owner=0;
		delete i;
		i=0;
	}
	~tgridcellremover()
	{
		remove();
	}
};

#endif
