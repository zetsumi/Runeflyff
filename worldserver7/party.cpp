#include "stdafx.h"
#include <set>
#include "party.h"
#include "pmutex.h"
#include "player.h"
#include "adat.h"

#include "cluster.h"
#include <windows.h>

pmutex tparty::partycreatemutex;
std::vector2<tparty*> tparty::parties;
std::list<int> tparty::freeparties;


tparty* tparty::getparty(int i)
{
	pmutex::unlocker m=partycreatemutex.lock();
	if((i>(int)parties.size())||(i<0))return 0;
	if(parties.at(i)->used)return parties.at(i);
	else return 0;
}

void tparty::playerdata::set(tplayer *p)
{
	pl=p;
	job=p->job;
	level=p->level;
	name=p->name;
	dbid=p->dbid;
}

tparty::tparty()
:level(1),exp(0),points(100),expmode(0),itemmode(0),seq(0)
,nmembers(0),advanced(false),linktime(0),fctime(0),gbtime(0),stime(0)
,alink(0), asc(0), linklevel(0), leaderfocus(0),used(false)
{
}
tparty* tparty::create(tplayer *p, tplayer *leader)
{
	ul m=partycreatemutex.lock();
	tparty *r=0;
	if(!freeparties.empty())
	{
		int a=freeparties.front();
		freeparties.pop_front();
		r=parties.at(a);
		if(!r->create1(p, leader, a))
		{
			r=0;
			freeparties.push_front(a);
		}
	}
	return r;
};

void tparty::initparties(int n)
{
	parties.resize(n);
	for(int a=0;a<n;a++)
	{
		parties[a]=new tparty();
		freeparties.push_front(a);
	}
}

bool tparty::create1(tplayer *p, tplayer *leader, int t)	//invited player calls this
{
	if((!p->validnl())||(!leader->valid()))return false;
	if(p->cl!=leader->cl)return false;

	{
		ul mm=partymutex.lock();
		if(used)return false;
		level=1;
		exp=0;
		points=100;
		expmode=0;
		itemmode=0;
		seq=0;
		nmembers=0;
		advanced=false;
		linktime=0;
		fctime=0;
		gbtime=0;
		stime=0;
		alink=0;
		asc=0;
		linklevel=0;
		leaderfocus=0;

		memberdata.resize(maxpartymembers);
		members.resize(maxpartymembers);
		for(int a=0;a<maxpartymembers;a++)members.at(a)=&memberdata.at(a);
		used=true;
		ticket=t;
	}
	{
		ul m=leader->playermutex.lock();
		join(leader);
		setplayerfocus(leader, leader->getfocus());
	}
	join(p);
	return true;
}

void tparty::loginpuffer(playerdata *p)
{
	int a,b;
	if(p->pl!=0)
	{
//		tplayer *q=p->pl;
		buffer *s=p->pl->bs;	//the player called this, this should be safe
		{
			ul m=p->pl->asyncbuffermutex.lock();
//			if(*p->bs==0)*p->bs=new buffer;
			a=0;
			if(advanced)a=1;
			s->cmd(p->dbid, 0x82) << members.at(0)->dbid << nmembers << a << a << nmembers << level << exp << points << expmode << itemmode << 0 << 0 << 0 << 0 << 0;
			if(advanced)s->sndpstr(name);
			for(a=0,b=0;(a<maxpartymembers)&&(b<nmembers);a++)
				if(members.at(a)->dbid!=-1)
				{
					b++;
					*s << members.at(a)->dbid << members.at(a)->level << members.at(a)->job << 0 << (char)0;
					s->sndpstr(members.at(a)->name);
				}
			if(advanced)
			{
				s->cmd(p->dbid, 0x88);
				s->sndpstr(name);
			}

		}
	}

}


