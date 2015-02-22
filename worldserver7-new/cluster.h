#ifndef __cluster_h__
#define __cluster_h__

#include "pmutex.h"
#include "vector2.h"
#include <list>
#include <queue>
#include "mysql.h"
#include <string>
#include "reciever.h"
#include "player.h"
#include "grid.h"
#include "ticket.h"
#include "buff.h"
#include "character_obj.h"
#include "m_queue.h"
#include "scheduler.h"
#include "listremover.h"

void* clusterthread(void *t);
void *recieverthread(void *t);
class cluster;
struct threadparm
{
	int n;
	cluster *c;
};



bool moverfunkc(character_base *p);
class clustercont;
class tparty;
class cluster
{
	friend class clustercont;
	friend bool moverfunkc(character_base *p);
	friend void *clusterthread(void *t);
	friend void *recieverthread(void *t);
	friend void *loadsavethread(void *t);
	bool lsendprg;
	pthread_attr_t ptca;
	threadparm tp;
	int clusternumber;
    pthread_t *csthread, *crthread, *lsthread;
//	pmutex toprocessmutex;
//	std::list<tplayer*> toprocess;
	m_queue<tplayer*> toprocess, toload, loaded, startrecv, cmain_toerase, doselect_toerase, doselect_add, logged_and_saved;
	std::string name;
	std::string errorstr;
	tplayercont<tplayer> players;
	std::vector2<character_mob*> mobs;
	std::vector2<character_npc*> npcs;
	std::vector2<character_item*> items;
	std::vector2<character_obj*> objs;
	schedulerl<character_buffable*> buffed;
	schedulerl<character_buffable*> dot;
	schedulerl<asstruct*> actionslots;
	std::priority_queue<objtimercont> objtimers;
	schedulerl<spawner*> objspawners;
	schedulerl<character_npc*> npcchat;

	bool loadplayer(tplayer *p);
	void checkbuffs();
	void doselect();
	std::list<tplayer*> dsplayers;
	bool pkval;
	int mobid,objid,itemid,npcid;
	std::list<int> mobidlist;
	std::list<int> objidlist;
	std::list<int> npcidlist;
	std::list<int> itemidlist;
	std::list<itemremover*> itemremove;
	void removeitem();
	schedulerl<tplayer*> playeract;
	schedulerl<character_mob*> mobact;
	schedulerl<character_mob*> mobattack;
	schedulerl<character_mob*> mobagro;
	schedulerl<character_npc*> petpick;
	long long sieget;
	void domobact();
	void doplayeract();
	schedulerl<character_base*> movers;
	char ppuffer[2*1024*1024];
	m_queue<std::string> markloggedlist;
	void processmarklogged();
	buffer *bbs;
	std::list<buffer> ooolist;
	pmutex asyncbuffermutex;
	buffer *asbs, *asooo;
	std::list<tparty*> linkattacks;
	std::list<tparty*> pscircle;
	void checkparties();
	bool cwcage;
	void genrndpacket(tplayer *p);
	std::list<tguild*> siegeguilds;
	std::list<tguild::tguildmember*> siegeplayers;
	void psiege();
	int siegetimertype;
	std::list<tplayer*> ongwmap;
	bool send_siege_combat;
public:
	enum tsiegestate {tsnone=0, apply, prepare, teleportin, teleportdown, running, ending};
private:
	tsiegestate siegestate;
public:
	void gw_attackable();
	float getclienttime();
	mcon* saveconn;
	struct charsavedata
	{
		int dbid, accid;
		sqlquery *s1;
		sqlquery *s2;
		tplayer *p;
	};
	m_queue<charsavedata> charsave_queue;
	void addtosiegemap(listremover<tplayer*> &a, tplayer *p)
	{
		a.set(&ongwmap,p);
	}
	void showsiegestats();
	void cancelsiege(int n);
	void sendsiegedata(tplayer *p);
	int getsiegetimertype()
	{
		return siegetimertype;
	}
	long long getsieget()
	{
		return sieget;
	}
	void siege_prepare();
	void siege_teleportin();
	void siege_teleportdown();
	void siege_running();
	void siege_ending();
	void addtosiege(tguild::tguildmember *a)
	{
		siegeplayers.push_front(a);
	}
	tsiegestate getsiegestate()
	{
		return siegestate;
	}
	void setsiegestate(tsiegestate a);
	pmutex savemutex;
	static const int nmaxplayers=1001;
	
