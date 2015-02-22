#ifndef __player_h__
#define __player_h__

#include <queue>
#include <list>
#include <map>
#include <set>
#include "reciever.h"
#include "buffer.h"
#include "inventory.h"
#include "bank.h"
#include "slot.h"
#include "buff.h"
#include "trade.h"
#include "playershop.h"
#include "party.h"
#include "pmutex.h"
#include "guild.h"
#include "quest.h"
#include "character_base.h"
#include "scheduler.h"

void initprochandlers();
void initplayerdbvals(sqlquery &s1);

class tplayer;
void loadingprocess(tplayer &p);
void pchat(tplayer &pl);
void spawnitem(char *t, tplayer &pl);
void movetopoint(tplayer &pl);
void teleportf(char *t, tplayer &pl);
void spawnmob(char *t, tplayer &pl);
void spawnobj(char *t, tplayer &pl);
void spawnboxobj(char *t, tplayer &pl);
void spawnnpc(char *t, tplayer &pl);
void chatcommands(tplayer &pl, char *t);

class tplayer;
class character_npc;
struct frienddata
{
	int dbid, listindex;
	tplayer *p;
};

class tquest;
struct pquestdata
{
	int id;
	int flags;
	int state;
	unsigned int n;


	unsigned __int8 operator[](int a)
	{
		return ((unsigned __int8*)(&n))[a];
	}

	void inc(int a)
	{
		++((unsigned __int8*)(&n))[a];
	}

	pquestdata(int i, int s, int fl, unsigned int v1, tquest *q1=0)
		:id(i),flags(fl),state(s),n(v1),q(q1)
	{
		if(q==0)
		{
			ul m=tquest::questsmutex.lock();
			std::map<int, tquest*>::iterator i=tquest::quests.find(id*256+state);
			if(i!=tquest::quests.end())q=i->second;
		}
	}
	bool setstate(int a)
	{
		state=a;
		ul m=tquest::questsmutex.lock();
		std::map<int, tquest*>::iterator i=tquest::quests.find(id*256+state);
		if(i!=tquest::quests.end())q=i->second;
		return (i!=tquest::quests.end());
	}
	tquest *q;
};

struct bonus
{
	int stat;
	int val;
	bonus(){};
	bonus(int s, int v):stat(s),val(v){};
	bonus(const bonus& b):stat(b.stat),val(b.val){};
	bonus& operator=(const bonus& b)
	{
		stat=b.stat;
		val=b.val;
		return *this;
	}
};

class cluster;
class tgridcell;
class asstruct;
class objpsywall;
class character_mob;
class tplayer:public character_buffable
{
	friend class cluster;
	long long last_upgrade_time;
protected:
	std::list<buffer> buffers;
	bool loaded;
	buffer *sp2;
private:
	bool loadingp;

	std::list<bonus> psetbonuses;
	std::map<int, bonus> tosendvals;
	int ticket;
	void updatesp();
	int *bsstate, *bsstate2;
	unsigned short state3, *bsstate3;
	char state4, *bsstate4;

	bool errorstate;
	std::set<int> shieldskills;
	std::set<int> weaponskills;
	bool addedact;
	schedulerl<tplayer*>::sremover playeractremover;

	bool save_bank;
	bool save_inv;
	bool save_slots;
	bool save_friendlist;
public:
	mtype<long long> last_active;
	void addmoney(int a);
	void submoney(int a);
	bool save_quests;
	void OnWeaponUnequip();
	void OnShieldUnequip();
	void AddWeaponSkill(int skill);
	void AddShieldSkill(int skill);
	void raise();
	void loadingprocess();
	void pchat();
	void spawnitem(const char *t);
	void movetopoint();
	void teleportf(const char *t);
	void teleportf2(const char *t);
	void spawnmob(const char *t);
	void spawnnpc(const char *t);
	void spawnobj(const char *t);
	void spawnboxobj(const char *t);
	void chatcommands(const char *t);
	void slashcommands(const char *t);

	reciever *sr, *srr;
	sendablebuffer *bs1;
	bool toprocess;
	bool srready;

	pmutex playermutex;
	void clearsp();
	void clearsp2();
	long long nextfood, nextskill, nextattack, nextchat, penelaty, lastsave, nextkarmarecover;
	int slotnum;
	std::list<objpsywall*> psywalls;
	void removepsywalls();
	void drop();
	bool duelbegan;
	long long duelcountdowntime, muted;
	int duelcountdown;
	int duelchallanged;
	tplayer *duelchallanger;
	listremover<tplayer*> siegeremover;
	bool siege_attackable;
	bool markedforrecv;
public:
	bool requested_teleportdown;
	void duelwin();
	void duellose(bool tosend=true);
	bool iswaring(tplayer *p);
	bool ispkable(character_buffable *p);
	void managekarma(character_buffable *p);
	void setkarma(int k)
	{
		if(pk!=k)
		{
			pk=k;
			tomulticast->cmd(id, 0x28) << pk << disp;
		}
	}
	long long activation, refresherhold, vitalx;
	int lodelight;
	std::list<objpsywall*>::iterator addpsywall(objpsywall *p)
	{
		psywalls.push_front(p);
		return psywalls.begin();
	}
	void removepsywall(std::list<objpsywall*>::iterator &i)
	{
		psywalls.erase(i);
	}

