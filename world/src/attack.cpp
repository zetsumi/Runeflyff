// DST_CHR_WEAEATKCHANGE DST_CHR_STEALHP
#include "stdafx.h"
#include "character_base.h"
#include "buff.h"
#include "monster.h"
#include "player.h"
#include "adat.h"
#include "grid.h"
#include "cluster.h"
#include "objactions.h"

#define GETA 8
#define GETD 0

void character_buffable::attack(int mot)
{
	buffer bs;
	attack(bs, mot);
	multicast(bs);
}

void character_buffable::attack(int mot, int szorzo)
{
	buffer bs;
	int a=atkmin,b=atkmax;
	if((szorzo>0)&&(szorzo<5))
	{
		float s=sqrt((float)szorzo);
		atkmin*=s;
		atkmax*=s;
	}
	attack(bs, mot);
	atkmin=a;
	atkmax=b;
	multicast(bs);
}

int iabs(int a)
{
	if(a<0)a=-a;
	return a;
}


void character_buffable::attack(buffer &bs, int motion, int basedmg, tskilldata *s)
{
	character_buffable *target=(character_buffable*)this->getfocus();
	if(target==0)return;
	if(((target->type!=ttplayer)&&(target->type!=ttmob))||(target->isDead))return;
	if((type==ttplayer)&&(target->type==ttmob))
		if(!((tplayer*)(this))->testLock((character_mob*)target))	//moblock
			return;

	if(target->type==ttplayer)
	{
		if(!((tplayer*)target)->ispkable(this))return;
	}


	int flag=1;
	bool ranged=((motion==0x23)||(motion==0x24));
	int dmg=getdmg(target, flag, basedmg, s, ranged);

	if(motion==0x23)bs.cmd(id, 0xe2) << motion << target->id << 0 << 0 << 0;
	else if(motion!=-1)bs.cmd(id, 0xe0) << motion << target->id << 0 << 0x00010000;

	damage(bs, target, dmg, flag, s, motion);
}

void character_buffable::aoeattack(buffer &bs, int basedmg, tskilldata *s, int duration, character_buffable *aoecenter)
{
	character_buffable *target;
	int flag,dmg;
	bool pk=cl->isPK();
	tgridcell *g=0;
	std::list<character_mob*>::iterator i1;
	std::list<tplayer*>::iterator i2;
	float distance=(float)s->range;
	distance*=distance;
	int a;
	std::list<character_buffable*> toattack;

try{
	for(a=0;a<9;a++)
	if(gridcell.getsz(a)!=0)
	{
		g=gridcell.getsz(a);
		g->setmobiterator(&i1);
		if(type==ttplayer)for(i1=g->mobs.begin();i1!=g->mobs.end();++i1)
		if(!(*i1)->isDead)if((*i1)->distance(*aoecenter)<=distance)
		{
			target=*i1;
			if((type==ttplayer)/*&&(target->type==ttmob)*/)
				if(!((tplayer*)(this))->testLock((character_mob*)target))
					continue;
			toattack.push_back(target);
			if(i1==g->mobs.end())break;
		}
		g->clearmobiterator();
		g->setplayeriterator(&i2);
		/*if((pk)||(type!=ttplayer))*/for(i2=g->players.begin();i2!=g->players.end();++i2)
		if(!(*i2)->isDead)if(((*i2)->distance(*aoecenter)<=distance)&&((*i2)->ispkable(this)))
		{
			target=*i2;
			toattack.push_back(target);
			if(i2==g->players.end())break; 
		}
		g->clearplayeriterator();
	}
}catch(...)
{
	if(g!=0)
	{
		g->clearmobiterator();
		g->clearplayeriterator();
	}
	throw;
}

	for(std::list<character_buffable*>::iterator i=toattack.begin();i!=toattack.end();++i)
	{
		target=*i;
		if(s->stat1>0)
		{
			if(((s->prob==-1)||((s->prob>0)&&(rnd(100)<=s->prob))))
				target->addbuff(id, s->id, s->level, duration);
		}else
		{
			if(s->dot>0)
			{
				target->addbuff(id, s->id, s->level, s->dot*2);
			}
		}
		if(s->min!=-1)
		{
			dmg=getdmg(target, flag, basedmg, s);
			damage(bs, target, dmg, flag, s);
		}
	}

}

