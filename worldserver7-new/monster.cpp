#include "stdafx.h"
#include "monster.h"
#include "cluster.h"
#include "adat.h"
#include "funkciok.h"
#include "stringf.h"
#include "quest.h"

tattacker basic_attack;

character_mob::character_mob(int t, cluster *c, int m, float x1, float y1, float z1, bool ag, float s1, mobspawn *ms, tattacker &at)
:agro(ag),kotime(0),actstate(mswait),mobs(ms),registeredAttack(false),endval(0),attacker(at)
, lastattack(0), attacktrate(2000),onwayback(false)
{
	type=ttmob;
	pos.x=x1;
	pos.y=y1;
	pos.z=z1;
	lastpos=pos;
	mapid=m;
	cl=c;
	id=cl->getmobid(this);
	modelid=t;
	modeltype=5;
	createremovepacket();

	int a=0;
	if(agro)a=1;
	hp=monsterlist.at(modelid).hp;
	attacktrate=monsterlist[modelid].attackdelay;
	size=monsterlist[modelid].size;
	if(size<1)size=100;
	size=(int)(size*s1);
	if(size<1)size=1;
	speed=monsterlist[modelid].speed*60.0f;
	basespeed=speed;
	str=monsterlist[modelid].str;
	sta=monsterlist[modelid].sta;
	dex=monsterlist[modelid].dex;
	intl=monsterlist[modelid].intl;
	atkmin=monsterlist[modelid].atkmin;
	atkmax=monsterlist[modelid].atkmax;
	defmin=monsterlist[modelid].def;
	defmax=monsterlist[modelid].def;
	level=monsterlist[modelid].level;
	hit=monsterlist[modelid].hit;
	flying=(monsterlist[modelid].flying!=0);
/*
	for(a=0;a<5;a++)
	{
		if(monsterlist[modelid].resistele[a]!=0.0f)stats.at(dstlknl[a+1])+=(int)(25*monsterlist[modelid].resistele[a]);
	}
*/

	if(monsterlist.at(modelid).element>0)
	{
		stats.at(stlknl[monsterlist.at(modelid).element])+=20;	//max:25
		stats.at(dstlknl[monsterlist.at(modelid).element])-=20;
	}

//	if(speed<1.0f)speed=1.0f;

	maxhp=hp;
	sp=new buffer;
	sp->cmd(id, 0xf0);
	*sp << (char)modeltype << modelid << (char)modeltype << (short)(modelid) << (short)size;
	bsx=sp->insert(pos.x);
	bsy=sp->insert(pos.y);
	bsz=sp->insert(pos.z);
	bsdir=sp->insert((short)dir);
	*sp<< id;
	*sp << (short)0 << (char)0;	//1 0
	bshp=sp->insert(hp);
	bsstate=sp->insert(state);
	bsstate2=sp->insert(state2);
	*sp << (char)0;	//0x0b

	*sp << 0 << (short)0 << (char)0 << (char)a << (short)0 << 0 << 0 << 0;

//	gridcell=0;
//	gridcell=cl->grid.get(mapid, (int)pos.x, (int)pos.z);
//	gridcell->addmob(this);
	gridcell.add();

	cl->addtomobact(this, mobremover);
	binit();
	if(agro)
	{
		cl->addtoagromob(this, agromobremover);
		agrocheck();
		act();
	}
	if((modelid>=556)&&(modelid<=569))staytime=60000;
	else staytime=10000;

}
buffer& character_mob::spawn()
{
	if((!isMoveing())&&(abletomove==0))
	{
		return *sp;
	}else
	{
		delete sp9;
		sp9=new buffer(*sp);
		if(isMoveing())getmovepacket(sp9);
		if(abletomove>0)
		{
			addstat(sp9, id, 11, 0);
			addstat(sp9, id, 64, 4096);
		}
		return *sp9;
	}
}

void character_mob::removethis()
{
	bdeinit();
	if(cl!=0)cl->releasemobid(id);
	if(gridcell.ggc()!=0)gridcell.remove();
	if(mobs!=0)mobs->unregister(this);
}