	bool cwcageempty()
	{
		return !cwcage;
	}
	void cwcageenter()
	{
		cwcage=true;
	}
	void cwcageleave()
	{
		cwcage=false;
	}

	void addobjtimer(objtimercont &a)
	{
		objtimers.push(a);
	}
	void addplayeract(tplayer *p, schedulerl<tplayer*>::sremover &i)
	{
		playeract.insert(p->getId(), p, i);
	}
	void removeplayeract(schedulerl<tplayer*>::sremover &i)
	{
		playeract.remove(i);
	}
	void addobjspawner(spawner *o, schedulerl<spawner*>::sremover &i)
	{
		objspawners.insert((int)(o), o, i);
	}
	void removeobjspawner(schedulerl<spawner*>::sremover &i)
	{
		objspawners.remove(i);
	}
	void addnpcchat(character_npc *o, schedulerl<character_npc*>::sremover &i)
	{
		npcchat.insert(/*o->getId()*/rnd(1000), o, i);
	}
	void removenpcchat(schedulerl<character_npc*>::sremover &i)
	{
		npcchat.remove(i);
	}


	void addtomobact(character_mob *p, schedulerl<character_mob*>::sremover &i)
	{
		mobact.insert(p->getId(), p, i);
	}
	void addtomobattack(character_mob *p, schedulerl<character_mob*>::sremover &i)
	{
		return mobattack.insert(p->getId(), p, i);
	}
	void addtoagromob(character_mob *p, schedulerl<character_mob*>::sremover &i)
	{
		return mobagro.insert(p->getId(), p, i);
	}

	void addpet(character_npc *p, schedulerl<character_npc*>::sremover &i)
	{
		return petpick.insert(p->getId(), p, i);
	}

	void removepet(schedulerl<character_npc*>::sremover &i)
	{
		petpick.remove(i);
	}

	void addmover(character_base *p, schedulerl<character_base*>::sremover &i)
	{
		return movers.insert(p->getId(), p, i);
	}

	void removemover(schedulerl<character_base*>::sremover &i)
	{
		movers.remove(i);
	}
	void addbuffed(character_buffable *p, schedulerl<character_buffable*>::sremover &i)
	{
		return buffed.insert(p->getId(), p, i);
	}

	void rembuffed(schedulerl<character_buffable*>::sremover &i)
	{
		buffed.remove(i);
	}

	void adddot(character_buffable *p, schedulerl<character_buffable*>::sremover &i)
	{
		return dot.insert(p->getId(), p, i);
	}

	void remdot(schedulerl<character_buffable*>::sremover &i)
	{
		dot.remove(i);
	}

	void addas(asstruct *a, schedulerl<asstruct *>::sremover &i)
	{
		return actionslots.insert(a->getPlayer()->getId(), a, i);
	}