void tparty::sendpartydata()
{
	buffer bs;
	bs << (char)0x82;
	bs.inc();
	int a=0;
	if(advanced)a=1;
	bs << members.at(0)->dbid;
	bs << nmembers;
	bs << a << a << nmembers << level << exp << points << expmode << itemmode << 0 << 0 << 0 << 0 << 0;
	if(advanced)bs.sndpstr(name);
	
	int b;
	for(a=0,b=0;(a<maxpartymembers)&&(b<nmembers);a++)
		if(members.at(a)->dbid!=-1)
		{
			b++;
			bs << members.at(a)->dbid << members.at(a)->level << members.at(a)->job << 0 << (char)0;
			bs.sndpstr(members.at(a)->name);
		}

	partymulticast(bs);
}

void tparty::partymulticast(buffer &bs)
{
	int a,b;
	playerdata *p;
	for(a=0,b=0;(a<maxpartymembers)&&(b<nmembers);a++)
		if(members.at(a)->dbid!=-1)
		{
			b++;
			p=members.at(a);
			if(p->pl!=0)
			{
				{
					pmutex::unlocker m=p->pl->asyncbuffermutex.lock();
					if(p->pl->asyncbuffer==0)p->pl->asyncbuffer=new buffer;
					*p->pl->asyncbuffer << p->dbid;
					p->pl->asyncbuffer->copy(bs);
				}
			}
		}
}

bool tparty::join(tplayer *p)
{
	ul m=partymutex.lock();
	if(!used)return false;
	bool retval=(nmembers<maxpartymembers);
	if(retval)
	{
		for(int a=0;a<maxpartymembers;a++)
			if(members.at(a)->dbid==-1)
			{
				members.at(a)->set(p);
				p->partyslot=a;
				nmembers++;
				if(nmembers>1)
				{
					p->party=this;
					sendpartydata();
				}
				break;
			}
	}
	return retval;
}

void tparty::leave(int id, tplayer *p)
{
	bool endthis=false;
	{
		ul m=partymutex.lock();
		if(id==members[0]->dbid)return;
		int a;
		if(p->dbid==id)
		{
			a=p->partyslot;
			if(members.at(a)->dbid==id)
			{
				if((members.at(0)->pl==p)||(members.at(a)->pl==p))endthis=leave2(members.at(a), a, p);
			}
		}
		else for(a=0;a<maxpartymembers;a++)
		{
			if(members.at(a)->dbid==id)
			{
				if((members.at(0)->pl==p)||(members.at(a)->pl==p))endthis=leave2(members.at(a), a, p);
				break;
			}
		}
	}
	if(endthis)
	{
		if(cl==p->cl)
		{
			if(alink!=0)cl->removelink(alinkr);
			if(asc!=0)cl->removesc(ascr);
		}else
		{
			if(alink!=0)cl->removelink_lock(alinkr);
			if(asc!=0)cl->removesc_lock(ascr);
		}
		alink=0;
		asc=0;
		memberdata.resize(0);
		members.resize(0);
		used=false;
		ul m=partycreatemutex.lock();
		freeparties.push_front(ticket);
		ticket=-1;
	}
}