void character_mob::onKo(character_buffable *koer)
{
	bool fc=false,gb=false;
	*bsstate=0x08000000;
	if(playerLock!=0)
	{
		if(playerLock->party!=0)
		{
			fc=(playerLock->party->getfc()>cl->ido);
			gb=(playerLock->party->getgb()>cl->ido);
		}
	}
	drop(playerLock, fc, gb);
}

void character_mob::arrived()
{
	onwayback=false;
}

void character_mob::onHpChanged()
{
	*bshp=hp;
	if((hp<maxhp)&&(actstate==mswait))
	{
		if((getfocus()==0)&&(this->playerLock!=0))setfocus(playerLock);
		act();
	}
}

bool character_mob::act()
{
	bool rv=false;
	bool lonwayback=false;
	if(getfollowed()!=getfocus())
	{
		setfollowed(getfocus(), md);
	}

	buffer bs;
	switch(actstate)
	{
	case mswait:
		if(getfocus()==0)
		{
			if(mobs!=0)lonwayback=((pos.x<mobs->x-mobs->rx)||(pos.x>mobs->x+mobs->rx)||(pos.z<mobs->z-mobs->ry)||(pos.z>mobs->z+mobs->ry));
			if(hp<=0)actstate=mskoed;
			else if((!isMoveing())&&((gridcell.playersaround()>0)||(lonwayback&&!onwayback))&&(this->stun<cl->ido))
			{
				vector3d<> v;
				v.y=pos.y;
				if(mobs!=0)
				{
					v.x=mobs->x+rnd(mobs->rx*2)-mobs->rx;
					v.z=mobs->z+rnd(mobs->ry*2)-mobs->ry;
				}else
				{
					v.x=pos.x+rnd(10)-5;
					v.z=pos.z+rnd(10)-5;
				}
				if((v-pos).sqrlength()>15*15)
				{
					v-=pos;
					v/=v.length();
					v*=15.0f;
					v+=pos;
				}

				{
					domove();
					bs.cmd(id, 0x5f) << pos.x << pos.y << pos.z << 0 << 0 << 0;
					moveto(v.x,v.y,v.z);
					gotoxyz(&bs, id, v.x, v.y, v.z);
					multicast(bs);
				}
			}
			onwayback=lonwayback;
			break;
		}
	case msattack:
		if(hp<=0)actstate=mskoed;
		else if(!registeredAttack)
		{
			stopmove();
			actstate=msattack;
			registeredAttack=true;
			cl->addtomobattack(this, mobattackremover);
			mobattack();
		}
		break;
	case mskoed:
			/*if(endval==0)*/endval=1;
			setfocus(0);
//			kotime=cl->ido;
//			kotime+=staytime;

//			drop();
			actstate=msdropped;
//		break;
	case msdropped:		
			if(/*(cl->ido>kotime)&&*/((endval>=2)||(!agro)))
			{
				rv=true;
			}
		break;
	};

	return rv;
}

bool character_mob::agrocheck()
{
	if((hp>0)&&(this->stun<cl->ido)&&(!onwayback))
	{
		tplayer *m=0;
		float l=0,f;
		if((actstate==mswait)&&(gridcell.playersaround()>0))
		{
			if(gridcell.valid())for(std::list<tplayer*>::iterator i=gridcell->players.begin();i!=gridcell->players.end();++i)
			{
				if(((*i)->gethp()>0)&&(!(*i)->di)&&(!(*i)->gm_di)&&((*i)->flying==flying))
				{
					if(m==0)
					{
						l=distance(*(*i));
						m=*i;
					}else
					{
						f=distance(*(*i));
						if(f<l)
						{
							l=f;
							m=*i;
						}
					}
				}
			}
			if((m!=0)&&(l<15*15))
			{
				chat("!!");
				setfocus(m);
				actstate=msattack;
				buffer bs;
				bs.cmd(id, 0x5f) << pos.x << pos.y << pos.z << 0 << 0 << 0;
				multicast(bs);
				act();
			}
		}
	}
	if(endval!=0)endval++;
	return (endval!=0);
}