void character_buffable::aoeattack(buffer &bs, int basedmg, float distance, character_buffable *aoecenter)
{
	character_buffable *target;
	int flag,dmg;
	bool pk=cl->isPK();
	tgridcell *g=0;
	std::list<character_mob*>::iterator i1;
	std::list<tplayer*>::iterator i2;
	distance*=distance;
	std::list<character_buffable*> toattack;
	int a;
try{
	for(a=0;a<9;a++)
	if(gridcell.getsz(a)!=0)
	{
		g=gridcell.getsz(a);
		g->setmobiterator(&i1);
		if(type==ttplayer)for(i1=g->mobs.begin();i1!=g->mobs.end();++i1)
		if(!(*i1)->isDead)if((*i1)->distance(*aoecenter)<=distance)
		{
			target=*i1;
			if((type==ttplayer)/*&&(target->type==ttmob)*/)
				if(!((tplayer*)(this))->testLock((character_mob*)target))
					continue;
			toattack.push_back(target);

			if(i1==g->mobs.end())break;
		}
		g->clearmobiterator();
		g->setplayeriterator(&i2);
		/*if((pk)||(type!=ttplayer))*/for(i2=g->players.begin();i2!=g->players.end();++i2)
		if(!(*i2)->isDead)if((*i2)->distance(*aoecenter)<=distance)
		{
			target=*i2;
			toattack.push_back(target);
			if(i2==g->players.end())break; 
		}
		g->clearplayeriterator();
	}
}catch(...)
{
	if(g!=0)
	{
		g->clearmobiterator();
		g->clearplayeriterator();
	}
	throw;
}

	for(std::list<character_buffable*>::iterator i=toattack.begin();i!=toattack.end();++i)
	{
		target=*i;
		dmg=getdmg(target, flag, basedmg);
		damage(bs, target, dmg, flag);
	}

}

