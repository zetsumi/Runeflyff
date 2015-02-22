#include "stdafx.h"
#include "inventory.h"
#include "cluster.h"

#include "network.h"
#include "funkciok.h"
#include "player.h"
#include "logger.h"
#include "adat.h"


void itemammount(buffer &bs, int id, int hely, int ammount)
{
	bs.cmd(id, 0x18);
	bs << (char)0 << (short)hely << (short)ammount << (short)0;
}

void inv_eq(buffer &bs, int playerid, int hely1, int hely2, item *targy, int equ)
{
	bs.cmd(playerid, 6);
	bs << hely1 << (char)0 << (char)equ << targy->getId() << (short)targy->upgrade << (char)targy->element << (char)targy->eupgrade << 0 << hely2;
}

void inv_move(buffer &bs, int playerid, int a, int b, int c, int d)
{
	bs.cmd(playerid, 4);
	bs << (char)0 << (char)b << (char)a;
}



void inventory::fillmask()
{
    unsigned int a;
    for(a=0;a<(unsigned int)eqoff2;a++)mask.at(a)=a;
    for(a=(unsigned int)eqoff2;a<(unsigned int)nitems;a++)if(itemek.at(a).getId()!=-1)mask.at(a)=a; else mask.at(a)=-1;
}

void inventory::filllookup()
{
    int a;

    for(a=0;a<nitems;a++)
	{
		lookup[a]=-1;
	}
    for(a=0;a<nitems;a++)
    {
		if(mask[a]!=-1)lookup[mask[a]]=a;
	}
}

void inventory::allocate(int b1)
{
	for(int c3=0;c3<nitems;c3++)
	{
		if((itemek.at(c3).getId()==-1)&&(lookup.at(c3)==-1))
		{
			mask.at(b1)=c3;
			lookup.at(mask.at(b1))=b1;
			return;
		}
	}
	throw error("Can't allocate for inventory (shouldn't reach this point ever)", "inventory::allocate");
}


class bonust
{
	int hat,suit,gaunt,shoes;
public:
	bonust(int a):hat(a),suit(a+1),gaunt(a+2),shoes(a+3){};
	bonust(int h, int s, int g, int sh):hat(h),suit(s),gaunt(g),shoes(sh){};
	bonust(const bonust& a):hat(a.hat), suit(a.suit), gaunt(a.gaunt), shoes(a.shoes)
	{
		bonuses[0]=a.bonuses[0];
		bonuses[1]=a.bonuses[1];
		bonuses[2]=a.bonuses[2];
	};
	bonust& operator=(const bonust& a)
	{
		hat=a.hat;
		suit=a.suit;
		gaunt=a.gaunt;
		shoes=a.shoes;
		bonuses[0]=a.bonuses[0];
		bonuses[1]=a.bonuses[1];
		bonuses[2]=a.bonuses[2];
		return *this;
	}
	int match(const bonust& a)
	{
		int r=0;
		if(hat==a.hat)r++;
		if(suit==a.suit)r++;
		if(gaunt==a.gaunt)r++;
		if(shoes==a.shoes)r++;
		return r;
	}
	std::vector2<bonus> bonuses[3];
};

std::vector2<bonust> setbonuses;
void initsetbonuses()
{
	int a=-1;
/*5*/
//leaf
	setbonuses.push_back(bonust(526));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_HP_MAX, 50));
	setbonuses[a].bonuses[0].push_back(bonus(DST_STR, 3));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 23));
	setbonuses[a].bonuses[1].push_back(bonus(DST_SPEED, 23));
	setbonuses[a].bonuses[2].push_back(bonus(DST_HP_MAX, 150));
//Parmil
	setbonuses.push_back(bonust(522));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_HP_MAX, 50));
	setbonuses[a].bonuses[0].push_back(bonus(DST_STR, 3));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 23));
	setbonuses[a].bonuses[1].push_back(bonus(DST_SPEED, 23));
	setbonuses[a].bonuses[2].push_back(bonus(DST_HP_MAX, 150));

/*15*/
//flower
	setbonuses.push_back(bonust(18000));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 17));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 22));
	setbonuses[a].bonuses[1].push_back(bonus(DST_HP_MAX_RATE, 10));
	setbonuses[a].bonuses[2].push_back(bonus(DST_AXE_DMG, 4));
	setbonuses[a].bonuses[2].push_back(bonus(DST_SWD_DMG, 4));
//petal
	setbonuses.push_back(bonust(18004));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 17));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 22));
	setbonuses[a].bonuses[1].push_back(bonus(DST_HP_MAX_RATE, 10));
	setbonuses[a].bonuses[2].push_back(bonus(DST_AXE_DMG, 4));
	setbonuses[a].bonuses[2].push_back(bonus(DST_SWD_DMG, 4));
//stinger
	setbonuses.push_back(bonust(18008));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 16));
	setbonuses[a].bonuses[0].push_back(bonus(DST_DEX, 3));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 20));
	setbonuses[a].bonuses[1].push_back(bonus(DST_FP_MAX_RATE, 10));
	setbonuses[a].bonuses[1].push_back(bonus(DST_MP_MAX_RATE, 10));
	setbonuses[a].bonuses[2].push_back(bonus(DST_CHR_CHANCECRITICAL, 10));
//slepint
	setbonuses.push_back(bonust(18012));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 16));
	setbonuses[a].bonuses[0].push_back(bonus(DST_DEX, 3));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 20));
	setbonuses[a].bonuses[1].push_back(bonus(DST_FP_MAX_RATE, 10));
	setbonuses[a].bonuses[1].push_back(bonus(DST_MP_MAX_RATE, 10));
	setbonuses[a].bonuses[2].push_back(bonus(DST_CHR_CHANCECRITICAL, 10));
//curus
	setbonuses.push_back(bonust(18016));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 15));
	setbonuses[a].bonuses[0].push_back(bonus(DST_DEX, 3));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 18));
	setbonuses[a].bonuses[1].push_back(bonus(DST_FP_MAX_RATE, 10));
	setbonuses[a].bonuses[1].push_back(bonus(DST_MP_MAX_RATE, 10));
	setbonuses[a].bonuses[2].push_back(bonus(DST_ATTACKSPEED, 200));
//sardine
	setbonuses.push_back(bonust(18020));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 15));
	setbonuses[a].bonuses[0].push_back(bonus(DST_DEX, 3));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 18));
	setbonuses[a].bonuses[1].push_back(bonus(DST_FP_MAX_RATE, 10));
	setbonuses[a].bonuses[1].push_back(bonus(DST_MP_MAX_RATE, 10));
	setbonuses[a].bonuses[2].push_back(bonus(DST_ATTACKSPEED, 200));