bool tparty::leave2(playerdata *pd,int a, tplayer *qc)
{
	bool r=nmembers>2;
	if(nmembers>2)
	{
		if(pd->pl!=0)
		{
			pmutex::unlocker m=members.at(a)->pl->asyncbuffermutex.lock();
			if(members.at(a)->pl->asyncbuffer==0)members.at(a)->pl->asyncbuffer=new buffer;
			members.at(a)->pl->asyncbuffer->cmd(members.at(a)->dbid, 0x82) << members.at(a)->dbid << 0;
			pd->pl->acmd.push(tplayer::ac_quit_party);
		}
		nmembers--;
		pd->clear();

		if(pd==members.at(0))
		{
			playerdata *q;
			for(a=1;a<maxpartymembers;a++)
				if(members.at(a)->dbid!=-1)
				{
					q=members.at(0);
					members.at(0)=members.at(a);
					members.at(a)=q;
					break;
				}
		}
		sendpartydata();

	}else
	{
		for(a=0;a<maxpartymembers;a++)
		{
			if((members.at(a)->dbid!=-1)&&(members.at(a)->pl!=0))
			{
				{
					pmutex::unlocker m=members.at(a)->pl->asyncbuffermutex.lock();
					if(members.at(a)->pl->asyncbuffer==0)members.at(a)->pl->asyncbuffer=new buffer;
					members.at(a)->pl->asyncbuffer->cmd(members.at(a)->dbid, 0x82) << members.at(a)->dbid << 0;
					members[a]->pl->acmd.push(tplayer::ac_quit_party);
				}
			}
			if(members.at(a)->dbid!=-1)members[a]->clear();
		}
		nmembers=0;
	}
	return nmembers<2;
/*
	bool delthis=false;
	{
		int a;
		playerdata *q;
		tplayer *p=0;
		{
			if(pd->pl!=0)
			{
				p=pd->pl;
				if(p->party!=this)
				{
					return delthis;
				}
				a=a2;
				{
					pmutex::unlocker m=members.at(a)->pl->asyncbuffermutex.lock();
					if(*members.at(a)->bs==0)*members.at(a)->bs=new buffer;
					(*members.at(a)->bs)->cmd(members.at(a)->dbid, 0x82) << members.at(a)->dbid << 0;
				}
				{
					partymutex.e_unlock();
					ul m=(qc==p)?pmutex::dontlock():members.at(a)->pl->playermutex.lock();
					members.at(a)->pl->party=0;
					partymutex.e_lock();
				}
			}
			pd->clear();
			nmembers--;
			if(nmembers<2)
			{
				for(a=0;a<maxpartymembers;a++)
					if((members.at(a)->dbid!=-1)&&(members.at(a)->pl!=0))
					{
						{
							pmutex::unlocker m=members.at(a)->pl->asyncbuffermutex.lock();
							if(*members.at(a)->bs==0)*members.at(a)->bs=new buffer;
							(*members.at(a)->bs)->cmd(members.at(a)->dbid, 0x82) << members.at(a)->dbid << 0;
						}
						{
							partymutex.e_unlock();
							ul m=(qc==members.at(a)->pl)?pmutex::dontlock():members.at(a)->pl->playermutex.lock();
							members.at(a)->pl->party=0;
							partymutex.e_lock();
						}
					}
//					sendpartyenddata();
					delthis=true;
			}else
			{
				if(pd==members.at(0))
				{
					for(a=0;a<maxpartymembers;a++)
						if(members.at(a)->dbid!=-1)
						{
							q=members.at(0);
							members.at(0)=members.at(a);
							members.at(a)=q;
							break;
						}
					
				}
				sendpartydata();
			}
		}
	}
	return delthis;
*/
}

int partylevelexp[9]={200,200,250,300,350,400,450,500,500};

bool tparty::levelto10(tplayer *p)
{
	ul m=partymutex.lock();
	if(!used)return false;
	if(level>=10) return false;
	while(level<10)
	{
		points+=level;
		level++;
	}
	exp=0;

	buffer bs;
	bs << (char)0x85 << this->exp << this->level << this->points;
    bs.inc();
    partymulticast(bs);

	return true;
}

void tparty::addexp(int Aexp, int Alevel, int Apoint)
{
	ul m=partymutex.lock();
	if(!used)return;
	buffer bs;
    int a, oldlevel;
    if(Aexp!=-1)this->exp+=Aexp*partyexpszorzo;
    if(Alevel!=-1)this->level+=Alevel;
    if(Apoint!=-1)this->points+=Apoint;

    do
	{
        oldlevel=level;
        if(level<10)
        {
            if(exp>=partylevelexp[level-1])
            {
                exp-=partylevelexp[level-1];
                level++;
                points+=partypointsperlevel;
            }
        }else{
            if(advanced)
            {
                a=(int)((50+level)*level/13)*10;
                if(exp>=a)
                {
                    exp-=a;
                    level++;
                    points+=level;//partypointsperlevel;
                }
            }
        }
    }while(oldlevel!=level);
    
    bs << (char)0x85 << this->exp << this->level << this->points;
    bs.inc();
    partymulticast(bs);
}

void tparty::setexp(int id, int mode)
{
	ul m=partymutex.lock();
	if(!used)return;
    if((members.at(0)->pl!=0)&&(members.at(0)->dbid==id))
    {
        expmode=mode;
		buffer bs;
		bs << (char)0x90 << mode;
		bs.inc();
		partymulticast(bs);
    }
}