	int nplayers;
	void addlink(listremover<tparty*> &r, tparty *p, cluster *&q)
	{
		q=this;
		r.set(&linkattacks, p);
//		linkattacks.push_front(p);
//		return linkattacks.begin();
	}
	void addsc(listremover<tparty*> &r, tparty *p, cluster *&q)
	{
		q=this;
		r.set(&pscircle, p);
//		pscircle.push_front(p);
//		return pscircle.begin();
	}
	void removelink(listremover<tparty*> &i)
	{
		try{
			i.remove();
		}catch(...)
		{}
	}
	void removesc(listremover<tparty*> &i)
	{
		try{
			i.remove();
		}catch(...){}
	}
	void removelink_lock(listremover<tparty*> &i)
	{
		pmutex::unlocker mm=this->clustermutex.lock();
		try{
			i.remove();
		}catch(...){}
	}
	void removesc_lock(listremover<tparty*> &i)
	{
		pmutex::unlocker mm=this->clustermutex.lock();
		try{
			i.remove();
		}catch(...){}
	}
	tplayer *tesztplayer;
	bool isPK()
	{
		return pkval;
	}
    pthread_t *cthread;

	int getclusternumber()
	{
		return clusternumber;
	}
	int loadnpces();
	int loadmobspawns();
	int loaditemspawns();
	void additem(itemremover *p);
	std::map<int, tplayer*> dbidplayers;
	std::map<std::string, tplayer*> nameplayers;
//	pmutex handedplayersmutex;
//	std::list<int> handedplayers;
	m_queue<int> handedplayers;
	tgrid grid;
	bool endprg;
	pmutex clustermutex;
	std::vector2<mcon*> connections;
	sqlquery dbaccounts;
	sqlquery dbaccounts4load;
	sqlquery dbcharacters;
	sqlquery dbcharacters4load;
	sqlquery dbitemlist;
	sqlquery dbmails;
	sqlquery dbslots;
	sqlquery dbskilllist2;
	sqlquery dbmonsterlist;
	sqlquery dbguilds;
	sqlquery dbclusters;

	cluster(int n, const char *nev, bool pk1);
	~cluster();
	void cmain();
	void lsmain();
	void crmain();
	void handower(int sck1, reciever &sr1, std::string cip1);
	int getitemid(character_item*);
	void releaseitemid(int id);
	int getmobid(character_mob*);
	void releasemobid(int id);
	int getobjid(character_obj*);
	void releaseobjid(int id);
	int getnpcid(character_npc*);
	void releasenpcid(int id);
	tplayer* getplayer(int id);
	character_mob* getmob(int id);
	character_npc* getnpc(int id);
	character_item* getitem(int id);
	character_obj* getobj(int id);
//	std::list<character_npc*>::iterator addpet(character_npc *p);
//	void removepet(std::list<character_npc*>::iterator &i);
//	void addtomobattack(character_mob*);
//	void addtoagromob(character_mob*);
//	void addas(asstruct *a);
//	std::list<character_buffable*>::iterator addbuffed(character_buffable *p);
//	void rembuffed(std::list<character_buffable*>::iterator &i);
//	std::list<character_buffable*>::iterator adddot(character_buffable *p);
//	void remdot(std::list<character_buffable*>::iterator &i);
	long long ido;
//	void addtomobact(character_mob*);
//	std::list<character_base*>::iterator addmover(character_base *);
	void removemover(std::list<character_base*>::iterator&);

	char* print(const char*, ...);
	bool npcinrange(int a);
	bool objinrange(int a);
	bool iteminrange(int a);
	bool mobinrange(int a);
	bool playerinrange(int a);
	void multicast(buffer &bs);
	void multicastooo(buffer &bs);
	void domulticasts();

	void marklogged(const std::string username);	//BYVAL!
	void notice2(const std::string &t);
};

class clustercont
{
	int nclusters;
	std::vector2<cluster*> clusters;
	pmutex clustercontmutex;
	buffer *bs;
	buffer *ooo;
public:
	size_t size()
	{
		pmutex::unlocker m=clustercontmutex.lock();
		return nclusters;
	}
	cluster* operator[](int a)
	{
		pmutex::unlocker m=clustercontmutex.lock();
		return clusters.at(a);
	}
	clustercont(mcon *con, int n, int servernumber);
	~clustercont();
	void multicast(buffer &bs);
	void multicastooo(buffer &bs);
};


#endif
