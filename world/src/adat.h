#ifndef __adat_h__
#define __adat_h__

#include "vector2.h"
#include <map>
#include <list>
#include <set>
#include "item.h"
#include "error.h"
#include "vector.h"

enum tweather {wnothing=0, rain, snow};
//extern float dambigfloat;

extern const bool forcedchangeonblink;

#define bpposter 2026
#define rmposter 2027
#define arrows 2028

extern const float md;
extern float dropmult;

extern double moneymultiplier;

extern vector3d<> arenapos;
extern float arenarange;


class tquest;
struct mobdata2
{
	int id;
	int level;
	int hp;
	int size;
	int str;
	int sta;
	int dex;
	int intl;
	int element;
	int atkmin;
	int atkmax;
	int def;
	int hit;
	int flee;
	int exp,fxp;
	int flying;
	bool giant;
	bool valid;
	int attackdelay;
	float speed;
	float resistele[5];
	int mdrop;
	int cash;
	std::string name;
	std::map<int, std::pair<int, int> > quest_killmobs;
	struct tdrop
	{
		int id;
		float dropchance;
		int nmin, nmax;
		tdrop(int i, float d, int min, int max):id(i),dropchance(d),nmin(min), nmax(max){};

	};
	std::list<tdrop> drops;
	struct questdata
	{
		int dropid;
		int number;
		long long dropchance;
		tquest *q;
		questdata(int a, int b, long long c, tquest *d):dropid(a),number(b), dropchance(c),q(d){}
	};
	std::list<questdata> quests;
	mobdata2()
	{
		valid=false;
		mdrop=0;
	}
};

struct tskilldata
{
	int id,level;
int min;
int max;
//int val5;
//int val6;
//int val7;
//int val8;
int prob;
//int val10;
//int val11;
int stat1;
int stat2;
int sval1;
int sval2;
//int val16;
//int val17;
//int val18;
//int val19;
//int val20;
//int val21;
//int val22;
//int val23;
int mpreq;
int fpreq;
//int val26;
int val27;
int range;
//int val29;
//int val30;
int duration;
//int val32;
//int val33;
int pxp;
int astime;
int element;
int elementval;
int element2;
int elementval2;
int skilltype;
int dot;
int cooldown;
int rstat1,rstat2;
int rval1, rval2;
int rtarget1, rtarget2;
int playerjob;
int handed;
int weapontype;
bool magic;
bool targetless_aoe;
tskilldata()
{
	targetless_aoe=false;
}
};

struct tguildexp
{
	int mdrop;
	int money;
	tguildexp(int a, int b):mdrop(a),money(b){};
};

struct setbonusdata
{
	int stat;
	int val;
	int nparts;
};

struct setdata
{
	int id;
	setbonusdata bonuses[7];
};

void datainit();
void datafree();


extern std::vector2<std::vector2<int> > skillmaxlevel;
extern std::vector2<std::vector2<int> > skills;
//extern btree<int, itemdata> itemlist;
extern std::vector2<itemdata> itemlist;

extern std::vector2<int> ultimatestats;
extern std::vector2<int> uphitbonus;
extern std::vector2<int> upblockbonus;
extern std::vector2<int> uphpbonus;
extern std::vector2<int> upmagicbonus;
extern std::vector2<int> upallstatbonus;
extern std::vector2<std::vector2<std::vector2<int> > > ultimatebonus;

extern std::vector2<unsigned int> itemstats;
extern std::vector2<int> ringbonus;
extern std::vector2<int> defbonus;
extern std::vector2<int> atkbonus;
extern std::vector2<int> hpbonus;
extern std::vector2<int> mpbonus;
extern std::vector2<int> fpbonus;

extern int elementl[6];
extern int delementl[6];
extern int stlk[6];
extern int dstlk[6];
extern int stlknl[6];
extern int dstlknl[6];
/*
extern std::vector2<int> elementl;
extern std::vector2<int> delementl;
extern std::vector2<int> stlk;
extern std::vector2<int> dstlk;
extern std::vector2<int> stlknl;
extern std::vector2<int> dstlknl;
*/
extern std::vector2<int> bufflist;
extern std::vector2<int> job1;
extern std::vector2<int> job2;
extern std::vector2<int> skillexp2;
extern std::vector2<int> skillacc;
extern int nbufflist;
extern std::vector2<int> petstats;
extern std::vector2<std::vector2<int> > petbonus;
extern std::vector2<int> enctable;
extern std::vector2<long double> hpj;
extern std::vector2<long double> mpj;
extern std::vector2<long double> fpj;
extern std::vector2<long double> stadef;
extern std::vector2<long double> leveldef;
extern std::vector2<long double> defbase;
extern std::vector2<std::vector2<int> > petids;
extern std::vector2<vector3d<> > towns;
extern std::vector2<vector3d<> > lodelights;
extern std::vector2<vector3d<> > gwtele;
extern std::vector2<tguildexp> guildexp;
extern std::vector2<int> skillsupliment;

extern std::vector2<int> upgradeChances;
extern std::vector2<int> upgradeChancesUlt;
extern std::vector2<int> upgradeChancesJewel;
extern std::vector2<std::vector2<int> > ultimateweap;
extern std::vector2<std::vector2<tskilldata> > skilllist;
extern std::vector2<tskilldata > cskilllist;
extern std::vector2<mobdata2> monsterlist;
extern std::vector2<unsigned long long > explist;
extern std::vector2<unsigned long long > pxplist;
//extern std::vector2<int> skilltype;
extern std::vector2<int> nguildmembers;

#define ST_ATTACK 1
#define ST_AOE 2
#define ST_HEAL 4
#define ST_BUFF 8
#define ST_SELF 16
#define ST_OTHERS 32


extern long double skillExpMultiplier;
extern int itemstaytime;
extern int itemlocktime;

extern long double expMultiplier;
extern int partypointsperlevel;
extern int partyexpszorzo;
extern unsigned long long explose;

#define w_unbound 0
#define wb_weapon 1
#define wb_shield 2
#define w_sword 4
#define w_axe 8
#define w_stick 16
#define w_knuckle 32
#define w_staff 64
#define w_wand 128
#define w_yoyo 256
#define w_bow 512
#define w_weapon 1024
#define w_shield 2048


//class tparty;
//extern btree<tparty*, tparty*> parties;
//extern pmutex partiesmutex;;
#endif
