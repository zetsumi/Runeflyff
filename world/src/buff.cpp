#include "stdafx.h"
#include "buff.h"
#include "buffer.h"
#include "grid.h"
#include "adat.h"
#include "cluster.h"
#include "player.h"


void character_buffable::stathandle(int stat, int val, bool remove)
{
//	bool speedWasNotPositive;
	switch(stat)
	{
	case 11:
		if((type!=ttplayer)||((type==ttplayer)&&(!((tplayer*)this)->flying)))
		{
			if(val==0)
			{
				if(!remove)abletomove++;
				else abletomove--;
			}else
			{
				speed=basespeed+basespeed*stats.at(DST_SPEED)*0.01f;
				if(speed<0)speed=0;
			}
		}
		break;
	};
}

void character_buffable::handle64(int stat, int level, tbuff *b)
{
	int s=1;
	if(stat<0)
	{
		s=-1;
		stat=-stat;
	}
	buffer bs;
	switch(stat)
	{
	case 16384:
		holycross=(s==1);
		break;
	case 8:		//stun
		if(s==1)
		{
			domove();
			stopmove();
			stun=b->lejarat;
			bs.cmd(id, 0x59) << 1;
		}
		else 
		{
			stun=0;
			bs.cmd(id, 0x59) << 0;
		}
		multicast(bs);
		break;
	case 32768:
		b->dmg=12*level;
		alterdotdmg(s*b->dmg);
		break;
	case 2048:	//poision
		b->dmg=12*level;
		alterdotdmg(s*b->dmg);
		break;
//	case 4096:
//		domove();
//		abletomove+=s;
//		break;
	case 256:	//darkness.........?
		break;
	case 65536:
		silenced=(s==1);
		break;
	}
}

void character_buffable::removeeffect(tbuff *b, bool needlock)
{
	buffer bs;

	switch(b->id)
	{
//	case 120:
//		bs.cmd(id, 0x59) << 1;
//		break;
	case 143:
		berserk=false;
		break;
	case 145:
		holycross=false;
		break;
	case 65:	//hot air
	case 217:	//flame arrow
	case 200:
	case 151:	//merkaba
		alterdotdmg(-b->dmg);
		break;
	case 193:
		di=false;
		break;
	case 204:
		counter=false;
		break;
	}

	if(bs.getcommandnumber()>0)multicast(bs);

	tskilldata *s;
	if(b->id>=0)s=&skilllist.at(b->id).at(b->level-1);
	else s=&cskilllist.at(-b->id-1);

	if(s->stat1>0)
	{
		addstat1(s->stat1, -s->sval1, (s->stat1==11));
		stathandle(s->stat1, -s->sval1, true);
		if(s->stat1==64)handle64(-s->sval1, b->level, b);
	}
	if(s->stat2>0)
	{
		addstat1(s->stat2, -s->sval2, (s->stat2==11));
		stathandle(s->stat2, -s->sval2, true);
		if(s->stat2==64)handle64(-s->sval2, b->level, b);
	}
//		buffs.erase(b->id);
	if(type==ttplayer)
	{
//		ul mmm=needlock?((tplayer*)this)->playermutex.lock():pmutex::dontlock();
		((tplayer*)this)->HMF();
	}
}
void character_buffable::bdeinit()
{
	if(nbuffs>0)
	{
		while(!buffs.empty())
		{
			buffplace.at(buffs.front()->id+camm)=0;
			delete buffs.front();
			buffs.pop_front();
		}
		nbuffs=0;
		if(abr)cl->rembuffed(buffedremover);
		abr=false;
	}
	if(dotdmg>0)cl->remdot(dotremover);
	dotdmg=0;
}

void character_buffable::bremovefromcluster()
{
	if(abr)cl->rembuffed(buffedremover);
	abr=false;
	if(dotdmg>0)cl->remdot(dotremover);
	dotdmg=0;
}

void character_buffable::alterdotdmg(int a)
{
	if((dotdmg==0)&&(a!=0))cl->adddot(this, dotremover);
	dotdmg+=a;
}

bool character_buffable::dot()
{
	int a=dotdmg;
	if(a>hp-1)a=hp-1;
	if((a>0)&&(hp>0))
	{
		alterhp(-a);

		buffer bs;
		damage(bs, this, a, 1);
		multicast(bs);
	}
	return ((dotdmg==0)||(hp<1));
}