	tplayer *duel;
	character_npc *pet;
	std::map<int, pquestdata> quests;
	std::set<int> finishedquests;
	int ncheers;
	short *bssize, *bsmodelid, *bsmodelid2, *bslevel, *bsdisp;
	int *bspvp, *bspk;
	long long nextcheer, cheered;
	int asspower;
	long long asstime;
	bool teleportmode;
	int *ooolen, *bslen;
	short *bscmd;
	bool used;
	std::string name, ip;
	int dbid,accid;
	std::vector2<int> skilllevels;
	std::vector2<int> skillexp;
	std::vector<bool> skillenabled;
	std::vector2<long long> cooldowns;


	int statpoints, mpconsumption, fpconsumption;
	bool berserk, silence, sitting;
	std::vector2<std::vector2<slot> > fslot;
	std::vector2<slot> aslot;
	std::vector2<slot> sslot;
	int gwspawn;
	int msgstate;
	int maxmp,maxfp,mp,fp;
	bank banks[3];
	asstruct *actionslotp;
	int reslevel;
	unsigned long long exp,maxexp,expbeforeko;
	unsigned int pxp;
	int money,bankmoney[3],flylevel,flyexp,gender,hair,haircolor,pk,disp,pvp,accesslevel,face;
//	bool silent;
	int addstat1type;
	int lastup;
	int stance;
	std::list<tplayer*>::iterator dslistpos;

	int getmodelid(){return modelid;}
	int getlastupnm()
	{
		return lastup;
	}
	void setlastupnm(int a)
	{
		lastup=a;
	}
	void altermoney(int a)	//do not send any packets from this
	{
		money+=a;
	}

	void setreslevel(int a)
	{
		reslevel=a;
	}

	void setmp(int a)
	{
		mp=a;
		if(mp<0)mp=0;
		if(mp>maxmp)mp=maxmp;
	}
	int getmp(){return mp;}
	void altermp(int a)
	{
		mp+=a;
		if(mp<0)mp=0;
		if(mp>maxmp)mp=maxmp;
	}
	void setfp(int a)
	{
		fp=a;
		if(fp<0)fp=0;
		if(fp>maxfp)fp=maxfp;
	}
	int getfp(){return fp;}
	void alterfp(int a)
	{
		fp+=a;
		if(fp<0)fp=0;
		if(fp>maxfp)fp=maxfp;
	}
	std::string username;

	void maxskillexp();

	void seterrorstatenl()
	{
		errorstate=true;
	}
	bool geterrorstate()
	{
		return errorstate;
	}
	int getticket()
	{
//		ul m=playermutex.lock();
		return ticket;
	}
	void gwprepare();
	bool gettoprocess()
	{
		ul m=playermutex.lock();
		return toprocess;
	}
	void settoprocess(bool f)
	{
		ul m=playermutex.lock();
		toprocess=f;
	}
	bool valid()
	{
		ul m=playermutex.lock();
		return (ticket!=-1)&&(!errorstate);
	}
	bool validnl()
	{
		return (ticket!=-1)&&(!errorstate);
	}
	int getticketnl(){return ticket;}
	tplayer& operator=(const tplayer& a){return *this;}
	buffer *tomulticast, *tomulticast2;
	buffer *bs, *ooo;
	int fuel;

	tradeclass *trade;
	inventory inv;
	playershop shop, *joinedshop;
	int guildid;
	int guildslot;
	int partyslot;
	tguild *guild;

//-----------------------------------
	pmutex asyncbuffermutex;
	buffer *asyncbuffer;
	std::list<buffer> asyncbuffer2;
	enum t_ac {ac_quit_party, ac_quit_guild, ac_sitdown};
	std::queue<t_ac> acmd;
private:
	void handleacmd();
public:
	std::vector2<frienddata> friendlist;
//-----------------------------------
	static const int maxfriends=200;
	
	tparty* party;

	int lparty;

	void adddelayed(buffer *b, long long t)
	{
		if(validnl())tbs.push(timedbuffer(b, t));
	}

	void townblink();
	void townblinkns();
	void updateas();

	bool flying;