//niz
	setbonuses.push_back(bonust(18024));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 14));
	setbonuses[a].bonuses[0].push_back(bonus(DST_INT, 3));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 18));
	setbonuses[a].bonuses[1].push_back(bonus(DST_MP_MAX_RATE, 10));
	setbonuses[a].bonuses[2].push_back(bonus(DST_HP_MAX_RATE, 10));
//seer
	setbonuses.push_back(bonust(18028));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 14));
	setbonuses[a].bonuses[0].push_back(bonus(DST_INT, 3));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 18));
	setbonuses[a].bonuses[1].push_back(bonus(DST_MP_MAX_RATE, 10));
	setbonuses[a].bonuses[2].push_back(bonus(DST_HP_MAX_RATE, 10));

/*30*/
//cylos
	setbonuses.push_back(bonust(18032));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 22));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 33));
	setbonuses[a].bonuses[1].push_back(bonus(DST_CHR_CHANCECRITICAL, 5));
	setbonuses[a].bonuses[2].push_back(bonus(DST_ADJ_HITRATE, 10));
//cylos
	setbonuses.push_back(bonust(18036));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 22));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 33));
	setbonuses[a].bonuses[1].push_back(bonus(DST_CHR_CHANCECRITICAL, 5));
	setbonuses[a].bonuses[2].push_back(bonus(DST_ADJ_HITRATE, 10));
//Miragle
	setbonuses.push_back(bonust(18040));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 20));
	setbonuses[a].bonuses[0].push_back(bonus(DST_HP_MAX_RATE, 10));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 30));
	setbonuses[a].bonuses[1].push_back(bonus(DST_FP_DEC_RATE, 30));
	setbonuses[a].bonuses[1].push_back(bonus(DST_MP_DEC_RATE, 30));
	setbonuses[a].bonuses[2].push_back(bonus(DST_ATTACKSPEED, 30));
//Cruiser
	setbonuses.push_back(bonust(18076));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 20));
	setbonuses[a].bonuses[0].push_back(bonus(DST_HP_MAX_RATE, 10));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 30));
	setbonuses[a].bonuses[1].push_back(bonus(DST_FP_DEC_RATE, 30));
	setbonuses[a].bonuses[1].push_back(bonus(DST_MP_DEC_RATE, 30));
	setbonuses[a].bonuses[2].push_back(bonus(DST_ATTACKSPEED, 30));
//Sayram
	setbonuses.push_back(bonust(18048));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 19));
	setbonuses[a].bonuses[0].push_back(bonus(DST_INT, 10));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 28));
	setbonuses[a].bonuses[1].push_back(bonus(DST_FP_DEC_RATE, 15));
	setbonuses[a].bonuses[1].push_back(bonus(DST_MP_DEC_RATE, 15));
	setbonuses[a].bonuses[2].push_back(bonus(DST_CHR_CHANCECRITICAL, 10));
//Talin
	setbonuses.push_back(bonust(18052));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 19));
	setbonuses[a].bonuses[0].push_back(bonus(DST_INT, 10));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 28));
	setbonuses[a].bonuses[1].push_back(bonus(DST_FP_DEC_RATE, 15));
	setbonuses[a].bonuses[1].push_back(bonus(DST_MP_DEC_RATE, 15));
	setbonuses[a].bonuses[2].push_back(bonus(DST_CHR_CHANCECRITICAL, 10));
//Merr
	setbonuses.push_back(bonust(18056));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_INT, 5));
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 17));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 27));
	setbonuses[a].bonuses[1].push_back(bonus(DST_HP_MAX_RATE, 10));
	setbonuses[a].bonuses[2].push_back(bonus(DST_MP_DEC_RATE, 10));
//Sorain
	setbonuses.push_back(bonust(18060));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_INT, 5));
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 17));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 27));
	setbonuses[a].bonuses[1].push_back(bonus(DST_HP_MAX_RATE, 10));
	setbonuses[a].bonuses[2].push_back(bonus(DST_MP_DEC_RATE, 10));

/*45*/
//Stitch
	setbonuses.push_back(bonust(18064));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_FP_DEC_RATE, 10));
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 22));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 23));
	setbonuses[a].bonuses[1].push_back(bonus(DST_HP_MAX_RATE, 10));
	setbonuses[a].bonuses[2].push_back(bonus(DST_ATTACKSPEED, 400));
//Panggril
	setbonuses.push_back(bonust(18068));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_FP_DEC_RATE, 10));
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 22));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 23));
	setbonuses[a].bonuses[1].push_back(bonus(DST_HP_MAX_RATE, 10));
	setbonuses[a].bonuses[2].push_back(bonus(DST_ATTACKSPEED, 400));
//Hyper
	setbonuses.push_back(bonust(18072));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_BOW_DMG, 11));
	setbonuses[a].bonuses[0].push_back(bonus(DST_YOY_DMG, 10));
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 20));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 21));
	setbonuses[a].bonuses[1].push_back(bonus(DST_FP_DEC_RATE, 20));
	setbonuses[a].bonuses[1].push_back(bonus(DST_MP_DEC_RATE, 20));
	setbonuses[a].bonuses[2].push_back(bonus(DST_CRITICAL_BONUS, 40));
//Cruiser
	setbonuses.push_back(bonust(18076));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_BOW_DMG, 11));
	setbonuses[a].bonuses[0].push_back(bonus(DST_YOY_DMG, 10));
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 20));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 21));
	setbonuses[a].bonuses[1].push_back(bonus(DST_FP_DEC_RATE, 20));
	setbonuses[a].bonuses[1].push_back(bonus(DST_MP_DEC_RATE, 20));
	setbonuses[a].bonuses[2].push_back(bonus(DST_CRITICAL_BONUS, 40));
//Flury
	setbonuses.push_back(bonust(18080));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_INT, 7));
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 19));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 19));
	setbonuses[a].bonuses[1].push_back(bonus(DST_FP_DEC_RATE, 20));
	setbonuses[a].bonuses[1].push_back(bonus(DST_MP_DEC_RATE, 20));
	setbonuses[a].bonuses[2].push_back(bonus(DST_SPELL_RATE, 30));