int character_buffable::getdmg(character_buffable *target, int &flag, int basedmg, tskilldata *s, bool ranged)
{
//	int a;
	int atk,def,dmg;
	int block,crit, hit_l=this->hit;
//	bool ranged=false;
	int critdmg=100;
	flag=1;


	/*if(s==0)*/atk=atkmin+rnd(atkmax-atkmin);
//	else atk=stats[DST_CHR_DMG];

	if(type==ttmob)atk+=stats[DST_CHR_DMG];
	def=target->defmin+rnd(target->defmax-target->defmin);

	critdmg+=stats[DST_CRITICAL_BONUS];

	int s_dex=dex+stats[DST_DEX]+stats[DST_STAT_ALLUP];
	crit=s_dex;
	block=crit;
	if(job==9)block*=4;
	if(job==8)crit*=4;

	block/=10;
	crit/=10;

	if(!ranged)block+=stats[DST_BLOCK_MELEE];
	else block+=stats[DST_BLOCK_RANGE];
	crit+=stats.at(DST_CHR_CHANCECRITICAL);
	
//	if(type==ttplayer)
	{
//		hit=70+s_dex/10;
		double f;

		if((target->type==ttmob)&&(type==ttplayer))
			f=((dex * 1.6) / (dex + monsterlist[target->getmodelid()].flee)) * (level * 1.2 / (level + target->level)) * 150.0;
		else if((target->type==ttplayer)&&(type==ttmob))
			f=(level * 0.5) / (target->level * 0.3 + level) * ((hit * 1.5) / (target->dex * 0.5 + hit)) * 200.0;
		else if((target->type==ttplayer)&&(type==ttplayer))
			f=((dex * 1.6) / (dex + (10) * (level * 1.2 / (level + target->level)))) * 120.0;
		else f= rnd(100);

		hit_l=f;
	}

	hit_l+=stats.at(DST_ADJ_HITRATE) + stats.at(DST_DEFHITRATE_DOWN);
	if(hit_l<20)hit_l=20;
	else if(hit_l>96)hit_l=96;
//	if(block<20)block=20;
/*	else */if(block>96)block=96;
/*
	if(level-5>target->level)
	{
		a=level-target->level;
		a=100-a;
		if(a<1)a=1;
		def=def*a/100;
	}
*/

	if(s!=0)
	{
		if((s->id==212)||(s->id==159))	//hop, asal
		{
			def=0;
		}else if(s->id==137)	//armor penetrate
		{
			def/=2;
		}
	}

	if(s!=0)
	{
		if(s->magic)
		{
			atk+=atk*stats[DST_ADDMAGIC]/100;
			def=atk*target->stats[DST_RESIST_MAGIC]/100;
		}
	}

	if((target->type==ttplayer)&&(type==ttplayer))
	{
		/*if(((tplayer*)this)->duel==(tplayer*)target)*/atk=atk*2/3;
	}

	if(def<0)def=0;
	dmg=0;
	if(s!=0)if(s->id==113)dmg=atk+basedmg-def;	//charge
	
	if(modelid==164)dmg=atk+basedmg-def;
	
	if(dmg==0)
	{
		int a=target->level-level;
		if(a>0)
		{
			if(a>24)a=24;
			atk=atk*(100-a*4)/100;
		}else
		{
			a=-a;
			if(a>24)a=24;
			def=def*(100-a*4)/100;
		}
		dmg=(atk+basedmg)-def;
		
	}

	if(s==0)
	{
		if(rnd(100)<=hit_l)
		{
			flag=1;
			if(rnd(100)<=crit)
			{
				flag+=64;
				if(level>=target->level)critdmg+=100;
				else critdmg+=50;
				dmg=dmg*critdmg/100;
//				if(rnd(100)<20)flag += 0x10000000;
			}
		}else
		{
			flag=2;
			dmg=0;
		}
	
		if(rnd(100)<=block)
		{
			flag+=4096;
			dmg/=10;
		}
	}else
	{
		if(s->id==212)	//hop
		{
			dmg*=4;
			if(type==ttplayer)
			{
				if(((tplayer*)this)->money<dmg)dmg=((tplayer*)this)->money;
				((tplayer*)this)->altermoney(-dmg);
				setstat(((tplayer*)this)->bs, id, 79, ((tplayer*)this)->money);
			}
		}else if(s->id==159)	//asal
		{
/*
			float sz=1.4;
			if(type==ttplayer)
			{
				sz+=((tplayer*)this)->mp/200.0f;
				if(((tplayer*)this)->refresherhold<cl->ido)
				{
					((tplayer*)this)->mp=0;
					setstat(((tplayer*)this)->bs, id, 39, 0);
				}
			}
			dmg=(int)(dmg*sz);
*/
			if(type==ttplayer)
			{
				dmg+=(str+stats[DST_STR]+stats[DST_STAT_ALLUP])/10 * ((tplayer*)this)->mp;
				if(((tplayer*)this)->refresherhold<cl->ido)
				{
					((tplayer*)this)->mp=0;
					setstat(((tplayer*)this)->bs, id, 39, 0);
				}
			}
		}
	}

	if(target->holycross)
	{
		dmg*=2;
		target->holycross=false;
		target->removebuffl(145);
	}
/*
	if((s==0)||((s!=0)&&(s->id!=133)))	//charge
		if(target->level-5>level)
		{
			a=target->level-level;
			a=100-a;
			if(a<1)a=1;
			dmg=dmg*a/100;
		}
*/
	if(dmg<0)dmg=0;
	return dmg;
}