void character_buffable::addbuff(int bufferid, int skill, int skilllevel, int duration)
{
	tskilldata *s;
	bool addeffect=true;

	if(skill>=0)s=&skilllist.at(skill).at(skilllevel-1);
	else s=&cskilllist.at(-skill-1);

	if(nbuffs>0)
	{
		avg-=cl->ido-avgt;
		avgt=(double)cl->ido;

		if(buffplace.at(skill+camm)!=0)
		{
			tbuff *b=buffplace.at(skill+camm);
			if(b->level<=skilllevel)
			{
				addeffect&=(b->level<skilllevel);
				if(addeffect)removeeffect(b, false);
				avg*=nbuffs;
				avg-=b->lejarat-avgt;
//				avg-=b->lejarat-cl->ido;
				buffplace.at(skill+camm)=0;
				buffs.erase(b->i);
				delete b;
				nbuffs--;
				if(nbuffs>1)avg/=nbuffs;
/*				if(nbuffs<1)
				{
					if(abr)cl->rembuffed(buffedremover);
					abr=false;
				}
*/
			}else return;
		}
	}else 
	{
		cl->addbuffed(this, buffedremover);
		abr=true;
	}

	buffer bs;
	if(skill>=0)
	{
		bs.cmd(bufferid, 0x4c);
		bs << id << (short)1 << (short)skill << skilllevel << (int)duration;
	}else
	{
		buffer bs1;
		bs1.cmd(bufferid, 0x3f) << (int)-skill-1 << (int)duration/1000;
		selfcast(bs1);
	}

	tbuff *b=new tbuff(skill, skilllevel, duration, cl->ido+duration);
	insertbuff(b);
	if(skill<0)
	{
		switch(skill)
		{
		case -7:
			if(type==ttplayer)((tplayer*)this)->refresherhold=cl->ido+20*60000;
			break;
		case -8:
			if(type==ttplayer)((tplayer*)this)->vitalx=cl->ido+20*60000;
			break;
		case -11:
			if(type==ttplayer)((tplayer*)this)->activation=cl->ido+10*60000;
			break;
		}
	}

	if(addeffect)
	{
		if(s->stat1>0)
		{
//			if((s->stat1==11)&&(s->sval1==0))
//			{
//				s->stat1=64;
//				s->sval1=4096;
//			}
			addstat1(s->stat1, s->sval1);
			stathandle(s->stat1, s->sval1, false);
			if(s->stat1==64)handle64(s->sval1, skilllevel, b);
		}
		if(s->stat2>0)
		{
			addstat1(s->stat2, s->sval2);
			stathandle(s->stat2, s->sval2, false);
			if((s->stat2==64)/*&&(s->stat1!=11)*/)handle64(s->sval2, skilllevel, b);
		}
		if(type==ttplayer)((tplayer*)this)->HMF();
		switch(skill)
		{
/*
		case -7:
			if(type==ttplayer)((tplayer*)this)->refresherhold=cl->ido+20*60000;
			break;
		case -8:
			if(type==ttplayer)((tplayer*)this)->vitalx=cl->ido+20*60000;
			break;
		case -11:
			if(type==ttplayer)((tplayer*)this)->activation=cl->ido+10*60000;
			break;
*/
		case 48:
			protection=cl->ido+duration;
			protectionlevel=skilllevel;
			break;
//		case 120:
//			bs.cmd(id, 0x59) << 0;
//			break;
		case 143:
			berserk=true;
			break;
		case 145:
			holycross=true;
			break;
		case 200:	//arrow rain
		case 217:	//flame arrow
			b->dmg=(dex+stats.at(DST_DEX)+stats.at(DST_STAT_ALLUP))*level/10;
			alterdotdmg(b->dmg);
			break;
		case 65:	//hot air
			b->dmg=(intl+stats.at(DST_INT)+stats.at(DST_STAT_ALLUP))*level/20;
			alterdotdmg(b->dmg);
			break;
		case 151:	//merkaba
//			b->dmg=12*skilllevel;
			b->dmg=(intl+stats.at(DST_INT)+stats.at(DST_STAT_ALLUP))*level/10;
			alterdotdmg(b->dmg);
			break;
		case 193:
			di=true;
			removefocuses();
			break;
		case 204:
			counter=true;
			break;
		}
	}
	multicast(bs);
}

bool character_buffable::checkbuffs()
{
	tbuff *b;

	buffer bs;

	while(!buffs.empty())
	{
		if(buffs.front()->lejarat>cl->ido)break;
		b=buffs.front();
		if(b->id<0)bs.cmd(getId(), 0x3f) << -b->id-1 << 0;
		removeeffect(b, true);
		avg*=nbuffs;
		avg-=b->lejarat-avgt;
		buffplace.at(b->id+camm)=0;
		delete b;
		buffs.pop_front();
		nbuffs--;
		if(nbuffs>1)avg/=nbuffs;
	}
	if(bs.getcommandnumber()>0)selfcast(bs);

	abr=!buffs.empty();
	return !abr;
}