bool character_mob::mobattack()
{
	character_buffable *target=(character_buffable*)getfocus();
	if((target!=0)&&(hp>0))
	{
		if((target->type==ttplayer)&&(((tplayer*)target)->flying!=flying))
		{
			actstate=mswait;
			playerLock=0;
			partyLock=0;
		}else if(target->gethp()<=0)
		{
			setfocus(0);
			if(actstate==msattack)
			{
				actstate=mswait;
				playerLock=0;
				partyLock=0;
				act();
			}
		}else
		{
			if(this->stun>=cl->ido)return (actstate!=msattack);
			float dist=distanceny(*target);
			if((!isMoveing())&&(dist<=attacker.nextattackrange()))
			{
				if(lastattack+attacktrate-attacktrate*stats.at(DST_SPEED)*0.07<cl->ido)
				{
					lastattack=cl->ido;
					buffer bs;
					bs.cmd(id, 0x5f) << pos.x << pos.y << pos.z << 0 << 0 << 0;
					multicast(bs);
					attacker(this, target);//attack(0x1d);
				}else 
				{
					setfollowed(target, md);
					movefollow();
				}
			}else if(dist>45*45)
			{
				chat("!2");
				setfocus(0);
				setfollowed(0, 0);
				if(actstate==msattack)
				{
					playerLock=0;
					partyLock=0;
					actstate=mswait;
					act();
				}

			}else 
			{
				setfollowed(target, md);
				movefollow();
			}
		}
	}else if(actstate==msattack)
	{
		actstate=mswait;
		playerLock=0;
		partyLock=0;
		act();
	}
	registeredAttack&=(actstate==msattack);
	return (actstate!=msattack);
}

void character_mob::dropitem(item &targy, vector3d<> &p, tplayer *k, bool i)
{
	if((!targy.valid())||(targy.getId()==15))return;
	tparty *ppt=0;
	if(k!=0)ppt=k->party;
	if((i||flying)&&(k!=0))
	{
		int n=targy.num;
		int a=k->pickup(&targy);
		if(a==n)new character_item(targy, cl, mapid, p.x,p.y,p.z, k, 0, ppt);
	}else
	{
		new character_item(targy, cl, mapid, p.x,p.y,p.z, k);
	}
}