int eattack[7]={DST_MASTRY_WIND, DST_MASTRY_FIRE, DST_MASTRY_WATER, DST_MASTRY_ELECTRICITY, DST_MASTRY_EARTH, DST_MASTRY_WIND, DST_MASTRY_FIRE};
int edef[7]={DST_RESIST_WIND, DST_RESIST_FIRE, DST_RESIST_WATER, DST_RESIST_ELECTRICITY, DST_RESIST_EARTH, DST_RESIST_WIND, DST_RESIST_FIRE};

void character_buffable::damage(buffer &bs, character_buffable *target, int dmg, int flag, tskilldata *s, int motion)
{
	int a,b,c,d,a2;
	if(target->type==ttplayer)if(!((tplayer*)target)->ispkable(this))return;
	if(target->counter)
	{
		target->removebuff(204);
		character_base *f=target->getfocus();
		target->setfocus(this);
		a=0;
		if(target->type==ttplayer)
		{
			((tplayer*)target)->cancelskill();
			s=&skilllist[204][((tplayer*)target)->skilllevels[204]-1];
		}else s=0;
		target->attack(bs, 0x24, (target->str+target->stats[DST_STR]+target->stats[DST_STAT_ALLUP])*s->rval1, s);
//		target->attack(0x24);	//0x1f
		target->setfocus(f);
		return;
	}

	int e=0;

	if((s==0)||((s!=0)&&(s->id!=159)))
	{
		if(s!=0)
		{
			if(s->element!=0)stats[stlknl[s->element]]+=s->elementval;
			if(s->element2!=0)stats[stlknl[s->element2]]+=s->elementval2;
		}

		d=stats.at(DST_CHR_WEAEATKCHANGE);
		for(a=1;a<6;a++)
		{
			c=target->stats.at(edef[a]);
			if(c!=0)
			{
				b=stats.at(eattack[a+1]);
				if(b!=0)e+=b+d-c;
	
				b=stats.at(eattack[a-1]);
				if(b!=0)e-=b+d-c;
			}
		}

		if(s!=0)
		{
			if(s->element!=0)stats[stlknl[s->element]]-=s->elementval;
			if(s->element2!=0)stats[stlknl[s->element2]]-=s->elementval2;
		}

		dmg+=dmg*e/200;
	}
	if(dmg<0)dmg=0;

	b=1;
	int delay=0;
	if(s!=0)
	{
		switch(s->id)
		{
		case 2:		//flury
		case 194:	//junk arrow
		case 222:	//triple shot
			b=3;
			dmg/=3;
			delay=s->astime/3;
			break;
		case 4:		//splash something (1st merc skill)
			b=2;
			dmg/=2;
			delay+=s->astime/2;
		case 155:	//sonic hand
			b=10;
			dmg/=10;
			delay=0;
			break;
		default:
			delay+=s->astime;
		}
	}

	if(motion==0x24)
	{
		float f98=(pos-target->getpos()).length();
//		logger.log("::%f\n", f98);
		delay=(int)(f98*200);
	}

	buffer *bs3;
	int cruciomult;
	for(a=0;(a<b)&&(!target->isDead);a++)
	{
		cruciomult=1;
		bs3=new buffer;
		if(b>1)dmg+=rnd(11)+5;
		a2=target->hp;
		int trf=target->stats.at(DST_REFLECT_DAMAGE);
		target->alterhp(-dmg, this);
		if(!target->isDead)
		{
			bs3->cmd(target->id, 0x13);
			*bs3 << id << dmg << flag;
			if((flag&0x10000000)!=0)
			{
				vector3d<> v=target->pos-pos;
				if(v.sqrlength()<=0)v.set(0,0,1);
				else v/=v.length();
				v*=10;
				v+=target->pos;
				*bs3 << v.x << v.y << v.z;
				target->pos=v;
				target->stopmove();
			}
			if((target->type==ttmob)&&(target->getfocus()==0))target->setfocus(this);
		}else
		{
			bs3->cmd(target->id, 0xc7);
			*bs3 << id << a2;
		}
		managekarma(target);

		if(s!=0)
			if(s->id==112)
				alterhp((int)(dmg*s->sval1*0.01f), target);	//bloody strike

		if(stats.at(DST_CHR_CHANCESTEALHP)>0)
			if(stats[DST_CHR_CHANCESTEALHP]>rnd(100))
				alterhp(dmg*stats[DST_CHR_CHANCESTEALHP]/100, target);

		if(target->isDead)
		{
			target->setfocus(0);
			target->koer=this;
			if(type==ttplayer)if(((tplayer*)this)->party!=0)target->koer=((tplayer*)this)->party;
			cruciomult=6;
		}else
		{
			if(stats[DST_CHR_CHANCEPOISON]>0)
				if(stats[DST_CHR_CHANCEPOISON]>rnd(100))
					target->addbuff(id, 226, 10, 10000);
			if(stats[DST_CHR_CHANCESTUN]>0)
				if(stats[DST_CHR_CHANCESTUN]>rnd(100))
					target->addbuff(id, 227, 1, 3000);
			if(stats[DST_CHR_CHANCEBLEEDING]>0)
				if(stats[DST_CHR_CHANCEBLEEDING]>rnd(100))
					target->addbuff(id, 228, 10, 10000);
		}
		{
			if(trf>0)
			{
				dmg=dmg*trf/100;
				dmg*=cruciomult;
				a2=hp;
				alterhp(-dmg, target);
				if(!isDead)
				{
					bs3->cmd(id, 0x13);
					*bs3 << target->id << dmg << 1;
				}else
				{
					bs3->cmd(id, 0xc7);
					*bs3 << target->id << a2;
				}
				if(!target->isDead)
				{
					target->managekarma(this);
		
					if(target->stats[DST_CHR_CHANCESTEALHP]>0)
						if(target->stats[DST_CHR_CHANCESTEALHP]>rnd(100))
							target->alterhp(dmg*target->stats[DST_CHR_CHANCESTEALHP]/100, this);
				}

				if(isDead)
				{
//					bs.cmd(id, 0xc7);
//					bs << target->id << dmg;
					setfocus(0);
					koer=target;
					if(target->type==ttplayer)if(((tplayer*)target)->party!=0)koer=((tplayer*)target)->party;
				}
			}

		}

		adddelayed(bs3, cl->ido+100+a*delay);
	}
	if((!isDead)&&(target->isDead))
		if((type==ttplayer)&&(target->type==ttmob))
		{
			((tplayer*)this)->manageexp(monsterlist[target->modelid].exp, monsterlist[target->modelid].level);
			((character_mob*)target)->act();
		}
	if((!target->isDead)&&(isDead))
		if((target->type==ttplayer)&&(type==ttmob))
		{
			((tplayer*)target)->manageexp(monsterlist[modelid].exp, monsterlist[modelid].level);
			((character_mob*)this)->act();
		}

}