void character_buffable::removebuffl(int skill)
{
    std::unique_lock<std::mutex> guard(static_cast<tplayer*>(this)->playermutex, std::defer_lock);

    if (type == ttplayer) {
        guard.lock();
    }

    if (skill != 193 || !gm_di) {
        removebuff(skill);
    }
}

void character_buffable::removebuff(int skill)
{
	avg-=cl->ido-avgt;
	avgt=(double)cl->ido;
	
	if(buffplace.at(skill+camm)!=0)
	{
		tbuff *b=buffplace.at(skill+camm);

		buffer bs;
		if(skill>=0)
		{
			bs.cmd(getId(), 0xf8) << (short)1 << (short)skill;
		}else
		{
			buffer bs1;
			bs1.cmd(getId(), 0x3f) << -skill-1 << 0;
			selfcast(bs1);
		}
		multicast(bs);
		this->removeeffect(b, false);
		avg*=nbuffs;
		avg-=b->lejarat-avgt;

		buffplace.at(skill+camm)=0;

		buffs.erase(b->i);
		delete b;
		nbuffs--;
		if(nbuffs<1)
		{
			if(abr)cl->rembuffed(buffedremover);
			abr=false;
		}
		else avg/=nbuffs;
	}
}


void character_buffable::spawnbufficons(buffer *sp9)
{
	long long ido=cl->ido;
	std::list<tbuff*>::iterator bi;
	for(bi=buffs.begin();bi!=buffs.end();++bi)
	{
		if((*bi)->id>=0)
		{
			sp9->cmd(getId(), 0x4c);
			*sp9 << getId() << (short)1 << (short)(*bi)->id << (*bi)->level << (int)((*bi)->lejarat-ido);
		}else
		{
			sp9->cmd(getId(), 0x3f) << (int)-(*bi)->id-1 << (int)((*bi)->lejarat-ido)/1000;
		}
	}
}


void character_buffable::sendbdata(buffer *bs)
{
	if(nbuffs>0)
	{
		tskilldata *s;
		long long ido=cl->ido;
		std::list<tbuff*>::iterator bi;
		for(bi=buffs.begin();bi!=buffs.end();++bi)
		{
			if((*bi)->id>=0)s=&skilllist.at((*bi)->id).at((*bi)->level-1);
			else s=&cskilllist.at(-(*bi)->id-1);

			if((*bi)->id>=0)
			{
				bs->cmd(getId(), 0x4c);
				*bs << getId() << (short)1 << (short)(*bi)->id << (*bi)->level << (int)((*bi)->lejarat-cl->ido);
			}else
			{
				bs->cmd(getId(), 0x3f) << (int)-(*bi)->id-1 << (int)((*bi)->lejarat-cl->ido)/1000;
			}

			if(s->stat1>0)
			{
				addstat(bs, id, s->stat1, s->sval1);
			}
			if(s->stat2>0)
			{
				addstat(bs, id, s->stat2, s->sval2);
			}
		}
	}
}

void character_buffable::sendbdataatlogin(buffer *bs)
{
	if(nbuffs>0)
	{
		tskilldata *s;
		long long ido=cl->ido;
		std::list<tbuff*>::iterator bi;
		for(bi=buffs.begin();bi!=buffs.end();++bi)
		{
			if((*bi)->id>=0)s=&skilllist.at((*bi)->id).at((*bi)->level-1);
			else s=&cskilllist.at(-(*bi)->id-1);
			if((*bi)->id>=0)
			{
				bs->cmd(id, 0x4c);
				*bs << id << (short)1 << (short)(*bi)->id << (*bi)->level << (int)((*bi)->lejarat-cl->ido);
			}else
			{
				bs->cmd(id, 0x3f) << (int)-(*bi)->id-1 << (int)((*bi)->lejarat-cl->ido)/1000;
			}
			if(s->stat1>0)
			{
				addstat(bs, id, s->stat1, s->sval1);
				((tplayer*)this)->addstat3(s->stat1, s->sval1);
			}
			if(s->stat2>0)
			{
				addstat(bs, id, s->stat2, s->sval2);
				((tplayer*)this)->addstat3(s->stat2, s->sval2);
			}
		}
	}
}


void character_buffable::loadbuffs(int *i)
{
	int skill, skilllevel;
	long long ido=cl->ido, duration;
	while(*i!=-2048)
	{
		skill=*i;++i;
		skilllevel=*i;++i;
		duration=*i;++i;
		insertbuff(new tbuff(skill, skilllevel, duration, ido+duration));
	}
	if((nbuffs>0)&&(!abr))
	{
		cl->addbuffed(this, buffedremover);
		abr=true;
	}
}