//	int b=setbonuses[a].bonuses[2].size();
//Wedge
	setbonuses.push_back(bonust(18084));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_INT, 7));
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 19));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 19));
	setbonuses[a].bonuses[1].push_back(bonus(DST_FP_DEC_RATE, 20));
	setbonuses[a].bonuses[1].push_back(bonus(DST_MP_DEC_RATE, 20));
	setbonuses[a].bonuses[2].push_back(bonus(DST_SPELL_RATE, 30));
//Misty
	setbonuses.push_back(bonust(18088));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_HP_MAX_RATE, 10));
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 18));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 19));
	setbonuses[a].bonuses[1].push_back(bonus(DST_MP_DEC_RATE, 15));
	setbonuses[a].bonuses[2].push_back(bonus(DST_SPELL_RATE, 10));
//Teba
	setbonuses.push_back(bonust(18092));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_HP_MAX_RATE, 10));
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 18));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 19));
	setbonuses[a].bonuses[1].push_back(bonus(DST_MP_DEC_RATE, 15));
	setbonuses[a].bonuses[2].push_back(bonus(DST_SPELL_RATE, 10));

/*60*/
//Rescorn
	setbonuses.push_back(bonust(22070));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 22));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 28));
	setbonuses[a].bonuses[1].push_back(bonus(DST_HP_MAX_RATE, 15));
	setbonuses[a].bonuses[2].push_back(bonus(DST_ATTACKSPEED, 400));
//Recorn
	setbonuses.push_back(bonust(22066));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 22));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 28));
	setbonuses[a].bonuses[1].push_back(bonus(DST_HP_MAX_RATE, 15));
	setbonuses[a].bonuses[2].push_back(bonus(DST_ATTACKSPEED, 400));
//Restron
	setbonuses.push_back(bonust(22078));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 20));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 28));
	setbonuses[a].bonuses[1].push_back(bonus(DST_STR, 10));
	setbonuses[a].bonuses[2].push_back(bonus(DST_ADJ_HITRATE, 15));
//Rebron
	setbonuses.push_back(bonust(22074));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 20));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 28));
	setbonuses[a].bonuses[1].push_back(bonus(DST_STR, 10));
	setbonuses[a].bonuses[2].push_back(bonus(DST_ADJ_HITRATE, 15));
//Shuran
	setbonuses.push_back(bonust(22086));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 19));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 26));
	setbonuses[a].bonuses[1].push_back(bonus(DST_INT, 8));
	setbonuses[a].bonuses[2].push_back(bonus(DST_HP_MAX_RATE, 15));
//Shurand
	setbonuses.push_back(bonust(22082));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 19));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 26));
	setbonuses[a].bonuses[1].push_back(bonus(DST_INT, 8));
	setbonuses[a].bonuses[2].push_back(bonus(DST_HP_MAX_RATE, 15));
//Anoos
	setbonuses.push_back(bonust(22094));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 18));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 22));
	setbonuses[a].bonuses[1].push_back(bonus(DST_MP_DEC_RATE, 13));
	setbonuses[a].bonuses[2].push_back(bonus(DST_INT, 10));
//Ahes
	setbonuses.push_back(bonust(22090));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 18));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 22));
	setbonuses[a].bonuses[1].push_back(bonus(DST_MP_DEC_RATE, 13));
	setbonuses[a].bonuses[2].push_back(bonus(DST_INT, 10));

/*knigth*/
//Warpon
	setbonuses.push_back(bonust(22098));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 25));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 25));
	setbonuses[a].bonuses[1].push_back(bonus(DST_CHR_CHANCECRITICAL, 15));
	setbonuses[a].bonuses[2].push_back(bonus(DST_ADJ_HITRATE, 15));
//Warspon
	setbonuses.push_back(bonust(22102));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 25));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 25));
	setbonuses[a].bonuses[1].push_back(bonus(DST_CHR_CHANCECRITICAL, 15));
	setbonuses[a].bonuses[2].push_back(bonus(DST_ADJ_HITRATE, 15));
//Weeshian
	setbonuses.push_back(bonust(22110));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 25));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 25));
	setbonuses[a].bonuses[1].push_back(bonus(DST_FP_DEC_RATE, 15));
	setbonuses[a].bonuses[2].push_back(bonus(DST_STR, 7));
//Wees
	setbonuses.push_back(bonust(22106));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 25));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 25));
	setbonuses[a].bonuses[1].push_back(bonus(DST_FP_DEC_RATE, 15));
	setbonuses[a].bonuses[2].push_back(bonus(DST_STR, 7));
//Extro
	setbonuses.push_back(bonust(22118));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 24));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 24));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ATTACKSPEED, 300));
	setbonuses[a].bonuses[2].push_back(bonus(DST_DEX, 7));
//Ectro
	setbonuses.push_back(bonust(22114));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 24));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 24));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ATTACKSPEED, 300));
	setbonuses[a].bonuses[2].push_back(bonus(DST_DEX, 7));

/*blade*/
//Akan
	setbonuses.push_back(bonust(22126));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 26));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 29));
	setbonuses[a].bonuses[1].push_back(bonus(DST_STR, 10));
	setbonuses[a].bonuses[2].push_back(bonus(DST_HP_MAX_RATE, 15));
//Einy
	setbonuses.push_back(bonust(22122));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 26));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 29));
	setbonuses[a].bonuses[1].push_back(bonus(DST_STR, 10));
	setbonuses[a].bonuses[2].push_back(bonus(DST_HP_MAX_RATE, 15));
//Dayst
	setbonuses.push_back(bonust(22130));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 26));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 29));
	setbonuses[a].bonuses[1].push_back(bonus(DST_CHR_CHANCECRITICAL, 15));
	setbonuses[a].bonuses[2].push_back(bonus(DST_DEX, 5));
//Dest
	setbonuses.push_back(bonust(22134));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 26));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 29));
	setbonuses[a].bonuses[1].push_back(bonus(DST_CHR_CHANCECRITICAL, 15));
	setbonuses[a].bonuses[2].push_back(bonus(DST_DEX, 5));
//Haganes
	setbonuses.push_back(bonust(22142));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 25));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 29));
	setbonuses[a].bonuses[1].push_back(bonus(DST_HP_MAX_RATE, 13));
	setbonuses[a].bonuses[2].push_back(bonus(DST_REFLECT_DAMAGE, 10));
//Hanes
	setbonuses.push_back(bonust(22138));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 25));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 29));
	setbonuses[a].bonuses[1].push_back(bonus(DST_HP_MAX_RATE, 13));
	setbonuses[a].bonuses[2].push_back(bonus(DST_REFLECT_DAMAGE, 10));

