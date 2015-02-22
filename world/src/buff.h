#ifndef __buff_h__
#define __buff_h__

#include <queue>
#include "vector2.h"
#include <map>
#include "character_base.h"
#include "scheduler.h"

#define DST_STR                      1
#define DST_DEX                      2
#define DST_INT                      3
#define DST_STA                      4
#define DST_YOY_DMG		             5
#define DST_BOW_DMG		             6
#define DST_CHR_RANGE                7
#define DST_BLOCK_RANGE              8
#define DST_CHR_CHANCECRITICAL       9
#define DST_CHR_BLEEDING            10
#define DST_SPEED                   11
#define DST_ABILITY_MIN             12
#define DST_ABILITY_MAX             13
#define DST_BLOCK_MELEE             14
#define DST_MASTRY_EARTH            15
#define DST_STOP_MOVEMENT           16
#define DST_MASTRY_FIRE             17
#define DST_MASTRY_WATER            18
#define DST_MASTRY_ELECTRICITY      19
#define DST_MASTRY_WIND             20
#define DST_KNUCKLE_DMG             21
#define DST_ATTACKSPEED             24
#define DST_SWD_DMG                 25
#define DST_ADJDEF                  26
#define DST_RESIST_MAGIC            27
#define DST_RESIST_ELECTRICITY      28
#define DST_REFLECT_DAMAGE          29
#define DST_RESIST_FIRE             30
#define DST_RESIST_WIND             31
#define DST_RESIST_WATER            32
#define DST_RESIST_EARTH            33
#define DST_AXE_DMG                 34
#define DST_HP_MAX                  35
#define DST_MP_MAX                  36
#define DST_FP_MAX                  37
#define DST_HP                      38
#define DST_MP                      39
#define DST_FP                      40
#define DST_HP_RECOVERY             41
#define DST_MP_RECOVERY             42
#define DST_FP_RECOVERY             43
#define DST_KILL_HP					44
#define DST_KILL_MP					45
#define DST_KILL_FP					46
#define DST_ADJ_HITRATE             47
#define DST_CLEARBUFF				49
#define DST_CHR_STEALHP_IMM         50
#define DST_ATTACKSPEED_RATE		51
#define DST_HP_MAX_RATE				52
#define DST_MP_MAX_RATE				53
#define DST_FP_MAX_RATE				54
#define DST_CHR_WEAEATKCHANGE	    55
#define DST_CHR_STEALHP				56
#define DST_CHR_CHANCESTUN			57
#define DST_AUTOHP					58
#define DST_CHR_CHANCEDARK			59
#define DST_CHR_CHANCEPOISON	    60
#define DST_IMMUNITY	 			61
#define DST_ADDMAGIC				62
#define DST_CHR_DMG                 63
#define DST_CHRSTATE                64
#define DST_PARRY                   65
#define DST_ATKPOWER_RATE			66
#define DST_JUMPING                 68
#define DST_CHR_CHANCESTEALHP		69
#define DST_CHR_CHANCEBLEEDING      70
#define DST_RECOVERY_EXP            71
#define DST_ADJDEF_RATE				72
#define	DST_MP_DEC_RATE				73
#define	DST_FP_DEC_RATE				74
#define	DST_SPELL_RATE				75
#define	DST_CAST_CRITICAL_RATE		76
#define	DST_CRITICAL_BONUS			77
#define	DST_SKILL_LEVEL				78
#define MAX_ADJPARAMARY             79
#define	DST_GOLD					79
#define	DST_PXP						80
#define DST_RESIST_ALL				81
#define DST_STAT_ALLUP				82
#define DST_HPDMG_UP				83
#define DST_DEFHITRATE_DOWN			84
#define DST_CURECHR					85
#define DST_HP_RECOVERY_RATE		89
#define DST_MP_RECOVERY_RATE		90
#define DST_FP_RECOVERY_RATE		91
#define	DST_LOCOMOTION				92
#define	DST_MASTRY_ALL				93
#define DST_ALL_RECOVERY			94
#define DST_ALL_RECOVERY_RATE		95
#define DST_KILL_ALL				96
#define DST_KILL_HP_RATE			97
#define DST_KILL_MP_RATE			98
#define DST_KILL_FP_RATE			99
#define DST_KILL_ALL_RATE			100
#define DST_ALL_DEC_RATE			101



class tplayer;
class tparty;
class character_buffable;
class tbuff
{
	friend class character_buffable;
	int id;
	int level;
	long long duration, lejarat;
	int dmg;
	~tbuff(){};
public:
	std::list<tbuff*>::iterator i;
	tbuff(int i, int l, long long d, long long le):id(i),level(l),duration(d),lejarat(le){}
	tbuff(int i, int l, long long d, long long le, int dd):id(i),level(l),duration(d),lejarat(le),dmg(dd){}
};