void tparty::setitem(int id, int mode)
{
	{
		ul m=partymutex.lock();
		if(!used)return;
		if((members.at(0)->pl!=0)&&(members.at(0)->dbid==id))
		{
			itemmode=mode;
			buffer bs;
			bs << (char)0x8f << mode;
			bs.inc();
			partymulticast(bs);
		}
	}
}

/*
	ul m=partymutex.lock();
	float x,z;
    for(int a=0;a<members.size();a++)
		if(members.at(a)->pl!=0)
			if(members.at(a)->pl->cl==p->cl)
			{
				x=p->getx()-members.at(a)->pl->getx();
				z=p->getz()-members.at(a)->pl->getz();
				x*=x;
				z*=z;
				if(x+z<45*45)l.push_back(members.at(a)->pl);
			}
*/
void tparty::partychat(tplayer *p, const char *nev, int id, const char *msg)
{
	ul m=partymutex.lock();
	if(!used)return;
	buffer bs;
	buffer bs1;
	float x,z;
	bool found;

	bs1 << (char)0x69 << id;
	bs1.sndpstr(nev);
	bs1.sndpstr(msg);
	bs1.inc();

	bs.cmd(p->getId(), 0x69) << p->getId();
	bs.sndpstr(nev);
	bs.sndpstr(msg);

	for(int a=0;a<(int)members.size();a++)
		if(members.at(a)->pl!=0)
		{
			found=false;
			if(members.at(a)->pl->cl==p->cl)
			{
				x=p->getx()-members.at(a)->pl->getx();
				z=p->getz()-members.at(a)->pl->getz();
				x*=x;
				z*=z;
				found=(x+z<45*45);
				if(found)
				{
					pmutex::unlocker mmm=members.at(a)->pl->asyncbuffermutex.lock();
					if(members.at(a)->pl->asyncbuffer==0)members.at(a)->pl->asyncbuffer=new buffer;
					members.at(a)->pl->asyncbuffer->add(bs);
				}
			}
			if(!found)
			{
				{
					pmutex::unlocker mm=members.at(a)->pl->asyncbuffermutex.lock();
					if(members.at(a)->pl->asyncbuffer==0)members.at(a)->pl->asyncbuffer=new buffer;
					*members.at(a)->pl->asyncbuffer << p->dbid;
					members.at(a)->pl->asyncbuffer->copy(bs1);
				}
			}
		}

/*
	{
		ul m=partymutex.lock();
	}
	std::list<tplayer*> pl;
	this->getpplaround(p, pl);
	if(!pl.empty())
	{
		buffer bs;
		bs.cmd(p->getId(), 0x69) << p->getId();
		bs.sndpstr(nev);
		bs.sndpstr(msg);
		for(std::list<tplayer*>::iterator i=pl.begin();i!=pl.end();++i)
		{
			ul m=(*i)->asyncbuffermutex.lock();
			if((*i)->asyncbuffer==0)(*i)->asyncbuffer=new buffer;
			(*i)->asyncbuffer->add(bs);
		}
	}
*/
}

void tparty::setname(int id, const std::string &nev)
{
	ul m=partymutex.lock();
	if(!used)return;
    if((members.at(0)->pl!=0)&&(members.at(0)->dbid==id)&&(!advanced))
	{
		name=nev;
		advanced=true;
		buffer bs;
        bs << (char)0x88;
        bs.sndpstr(nev);
    	bs.inc();
        partymulticast(bs);
    }
}