/*Jester*/
//Katria
	setbonuses.push_back(bonust(22146));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 20));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 27));
	setbonuses[a].bonuses[1].push_back(bonus(DST_FP_DEC_RATE, 15));
	setbonuses[a].bonuses[2].push_back(bonus(DST_DEX, 8));
//Kanarin
	setbonuses.push_back(bonust(22150));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 20));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 27));
	setbonuses[a].bonuses[1].push_back(bonus(DST_FP_DEC_RATE, 15));
	setbonuses[a].bonuses[2].push_back(bonus(DST_DEX, 8));
//Restra
	setbonuses.push_back(bonust(22154));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 21));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 28));
	setbonuses[a].bonuses[1].push_back(bonus(DST_CHR_CHANCECRITICAL, 15));
	setbonuses[a].bonuses[2].push_back(bonus(DST_CRITICAL_BONUS, 40));
//Rasra
	setbonuses.push_back(bonust(22158));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 21));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 28));
	setbonuses[a].bonuses[1].push_back(bonus(DST_CHR_CHANCECRITICAL, 15));
	setbonuses[a].bonuses[2].push_back(bonus(DST_CRITICAL_BONUS, 40));
//Neis
	setbonuses.push_back(bonust(22162));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 20));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 28));
	setbonuses[a].bonuses[1].push_back(bonus(DST_STR, 10));
	setbonuses[a].bonuses[2].push_back(bonus(DST_DEX, 10));
//Nation
	setbonuses.push_back(bonust(22166));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 20));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 28));
	setbonuses[a].bonuses[1].push_back(bonus(DST_STR, 10));
	setbonuses[a].bonuses[2].push_back(bonus(DST_DEX, 10));

/*Ranger*/	
//Oska
	setbonuses.push_back(bonust(22170));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 21));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 29));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJ_HITRATE, 15));
	setbonuses[a].bonuses[2].push_back(bonus(DST_HP_MAX_RATE, 15));
//Okas
	setbonuses.push_back(bonust(22174));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 21));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 29));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJ_HITRATE, 15));
	setbonuses[a].bonuses[2].push_back(bonus(DST_HP_MAX_RATE, 15));
//Asren
	setbonuses.push_back(bonust(22178));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 21));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 29));
	setbonuses[a].bonuses[1].push_back(bonus(DST_STR, 7));
	setbonuses[a].bonuses[2].push_back(bonus(DST_CHR_CHANCECRITICAL, 12));
//Aren
	setbonuses.push_back(bonust(22182));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 21));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 29));
	setbonuses[a].bonuses[1].push_back(bonus(DST_STR, 7));
	setbonuses[a].bonuses[2].push_back(bonus(DST_CHR_CHANCECRITICAL, 12));
//Tyrent
	setbonuses.push_back(bonust(22186));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 20));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 28));
	setbonuses[a].bonuses[1].push_back(bonus(DST_HP_MAX_RATE, 15));
	setbonuses[a].bonuses[2].push_back(bonus(DST_REFLECT_DAMAGE, 10));
//Tight
	setbonuses.push_back(bonust(22190));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 20));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 28));
	setbonuses[a].bonuses[1].push_back(bonus(DST_HP_MAX_RATE, 15));
	setbonuses[a].bonuses[2].push_back(bonus(DST_REFLECT_DAMAGE, 10));

/*Ringmaster*/
//Pact
	setbonuses.push_back(bonust(22194));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 20));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 27));
	setbonuses[a].bonuses[1].push_back(bonus(DST_MP_MAX_RATE, 15));
	setbonuses[a].bonuses[2].push_back(bonus(DST_INT, 10));
//Paxt
	setbonuses.push_back(bonust(22198));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 20));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 27));
	setbonuses[a].bonuses[1].push_back(bonus(DST_MP_MAX_RATE, 15));
	setbonuses[a].bonuses[2].push_back(bonus(DST_INT, 10));
//Shupon
	setbonuses.push_back(bonust(22202));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 20));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 27));
	setbonuses[a].bonuses[1].push_back(bonus(DST_INT, 7));
	setbonuses[a].bonuses[2].push_back(bonus(DST_MP_DEC_RATE, 20));
//Shrian
	setbonuses.push_back(bonust(22206));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 20));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 27));
	setbonuses[a].bonuses[1].push_back(bonus(DST_INT, 7));
	setbonuses[a].bonuses[2].push_back(bonus(DST_MP_DEC_RATE, 20));
//Navil
	setbonuses.push_back(bonust(22210));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 21));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 27));
	setbonuses[a].bonuses[1].push_back(bonus(DST_HP_MAX_RATE, 15));
	setbonuses[a].bonuses[2].push_back(bonus(DST_SPELL_RATE, 20));
//Neclus
	setbonuses.push_back(bonust(22214));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 21));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 27));
	setbonuses[a].bonuses[1].push_back(bonus(DST_HP_MAX_RATE, 15));
	setbonuses[a].bonuses[2].push_back(bonus(DST_SPELL_RATE, 20));

/*Billposter*/
//Alext
	setbonuses.push_back(bonust(22218));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 21));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 22));
	setbonuses[a].bonuses[1].push_back(bonus(DST_DEX, 7));
	setbonuses[a].bonuses[2].push_back(bonus(DST_HP_MAX_RATE, 15));
//Ales
	setbonuses.push_back(bonust(22222));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 21));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 22));
	setbonuses[a].bonuses[1].push_back(bonus(DST_DEX, 7));
	setbonuses[a].bonuses[2].push_back(bonus(DST_HP_MAX_RATE, 15));
//Akent
	setbonuses.push_back(bonust(22226));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 21));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 22));
	setbonuses[a].bonuses[1].push_back(bonus(DST_MP_DEC_RATE, 15));
	setbonuses[a].bonuses[2].push_back(bonus(DST_DEX, 5));
//Aken
	setbonuses.push_back(bonust(22230));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 21));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 22));
	setbonuses[a].bonuses[1].push_back(bonus(DST_MP_DEC_RATE, 15));
	setbonuses[a].bonuses[2].push_back(bonus(DST_DEX, 5));
//Rody
	setbonuses.push_back(bonust(22234));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 20));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 27));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJ_HITRATE, 12));
	setbonuses[a].bonuses[2].push_back(bonus(DST_CRITICAL_BONUS, 40));
//Rodey
	setbonuses.push_back(bonust(22238));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 20));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 27));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJ_HITRATE, 12));
	setbonuses[a].bonuses[2].push_back(bonus(DST_CRITICAL_BONUS, 40));