bool character_buffable::doskill(int skill, int level, int target, int as, bool checkcooldown, character_buffable *focus)
{
	//	character_buffable *focus=(character_buffable*)getfocus();
	if(focus==0)focus=this;
	if((silenced)||(berserk)||(stun>=cl->ido)||((focus->isDead&&(skill!=45))))
	{
		if(type==ttplayer)((tplayer*)this)->cancelskill();
		return false;
	}
	else if((focus->type!=ttplayer)&&(focus->type!=ttmob))focus=this;

	if((skill==206)&&(!di))
	{
		if(type==ttplayer)((tplayer*)this)->cancelskill();
		return false;
	}
	int c,e,n;
	buffer bs2;

	tskilldata *s=&skilllist.at(skill).at(level-1);
	if(checkcooldown)
	{
		if((type==ttplayer)&&(s->cooldown>0))
		{
			if(((tplayer*)this)->cooldowns.at(skill)>cl->ido)
			{
				((tplayer*)this)->cancelskill();
				return false;
			}
			else
			{
				((tplayer*)this)->cooldowns.at(skill)=cl->ido+s->cooldown;
			}
		}
	}
	int statdmg=0;
	int duration=s->duration;

	std::vector2<int> pstats(5);
	pstats[0]=0;
	pstats[1]=this->str+stats.at(DST_STR)+stats.at(DST_STAT_ALLUP);
	pstats[2]=this->dex+stats.at(DST_DEX)+stats.at(DST_STAT_ALLUP);
	pstats[3]=this->intl+stats.at(DST_INT)+stats.at(DST_STAT_ALLUP);
	pstats[4]=this->sta+stats.at(DST_STA)+stats.at(DST_STAT_ALLUP);

	if(skill==45)
	{
		//res
		if(focus->type!=ttplayer)return false;
		((tplayer*)focus)->bs->cmd(focus->id, 0x27);
		((tplayer*)focus)->setreslevel(level);
		skilleffect(&bs2, id, focus->id, skill, level, 0);
		multicast(bs2);
		return true;
	}
	else if(skill==44)
	{
		//heal
		if((type==ttplayer)&&(focus->type!=ttplayer))focus=this;
		c=s->sval1+pstats[3]*2.7f;
		focus->alterhp(c);
		skilleffect(&bs2, id, focus->id, skill, level, 0);
		addstat(&bs2, focus->id, 38, c);
		multicast(bs2);
		return true;
	}else if(skill==149)
	{
		//TODO removes harmfull skills from target here;
		skilleffect(&bs2, id, focus->id, skill, level, 0);
		multicast(bs2);
		return true;
	}else if(skill==163)
	{
		skilleffect(&bs2, id, focus->id, skill, level, 0);
		multicast(bs2);
		if(type==ttplayer)new character_obj(29, cl, mapid, pos.x, pos.y, pos.z, getdir(focus->pos-pos), 100, new objpsywall((tplayer*)this));
		return true;
	}
	
	std::list<tplayer*> forparty;
	tplayer *pp;
	character_buffable *aoecenter=this;

	if(s->rstat1>0)
	{
		statdmg=s->min+rnd(s->max-s->min);
		statdmg*=(int)(pstats.at(s->rstat1)/9/**s->rval1*/);
		
//		statdmg=statdmg*(stats[DST_ATKPOWER_RATE]+100)/100;
	}
//	statdmg=(int)(pstats.at(s->rstat1)*7.5f)+(int)(pstats.at(s->rstat2)*7.5f)+s->min+rnd(s->max-s->min);

	if(type==ttplayer)
	{
		pp=(tplayer*)this;
		if(s->weapontype!=0)
		{
			if((s->weapontype&sweapontype)==0)
			{
				pp->bs->cmd(id, 0x94)<<632;
				pp->cancelskill();
				return false;
			}
			if(((s->weapontype&wb_weapon)!=0))pp->AddWeaponSkill(skill);
			if(((s->weapontype&wb_shield)!=0))pp->AddShieldSkill(skill);
		}
	}

	if(s->targetless_aoe)focus=this;
	switch(s->skilltype)
	{
	case 2:		//stonehand
	case 1:		//selfbuff	(target self only)
		focus=this;
	case 9:		//buff(/heal)	(any target)
//		if((focus->type==ttmob)||(focus->type==ttplayer))
		if((skill!=238)&&(s->skilltype==9))
		{
			if((focus->type==ttmob)&&(type==ttplayer))focus=this;
		}
		if((job==3)||(job==10)||(job==11)||(job==4)||(job==12)||(job==13))
			duration+=pstats[3]*2000;
		break;

	case 13:	//aoeheal/gt (party)
		if(type==ttplayer)
		{
			pp=((tplayer*)this);
			if((job==3)||(job==10)||(job==11))duration+=pstats[3]*2000;
			if(pp->party!=0)
			{
				pp->party->getpplaround(pp, forparty);
			}else
			{
				if(type==ttplayer)((tplayer*)this)->cancelskill();
				return false;
			}
			if(skill==150)
			{
				e=362;
				n=0;
				for(std::list<tplayer*>::iterator i=forparty.begin();i!=forparty.end();++i)
				{
					if(((*i)!=pp)&&(distanceny(*(*i))<s->range*s->range))
					{
						skilleffect(&bs2, id, (*i)->id, skill, level, 0);
						(*i)->addbuff(id, skill, level, duration);
						effect(&bs2, (*i)->id, e);
						n++;
					}
				}
				if((n==0)&&(type==ttplayer))
				{
					((tplayer*)this)->cancelskill();
					return false;
				}
			}else
			{
				if(skill==144)
				{
					e=356;	//heal rain
					c=s->sval1+pstats[3]*4.5f;
				}
				else if(skill==51)
				{
					e=280;	//circle healing;
					c=s->sval1+pstats[3]*2.7f;
				}
				n=0;
				for(std::list<tplayer*>::iterator i=forparty.begin();i!=forparty.end();++i)
					if(distanceny(*(*i))<s->range*s->range)
					{
						n++;
						skilleffect(&bs2, id, (*i)->id, skill, level);
						(*i)->alterhp(c);
						addstat(&bs2, (*i)->id, 38, c);
						effect(&bs2, (*i)->id, e);
					}
				if((n==0)&&(type==ttplayer))
				{
					((tplayer*)this)->cancelskill();
					return false;
				}

			}
		}
		break;

	case 4:
		break;
	case 14:	
		aoecenter=focus;
		if(aoecenter==0)aoecenter=this;
		break;
	case 17:
		break;
	case 8:		//res, status magic	(target cannot be self)
		if(focus==this)
		{
			if(type==ttplayer)((tplayer*)this)->cancelskill();
			return false;
		}
		if((job==4)||(job==12)||(job==13))
			duration+=pstats[3]*2000;

		break;
//	case 7:		//blinkpool	(targetless, handled elsewhere)
	}

	if(s->skilltype!=13)
	{
		skilleffect(&bs2, id, focus->id, skill, level, 0);
		if(s->range>0)
		{	//aoe
			if(focus!=0)aoecenter=focus;
			aoeattack(bs2, statdmg, s, duration, aoecenter);
		}else
		{	//1v1
			if(s->stat1>0)
			{
				if(((s->prob<=0)||((s->prob>0)&&(rnd(100)<=s->prob))))
					focus->addbuff(id, skill, level, duration);
			}
			else
			{
				if(s->dot>0)
				{
					focus->addbuff(id, skill, level, s->dot*2);
				}
			}
			if((s->min!=-1)&&(s->id!=204))attack(bs2, -1, statdmg, s);
		}
		if(skill==206)	//pulling
		{
//			domove(); 			
			bs2.cmd(focus->getId(), 0x5f) << pos.x << pos.y << pos.z;
			bs2 << 0 << 0 << 0;
			bs2.cmd(focus->getId(), 0x41) << pos.x << pos.y << pos.z << -1;
//			focus->stopmove();
			focus->setlastpos();
			focus->pos=pos;
//			focus->checkmove();
//			onChangePosition();
//			focus->altermove(vector3d<>(x,y,z));
		}else if(skill==70)	//strongwind
		{
//			focus->stopmove();
			focus->setlastpos();
			vector3d<> v=focus->getpos()-pos;
			v/=v.length();
			v*=5;
			focus->pos+=v;
//			focus->checkmove();
//			focus->onChangePosition();
		}
	}

	multicast(bs2);
	return true;
}