void tparty::memberassign(int id, int nid, tplayer *p)
{
	if(id==nid)return;
	ul m=partymutex.lock();
	if(!used)return;
    int b;
	const int a=0;
	playerdata *d;

	if((members.at(a)->pl!=0)&&(members.at(a)->dbid==id))
	{
		for(b=0;b<maxpartymembers;b++)
		if((members[b]->pl!=0)&&(members[b]->dbid==nid))
		{
			if(alink!=0)members[0]->pl->cl->removelink(alinkr);
			if(asc!=0)members[0]->pl->cl->removesc(ascr);
			alink=0;
			asc=0;

			d=members[b];
			members[b]=members.at(a);
			members.at(a)=d;
			if(members[a]->pl!=0)
			{
				ul m=(members[a]->pl==p)?pmutex::dontlock():members[a]->pl->playermutex.lock();
				members[a]->pl->partyslot=a;
			}
			if(members[b]->pl!=0)
			{
				ul m=(members[b]->pl==p)?pmutex::dontlock():members[b]->pl->playermutex.lock();
				members[b]->pl->partyslot=b;
			}
			buffer bs;
			bs << (char)0x79 << nid;
			bs.inc();
			partymulticast(bs);
			break;
		}
	}
}

tparty* tparty::login(tplayer *p)
{
/*
	ul m=partymutex.lock();
	int a;
	for(a=0;a<maxpartymembers;a++)
		if(members.at(a)->dbid==p->dbid)
		{
			members.at(a)->pl=p;
			p->partyslot=a;
			loginpuffer(members.at(a));
			return this;
		}
	return 0;
*/
	ul m=partymutex.lock();
	if(!used)return false;
	for(int a=0;a<maxpartymembers;a++)
		if(members.at(a)->dbid==p->dbid)
		{
			members.at(a)->set(p);
			p->partyslot=a;
			p->party=this;
			loginpuffer(members.at(a));
			return this;
		}
	return 0;
}

void tparty::logout(tplayer *p)
{
	ul m=partymutex.lock();
	if(!used)return;
	if(p->partyslot==0)
	{
		if(alink!=0)cl->removelink(alinkr);
		if(asc!=0)cl->removesc(ascr);
		alink=0;
		asc=0;
	}
	if(members[p->partyslot]->pl==p)
		members[p->partyslot]->pl=0;
}
/*
bool tparty::isleader(tplayer *p)
{
	ul m=partymutex.lock();
	if(!used)return false;
	return (members.at(0)->pl==p);
}
*/

void tparty::gotmoney(tplayer *p, int m)
{
	ul mmm=partymutex.lock();
	std::list<tplayer*> l;
	this->getpplaround_l(p, l);
	if(l.empty())
	{
		p->addmoney(m);
		return;
	}

	m/=l.size();
	for(std::list<tplayer*>::iterator i=l.begin();i!=l.end();++i)
	{
		(*i)->addmoney(m);
	}
}

tplayer* tparty::getseq(tplayer *p)
{
//	return p;	//disabled for stability
	ul m=partymutex.lock();
	int a;
	if(itemmode==0)return p;
	else if(itemmode==2)
	{
		if(members.at(0)->pl!=0)if(members[0]->pl->cl==p->cl)return members.at(0)->pl;
		return p;
	}else if(itemmode==3)
	{
		std::vector<tplayer*> l;
		this->getpplaround_l(p, l);
		if(l.empty())return p;
		return l[rnd(l.size())];
	}else if(itemmode==1)
	{
		std::vector<tplayer*> l;
		this->getpplaround_l(p, l);
		if(l.empty())return p;
		seq++;
		seq%=l.size();
		return l[seq];
	}

	return p;

/*
	switch(itemmode)
	{
	case 0:	//individually
		return p;
		break;
	case 1:	//seq
		do
		{
			seq++;
			if(seq>=(int)members.size())seq=0;
			
		}while(members[seq]->pl==0);
		return members[seq]->pl;
		break;
	case 2:	//manually
		if(members.at(0)->pl!=0)if(members[0]->pl->cl==p->cl)return members.at(0)->pl;
		return p;
		break;
	case 3:	//randomly

		a=rnd(members.size());
		while((members[seq]->pl==0)||(a>0));
		{
			seq++;
			if(seq>=(int)members.size())seq=0;
			if(members[seq]->pl!=0)a--;
			
		}
		break;
	}
	return p;
*/
}


void tparty::getpplaround(tplayer *p, std::list<tplayer*> &l)
{
	ul m=partymutex.lock();
	getpplaround_l(p, l);
}