/*Psykeeper*/
//Ropang
	setbonuses.push_back(bonust(22242));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 20));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 27));
	setbonuses[a].bonuses[1].push_back(bonus(DST_INT, 7));
	setbonuses[a].bonuses[2].push_back(bonus(DST_MP_DEC_RATE, 15));
//Roshel
	setbonuses.push_back(bonust(22246));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 20));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 27));
	setbonuses[a].bonuses[1].push_back(bonus(DST_INT, 7));
	setbonuses[a].bonuses[2].push_back(bonus(DST_MP_DEC_RATE, 15));
//Geniun
	setbonuses.push_back(bonust(22250));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 20));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 27));
	setbonuses[a].bonuses[1].push_back(bonus(DST_SPELL_RATE, 7));
	setbonuses[a].bonuses[2].push_back(bonus(DST_INT, 6));
//Gynas
	setbonuses.push_back(bonust(22254));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 20));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 27));
	setbonuses[a].bonuses[1].push_back(bonus(DST_SPELL_RATE, 7));
	setbonuses[a].bonuses[2].push_back(bonus(DST_INT, 6));
//Mekatro
	setbonuses.push_back(bonust(22258));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 20));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 27));
	setbonuses[a].bonuses[1].push_back(bonus(DST_MP_DEC_RATE, 15));
	setbonuses[a].bonuses[2].push_back(bonus(DST_INT, 5));
//Mekaron
	setbonuses.push_back(bonust(22262));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 20));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 27));
	setbonuses[a].bonuses[1].push_back(bonus(DST_MP_DEC_RATE, 15));
	setbonuses[a].bonuses[2].push_back(bonus(DST_INT, 5));

/*Elementor*/
//Myuran
	setbonuses.push_back(bonust(22266));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 22));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 33));
	setbonuses[a].bonuses[1].push_back(bonus(DST_SPELL_RATE, 20));
	setbonuses[a].bonuses[2].push_back(bonus(DST_MP_DEC_RATE, 15));
//Myurian
	setbonuses.push_back(bonust(22270));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 22));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 33));
	setbonuses[a].bonuses[1].push_back(bonus(DST_SPELL_RATE, 20));
	setbonuses[a].bonuses[2].push_back(bonus(DST_MP_DEC_RATE, 15));
//Elder
	setbonuses.push_back(bonust(22274));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 22));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 28));
	setbonuses[a].bonuses[1].push_back(bonus(DST_INT, 7));
	setbonuses[a].bonuses[2].push_back(bonus(DST_MP_DEC_RATE, 15));
//Ellean
	setbonuses.push_back(bonust(22278));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 22));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 28));
	setbonuses[a].bonuses[1].push_back(bonus(DST_INT, 7));
	setbonuses[a].bonuses[2].push_back(bonus(DST_MP_DEC_RATE, 15));
//Shabel
	setbonuses.push_back(bonust(22282));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 21));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 27));
	setbonuses[a].bonuses[1].push_back(bonus(DST_SPELL_RATE, 15));
	setbonuses[a].bonuses[2].push_back(bonus(DST_HP_MAX_RATE, 15));
//Shurian
	setbonuses.push_back(bonust(22286));a++;
	setbonuses[a].bonuses[0].push_back(bonus(DST_ADJDEF, 21));
	setbonuses[a].bonuses[1].push_back(bonus(DST_ADJDEF, 27));
	setbonuses[a].bonuses[1].push_back(bonus(DST_SPELL_RATE, 15));
	setbonuses[a].bonuses[2].push_back(bonus(DST_HP_MAX_RATE, 15));


//	setbonuses[a].bonuses[1].push_back(bonus(, 30));
};

void inventory::checksetbonus()
{
	int hat=-1,suit=-1,gaunt=-1,shoes=-1;
	int a,b,c,match,am=0;
	player->removesetbonuses();

	if(mask[eqoff2+6]>=0)hat  =itemek[mask[eqoff2+6]].getId();
	if(mask[eqoff2+2]>=0)suit =itemek[mask[eqoff2+2]].getId();
	if(mask[eqoff2+4]>=0)gaunt=itemek[mask[eqoff2+4]].getId();
	if(mask[eqoff2+5]>=0)shoes=itemek[mask[eqoff2+5]].getId();

	bonust tocheck(hat, suit, gaunt, shoes);
	for(a=0;(a<(int)setbonuses.size())&&(am<4);a++)
	{
		match=setbonuses[a].match(tocheck);
		am+=match;
		match--;
		for(b=0;b<match;b++)
		{
			for(c=0;c<(int)setbonuses[a].bonuses[b].size();c++)
				player->addsetbonus(setbonuses[a].bonuses[b][c]);
		}
	}

}

void inventory::checkset()
{

	int up=10;
	int i;
	int cups[4]={2+eqoff2,4+eqoff2,5+eqoff2,6+eqoff2};

	for(i=0;i<4;i++)
		if(mask.at(cups[i])==-1)
		{
			up=0;
			break;
		}else
			if(itemek.at(mask.at(cups[i])).upgrade<up)up=itemek.at(mask.at(cups[i])).upgrade;

//	if(player!=0)
	{
		int lastup=player->getlastupnm();
		if(up!=lastup)
		{
			if(lastup>2)remupbonus();
			if(up>2)
			{
				if(uphitbonus.at(up)!=0)player->addstat1(DST_ATKPOWER_RATE, uphitbonus.at(up));
				if(uphitbonus.at(up)!=0)player->addstat1(DST_ADJ_HITRATE, uphitbonus.at(up));

				if(upblockbonus.at(up)!=0)player->addstat1(DST_BLOCK_RANGE, upblockbonus.at(up));
				if(upblockbonus.at(up)!=0)player->addstat1(DST_BLOCK_MELEE, upblockbonus.at(up));
				if(upblockbonus.at(up)!=0)player->addstat1(DST_ADJDEF_RATE, upblockbonus.at(up));

				if(upallstatbonus.at(up)!=0)player->addstat1(DST_STAT_ALLUP, upallstatbonus.at(up));

				if(uphpbonus.at(up)!=0)player->addstat1(DST_HP_MAX_RATE, uphpbonus.at(up));

				if(upmagicbonus.at(up)!=0)player->addstat1(DST_ADDMAGIC, upmagicbonus.at(up));
			}
		}
		checksetbonus();
		player->setlastupnm(up);
	}

}

