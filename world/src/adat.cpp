
#include "stdafx.h"
#include "main.h"
#include "logger.h"
#include "vector2.h"
#include <array>
#include <string>
#include "item.h"
#include "adat.h"
#include "mysql.h"
#include "buff.h"
#include "quest.h"
#include "io/file/csv.h"


using namespace std;

#define __GenDropsByItemLevel__

struct tmdrop
{
	int dropid;
	int mobid;
	tmdrop(int a,int b):dropid(a), mobid(b){}
};

std::vector2<tmdrop> mdrops;

std::vector2<int> nguildmembers;

//-----

std::vector2<int> skillsupliment;

std::vector2<itemdata> itemlist;

std::vector2<std::vector2<tskilldata> > skilllist;
std::vector2<tskilldata > cskilllist;
std::vector2<mobdata2> monsterlist;

void setskillelement(int skill, int e, float d=0.5)
{
	for(int c=0;c<20;c++)
	{
		skilllist[skill][c].element=e;
		skilllist[skill][c].elementval=(int)(c*d);
	}
}

template <typename Model> void fromCSV(const std::string &csvFile);

template<> void fromCSV<mobdata2::tdrop>(const std::string &csvFile);

void datainit()
{
	int a,b,c;

	nguildmembers.resize(50);
	for(a=0;a<20;a++)nguildmembers[a]=(a&0xfffffffe)+30;
	for(a=20;a<49;a++)nguildmembers[a]=a+29;
	nguildmembers[49]=50;

	skilllist.resize(character_buffable::maxskillid);
	skillsupliment.resize(character_buffable::maxskillid, 0);
	for(a=0;a<400;a++)skilllist[a].resize(20);
	monsterlist.resize(1000);

	itemdata id1;
	sqlquery s1(connections[0], string("itemlist"));
	s1.select("max(id)");
	if(s1.next())
	{
		b=toInt(s1[0]);
		itemlist.resize(b+1);
	}
	s1.freeup();

	s1.selectw("enabled!=0");
	int c_id=s1.getColumnIndex("id");
	int c_name=s1.getColumnIndex("name");
	int c_itemLV=s1.getColumnIndex("limitlevel1");
	int c_itemjob=s1.getColumnIndex("itemjob");
	int c_abilitymin=s1.getColumnIndex("abilitymin");
	int c_abilitymax=s1.getColumnIndex("abilitymax");
	int c_packmax=s1.getColumnIndex("packmax");
	int c_parts=s1.getColumnIndex("parts");
	int c_partsub=s1.getColumnIndex("partsub");
	int c_cost=s1.getColumnIndex("cost");
	int c_handed=s1.getColumnIndex("handed");
	int c_weapontype=s1.getColumnIndex("weapontype");
	int c_attackspeed=s1.getColumnIndex("attackspeed");
	int c_itemsex=s1.getColumnIndex("itemsex");
	int c_destparam1=s1.getColumnIndex("destparam1");
	int c_destparam2=s1.getColumnIndex("destparam2");
	int c_destparam3=s1.getColumnIndex("destparam3");
	int c_adjparamval1=s1.getColumnIndex("adjparamval1");
	int c_adjparamval2=s1.getColumnIndex("adjparamval2");
	int c_adjparamval3=s1.getColumnIndex("adjparamval3");
	int c_flightspeed=s1.getColumnIndex("flightspeed");
	int c_duration=s1.getColumnIndex("endurance");
	int c_sfx=s1.getColumnIndex("sfxobj3");
	int c_gendrop=s1.getColumnIndex("gendrop");
	logger.log("Item data\n");
	a=0;
	while(s1.next())
	{
		a++;
		id1.id=toInt(s1[c_id]);
		id1.name=s1[c_name];
		id1.name=(char)34 + id1.name + (char)34;
		id1.level=toInt(s1[c_itemLV]);
		if(id1.level>150)id1.level=1;
		id1.job=toInt(s1[c_itemjob]);
		id1.min=toInt(s1[c_abilitymin]);
		id1.max=toInt(s1[c_abilitymax]);
		id1.num=toInt(s1[c_packmax]);
		id1.eqslot=toInt(s1[c_parts]);
		id1.eqslot2=toInt(s1[c_partsub]);
		id1.cost=toInt(s1[c_cost]);
		id1.handed=toInt(s1[c_handed]);
		id1.weapontype=toInt(s1[c_weapontype]);
		id1.aspd=toFloat(s1[c_attackspeed]);
		id1.gender=toInt(s1[c_itemsex]);
		id1.duration=toInt(s1[c_duration]);
		id1.gendrop=toInt(s1[c_gendrop]);

		id1.destparam1=toInt(s1[c_destparam1]);
		id1.destparam2=toInt(s1[c_destparam2]);
		id1.destparam3=toInt(s1[c_destparam3]);
		id1.adjparamval1=toInt(s1[c_adjparamval1]);
		id1.adjparamval2=toInt(s1[c_adjparamval2]);
		id1.adjparamval3=toInt(s1[c_adjparamval3]);
		id1.flightspeed=toFloat(s1[c_flightspeed]);
		id1.sfx=toInt(s1[c_sfx]);
		id1.valid=true;
		itemlist[id1.id]=id1;
	}
	s1.freeup();
	logger.log("Skill data\n");

	itemlist[3204].destparam1=52;
	itemlist[3204].adjparamval1=2;
	itemlist[3207].destparam1=52;
	itemlist[3207].adjparamval1=4;
	itemlist[3208].destparam1=52;
	itemlist[3208].adjparamval1=7;

	itemlist[3209].destparam1=53;
	itemlist[3209].adjparamval1=2;
	itemlist[3212].destparam1=53;
	itemlist[3212].adjparamval1=4;
	itemlist[3213].destparam1=53;
	itemlist[3213].adjparamval1=7;

	itemlist[3214].destparam1=66;
	itemlist[3214].adjparamval1=2;
	itemlist[3217].destparam1=66;
	itemlist[3217].adjparamval1=4;
	itemlist[3218].destparam1=66;
	itemlist[3218].adjparamval1=7;

	itemlist[3219].destparam1=72;
	itemlist[3219].adjparamval1=2;
	itemlist[3222].destparam1=72;
	itemlist[3222].adjparamval1=4;
	itemlist[3223].destparam1=72;
	itemlist[3223].adjparamval1=7;

	itemlist[3224].destparam1=54;
	itemlist[3224].adjparamval1=3;
	itemlist[3227].destparam1=54;
	itemlist[3227].adjparamval1=5;
	itemlist[3228].destparam1=54;
	itemlist[3228].adjparamval1=10;

	itemlist[3229].destparam1=52;
	itemlist[3229].adjparamval1=1;

	itemlist[3240].destparam1=52;
	itemlist[3240].adjparamval1=3;

	itemlist[3241].destparam1=53;
	itemlist[3241].adjparamval1=1;

	itemlist[3242].destparam1=53;
	itemlist[3242].adjparamval1=3;

	itemlist[3237].destparam1=66;
	itemlist[3237].adjparamval1=1;

	itemlist[3238].destparam1=72;
	itemlist[3238].adjparamval1=1;

	itemlist[3243].destparam1=54;
	itemlist[3243].adjparamval1=1;

	itemlist[3244].destparam1=54;
	itemlist[3244].adjparamval1=3;

	itemlist[3245].destparam1=76;
	itemlist[3245].adjparamval1=100;

	itemlist[3246].destparam1=76;
	itemlist[3246].adjparamval1=400;






	itemlist[3251].destparam1=300;
	itemlist[3251].adjparamval1=20;

	itemlist[3252].destparam1=300;
	itemlist[3252].adjparamval1=40;

	itemlist[3253].destparam1=14;
	itemlist[3253].adjparamval1=2;

	itemlist[3254].destparam1=14;
	itemlist[3254].adjparamval1=5;




	itemlist[3521].destparam1=11;
	itemlist[3521].adjparamval1=200;
	itemlist[3522].destparam1=11;
	itemlist[3522].adjparamval1=500;


	itemlist[3523].destparam1=1;
	itemlist[3523].adjparamval1=1;
	itemlist[3524].destparam1=1;
	itemlist[3524].adjparamval1=2;
	itemlist[3525].destparam1=4;
	itemlist[3525].adjparamval1=1;
	itemlist[3526].destparam1=4;
	itemlist[3526].adjparamval1=2;
	itemlist[3527].destparam1=2;
	itemlist[3527].adjparamval1=1;
	itemlist[3528].destparam1=2;
	itemlist[3528].adjparamval1=2;
	itemlist[3529].destparam1=3;
	itemlist[3529].adjparamval1=1;
	itemlist[3530].destparam1=3;
	itemlist[3530].adjparamval1=2;
	itemlist[3523].destparam1=1;
	itemlist[3523].adjparamval1=1;
	itemlist[3524].destparam1=1;
	itemlist[3524].adjparamval1=2;
	itemlist[3531].destparam1=82;
	itemlist[3531].adjparamval1=1;
	itemlist[3532].destparam1=82;
	itemlist[3532].adjparamval1=2;

	sqlquery &s2=dbskilllist2;
	sqlquery s9(connections[0], "skilllist");
	s2.select();
	a=0;
	while(s2.next())
	{
		b=toInt(s2[1]);
		c=toInt(s2[2]);
		--c;
		skilllist[b][c].id=b;
		skilllist[b][c].level=c+1;

		s9.selectw("id="+toString(b), "exetarget, continuouspain, skillready, itemtype, referstat1, referstat2, spelltype, expertmax, refertarget1, refertarget2, refervalue1, refervalue2, itemjob, handed, weapontype");
		if(s9.next())
		{
			skilllist[b][c].skilltype=toInt(s9[0]);
			skilllist[b][c].dot=toInt(s9[1]);
			skilllist[b][c].cooldown=toInt(s9[2]);
			skilllist[b][c].element=0;//toInt(s9[3]);
			skilllist[b][c].element2=0;
			skilllist[b][c].elementval=c*25/toInt(s9[7]);
			skilllist[b][c].elementval2=c*25/toInt(s9[7]);
			skilllist[b][c].rstat1=toInt(s9[4]);
			skilllist[b][c].rstat2=toInt(s9[5]);
			if(skilllist[b][c].rstat1<0)skilllist[b][c].rstat1=0;
			if(skilllist[b][c].rstat2<0)skilllist[b][c].rstat2=0;
			skilllist[b][c].rval1=toInt(s9[10]);
			skilllist[b][c].rval2=toInt(s9[11]);
			skilllist[b][c].rtarget1=toInt(s9[8]);
			skilllist[b][c].rtarget2=toInt(s9[9]);
			skilllist[b][c].playerjob=toInt(s9[12]);
			skilllist[b][c].handed=toInt(s9[13]);
			skilllist[b][c].weapontype=toInt(s9[14]);

			{
				int a=toInt(s9[6]);
				skilllist[b][c].magic=(a>0);

				switch(a)
				{
				case 4:
					skilllist[b][c].element=3;
					break;
				case 5:
					skilllist[b][c].element=1;
					break;
				case 6:
					skilllist[b][c].element=4;	//wind
					break;
				case 7:
					skilllist[b][c].element=2;
					break;
				case 8:
					skilllist[b][c].element=5;	//earth
					break;
				case 11:
					skilllist[b][c].element=1;
					skilllist[b][c].element2=5;	//earth
					break;
				case 12:
					skilllist[b][c].element=4;	//wind
					skilllist[b][c].element2=3;	//
					break;
				case 13:
					skilllist[b][c].element=5;	//earth
					skilllist[b][c].element2=4;	//wind
					break;
				case 14:
					skilllist[b][c].element=5;	//earth
					skilllist[b][c].element2=2;
					break;
				}
			}

		}
		s9.freeup();

		skilllist[b][c].min=toInt(s2[3]);
		skilllist[b][c].max=toInt(s2[4]);
//		skilllist[b][c].val5=toInt(s2[5]);
//		skilllist[b][c].val6=toInt(s2[6]);
//		skilllist[b][c].val7=toInt(s2[7]);
//		skilllist[b][c].val8=toInt(s2[8]);
		skilllist[b][c].prob=toInt(s2[9]);
//		skilllist[b][c].val10=toInt(s2[10]);
//		skilllist[b][c].val11=toInt(s2[11]);
		skilllist[b][c].stat1=toInt(s2[12]);
		skilllist[b][c].stat2=toInt(s2[13]);
		skilllist[b][c].sval1=toInt(s2[14]);
		skilllist[b][c].sval2=toInt(s2[15]);
//		skilllist[b][c].val16=toInt(s2[16]);
//		skilllist[b][c].val17=toInt(s2[17]);
//		skilllist[b][c].val18=toInt(s2[18]);
//		skilllist[b][c].val19=toInt(s2[19]);
//		skilllist[b][c].val20=toInt(s2[20]);
//		skilllist[b][c].val21=toInt(s2[21]);
//		skilllist[b][c].val22=toInt(s2[22]);
//		skilllist[b][c].val23=toInt(s2[23]);
		skilllist[b][c].mpreq=toInt(s2[24]);
		skilllist[b][c].fpreq=toInt(s2[25]);
//		skilllist[b][c].val26=toInt(s2[26]);
		skilllist[b][c].val27=toInt(s2[27]);	//motion time?
		skilllist[b][c].range=toInt(s2[28]);
//		skilllist[b][c].val29=toInt(s2[29]);
//		skilllist[b][c].val30=toInt(s2[30]);
		skilllist[b][c].duration=toInt(s2[31]);
		if(skilllist[b][c].duration<0)skilllist[b][c].duration=0;
//		skilllist[b][c].val32=toInt(s2[32]);
//		skilllist[b][c].val33=toInt(s2[33]);
		skilllist[b][c].pxp=toInt(s2[34]);
		if(skilllist[b][c].pxp<1)skilllist[b][c].pxp=1;
		skilllist[b][c].astime=toInt(s2[35]);
		a++;
	}
	s2.freeup();

	b=139;for(a=0;a<skilllist[b].size();a++)skilllist[b][a].targetless_aoe=true;
	b=151;for(a=0;a<skilllist[b].size();a++)skilllist[b][a].targetless_aoe=true;
	b=158;for(a=0;a<skilllist[b].size();a++)skilllist[b][a].targetless_aoe=true;
	b=168;for(a=0;a<skilllist[b].size();a++)skilllist[b][a].targetless_aoe=true;
	b=173;for(a=0;a<skilllist[b].size();a++)skilllist[b][a].targetless_aoe=true;

	skillsupliment[156]=bpposter;
	skillsupliment[157]=bpposter;
	skillsupliment[158]=bpposter;
	skillsupliment[159]=bpposter;

	skillsupliment[148]=rmposter;
	skillsupliment[149]=rmposter;
	skillsupliment[150]=rmposter;
	skillsupliment[151]=rmposter;

	skillsupliment[194]=arrows;
	skillsupliment[198]=arrows;
	skillsupliment[202]=arrows;
	skillsupliment[196]=arrows;
	skillsupliment[200]=arrows;

	skillsupliment[215]=arrows;
	skillsupliment[216]=arrows;
	skillsupliment[217]=arrows;
	skillsupliment[218]=arrows;
	skillsupliment[219]=arrows;
	skillsupliment[222]=arrows;
	skillsupliment[220]=arrows;


	cskilllist.resize(character_buffable::camm);

	cskilllist[8].stat1=35;
	cskilllist[8].sval1=300;
	cskilllist[9].stat1=52;
	cskilllist[9].sval1=50;
	cskilllist[15].stat1=66;
	cskilllist[15].sval1=20;
	cskilllist[21].stat1=11;
	cskilllist[21].sval1=50;
	cskilllist[21].stat2=68;
	cskilllist[21].sval2=150;

	logger.log("Monster data\n");
	sqlquery &s3=dbmonsterlist;
	s3.select();
	int m_id=s3.getColumnIndex("id");
	int m_str=s3.getColumnIndex("str");
	int m_sta=s3.getColumnIndex("sta");
	int m_dex=s3.getColumnIndex("dex");
	int m_intl=s3.getColumnIndex("intl");
	int m_Level=s3.getColumnIndex("Level");
	int m_AtkMin=s3.getColumnIndex("AtkMin");
	int m_AtkMax=s3.getColumnIndex("AtkMax");
	int m_Size=s3.getColumnIndex("Size");
	int m_ElementalType=s3.getColumnIndex("ElementalType");
	int m_AddHp=s3.getColumnIndex("AddHp");
	int m_HR=s3.getColumnIndex("HR");
	int m_ER=s3.getColumnIndex("ER");
	int m_ExpValue=s3.getColumnIndex("ExpValue");
	int m_FxpValue=s3.getColumnIndex("FxpValue");
	int m_Flying=s3.getColumnIndex("Flying");
	int m_giant=s3.getColumnIndex("giant");
	int m_NeturealArmor=s3.getColumnIndex("NeturealArmor");
	int m_speed=s3.getColumnIndex("Speed");
	int m_atkd=s3.getColumnIndex("ReAttackDelay");
	int m_name=s3.getColumnIndex("name");
	int m_cash=s3.getColumnIndex("Cash");
	int m_relectric=s3.getColumnIndex("ResistElectricity");
	int m_rfire=s3.getColumnIndex("ResistFire");
	int m_rwind=s3.getColumnIndex("ResistWind");
	int m_rwater=s3.getColumnIndex("ResistWater");
	int m_rearth=s3.getColumnIndex("ResistEarth");
	a=0;
	while(s3.next())
	{
		b=toInt(s3[m_id]);
		while((int)monsterlist.size()<=b)monsterlist.push_back(mobdata2());
		monsterlist[b].id=b;
		monsterlist[b].name=s3[m_name];
		monsterlist[b].str=toInt(s3[m_str]);
		monsterlist[b].sta=toInt(s3[m_sta]);
		monsterlist[b].dex=toInt(s3[m_dex]);
		monsterlist[b].intl=toInt(s3[m_intl]);
		monsterlist[b].level=toInt(s3[m_Level]);
		monsterlist[b].atkmin=toInt(s3[m_AtkMin]);
		monsterlist[b].atkmax=toInt(s3[m_AtkMax]);
		monsterlist[b].size=toInt(s3[m_Size]);
		if(monsterlist[b].size<=0)monsterlist[b].size=100;
		monsterlist[b].element=toInt(s3[m_ElementalType]);
		monsterlist[b].hp=toInt(s3[m_AddHp]);
		monsterlist[b].hit=toInt(s3[m_HR]);
		monsterlist[b].flee=toInt(s3[m_ER]);
		monsterlist[b].exp=toInt(s3[m_ExpValue]);
		monsterlist[b].fxp=toInt(s3[m_FxpValue]);
		monsterlist[b].flying=toInt(s3[m_Flying]);
		monsterlist[b].giant=(toInt(s3[m_giant])!=0);
		monsterlist[b].def=toInt(s3[m_NeturealArmor]);
		monsterlist[b].speed=toFloat(s3[m_speed]);
		monsterlist[b].attackdelay=toInt(s3[m_atkd]);
		monsterlist[b].cash=toInt(s3[m_cash]);
		monsterlist[b].resistele[0]=toFloat(s3[m_rfire]);
		monsterlist[b].resistele[1]=toFloat(s3[m_rwater]);
		monsterlist[b].resistele[2]=toFloat(s3[m_relectric]);
		monsterlist[b].resistele[3]=toFloat(s3[m_rwind]);
		monsterlist[b].resistele[4]=toFloat(s3[m_rearth]);
		monsterlist[b].valid=true;
		a++;
	}
	s3.freeup();


	for(std::map<int, tquest*>::iterator questIter=tquest::quests.begin();questIter!=tquest::quests.end();++questIter)
	{
		for(std::map<int, tquest::tquestitems>::iterator questItemsIter=questIter->second->questitems.begin();questItemsIter!=questIter->second->questitems.end();++questItemsIter)
		{
			monsterlist.at(questItemsIter->second.monsterid)
                .quests.push_back( mobdata2::questdata(
                    questItemsIter->second.dropid,
                    questItemsIter->second.number,
                    questItemsIter->second.dropchance,
                    questIter->second) );
		}
		a=0;
		for(std::vector2<std::pair<int, int> >::iterator j=questIter->second->killmobs.begin();j!=questIter->second->killmobs.end();++j)
		{
			monsterlist.at(j->first).quest_killmobs.insert(std::pair<int, std::pair<int, int> >(questIter->second->questid, std::pair<int, int>(j->second, a)));
			a++;
		}
	}

	fromCSV<mobdata2::tdrop>("resources/drops.csv");

//	logger.log("%d %d\n", itemlist.size(), monsterlist.size());
	c=0;

#ifdef __GenDropsByItemLevel__
{
	logger.log("Drops\n");
	std::vector2<std::list<mobdata2*> > levelmobs;
	levelmobs.resize(explist.size()+1);
	for(a=0;a<(int)monsterlist.size();a++)
	{
		if(monsterlist[a].id>0)
		{
			if((monsterlist[a].level>0)&&(monsterlist[a].level<(int)explist.size()+1))
			{
				levelmobs[monsterlist[a].level].push_back(&monsterlist[a]);
			}
		}
	}

	for(c=0;c<(int)itemlist.size();c++)
	{
		if(itemlist[c].gendrop!=0)
		{
			float dch=0.1f;	//drop chance for regular
			if(c>=22000)dch=0.0001f;	//greens
			int d,e;
			float b;
			if(itemlist[c].gendrop==1)
			{
				d=itemlist[c].level-5;
				e=itemlist[c].level+5;
			}else if(itemlist[c].gendrop==2)
			{
				d=1;
				e=150;
			}
			if(d<1)d=1;
			if(e>(int)explist.size()+1)e=(int)explist.size()+1;
			for(;d<e;d++)
			{
				if(d<levelmobs.size())
				{
					for(std::list<mobdata2*>::iterator j=levelmobs[d].begin();j!=levelmobs[d].end();++j)
					{
						float b=dch;
						if((*j)->giant)b*=5.0f;	//giants
						(*j)->drops.push_back(mobdata2::tdrop(c, b, 1, 1));
					}
				}else break;
			}
		}
	}
}
#endif

#define dmd(a,b) mdrops.push_back(tmdrop(a, b));monsterlist[b].mdrop=a;monsterlist[b+1].mdrop=a;monsterlist[b+2].mdrop=a;monsterlist[b+3].mdrop=a;

	dmd(2950, 20);
	dmd(2951, 96);
	dmd(2952, 24);
	dmd(2953, 28);
	dmd(2954, 88);
	dmd(2955, 32);
	dmd(2956, 36);
	dmd(2957, 60);
	dmd(2958, 40);
	dmd(2959, 44);
	dmd(2960, 48);
	dmd(2969, 120);
	dmd(2962, 68);
	dmd(2961, 64);
	dmd(2975, 552);
	dmd(2976, 400);
	dmd(2963, 92);
	dmd(2977, 528);
	dmd(2978, 544);
	dmd(2964, 103);
	dmd(2968, 84);
	dmd(2972, 56);
	dmd(2965, 72);
	dmd(2966, 80);
	dmd(2979, 540);
	dmd(2973, 107);
	dmd(2980, 508);
	dmd(2981, 548);
	dmd(2982, 504);
	dmd(2996, 622);
	dmd(2971, 112);
	dmd(2983, 512);
	dmd(2997, 646);
	dmd(2984, 404);
	dmd(2985, 516);
	dmd(2998, 618);
	dmd(2986, 568);
	dmd(2987, 500);
	dmd(2999, 602);
	dmd(2988, 536);
	dmd(3000, 638);
	dmd(3001, 630);
	dmd(3002, 598);
	dmd(3003, 594);
	dmd(3004, 626);
	dmd(3005, 606);
	dmd(3006, 642);
	dmd(3007, 614);
	dmd(3008, 650);
	dmd(3009, 610);	//glaph
	dmd(3010, 634);

	dmd(2992,581);
	dmd(2993, 585);
	dmd(2994, 589);

	dmd(2967, 52);
	dmd(2991, 572);
	dmd(2990, 560);
	dmd(2989, 564);


	dmd(5995, 577);


	for(int a=0;a<(int)mdrops.size();a++)
	{
		itemlist[mdrops[a].dropid].mobid=mdrops[a].mobid;
//		monsterlist[mdrops[a].mobid].mdrop=mdrops[a].dropid;
	}
/*
	monsterlist[21].mdrop=2950;mdrops.push_back(tmdrop(2950, 21));
	monsterlist[24].mdrop=2951;
	mdrops.push_back(tmdrop(2951, 24));
	monsterlist[28].mdrop=2952;
	mdrops.push_back(tmdrop(2952, 28));
	monsterlist[32].mdrop=2953;
	mdrops.push_back(tmdrop(2953, 32));
	monsterlist[36].mdrop=2954;
	mdrops.push_back(tmdrop(2954, 36));
*/

#undef dmd
}

template<> void fromCSV<mobdata2::tdrop>(const std::string &csvFile) {
    io::CSVReader<7> reader("resources/drops.csv");
    reader.read_header(io::ignore_extra_column,
                       "mobid", "minlvl", "maxlvl", "itemid", "dropchance", "nmin", "nmax");

    int mobId, minLvl, maxLvl, itemId, min, max;
    float dropchance;

    while(reader.read_row(mobId, minLvl, maxLvl, itemId, dropchance, min, max))
    {
        const mobdata2::tdrop mobDropData(itemId, dropchance, min, max);
        if (mobId == -1)
        {
            for (mobdata2 &mob : monsterlist) {
                if ((mob.level >= minLvl) && (mob.level <= maxLvl))
                {
                    mob.drops.push_back(mobDropData);
                }
            }
        }
        else
        {
            monsterlist[mobId].drops.push_back(mobDropData);
        }
    }
}