void tparty::getpplaround(tplayer *p, std::vector<tplayer*> &l)
{
	ul m=partymutex.lock();
	getpplaround_l(p, l);
}


void tparty::getpplaround_l(tplayer *p, std::list<tplayer*> &l)
{
	float x,z;
    for(int a=0;a<(int)members.size();a++)
		if(members.at(a)->pl!=0)
			if(members.at(a)->pl->cl==p->cl)
			{
				x=p->getx()-members.at(a)->pl->getx();
				z=p->getz()-members.at(a)->pl->getz();
				x*=x;
				z*=z;
				if(x+z<45*45)l.push_back(members.at(a)->pl);
			}
}

void tparty::getpplaround_l(tplayer *p, std::vector<tplayer*> &l)
{
	l.reserve(5);
	float x,z;
    for(int a=0;a<(int)members.size();a++)
		if(members.at(a)->pl!=0)
			if(members.at(a)->pl->cl==p->cl)
			{
				x=p->getx()-members.at(a)->pl->getx();
				z=p->getz()-members.at(a)->pl->getz();
				x*=x;
				z*=z;
				if(x+z<45*45)l.push_back(members.at(a)->pl);
			}
}

double membersexp[8]={1.0, 1.2, 1.5, 1.8, 2.2, 2.5, 2.8, 3.0};

void tparty::managePexp(tplayer *tamado, double expval, int elevel)
{
	{
	ul m=partymutex.lock();
	{
		int a,b=0;
		float x,z;
//		tplayer* around[8];
		std::vector2<tplayer*> around;
		around.resize(members.size(), 0);
	
	    for(a=0;a<(int)members.size();a++)
			if(members.at(a)->pl!=0)if(members.at(a)->pl->cl==tamado->cl)
		{
			x=tamado->getx()-members.at(a)->pl->getx();
			z=tamado->getz()-members.at(a)->pl->getz();
			x*=x;
			z*=z;
			if((x+z<45*45)&&(abs(tamado->level-members.at(a)->pl->level)<20)&&(members[a]->pl->gethp()>0))
			{
				around[b]=members.at(a)->pl;
				b++;
			}
		}
		if(b<2)
		{
			tamado->addexp((long long)expval, elevel);
			return;
		}
		expval*=membersexp[b-1];
		if(expmode==1)
		{
			tamado->addexp(((long long)expval)*0.7, elevel);
			expval*=0.3;
			expval/=(b-1);
			
			for(a=0;a<b;a++)
				if(around.at(a)!=tamado)around.at(a)->addexp((long long)expval, elevel);
		}else
		{
			
			for(a=0;a<b;a++)around.at(a)->addexp((long long)expval, elevel);
		}
	}    
	}
	addexp(elevel);
}

void tparty::useskill(tplayer *p, int skill)
{
	ul m=partymutex.lock();
	buffer bs;
	character_base *q;
	long long ido;
	if(p==members.at(0)->pl)
	{
		ido=GetTickCount();
		switch(skill)
		{
		case 1:	//call
			bs << (char)0x8a << 0 << 0 << 0;
			bs.inc();
//			p->tomulticast->cmd(p->getId(), 0x8a) << 0 << 0 << 0;
//			return;
			break;
		case 2:	//blitz
			q=p->getfocus();
			if(q==0)return;
			bs << (char)0x8b << q->getId();
			bs.inc();
			break;
		case 3:	//retreat
			bs << (char)0x8c;
			bs.inc();
//			partymulticast(bs);
			break;
		case 4:	//sphere circle
			//bs << (char)0x8e;
			if(asc<ido)
			{
				asc=ido+19000;
				p->cl->addsc(ascr, this, cl);
				p->tomulticast->cmd(p->getId(), 0x8e);
				points--;
				if(20+10*linklevel!=0)
				{
					p->addstat1(DST_CHR_CHANCECRITICAL, 20+10*linklevel);
					p->HMF();
				}
			}
			return;
			break;
		case 5:	//link attack
			if(alink<ido)
			{
				alink=ido+19000;
				p->cl->addlink(alinkr, this, cl);
				points--;
				bs << (char)0x8d << 0 << 1 << points;
				bs.inc();
				if(20+10*linklevel!=0)
				{
					p->addstat1(DST_ATKPOWER_RATE, 20+10*linklevel);
					p->HMF();
				}
			}else return;
			break;
		case 6:	//fc
			points-=5;
			bs << (char)0x8d << 1 << 1 << points;
			bs.inc();
			fctime=ido+5*60*1000;
			break;
		case 7:	//stretching
			points-=2;
			bs << (char)0x8d << 2 << 1 << points;
			bs.inc();

			for(int a=0;a<tparty::maxpartymembers;a++)
				if(members.at(a)->pl!=0)
				{
					ul mmm=(p==members.at(a)->pl)?pmutex::dontlock():partymutex.relockwhen(members.at(a)->pl->playermutex);
					if(members.at(a)->pl!=0)members.at(a)->pl->sitting=true;
				}

			stime=ido+2*60*1000;
//			return;
			break;
		case 8:	//gb
			bs << (char)0x8d << 3 << 1 << points;
			bs.inc();
			points-=8;
			gbtime=ido+5*60*1000;
			break;
		}
//		bs.inc();
		partymulticast(bs);
	}
}