void inventory::remupbonus()
{
//	if(player!=0)
	{
		int up=player->getlastupnm();
		if(up>2)
		{
			if(uphitbonus.at(up)!=0)player->addstat1(DST_ATKPOWER_RATE, -uphitbonus.at(up));
			if(uphitbonus.at(up)!=0)player->addstat1(DST_ADJ_HITRATE, -uphitbonus.at(up));

			if(upblockbonus.at(up)!=0)player->addstat1(DST_BLOCK_RANGE, -upblockbonus.at(up));
			if(upblockbonus.at(up)!=0)player->addstat1(DST_BLOCK_MELEE, -upblockbonus.at(up));
			if(upblockbonus.at(up)!=0)player->addstat1(DST_ADJDEF_RATE, -upblockbonus.at(up));

			if(upallstatbonus.at(up)!=0)player->addstat1(DST_STAT_ALLUP, -upallstatbonus.at(up));

			if(uphpbonus.at(up)!=0)player->addstat1(DST_HP_MAX_RATE, -uphpbonus.at(up));

			if(upmagicbonus.at(up)!=0)player->addstat1(DST_ADDMAGIC, -upmagicbonus.at(up));
		}
		player->setlastupnm(0);
	}

}


int inventory::removeitem(int place, int num)
{
	if(num<=0)return 0;
	if((place<0)||(place>=nitems))throw error(std::string("Incorrect item slot ")+toString(place), "inventory::removeitem");


    if((itemek.at(place).num<num)||(num<0))num=itemek.at(place).num;
    itemek.at(place).num-=num;
    if(itemek[place].num<=0)
	{
		itemek[place].setId(-1);
		itemek[place].num=0;
	}
	if(player!=0)itemammount(*player->bs, playerid, place, itemek[place].num);
    return num;
}

int inventory::removeitemid(int id, int num)
{
	int a;
	for(a=0;(a<eqoff2)&&(num>0);a++)
		if(itemek.at(a).getId()==id)
			num-=removeitem(a,num);
    return num;
}

void	inventory::moveitem(int a, int b)
{
		if(a==b)return;
		if((a<0)||(a>=nitems)||(b<0)||(b>=nitems))throw error(std::string("Incorrect item slot ")+toString(a) + " " + toString(b), "inventory::moveitem");
        int c;
        bool stack=false;
		int nstack=0;
		int q=getitempos(itemek.at(mask.at(a)).getId());
		if(q>=0)nstack=itemlist.at(q).num;

//        if((a>=eqoff2)||(b>=eqoff2))return;
//		if((mask[a]>=eqoff2)||(mask[a]<1)||(mask.at(b)>=eqoff2)||(mask[b]<0))return;
		if((mask[a]<0)||(mask[b]<0))return;

        if(itemek[mask[a]].getId() == itemek.at(mask.at(b)).getId())
		{
			if(itemek[mask[a]].getId()==-1)return;
            stack=(nstack>1);
        }

        if(stack)
		{
			c=itemek[mask[b]].num;
			if(c>nstack-itemek[mask[a]].num)c=nstack-itemek[mask[a]].num;
			itemek[mask[b]].num-=c;
			itemek[mask[a]].num+=c;
			itemammount(*player->bs, playerid, mask[a], itemek[mask[a]].num);
			itemammount(*player->bs, playerid, mask[b], itemek[mask[b]].num);
			if(itemek[mask[b]].num==0)itemek[mask[b]].setId(-1);
        }else{
			if(mask[a]!=-1)lookup[mask[a]]=-1;
			if(mask[b]!=-1)lookup[mask[b]]=-1;

            c=mask[a];
            mask[a]=mask[b];
            mask[b]=c;

			if(mask[a]!=-1)lookup[mask[a]]=a;
			if(mask[b]!=-1)lookup[mask[b]]=b;
			inv_move(*player->bs, playerid, a, b, this->itemek[mask[a]].num, this->itemek[mask[b]].num);
        }
}

bool inventory::unequip2(int a)
{
	int c1,a1;
	if((a<0)||(a>=nitems))throw error(std::string("Incorrect item slot ")+toString(a), "inventory::unequip");

	for(c1=0;c1<eqoff2;c1++)
        if((mask[c1]!=-1)&&(itemek.at(mask.at(c1)).getId()==-1))
		{
            break;
		}

	if(c1>=eqoff2)
	{
		player->bs->cmd(playerid, 0x55);
        return false;
    }

	itemek.at(a).rembonus(player);

	bool tset=false;
	
	if(itemek[a].GetI()!=0)
	{
		int k=itemek[a].GetI()->eqslot;
		tset=((k==2)||(k==4)||(k==5)||(k==6));
		if(tset)remupbonus();
	}

	a1=lookup[a];
	if(a1==10+eqoff2)player->OnWeaponUnequip();
	else if(a1==11+eqoff2)player->OnShieldUnequip();

	/*if(mask.at(a1)!=-1)*/lookup.at(mask.at(a1))=-1;
	/*if(mask[c1]!=-1)*/lookup[mask[c1]]=-1;


//    d=mask[c1];
    mask[c1]=mask[a1];
    mask[a1]=-1;

//	if(mask[a1]!=-1)lookup[mask[a1]]=a1;
	/*if(mask[c1]!=-1)*/lookup.at(mask[c1])=c1;

	if((a1==10+eqoff2)&&(mask[9+eqoff2]!=-1))
	{
		lookup[mask[9+eqoff2]]=a1;
		mask[a1]=mask[9+eqoff2];
		mask[9+eqoff2]=-1;
		lookup[mask[a1]]=a1;
	}

	if(tset)checksetbonus();
	return true;
}


bool inventory::unequip(int a)
{
	if((a<0)||(a>=nitems))throw error(std::string("Incorrect item slot ")+toString(a), "inventory::unequip");
	if(lookup[a]<eqoff2)return equip(a);
	int b=lookup[a]-(eqoff2);

	bool retval=unequip2(a);
	if(retval)
	{
		if(player->addstat1type==0)inv_eq(*player->bs, playerid, a, b, &itemek[a], 0);
		if(b>=0)inv_eq(*player->tomulticast2, playerid, b, b, &itemek[a], 0);
		player->HMF();

		bool oldflying=player->flying;
		player->flying=mask.at(eqoff2+13)!=-1;
		if(oldflying!=player->flying)
		{
			player->stopmove();
			player->unsummonpet();
			player->setlastpos();
//			player->checkmove();
			player->onChangePosition();
		}
#ifdef _DEBUG
#ifdef _LOGALOT
		logger.log("Flying %d\n", (int)player->flying);
#endif
#endif
	}
	return retval;
}