void character_mob::drop(character_buffable *koer, bool fc, bool gb)
{
	item penz(15);
	item targy;
	int num=1,a,b, s=1000*::dropmult;
	float s2=1000;
	tplayer *p=0;
	std::vector2<__int8> eq;
	eq.resize(73-42, 0);

	std::map<int, pquestdata>::iterator qi;
	std::map<int, tquest::tquestitems>::iterator qim;
	if(fc)num=2;
	if(gb)
	{
		s/=2;
		s2/=2;
	}

	if(modelid==164)koer=0;

	if(koer!=0)
	{
		float a=abs(level-koer->level);
		if(a>5)s2*=a-4;
	}

	if(koer!=0)if(koer->type==ttplayer)
	{
		p=(tplayer*)koer;
	}

	for(a=0;a<num;a++)
	{
		targy.upgrade=0;
		if(rnd(s)<600)
		{
			penz.num=(int)((monsterlist[modelid].cash*moneymultiplier)+rnd(10+moneymultiplier)-5);
			if(penz.num>0)dropitem(penz, pos, p);//new character_item(penz, cl, mapid, pos.x,pos.y,pos.z, koer);
		}

		for(std::list<mobdata2::tdrop>::iterator i=monsterlist[modelid].drops.begin();i!=monsterlist[modelid].drops.end();++i)
		{

			if(itemlist[i->id].eqslot>=0)
			{
				if(eq[itemlist[i->id].eqslot]>0)continue;
			}

//			float dropmultbox = 0.02;
//			if(!monsterlist[modelid].giant)
//				dropmultbox = dropmult;
			if(rnd((int)s2/dropmult)/10.0f<=i->dropchance)
			{
				targy.setId(i->id);
				if(targy.GetI()!=0)
				{
					targy.upgrade=0;
					targy.num=i->nmin+rnd(i->nmax-i->nmin);
						
					//check for the item+ 
					int seed = 10000;
					int modif = 1;
					if(fc)modif = 2;
					int eqq=0;
					if(targy.GetI()!=0)eqq = targy.GetI()->eqslot;
					else continue;
					if((((eqq>=4)&&(eqq<=6))||(eqq==9)||(eqq==10)||(eqq==2))&&(targy.getId()<10000))
					{
						int result = rnd(seed);
						if(result<=1*modif)
							targy.upgrade=10;
						else if(result<=2*modif)
							targy.upgrade=9;
						else if(result<=5*modif)
							targy.upgrade=8;
						else if(result<=10*modif)
							targy.upgrade=7;
						else if(result<=20*modif)
							targy.upgrade=6;
						else if(result<=50*modif)
							targy.upgrade=5;
						else if(result<=100*modif)
							targy.upgrade=4;
						else if(result<=200*modif)
							targy.upgrade=3;
						else if(result<=500*modif)
							targy.upgrade=2;
						else if(result<=1000*modif)
							targy.upgrade=1;
					}
					dropitem(targy, vector3d<>((rnd(20)-10)/10.0f+pos.x,pos.y,(rnd(20)-10)/10.0f+pos.z), p);
					if(itemlist[i->id].eqslot>=0)eq[itemlist[i->id].eqslot]++;
//					new character_item(targy, cl, mapid, (rnd(20)-10)/10.0f+pos.x,pos.y,(rnd(20)-10)/10.0f+pos.z, koer);
				}
			}
		}

		if(monsterlist.at(modelid).mdrop!=0)
		{
			if(rnd(s)<200)
			{
				targy.setId(monsterlist.at(modelid).mdrop);
				targy.num=1;
				dropitem(targy, pos, p);//new character_item(targy, cl, mapid, pos.x,pos.y,pos.z, koer);
			}
		}
	}
	if(p!=0)
	{
		if((!p->quests.empty())&&(!monsterlist[modelid].quests.empty()||!monsterlist[modelid].quest_killmobs.empty()))
		{
			for(qi=p->quests.begin();qi!=p->quests.end();++qi)
			{
				qim=qi->second.q->questitems.find(modelid);
				if(qim!=qi->second.q->questitems.end())
				{
					a=qim->second.dropchance;
					if(rnd(s)<a)
					{
						targy.setId(qim->second.dropid);
						targy.num=qim->second.number;
						dropitem(targy, pos, p, true);
						//new character_item(targy, cl, mapid, pos.x,0,pos.z, koer);
					}
				}
				std::map<int, std::pair<int,int> >::iterator k=monsterlist[modelid].quest_killmobs.find(qi->second.id);

				if(k!=monsterlist[modelid].quest_killmobs.end())
				{
					if(qi->second[k->second.second]<k->second.first)
					{
						qi->second.inc(k->second.second);
						p->bs->cmd(p->getId(), 0xb0) << qi->second.state << (short)qi->second.id << qi->second.n;
						p->save_quests=true;
					}
				}

			}
		}
	}
}

void character_mob::addstat1(int stat, int val, bool forcesub)
{
	int val2=-1;
	if(stat!=64)stats.at(stat)+=val;
	else
	{
		if(val>=0)stats.at(stat)|=val;
		else stats.at(stat)&=~(-val);
		if(val==1024)val2=0x7fffffff;
	}
	buffer bs1;
	if((val>=0)&&(!forcesub))addstat(&bs1, id, stat, val, val2);
	else substat(&bs1, id, stat, -val);
	multicast(bs1);
}
/*
tcwattack cwattack;

cw_caged::cw_caged(int t, cluster *cl, int m, float x1, float y1, float z1, bool a, mobspawn* ms, tplayer *gl)
	:character_mob(t, cl, m, x1, y1, z1, a, ms, cwattack),guild(gl->guild)
{
	begintime=cl->ido;
	guild->cwstart();
	cl->cwcageenter();
}

void cw_caged::onKo()
{
	long long endtime=cl->ido-begintime;
	guild->cwend();
	if(endtime<=1000*60*60)
	{
		staytime=120000;
		character_mob::onKo();
		guild->setcw(time(0));
	}else
	{
		*bsstate=0x08000000;
	}
}

void cw_caged::teleportout()
{
	guild->cwteleportout(cl);
	cl->cwcageleave();
}
*/