bool tparty::testlink(long long ido)
{
	ul m=partymutex.lock();
	if(alink<ido)
	{
		alink=0;
		if(members.at(0)->pl!=0)
		{
			ul mmm=members.at(0)->pl->playermutex.lock();
			if(-20-10*linklevel!=0)
			{
				members.at(0)->pl->addstat1(DST_ATKPOWER_RATE, -20-10*linklevel);
				members.at(0)->pl->HMF();
			}
		}
	}
	return (alink<ido);
}

bool tparty::testasc(long long ido)
{
	ul m=partymutex.lock();
	if(asc<ido)
	{
		asc=0;
		if(members.at(0)->pl!=0)
		{
			ul mmm=members.at(0)->pl->playermutex.lock();
			if(-20-10*linklevel)
			{
				members.at(0)->pl->addstat1(DST_CHR_CHANCECRITICAL, -20-10*linklevel);
				members.at(0)->pl->HMF();
			}
		}
	}
	return (asc<ido);
}

void tparty::setplayerfocus(tplayer *p, character_base *f)
{
	ul m=partymutex.lock();
	if((members[0]->pl==0)||(p==0))return;
	if(p->cl!=members[0]->pl->cl)return;
/*
	int b,oldlinklevel=linklevel;
	if(p==members[0]->pl)
	{
		if(leaderfocus!=f)
		{
			leaderfocus=f;
			linklevel=0;
			for(int b=1;b<maxpartymembers;++b)
				if(members[b]->pl!=0)
					if(members[b]->pl->getfocus()==leaderfocus)
						linklevel++;
		}
		if(leaderfocus==0)linklevel=0;
	}else
	{
		if(leaderfocus==f)linklevel++;
		else linklevel--;
	}

	b=linklevel-oldlinklevel;
	if((b!=0)&&(members.at(0)->pl!=0))
	{
		if(alink!=0)members.at(0)->pl->addstat1(DST_ATKPOWER_RATE, b*10);
		if(asc!=0)members.at(0)->pl->addstat1(DST_CHR_CHANCECRITICAL, b*10);
		if((alink!=0)||(asc!=0))members[0]->pl->HMF();
	}
*/
}

void tparty::mmp(tplayer *p, float x, float z)
{
	buffer bs;
	bs.cmd(p->dbid, 0xc6) << x << 0 << z;
	bs.sndpstr(p->name);

	ul m=partymutex.lock();

	for(int a=0;a<tparty::maxpartymembers;a++)
		if(members.at(a)->pl!=0)
		{
			pmutex::unlocker m=members.at(a)->pl->asyncbuffermutex.lock();
			if(members.at(a)->pl->asyncbuffer==0)members.at(a)->pl->asyncbuffer=new buffer;
			members.at(a)->pl->asyncbuffer->add(bs);
		}
}