bool inventory::equip(int a)
{
	if((a<0)||(a>=nitems))throw error(std::string("Incorrect item slot ")+toString(a), "inventory::equip");
	if(lookup.at(a)>=eqoff2)return unequip(a);
	int e,f,c1,c3,job,job2;

	e=itemek[a].GetI()->eqslot;
	if((e==13)&&((player->abletomove>0)||(player->getstun()>player->cl->ido)))return false;
	if(e>=0)
	{
		job=player->job;
		if(job>5)job2=(job-4)/2; else job2=job;

		if((player->gender!=itemek[a].GetI()->gender)&&(itemek[a].GetI()->gender!=-1))
		{
			player->bs->cmd(playerid, 0x95);
			*player->bs << 622;
			player->bs->sndpstr(itemek[a].GetI()->name);
			return false;
		}

		if((player->level<itemek[a].GetI()->level)&&(e!=13))
		{
			player->bs->cmd(playerid, 0x95);
			*player->bs << 999;
			player->bs->sndpstr(toString(itemek[a].GetI()->level));
			return false;
		}

		if((itemek[a].GetI()->job!=-1)&&(job!=itemek[a].GetI()->job)&&(job2!=itemek[a].GetI()->job)&&(itemek[a].GetI()->job!=0))
		{
			player->bs->cmd(playerid, 0x95);
			*player->bs << 621;
			player->bs->sndpstr(itemek[a].GetI()->name);
			return false;
		}

		if((mask.at(e+eqoff2)!=-1)&&(e!=10)&&(itemek[a].GetI()->eqslot2!=-1))
			if(mask.at(itemek[a].GetI()->eqslot2+eqoff2)==-1)
				e=itemek[a].GetI()->eqslot2;

		c1=lookup[a];
		f=e+eqoff2;


		if(itemek[a].getId()==2028)
		{
			if(mask.at(eqoff2+10)==-1)return false;
			if(itemek.at(mask.at(eqoff2+10)).GetI()->weapontype!=21)return false;
		}

		if((e==10)&&(itemek[a].GetI()->handed==1)&&(mask.at(f)!=-1)/*&&(mask[f+1]==-1)*/&&(job==7))
			if(itemek.at(mask.at(f)).GetI()->handed==1)
			{
				e--;
				f--;
			}

		if((e==10)&&((itemek[a].GetI()->handed==2)))
		{
			if(mask.at(eqoff2+11)!=-1)
				if(!unequip(mask.at(eqoff2+11)))
					return false;
			if(mask.at(eqoff2+9)!=-1)
				if(!unequip(mask.at(eqoff2+9)))
					return false;
		}else if(e==11)
		{
			if(mask.at(eqoff2+10)!=-1)
				if(itemek.at(mask.at(eqoff2+10)).GetI()->handed==2)
					if(!unequip(mask.at(eqoff2+10)))
						return false;
			if(mask.at(eqoff2+9)!=-1)
				if(!unequip(mask.at(eqoff2+9)))
					return false;
		}
		else if(e==9)
		{
			if(mask.at(eqoff2+11)!=-1)
				if(!unequip(mask.at(eqoff2+11)))
					return false;
			if(mask.at(eqoff2+10)!=-1)
				if(itemek.at(mask.at(eqoff2+10)).GetI()->handed==2)
					if(!unequip(mask.at(eqoff2+10)))
						return false;
		}
		if(mask.at(f)!=-1)
			if(!unequip2(mask[f]))
				return false;

		itemek.at(a).addbonus(player);

		if(mask[c1]!=-1)lookup.at(mask[c1])=-1;
		if(mask[f]!=-1)lookup.at(mask[f])=-1;

		c3=mask[c1];
		mask[c1]=mask[f];
		mask[f]=c3;


		if(mask[f]!=-1)lookup[mask[f]]=f;
		else allocate(f);
		if(mask[c1]!=-1)lookup[mask[c1]]=c1;
		else allocate(c1);

		if(mask[f]!=-1)
		{
			if(itemek[mask[f]].GetI()!=0)
			{
				int k=itemek[mask[f]].GetI()->eqslot;
				if((k==2)||(k==4)||(k==5)||(k==6))checkset();
			}
		}

		inv_eq(*player->bs, playerid, a, e, &itemek[a], 1);
		inv_eq(*player->tomulticast2, playerid, e, e, &itemek[a], 1);
	};
	bool oldflying=player->flying;
	player->flying=(mask[eqoff2+13]!=-1);
	if(player->flying!=oldflying)
	{
		player->sitting=false;
		player->clearfollowers();
		player->stopmove();
		player->unsummonpet();
		player->onChangePosition();
	}
#ifdef _DEBUG
#ifdef _LOGALOT
	logger.log("Flying %d\n", (int)player->flying);
#endif
#endif
	player->HMF();

	return true;
}


void inventory::updateitem(int a)
{
	if((a<0)||(a>=nitems))throw error(std::string("Incorrect item slot ")+toString(a), "inventory::updateitem");
	if(player!=0)
	{
		player->bs->cmd(playerid, 3);
		*player->bs << (char)0 << -1;
		itemek[a].snddata(player->bs);
		*player->bs << (char)1 << (unsigned char)(a) << (short)itemek[a].num;
	}
}

void inventory::updateitemammount(int a)
{
	if((a<0)||(a>=nitems))throw error(std::string("Incorrect item slot ")+toString(a), "inventory::updateitemammount");
	if(player!=0)
	{
		player->bs->cmd(playerid, 0x18);
		*player->bs << (char)0 << (short)a << (short)itemek[a].num << (short)0;
	}
}

int inventory::getCapacity(item *targy)
{
	if(targy==0)throw error("Called with null item*", "inventory::getCapacity");
	int nstack=1,a, cap=0;
	int id=targy->getId();
	if(targy->GetI()!=0)nstack=targy->GetI()->num;

	if(nstack>1)
	{
		for(a=0;a<eqoff2;a++)
		{
			if(itemek[mask[a]].getId()==id)cap+=nstack-itemek[mask[a]].num;
			else if(itemek[mask[a]].getId()==-1)cap+=nstack;
		}
	}else
	{
		for(a=0;a<eqoff2;a++)
			if(itemek[mask[a]].getId()==-1)
				cap+=nstack;
	}
	return cap;
}