class character_buffable:public character_base
{
	friend class tbuff;
	void removeeffect(tbuff *b, bool needlock);
	void insertbuff(tbuff *b);
	double avg,avgt;
	bool abr;
protected:
	std::list<tbuff*> buffs;
	int nbuffs;
	void sendbdata(buffer *bs);
	void sendbdataatlogin(buffer *bs);
	void loadbuffs(int *);
	int savebuffs(int *);
	void bdeinit();
	void bremovefromcluster();

	int *bshp;
	int hp;
	bool holycross;
	int maxhp;
	virtual void onHpChanged()=0;
	virtual void onKo(character_buffable *koer=0)=0;
	void *koer;
	bool additionalcrit;
	long long protection, stun;
	int protectionlevel;
	bool silenced;
	int dotdmg;
	void alterdotdmg(int a);
	void stathandle(int stat, int val, bool remove=false);
	void handle64(int stat, int level, tbuff *b);
	bool berserk;
	bool counter;
	std::vector2<tbuff*> buffplace;
	int weapontype, weapontype2;
	int sweapontype;
public:
	virtual void selfcast(buffer &bs1){};
	schedulerl<character_buffable*>::sremover buffedremover, dotremover;
	int level,job;
	int str,sta,dex,intl,hit;
	int atkmin,atkmax,defmin,defmax;

	std::vector2<int> stats;
	tplayer *playerLock;
	tparty *partyLock;
	static const int camm=100;
	static const int maxskillid=400;
	bool di;
	bool gm_di;
	bool isDead;
	long long getstun(){return stun;}
	bool dot();
	character_buffable():
		protection(0),stun(0),silenced(false),
		holycross(false), koer(0),additionalcrit(false),nbuffs(0),
		avg(0),avgt(0),dotdmg(0),berserk(false),counter(false),di(false),gm_di(false)
		,playerLock(0), partyLock(0), job(0),weapontype(0),weapontype2(0),abr(false),isDead(false)
	{
		stats.resize(102, 0);
		buffplace.resize(maxskillid+camm, 0);
		binit();
	}
	bool checkbuffs();
	void setbuffplace(int id, tbuff *p)
	{
		buffplace.at(id)=p;
	};
	int gethp()
	{
		return hp;
	}
/*
	void sethp(int a)
	{
		hp=a;
		onHpChanged();
		if(hp<0)
		{
			hp=0;
			onKo();
		}
	}
*/
	void alterhp(int a, character_base *dd=0);
	virtual void addstat1(int stat,int val, bool forcesub=false)=0;
	void binit()
	{
		abr=false;
		sweapontype=0;
		buffs.clear();
		holycross=false;
		berserk=false;
		di=false;
		gm_di=false;
		isDead = false;
		playerLock=0;
		partyLock=0;
	}
	void spawnbufficons(buffer *bs);
	void addbuff(int bufferid, int skill, int skilllevel, int duration);
	void removebuffl(int skill);
	void removebuff(int skill);
	void clearbuffs();


	bool doskill(int skill, int level, int target, int as=0, bool checkcooldown=true, character_buffable *focus=0);
	int getdmg(character_buffable *target, int &flag, int basedmg=0, tskilldata *s=0, bool ranged=false);
	void damage(buffer &bs, character_buffable *target, int dmg, int flag, tskilldata *s=0, int motion=-1);
	void attack(buffer &bs, int mot, int basedmg=0, tskilldata *s=0);
	void attack(int mot);
	void attack(int mot, int sz);
	void aoeattack(buffer &bs, int basedmg, tskilldata *s, int duration, character_buffable *aoecenter);
	void aoeattack(buffer &bs, int basedmg, float distance, character_buffable *aoecenter);

	virtual void adddelayed(buffer *b, long long t)=0;

	class timedbuffer
	{
	public:
		long long t;
		buffer *bs;
		timedbuffer(buffer *b, long long a):bs(b),t(a){};
		timedbuffer(const timedbuffer &a):bs(a.bs),t(a.t){};
		timedbuffer& operator=(const timedbuffer &a)
		{
			bs=a.bs;
			t=a.t;
			return *this;
		}
		bool operator==(const timedbuffer &a) const
		{
			return t==a.t;
		}
		bool operator<=(const timedbuffer &a) const
		{
			return t>=a.t;
		}
		bool operator<(const timedbuffer &a) const
		{
			return t>a.t;
		}
	};

	std::priority_queue<timedbuffer> tbs;
	virtual void managekarma(character_buffable *p)
	{
	}

};


#endif