	buffer& spawn();
	void multicast(buffer &bs)
	{
		if((bs.getcommandnumber()>0)&&(tomulticast!=0)&&(tomulticast!=0))
		{
			if(bs.forbidden==this)tomulticast2->add(bs);
			else tomulticast->add(bs);
		}
	}
	void domulticast();
	void add(buffer &bs1)
	{
		if((bs1.forbidden!=this)&&(bs!=0))
		{
			if(bs!=0)bs->add(bs1);
		}
	}
	void getasyncbuffer();
	int sck;
	bool isUsed(){return getticket()!=-1;}
	tplayer();
	tplayer(const tplayer& a);
	~tplayer();
	void init(int sck1, reciever &sr, std::string &cip, int ticket1, cluster *cl1);
	int deinit();
	void removefromcluster();
	void process();

	enum tdelayedaction {da_none, da_flarisblink, da_smblink, da_drakonblink, da_fly, da_ttnt, da_townblinkwing};
	tdelayedaction da;
	long long delay, nexthprec;
	void setdelayed(tdelayedaction da1, long long d);
	void canceldelayed();

	int daflyitem;

	char sendpuffer[buffer::nsize];
	void send();
	void load(int &lparty);
	void save(bool logout=false);
	void firstspawn(bool gotmail=false);
	void logincommands();
	void greentext(const std::string &c);
	void messagebox(const std::string &c);
	void amessagebox(const std::string &c);
	void siege_teleportdown();
	void HMF();
	void sendskills();
	void sendskills2(int b);
	void onChangePosition();
	void docheer();

	void raiseooo();
	void newbuffer();
	void newooobuffer();
	void changemap(int a, float fx, float fy, float fz, bool forcedchange=false);
	void changemap2(int a, float fx, float fy, float fz);
	void timer();

	int pickup(item *targy);
	void jobchange();
	void setgp();
	void setlevelexp();
	void sztext(int a);
	void reanimate();
	void incskillexp(int b);
	bool managempfp(int b);
	void statset(int a, int b, int c, int d);
	void cancelskill()
	{
		bs->cmd(id, 0x1a);
	}

	void sitdown();
	void gotmail();
//	void restat(int a);
	void say(const std::string &nev, const std::string &cimzett, const std::string &message, int kuldoid, int cimzettid);
	void whisper(const std::string &nev, const std::string &cimzett, const std::string &message, int kuldoid, int cimzettid);
	void saynotconnected();
	void addstat1(int stat, int val, bool forcesub=false);
	void addstat2(int stat, int val);
	void addstat3(int stat, int val);

	void npcsell();
	void npcbuy();
	void unequip();
	void equip();
	void moveitem();
	void deleteitem();
	void dropitem();
	void pfollow();
	void psetfocus();
	void fly();
	void flyturn06();
	void flyturn();
	void motion();
	void wasd();

	void addtoFslot();
	void addtoSslot();
	void addtoAslot();
	void removefromFslot();
	void removefromSslot();

	void restat();
	void usereskill();
	void manageexp(long long expval, int l);
	void addexp(long long expval, int l);
	void removesetbonuses();
	void addsetbonus(const bonus& b);
	void onKo(character_buffable *p);
	void onHpChanged();
	bool testLock(character_mob *p);
	void unsummonpet();
	void summonpet(int a);
	void stopmove();
	int  pickup(character_item *p);
	void selfcast(buffer &bs1)
	{
		bs->add(bs1);
	}
//private:
	void skilluse();
	void blinkpool();
	void upgradeitem();
	void banktoinv();
	void itemtobank();
	void banktobank();
	void blockedmove();
	void reskill();
	long double calcWeaponAtk(int weapon);
	int calcAtk(int weapon, int baseAtk, int refine, int adjAtk);
	void calcdef(int a, float *min, float *max);
	void psay(const char *puffer);
	void pwhisper(const char *puffer);
	void addfriendbyid();
	void asktobefriend();
	void refusefriend();
	void deletefriend();
	void addfriendbyname();
	void tradeagree();
	void tradeinsertitem();
	void sendtraderequest();
	void openshop();
	void closeshop();
	void lookinshop();
	void itemaddtoshop();
	void buyfromshop();
	void talkinshop();
	void checkmail();
	void sendmail();
	void deletemail();
	void getmoneyfrommail();
	void getitemfrommail();
	void markmail();
	void partyname();
	void partyleave();
	void partyinvite();
	void ppartyinvite(const std::string &pname);
	void partyjoin();
	void setguildnotice();
	void setguildrigths();
	void setguildname();
	void challangeguild();
	void createnick();
	void npcchat();
	void removequest();
	void itemtowh();
	void whtoinv();
};

class asstruct
{
	friend class tplayer;
	int pos;
	int a,c;
	tplayer *player;
	character_buffable *initialFocus;
	bool endit;
	long long nexttime, td;
	int askills[5];
	int pjob;
public:
	schedulerl<asstruct*>::sremover remover;
	void setendit(){endit=true;}
	tplayer *getPlayer(){return player;}
	asstruct(int a, int c, tplayer *p, character_buffable *f, long long td1, int s0, int s1, int s2, int s3, int s4, int pjob);
	~asstruct();
	bool process();
};

#endif