int inventory::getRemainder(item *targy, int cap)
{
	if(targy==0)throw error("Called with null item*", "inventory::getRemainder(item*,int)");
	int nstack=1,a,id=targy->getId();
	if(targy->GetI()!=0)nstack=targy->GetI()->num;

	if(nstack>1)
	{
		for(a=0;a<eqoff2;a++)
		{
			if(itemek[mask[a]].getId()==id)
			{
				cap-=nstack-itemek[mask[a]].num;
				if(cap<=0)return 0;
			}
			else if(itemek[mask[a]].getId()==-1)
			{
				cap-=nstack;
				if(cap<=0)return 0;
			}
		}
	}else
	{
		for(a=0;a<eqoff2;a++)
			if(itemek[mask[a]].getId()==-1)
			{
				cap-=nstack;
				if(cap<=0)return 0;
			}
	}
	return cap;
}

int inventory::getRemainder(item *targy)
{
	if(targy==0)throw error("Called with null item*", "inventory::getRemainder(item*)");
	int nstack=1,a,id=targy->getId();
	if(targy->GetI()!=0)nstack=targy->GetI()->num;
	int cap=targy->num;

	if(nstack>1)
	{
		for(a=0;a<eqoff2;a++)
		{
			if(itemek[mask[a]].getId()==id)
			{
				cap-=nstack-itemek[mask[a]].num;
				if(cap<=0)return 0;
			}
			else if(itemek[mask[a]].getId()==-1)
			{
				cap-=nstack;
				if(cap<=0)return 0;
			}
		}
	}else
	{
		for(a=0;a<eqoff2;a++)
			if(itemek[mask[a]].getId()==-1)
			{
				cap-=nstack;
				if(cap<=0)return 0;
			}
	}
	return cap;
}


int inventory::pickup(item *t, bool silent)
{
	if(t==0)throw error("Called with null item*", "inventory::pickup");
	int nstack=1,a,id;
	item targy;

	if(t==0)return 0;
	targy=*t;
	if(targy.GetI()!=0)nstack=targy.GetI()->num;
	id=targy.getId();

	if(nstack>1)
	{
		for(a=0;a<eqoff2;a++)
		{
			if(itemek[mask[a]].getId()==id)
			{
				if(itemek[mask[a]].num+targy.num<=nstack)
				{
					itemek[mask[a]].num+=targy.num;
					if(!silent)updateitemammount(mask[a]);
					return 0;
				}else{
					targy.num-=nstack-itemek[mask[a]].num;
					itemek[mask[a]].num=nstack;
					if(!silent)updateitemammount(mask[a]);
				}
			}
		}
	}
	if(targy.num>0)
		for(a=0;a<eqoff2;a++)
		{
			if(itemek[mask[a]].getId()==-1)
			{
				itemek[mask[a]]=targy;
				if(targy.num>nstack)
				{
					itemek[mask[a]].num=nstack;
					if(!silent)updateitem(mask[a]);
					targy.num-=nstack;
				}else{
					if(!silent)updateitem(mask[a]);
					return 0;
				}
			}
		}
	if(targy.num>0)
		if(player!=0)player->bs->cmd(playerid, 0x55);
	return targy.num;
}

void inventory::dump()
{
#ifdef _DEBUG
#ifdef _LOGALOT
	int a;
	for(a=0;a<nitems;a++)
	{
		logger.log("%d:%c%d%c%d%c%d\n", a, (char)9, mask[a],(char)9, itemek[a].getId(),char(9), lookup[a]);
	}
#endif
#endif
}


item* inventory::getItem(int a)
{
	item *retval=0;
	if((a<nitems)&&(a>=0))
	{
		retval=&itemek.at(a);
	}else throw error(std::string("Incorrect item slot ")+toString(a), "inventory::getItem");
	return retval;
}

item* inventory::getMaskedItem(int a)
{
	item *retval=0;
	if((a<nitems)&&(a>=0))a=mask.at(a);
	else throw error(std::string("Incorrect mask slot ")+toString(a), "inventory::getMaskedItem");
	if((a<nitems)&&(a>=0))retval=&itemek[a];
	return retval;
}


int inventory::getMask(int a)
{
	if((a<0)||(a>=nitems))throw error(std::string("Incorrect item slot ")+toString(a), "inventory::getMask");
	return mask.at(a);
}

int inventory::getLookup(int a)
{
	if((a<0)||(a>=nitems))throw error(std::string("Incorrect item slot ")+toString(a), "inventory::getMask");
	return lookup.at(a);
}

inventory::inventory(tplayer *ch):player(ch)
{
	mask.resize(nitems);
	itemek.resize(nitems);
	lookup.resize(nitems);
	fillmask();
	filllookup();
}
void inventory::init(tplayer *ch)
{
	player=ch;
}

inventory::inventory(const inventory &a)
{
//	int b;
	if(this!=&a)
	{
		mask=a.mask;
		itemek=a.itemek;
		lookup=a.lookup;

		player=a.player;
		playerid=a.playerid;
	}
}

void inventory::load(char *q)
{
	int a;
	if(q!=0)
		for(a=0;a<nitems;a++)
			itemek[a].load(&q[a*item::itemdatasize]);
	fillmask();
	filllookup();
}

void inventory::save(char *q)
{
	int a,b;
	item dummy;
	if(q!=0)
	{
//		for(a=0;a<nitems;a++)
//			*(int*)&q[a*item::itemdatasize]=-1;
		for(a=0;a<nitems;a++)
		{
			b=mask[a];
			if(b>=0)itemek[b].save(&q[a*item::itemdatasize]);
			else dummy.save(&q[a*item::itemdatasize]);
		}
	}
	
}

bool inventory::contains(int id)
{
	for(int a=0;a<eqoff2;a++)
	{
		if(mask[a]!=-1)
			if(itemek[mask[a]].getId()==id)
				return true;
	}
	return false;
}

bool inventory::decsuppliment(int s)
{
	int a=inventory::eqoff2+25;
	int f;
	if(mask[a]==-1)return false;
	f=mask[a];
	if((itemek[f].getId()!=s)||(itemek[f].num<1))return false;
	itemek[f].num--;
	updateitemammount(f);
	if(itemek[f].num>0)return true;
	if(!unequip2(f))
	{
//		lookup[f]=-1;
//		mask[a]=-1;
	}else
	{
		itemek[f].setId(-1);
	}
	int c1;
	for(c1=0;c1<eqoff2;c1++)
        if((mask[c1]!=-1)&&(itemek.at(mask.at(c1)).getId()==s))
		{
            break;
		}
	if(c1>=eqoff2)return true;
	equip(mask[c1]);
	return true;
}