void character_buffable::insertbuff(tbuff *b)
{
//#ifdef _DEBUG
	if(buffplace.at(b->id+camm)!=0)logger.elog("Buff was not cleared\n");
//#endif

	buffplace.at(b->id+camm)=b;
	if(nbuffs<1)
	{
		nbuffs=1;
		buffs.push_front(b);
		b->i=buffs.begin();
		avgt=(double)cl->ido;
		avg=(double)b->duration;
	}else
	{
		avg-=cl->ido-avgt;
		avgt=(double)cl->ido;
		if(b->duration<avg)
		{
			std::list<tbuff*>::iterator i=buffs.begin();
			for(;i!=buffs.end();++i)
			{
				if((*i)->lejarat>=b->lejarat)
				{
					buffs.insert(i, b);
					--i;
					b->i=i;
					avg*=nbuffs;
					avg+=b->duration;
					nbuffs++;
					avg/=nbuffs;
					return;
				}
			}
			buffs.push_back(b);
			--i;
			b->i=i;
		}else
		{
			std::list<tbuff*>::iterator i=buffs.end();
			int a=nbuffs;
			--i;
			for(;a>0;--i,--a)
			{
				if((*i)->lejarat<=b->lejarat)
				{
					++i;
					if(i!=buffs.end())
					{
						buffs.insert(i, b);
						--i;
					}else
					{
						buffs.push_back(b);
						--i;
					}
					b->i=i;
					avg*=nbuffs;
					avg+=b->duration;
					nbuffs++;
					avg/=nbuffs;
					return;
				}
			}
			buffs.push_front(b);
			b->i=buffs.begin();
		}

		avg*=nbuffs;
		avg+=b->duration;
		nbuffs++;
		avg/=nbuffs;
	}
}

int character_buffable::savebuffs(int *i)
{
	int r=4;
	long long ido=cl->ido;
	std::list<tbuff*>::iterator bi;
	for(bi=buffs.begin();bi!=buffs.end();++bi)
	{
		*i=(*bi)->id;r+=4;++i;
		*i=(*bi)->level;r+=4;++i;
		*i=(int)((*bi)->lejarat-ido);r+=4;++i;
	}
	*i=-2048;
	return r;
}

void character_buffable::clearbuffs()
{
	if(nbuffs>0)
	{
		buffer bs;
		while(!buffs.empty())
		{
			if(buffs.front()->id>=0)
			{
				bs.cmd(id, 0xf8) << (short)1 << (short)buffs.front()->id;
			}else
			{
				buffer bs1;
				bs1.cmd(id, 0x3f) << (int)-buffs.front()->id-1 << 0;
				selfcast(bs1);
			}
			removeeffect(buffs.front(), false);
			buffplace.at(buffs.front()->id+camm)=0;
			delete buffs.front();
			buffs.pop_front();
		}
		nbuffs=0;
		if(abr)cl->rembuffed(buffedremover);
		abr=false;
		multicast(bs);
	}
}

void character_buffable::alterhp(int b, character_base *dd)
{
	if(isDead) return;
	int a;
	if((b<0)&&(type==ttplayer))((tplayer*)this)->canceldelayed();
	hp+=b;
	if(hp>maxhp)hp=maxhp;
	if(hp<0)hp=0;
	if((hp<=0)&&(protection>cl->ido))
	{
		hp=0;
		a=protectionlevel;
		if(a>18)
		{
			a=18;
			protectionlevel-=18;
		}else
		{
			protection=0;
			this->removebuff(48);
		}
		hp=maxhp*a/18;
		buffer bs;
		effect(&bs, id, 62);
		setstat(&bs, id, 38, hp);
		multicast(bs);
	}
	onHpChanged();

	if(hp<=0)
	{
		isDead = true;
		if(additionalcrit)
		{
			addstat1(DST_CHR_CHANCECRITICAL, -15);
			additionalcrit=false;
		}
		hp=0;
		if(dd!=0)
		{
			if(dd->type==ttplayer)
			{
				playerLock=(tplayer*)dd;
				partyLock=&(*playerLock->party);
			}
/*
			buffer bs;
			bs.cmd(id, 0xc7);
			bs << dd->getId() << b;
			multicast(bs);
*/
		}/*else
		{
			logger.log("damager is 0 on ko\n");
		}*/
		if(isMoveing())
		{	
			domove();
			stopmove();
		}

		if(dd==0)onKo();
		else if((dd->type==ttmob)||(dd->type==ttplayer))onKo((character_buffable*)dd);
		else onKo();

	}else if((hp<=maxhp/4)&&!additionalcrit)
	{
		addstat1(DST_CHR_CHANCECRITICAL, 15);
		additionalcrit=true;
	}else if(additionalcrit)
	{
		addstat1(DST_CHR_CHANCECRITICAL, -15);
		additionalcrit=false;
	}
}
