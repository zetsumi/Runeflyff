#include "stdafx.h"
#include "player.h"
#include "logger.h"
#include "grid.h"
#include "character_base.h"
#include "funkciok.h"
#include "cluster.h"
#include <string>
#include "stringf.h"
#include "main.h"
#include "charserver.h"
#include "adat.h"
#include "party.h"
#include "guild.h"
#include "quest.h"
#include "objactions.h"

using namespace std;

int flyspeed=5900;
float p180=(180.0f/3.1415f);

typedef void (*PROCFUNKC)(tplayer*);

PROCFUNKC t_0000[65536];
PROCFUNKC t_00ff[65536];
PROCFUNKC t_f000[65536];
PROCFUNKC t_ffff[65536];

unsigned long long s_0000[65536]={0};
unsigned long long m_0000[65536]={0};
unsigned long long n_0000[65536]={0};
unsigned long long s_00ff[65536]={0};
unsigned long long m_00ff[65536]={0};
unsigned long long n_00ff[65536]={0};
unsigned long long s_f000[65536]={0};
unsigned long long m_f000[65536]={0};
unsigned long long n_f000[65536]={0};
unsigned long long s_ffff[65536]={0};
unsigned long long m_ffff[65536]={0};
unsigned long long n_ffff[65536]={0};

void printft()
{
	logger.log("function times:\n");
	int a;
	unsigned int b;
	for(a=0;a<65536;++a)
	{
		if(n_0000[a]>0)
		{
			b=0x00000000|a;
			logger.log("%08x: %08x %08x %08x %d\n", b, (unsigned int)s_0000[a], (unsigned int)m_0000[a], (unsigned int)(s_0000[a]/n_0000[a]), (unsigned int)n_0000[a]);
		}
	}
	for(a=0;a<65536;++a)
	{
		if(n_00ff[a]>0)
		{
			b=0x00ff0000|a;
			logger.log("%08x: %08x %08x %08x %d\n", b, (unsigned int)s_00ff[a], (unsigned int)m_00ff[a], (unsigned int)(s_00ff[a]/n_00ff[a]), (unsigned int)n_00ff[a]);
		}
	}
	for(a=0;a<65536;++a)
	{
		if(n_f000[a]>0)
		{
			b=0x00000000|a;
			logger.log("%08x: %08x %08x %08x %d\n", b, (unsigned int)s_f000[a], (unsigned int)m_f000[a], (unsigned int)(s_f000[a]/n_f000[a]), (unsigned int)n_f000[a]);
		}
	}
	for(a=0;a<65536;++a)
	{
		if(n_ffff[a]>0)
		{
			b=0x00000000|a;
			logger.log("%08x: %08x %08x %08x %d\n", b, (unsigned int)s_ffff[a], (unsigned int)m_ffff[a], (unsigned int)(s_ffff[a]/n_ffff[a]), (unsigned int)n_ffff[a]);
		}
	}
}

void itemidsearch(tplayer &player, const char *p);
void mobidsearch(tplayer &player, const char *p);

bool sc(const std::string& str1, const std::string& str2) {
    if(str1.c_str() == NULL && str2.c_str() == NULL) {
        return true;
    }
    else {
        if(str1.length() != str2.length()) {
            return false;
        }
        else {
            for (int x = 0; x < str1.length(); x++) {
                if(str1[x] != str2[x]) {
                    return false;
                }
            }
        }
    }
    return true;
}

/*
bool sc(const char *t, char *r)
{
	for(;(*r!=0)&&(*t!=0);r++,t++)if(*t!=*r)return false;
	return (*r==0);
}
*/

void tplayer::process()
{
	handleacmd();
	unsigned int a;
	std::string str;
	unsigned long long t1,t2;
	unsigned int commandnumber=-1;
	sr->seek(4);
	a=0;
	sr->get(a);
	try{
		if(a==0xffffffff)
		{
			a=0;
			sr->get(a);
//#ifdef _DEBUG
//#ifdef _LOGALOT
			logger.log("%x\nRECV:\n", a);
			sr->dump();
			logger.log("\n");
//#endif
//#endif
			commandnumber=a;
			switch(a&0xffff0000)
			{
				case 0:
					if(t_0000[a&0xffff]!=0)
					{
						t1=GetTickCount();
						t_0000[a&0xffff](this);
						n_0000[a&0xffff]++;
						t2=GetTickCount();
						if(t2-t1>0)
						{
							s_0000[a&0xffff]+=t2-t1;
							if(m_0000[a&0xffff]<t2-t1)m_0000[a&0xffff]=t2-t1;
						}
					}
					break;
				case 0x00ff0000:
					if(t_00ff[a&0xffff]!=0)
					{
						t1=GetTickCount();
						t_00ff[a&0xffff](this);
						n_00ff[a&0xffff]++;
						t2=GetTickCount();
						if(t2-t1>0)
						{
							s_00ff[a&0xffff]+=t2-t1;
							if(m_00ff[a&0xffff]<t2-t1)m_00ff[a&0xffff]=t2-t1;
						}
					}
					break;
				case 0xf0000000:
					if(t_f000[a&0xffff]!=0)
					{
						t1=GetTickCount();
						t_f000[a&0xffff](this);
						n_f000[a&0xffff]++;
						t2=GetTickCount();
						if(t2-t1>0)
						{
							s_f000[a&0xffff]+=t2-t1;
							if(m_f000[a&0xffff]<t2-t1)m_f000[a&0xffff]=t2-t1;
						}
					}
					break;
				case 0xffff0000:
					if(t_ffff[a&0xffff]!=0)
					{
						t1=GetTickCount();
						t_ffff[a&0xffff](this);
						n_ffff[a&0xffff]++;
						t2=GetTickCount();
						if(t2-t1>0)
						{
							s_ffff[a&0xffff]+=t2-t1;
							if(m_ffff[a&0xffff]<t2-t1)m_ffff[a&0xffff]=t2-t1;
						}
					}
					break;
				case 0x000f0000:
					switch(a&0x0000ffff)
					{
					case 3:
						reskill();
						break;
					}
					break;
				default:;
//				if(a==0x70000007)itemtobackpack();
			}
#ifdef _DEBUG
#ifdef _LOGALOT
			logger.log("processed\n");
#endif
#endif
		}
#ifdef _DEBUG
#ifdef _LOGALOT

		else
		{
			logger.log("Command is not -1: %08x\n", a);
		}
#endif
#endif
	}
	catch(error &e)
	{
		logger.elog("Error at processing player %d %s %s:%d, command %08x %s\n", dbid, name.c_str(), ip.c_str(), sck, commandnumber, e.what());
		sr->dump();
		logger.log("Sr at %d\n", sr->getposition());
		errorstate=true;
	}
	catch(std::exception &e)
    {
		logger.elog("Error at processing player %d %s %s:%d, command %08x %s\n", dbid, name.c_str(), ip.c_str(), sck, commandnumber, e.what());
		sr->dump();
		logger.log("Sr at %d\n", sr->getposition());
		errorstate=true;
	}
#ifndef _DEBUG
	catch(...)
	{
		logger.elog("[%s::crmain::process]exception...\n", this->name.c_str());
		logger.elog("player %d %s %s:%d, command %08x\n", dbid, name.c_str(), ip.c_str(), sck, commandnumber);
		sr->dump();
		logger.log("Sr at %d\n", sr->getposition());
		errorstate=true;
	}
#endif
	sr->clear();
	toprocess=false;
	return;
}

void t_sendmail(tplayer *p)
{
	p->sendmail();
}

void t_deletemail(tplayer *p)
{
	p->deletemail();
}

void t_getitemfrommail(tplayer *p)
{
	p->getitemfrommail();
}

void t_checkmail(tplayer *p)
{
	p->checkmail();
}

void t_getmoneyfrommail(tplayer *p)
{
	p->getmoneyfrommail();
}

void t_markmail(tplayer *p)
{
	p->markmail();
}

void t_townblink(tplayer *p)
{
	if(p->shop.getopened())return;
	p->setdelayed(tplayer::da_ttnt, 300000);
//	p->townblink();
}

void t_loadingprocess(tplayer *p)
{
	p->loadingprocess();
}

void t_attack3(tplayer *p)
{
	if(p->getfocus()==0)return;
	if((p->getfocus()->type==character_base::ttplayer)&&(!((tplayer*)(p->getfocus()))->ispkable(p) ))return;

	if(p->shop.getopened())return;
	if(p->trade!=0)p->trade->del(p);
	p->trade=0;

	int a,b;
	if(!p->flying)
	{
		p->canceldelayed();
		if(p->isMoveing()>1)
		{
			p->domove();
			p->stopmove();
		}
		p->domove();
	}
	if(p->cl->ido<p->nextattack)return;
	else p->nextattack=p->cl->ido+100;
	p->setfollowed(0, 0);
	p->sr->get(b);
	p->sr->forward(4);
	p->sr->get(a);

	if(b==0x23)
	{
		if(p->inv.decsuppliment(2028))
		{
			if(p->di&&!p->gm_di)p->removebuff(193);
			p->attack(b, a);
		}//else p->greentext("Equip arrows first!");
	}else
	{
		if(p->di&&!p->gm_di)p->removebuff(193);
		p->attack(b, a);
	}
}

void t_attack2(tplayer *p)
{
	if(p->getfocus()==0)return;
	if((p->getfocus()->type==character_base::ttplayer)&&(!((tplayer*)(p->getfocus()))->ispkable(p) ))return;
	if(p->shop.getopened())return;
	if(p->trade!=0)p->trade->del(p);
	p->trade=0;

	int a,b;
	if(!p->flying)
	{
		p->canceldelayed();
		if(p->isMoveing()>1)
		{
			p->domove();
			p->stopmove();
		}
		p->domove();
	}
	if(p->cl->ido<p->nextattack)return;
	else p->nextattack=p->cl->ido+100;
	p->setfollowed(0, 0);
	p->sr->get(b);
	p->sr->forward(12);
	p->sr->get(a);
	if(p->di&&!p->gm_di)p->removebuff(193);
	p->attack(b, a);
}

void t_attack(tplayer *p)
{
	if(p->getfocus()==0)return;
	if((p->getfocus()->type==character_base::ttplayer)&&(!((tplayer*)(p->getfocus()))->ispkable(p) ))return;
	if(p->shop.getopened())return;
	if(p->trade!=0)p->trade->del(p);
	p->trade=0;

	int b;
	if(!p->flying)
	{
		p->canceldelayed();
		if(p->isMoveing()>1)
		{
			p->domove();
			p->stopmove();
		}
		p->domove();
	}
	if(p->cl->ido<p->nextattack)return;
	else p->nextattack=p->cl->ido+100;
	p->setfollowed(0, 0);
	p->sr->get(b);
//	p->sr->get(a);
	if(p->di&&!p->gm_di)p->removebuff(193);
	p->attack(b);
}

void t_skilluse(tplayer *p)
{
	p->skilluse();
}

void t_as(tplayer *p)
{
	if(p->actionslotp!=0)
	{
		p->actionslotp->setendit();
		p->actionslotp=0;
	}
	p->cancelskill();
}

void t_chat(tplayer *p)
{
	if(p->nextchat<p->cl->ido)
	{
//		if((p->muted<p->cl->ido)||(p->muted==0))
		{
			p->pchat();
			p->nextchat=p->cl->ido+250;
		}
	}
}

void t_unequip(tplayer *p)
{
	p->unequip();
	p->clearsp2();
}

void t_equip(tplayer *p)
{
	p->equip();
	p->clearsp2();
}

void t_focus(tplayer *p)
{
	p->psetfocus();
}

void t_moveitem(tplayer *p)
{
	p->moveitem();
}

void t_dropitem(tplayer *p)
{
	p->dropitem();
}

void t_sitdown(tplayer *p)
{
	p->sitdown();
}

void t_deleteitem(tplayer *p)
{
	p->deleteitem();
}

void t_npcchat(tplayer *p)
{
	p->npcchat();
}

void t_npcshop(tplayer *p)
{
	if((p->shop.getopened())||(p->trade!=0))return;
	int b;
	p->sr->get(b);
	if(p->getfocus()!=0)
		if((p->getfocus()->type==character_base::ttnpc)&&(p->getfocus()->getId()==b))
			((character_npc*)p->getfocus())->sndshop(p->bs);
}

void t_npcbuy(tplayer *p)
{
	if((p->shop.getopened())||(p->trade!=0))return;
	if(p->getfocus()!=0)
		if((p->getfocus()->type==character_base::ttnpc))
			p->npcbuy();
}

void t_npcsell(tplayer *p)
{
	if((p->shop.getopened())||(p->trade!=0))return;
	if(p->getfocus()!=0)
		if((p->getfocus()->type==character_base::ttnpc))
			p->npcsell();
}

void t_changeface(tplayer *p)
{
	if(p->getfocus()!=0)
		if((p->getfocus()->type==character_base::ttnpc))
		{
			int b;
			p->sr->forward(4);
			p->sr->get(b);
			if((b>=0)&&(b<10))/**/
			{
				p->face=b;
				changeface(p->tomulticast, p->dbid, p->face);
				p->clearsp();
			}
		}
}

void t_haircolor(tplayer *p)
{
	if(p->getfocus()!=0)
		if((p->getfocus()->type==character_base::ttnpc))
		{
			int a,b;
			p->sr->get(b);
			if((b&0xff)<10)
			{
				p->hair=b&0xff;
				a=b>>8;
				p->haircolor=((a&0xff)<<16) + (a&0xff00) + ((a>>16)&0xff);
				p->haircolor|=0xff000000;
				p->tomulticast->cmd(p->getId(), 0x48) << b;
				p->clearsp();
			}
		}
}

void t_blinkpool(tplayer *p)
{
	p->blinkpool();
}

void t_reanimate(tplayer *p)
{
	if(p->gethp()<=0)
	{
		p->reanimate();
		p->townblink();
	}
}

void t_lodeligth(tplayer *p)
{
	if(p->gethp()<=0)
	{
		p->reanimate();
		if(p->pk>=0)p->changemap(1, lodelights[p->lodelight].x, lodelights[p->lodelight].y, lodelights[p->lodelight].z, forcedchangeonblink);
		else p->townblink();
	}
}

void t_marklodeligth(tplayer *p)
{
	if(p->getfocus()!=0)
	{
		if(((p->getfocus()->type==character_base::ttnpc))&&(p->getfocus()->getmodelid()==200))
		{
			p->bs->cmd(p->getId(), 0x94) << 629;
			int b=1;
			float t2,t=(p->getpos()-lodelights[b]).sqrlength();
			for(int a=2;a<(int)lodelights.size();a++)
			{
				t2=(p->getpos()-lodelights[a]).sqrlength();
				if(t2<t)
				{
					b=a;
					t=t2;
				}
			}
			p->lodelight=b;
		}
	}

}

void t_statset(tplayer *p)
{
	int b;
	if(p->statpoints>0)
	{
		b=p->sr->get1char();
		switch(b)
		{
		case 0x64:
			p->str++;
			p->statpoints--;
			p->bs->cmd(p->getId(), 0x94) << 603;
			addstat(p->tomulticast2, p->getId(), DST_STR, 1);
			break;
		case 0x65:
			p->sta++;
			p->statpoints--;
			p->bs->cmd(p->getId(), 0x94) << 604;
			addstat(p->tomulticast2, p->getId(), DST_STA, 1);
			break;
		case 0x66:
			p->dex++;
			p->statpoints--;
			p->bs->cmd(p->getId(), 0x94) << 605;
			addstat(p->tomulticast2, p->getId(), DST_DEX, 1);
			break;
		case 0x67:
			p->intl++;
			p->statpoints--;
			p->bs->cmd(p->getId(), 0x94) << 606;
			addstat(p->tomulticast2, p->getId(), DST_INT, 1);
			break;
		}
		p->bs->cmd(p->getId(), 0x6a);
		*p->bs << p->str << p->sta << p->dex << p->intl << 3 << p->statpoints;
		p->clearsp();

		p->HMF();
	}
}

void t_sendtraderequest(tplayer *p)
{
	p->sendtraderequest();
}

void t_tradeinsertitem(tplayer *p)
{
	p->tradeinsertitem();
}

void t_tradesetok(tplayer *p)
{
	if(p->trade!=0)p->trade->setok(p);
}

void t_tradeend(tplayer *p)
{
	int a;
	if(p->trade!=0)
	{
		p->sr->get(a);
		p->trade->setender(a, p->getId());
		p->trade->del(p);
		p->trade=0;
	}
}

void t_tradesetok2(tplayer *p)
{
	if(p->trade!=0)if(p->trade->setok2(p))
	{
		p->trade->del(p);
		p->trade=0;
	}
}

void t_tradesetmoney(tplayer *p)
{
	int a;
	if(p->trade!=0)
	{
		p->sr->get(a);
		p->trade->setmoney(p, a);
	}
}

void t_tradeagree(tplayer *p)
{
	p->tradeagree();
}

void t_openshop(tplayer *p)
{
	p->openshop();
}

void t_closeshop(tplayer *p)
{
	if(p->shop.getopened())p->closeshop();
	else if(p->joinedshop!=0)
	{
		p->joinedshop->logout(p);
		p->joinedshop=0;
	}
}

void t_itemaddtoshop(tplayer *p)
{
	p->itemaddtoshop();
}

void t_lookinshop(tplayer *p)
{
	p->lookinshop();
}

void t_buyfromshop(tplayer *p)
{
	p->buyfromshop();
}

void t_itemremovefromshop(tplayer *p)
{
	int a;
	a=p->sr->get1char();
	if(p->shop.removeitem(a))p->bs->cmd(p->getId(), 0x47) << (char)a;
}

void t_removequest(tplayer *p)
{
	p->removequest();
}

void t_minimapping(tplayer *p)
{
	float fx,fy;
	if(p->party!=0)
	{
		p->sr->get(fx);
		p->sr->forward(4);
		p->sr->get(fy);
		p->party->mmp(p, fx, fy);
	}
}

void t_createcloack(tplayer *p)
{
	if(p->guild!=0)p->guild->createcloack(p);
}

void t_piercing1(tplayer *p)
{
	int a,b,e;
			p->sr->get(a);
			p->sr->get(b);

			if((p->inv.getItem(a)==0)||(p->inv.getItem(b)==0))return;

//			c=p->inv.getItem(a)->getId();

			for(e=0;e<5;e++){
				if(p->inv.getItem(a)->slots[e]==65535)
				{
					if(p->money>=80000+e*40000)
					{
						p->inv.getItem(a)->slots[e]=0;
						p->inv.updateitem(a);
						effect(p->tomulticast, p->getId(), 1714);
						sound(p->tomulticast, 0x0e);
						p->inv.removeitem(b,1);
						p->money-=80000+e*40000;
						setstat(p->bs, p->getId(), 79, p->money);
					}else
					{
						p->bs->cmd(p->getId(), 0x94) << 1128;
					}
					break;
				}
			}
}

void t_piercing2(tplayer *p)
{
	int a,b,c,e;
	p->sr->get(a);
	p->sr->get(b);

	if((p->inv.getItem(a)==0)||(p->inv.getItem(b)==0))return;
	c=p->inv.getItem(b)->getId();
	if((c>=2029)&&(c<=2033))c=c+8+rnd(9)*5;

	for(e=0;e<5;e++)
		if(p->inv.getItem(a)->slots[e]==0)
		{
			p->inv.getItem(a)->slots[e]=(unsigned short)c;
			p->inv.updateitem(a);
			effect(p->tomulticast, p->getId(), 1714);
 			sound(p->tomulticast, 0x0e);
			p->inv.removeitem(b, 1);
			break;
		}
}

void t_upgrade(tplayer *p)
{
	p->upgradeitem();
}

void t_talkinshop(tplayer *p)
{
	p->talkinshop();
}

void t_guildlogo(tplayer *p)
{
	int b;
	if((p->guild!=0)&&(p->guild->getowner()==p->dbid))
	{
		p->sr->get(b);
		p->guild->setlogo(b);
	}
}

void t_guildnotice(tplayer *p)
{
	if(p->guild!=0)
		if(p->guild->getowner()==p->dbid)
			p->setguildnotice();
}

void t_guildname(tplayer *p)
{
	if(p->guild!=0)if((p->guild->getowner()==p->dbid))
	{
		p->setguildname();
	}
}

void t_guildsetpay(tplayer *p)
{
	int a,b;
	if((p->guild!=0)&&(p->guild->getowner()==p->dbid))
	{
		p->sr->forward(8);
		p->sr->get(a);
		p->sr->get(b);
		p->guild->setpay(a, b);
	}
}

void t_guildsetrigths(tplayer *p)
{
	if((p->guild!=0)&&(p->guild->getowner()==p->dbid))
	{
		p->setguildrigths();
	}
}

void t_guildwar(tplayer *p)
{
	p->challangeguild();
}

void t_acceptwar(tplayer *p)
{
	int a;
	if(p->guild!=0)
	{
		p->sr->forward(4);
		p->sr->get(a);
		p->guild->acceptwar(p, a);
	}
}

void t_giveupwar(tplayer *p)
{
	int a;
	p->sr->get(a);
	if((p->guild!=0)&&(a==p->dbid))p->guild->giveupwar(p, a);
}

void t_guildpiece(tplayer *p)
{
	int a;
	p->sr->get(a);
	if((p->guild!=0)&&(a==p->dbid))p->guild->piece(a);
}

void t_invtowh(tplayer *p)
{
	if(p->guild!=0)p->itemtowh();
}

void t_whtoinv(tplayer *p)
{
	if(p->guild!=0)p->whtoinv();
}

void t_moneytoguild(tplayer *p)
{
	int a,b,c;
	item *t;
	if(p->guild!=0)
	{
		c=p->sr->get1char();	//pxp
		p->sr->get(a);			//money
		b=p->sr->get1char();	//questies
		if(p->money>=a)
		{
			p->money-=a;
			setstat(p->bs, p->getId(), 79, p->money);
			p->guild->contribution(p, p->dbid, 0, a);
		}
		if(b>0)
		{
			b=0;
			for(a=0;a<inventory::eqoff2;a++)
			{
				t=p->inv.getItem(a);
				if(t!=0)
					if(t->getId()>0)
						if(t->GetI()!=0)
							if(t->GetI()->mobid>0)
							{
								b+=monsterlist.at(t->GetI()->mobid).level*t->num;
								p->inv.removeitem(a, t->num);
							}
			}
			if(b>0)p->guild->contribution(p, p->dbid, b, 0);
		}
		if((c>0)&&(p->pxp>0))
		{
			setstat(p->bs, p->getId(), 80, 0);
			if(p->pxp>0)p->guild->contribution(p, p->dbid, p->pxp, 0);
			p->pxp=0;
		}
	}
}

void t_movetopoint(tplayer *p)
{
//	if(p->sitting)p->clearsp2();
//	p->sitting=false;
	if((p->abletomove<=0)&&(!p->sitting))
	{
		p->movetopoint();
		p->setfollowed(0, 0);
	}
}

void t_follow(tplayer *p)
{
	if((p->abletomove<=0)&&(!p->sitting))
	{
		p->pfollow();
	}
}

void t_removefromSslot(tplayer *p)
{
	p->removefromSslot();
}

void t_addtoSslot(tplayer *p)
{
	p->addtoSslot();
}

void t_addtoFslot(tplayer *p)
{
	p->addtoFslot();
}

void t_removefromFslot(tplayer *p)
{
	p->removefromFslot();
}

void t_addtoAslot(tplayer *p)
{
	p->addtoAslot();
}

void t_bank_asszem(tplayer *p)
{
	p->bs->cmd(p->getId(), 0x58) << 1 << 1 << 1;
}

void t_itemtobank(tplayer *p)
{
	p->itemtobank();
}

void t_moneytobank(tplayer *p)
{
	int a,b;
	a=p->sr->get1char();   //slot
	p->sr->get(b);     //money
	if((p->money>=b)&&(a>=0)&&(a<3))
	{
		int m=p->bankmoney[a]+b;
		if(m<0)
		{
			p->messagebox("You can't store that much in your bank!");
			return;
		}

		p->bankmoney[a]+=b;
		p->money-=b;
		p->bs->cmd(p->getId(), 0x52);
		*p->bs << (char)a << p->money << p->bankmoney[a];
	}
}

void t_banktoinv(tplayer *p)
{
	p->banktoinv();
}

void t_moneyfrombank(tplayer *p)
{
	int a,b;
	a=p->sr->get1char();   //slot
	p->sr->get(b);     //money
	if((p->bankmoney[a]>=b)&&(a>=0)&&(a<3))
	{
		int m=p->money+b;
		if(m<0)
		{
			p->messagebox("You can't store that much money in your inv");
			return;
		}
		p->bankmoney[a]-=b;
		p->money+=b;
		p->bs->cmd(p->getId(), 0x52);
		*p->bs << (char)a << p->money << p->bankmoney[a];
	}else
	{
		p->bs->cmd(p->getId(), 0x94) << 662;
	}
}

void t_banktobank(tplayer *p)
{
	p->banktobank();
}

void t_usepartyskill(tplayer *p)
{
	int a;
	p->sr->forward(4);
	p->sr->get(a);
	if(p->party!=0)p->party->useskill(p, a);
}

void t_wasd(tplayer *p)
{
	if((p->abletomove<=0)&&(!p->sitting))
	{
//		if(p->sitting)p->clearsp2();
//		p->sitting=false;
		if(p->flying)
		{
//			p->silent=true;
			p->addstat1type=1;
			if(p->inv.getMask(inventory::eqoff2+13)!=-1)
				p->inv.unequip(p->inv.getMask(inventory::eqoff2+13));
			p->addstat1type=0;
		}
		p->wasd();
		p->setfollowed(0, 0);
	}
}

void t_motion(tplayer *p)
{
	if(p->abletomove<=0)
	{
//		if(p->sitting)p->clearsp2();
//		p->sitting=false;
		p->motion();
		p->setfollowed(0, 0);
	}
}

void t_blockedmove(tplayer *p)
{
//	p->blockedmove();
}

void t_reachedp(tplayer *p)
{
	int a;
	p->sr->get(a);
	if(a==p->getId())p->setfollowed(0, 0);
}

void t_flyturn(tplayer *p)
{
	if(p->abletomove<=0)
	{
		if(p->sitting)p->clearsp2();
		p->sitting=false;
		p->flyturn();
	}
}

void t_flyturn06(tplayer *p)
{
	if(p->abletomove<=0)
	{
		if(p->sitting)p->clearsp2();
		p->sitting=false;
		p->flyturn06();
	}
}

void t_fly(tplayer *p)
{
	if(p->abletomove<=0)
	{
		if(p->sitting)p->clearsp2();
		p->sitting=false;
		p->fly();
	}
}

void t_addfriendbyid(tplayer *p)
{
	p->addfriendbyid();
}

void t_asktobefriend(tplayer *p)
{
	p->asktobefriend();
}

void t_refusefriend(tplayer *p)
{
	p->refusefriend();
}

void t_deletefriend(tplayer *p)
{
	p->deletefriend();
}

void t_addfriendbyname(tplayer *p)
{
	p->addfriendbyname();
}

void t_setmsgstate(tplayer *p)
{
	int a,b;
	p->sr->get(b);
	if(b==p->dbid)
	{
		p->sr->get(b);
		p->msgstate=b;
		buffer bs;
		bs << 0xffffff67 << p->dbid << b;
		p->ooo->add(bs);
		p->raiseooo();
		for(a=0;a<(int)p->friendlist.size();a++)
		{
			if((p->friendlist[a].dbid!=-1)&&(p->friendlist[a].p!=0))
			{
			    std::lock_guard<std::mutex> guard(p->friendlist[a].p->asyncbuffermutex);
				if(p->friendlist[a].p!=0)
				{
					p->friendlist[a].p->asyncbuffer2.push_back(buffer());
					p->friendlist[a].p->asyncbuffer2.back().copy(bs);
				}
			}
		}
	}
}

void t_docheer(tplayer *p)
{
	if(p->ncheers>0)p->docheer();
}

void t_partyjoin(tplayer *p)
{
	p->partyjoin();
}

void t_partyleave(tplayer *p)
{
	p->partyleave();
}

void t_partyinvite(tplayer *p)
{
	p->partyinvite();
}

void t_partyname(tplayer *p)
{
	p->partyname();
}

void t_partysetitem(tplayer *p)
{
	int a,b;
	p->sr->get(a);
	p->sr->get(b);
	if(p->party!=0)p->party->setitem(a,b);
}

void t_partysetexp(tplayer *p)
{
	int a,b;
	p->sr->get(a);
	p->sr->get(b);
	if(p->party!=0)p->party->setexp(a,b);
}

void t_partymemberassign(tplayer *p)
{
	int a,b;
	p->sr->get(a);
	p->sr->get(b);
	if(p->party!=0)p->party->memberassign(a,b,p);
}

void t_guilddisband(tplayer *p)
{
	if(p->guild!=0)p->guild->deleteguild(p);
}

void t_guildleave(tplayer *p)
{
	int a,b;
	p->sr->forward(4);
	p->sr->get(a);
	if(p->guild!=0)
	{
		b=p->guild->leave(p, a);
		if(b!=1)
		{
			if(b==-1)p->bs->cmd(p->getId(), 0x94) << 691;
			else if(b==0)p->bs->cmd(p->getId(), 0x94) << 690;
			//p->messagebox("Can't leave guild");
		}
	}
}

void t_guildinvite(tplayer *q)
{
	int a;
	q->sr->get(a);
	tplayer *p=q->cl->getplayer(a);
	if((p==q)||(p==0))return;
    std::lock_guard<std::mutex> guard(p->playermutex);
	if(q->guild!=0)
	{
		p->bs->cmd(p->getId(), 0x9a)<< q->guild->getowner() << p->dbid;
		p->guildid=q->guild->getguildid();
		q->bs->cmd(q->getId(), 0x95) << 695;
		q->bs->sndpstr(p->name);
	}
}

void t_guildpromotemember(tplayer *p)
{
	int a,b;
	p->sr->get(a);
	if((a==p->dbid)&&(p->guild!=0))
	{
		p->sr->get(a);
		p->sr->get(b);
		p->guild->promote(p, a, b);
	}
}

void t_guildclassup(tplayer *p)
{
	int a,b;
	b=p->sr->get1char();
	p->sr->get(a);
	if((a==p->dbid)&&(p->guild!=0))
	{
		p->sr->get(a);
		p->guild->classup(p, a, b);
	}
}

void t_guildjoin(tplayer *p)
{
	if((p->guild==0)&&(p->guildid!=0))
	{
		p->guild=tguild::getguild(p->guildid);
		if(p->guild!=0)
		{
			if(!p->guild->invited(p, p->dbid, p->level, p->job, p->name, p->getId()))
			{
				p->guild=0;
				p->guildid=0;
				p->bs->cmd(p->getId(), 0x94) << 685;
			}else
			{
				p->tomulticast->cmd(p->getId(), 0x9b) << p->guild->getguildid();
			}
		}
	}
}

void t_originalplace(tplayer *p)
{
	int a;
	item *t;
	for(a=0;a<inventory::eqoff;a++)
	{
		t=p->inv.getMaskedItem(a);
		if(t!=0)
		{
			if(t->getId()==10431)
			{
				t->itemuse(p);
				p->inv.removeitem(p->inv.getLookup(a), 1);
				return;
			}
		}
	}
}

void t_ressed(tplayer *p)
{
	if(p->reslevel>0)
	{
		p->reslevel=0;
		p->reanimate();
	}
}

void t_createnick(tplayer *p)
{
	if(p->guild!=0)p->createnick();
}

void t_guildnemtom(tplayer *p)
{
	if(p->guild==0)p->guildid=0;
}

void t_fixmobplace(tplayer *p)
{
/*
	int a;
	p->sr->get(a);
	character_base *f=0;
	if(p->cl->playerinrange(a))
		f=p->cl->getplayer(a);//gridcell->findplayer(a);
	else if(p->cl->mobinrange(a))
		f=p->cl->getmob(a);//gridcell->findmob(a);
	else if(p->cl->npcinrange(a))
		f=p->cl->getnpc(a);//gridcell->findnpc(a);
	if(f!=0)
	{
//		vector3d<> v=f->getpos();
//		if(f->type==character_base::ttmob)
			p->bs->cmd(f->getId(), 0x5f) << f->getx() << f->gety() << f->getz() << 0 << 0 << 0;
//		else
//		{
//			p->bs->cmd(f->getId(), 0x5f) << f->getx() << f->gety() << f->getz() << 0 << 0 << 0;
//			if(f->isMoveing())gotoxyz(p->bs, f->getId(), f->getcel().x, f->getcel().y, f->getcel().z);
//		}
	}
*/
}

void t_duelagree(tplayer *p)
{
	if((p->duelchallanger!=0)&&(p->duel==0))
	{
		if(p->duelchallanger->duelchallanged==p->dbid)
		p->duelcountdowntime=p->cl->ido+1000;
		p->duelcountdown=3;

		p->duel=p->duelchallanger;
		p->duel->duel=p;
		p->duelbegan=false;
		p->duel->duelbegan=false;
		p->duelchallanger=0;
	}
}

void t_duelask(tplayer *p)
{
	int a,b;
	p->sr->get(a);
	p->sr->get(b);
	if(a==p->dbid)
	{
		std::map<int, tplayer*>::iterator i;

		i=p->cl->dbidplayers.find(b);
		if(i!=p->cl->dbidplayers.end())
		{
			if(i->second->duel!=0)
			{
				p->messagebox("That player is already dueling someone");
			}else
			{
				p->duelchallanged=i->second->dbid;
				i->second->duelchallanger=p;
				i->second->bs->cmd(i->second->getId(), 0x30) << a << b;
			}
		}else
		{
			p->messagebox("Can't find player");
		}
	}
}

void t_guildleadchange(tplayer *p)
{
/*
	int a,b;
	p->sr->get(a);
	p->sr->get(b);
	if((p->dbid==a)&&(p->guild!=0))
	{
		p->changelead(this, a, b);
	}
*/
}


void t_obj_something(tplayer *p)
{
	int a,b;
	p->sr->get(a);
	p->sr->get(b);
	p->bs->cmd(p->getId(), 0xbd) << b << 0 << 0xe31272e5 << -1;
}

void t_wh(tplayer *p)
{
	if((p->trade!=0)||(p->shop.getopened()))return;
	if(p->guild!=0)p->guild->showwh(p);
}

void t_entergw(tplayer *p)
{
	if((p->trade!=0)||(p->shop.getopened()))return;
	p->changemap(202, 1366, 105, 1280);
}

void t_gwstatus(tplayer *p)
{
	if(p->cl->getclusternumber()!=0)return;
	if(p->cl->getsiegestate()!=cluster::apply)
	{
		p->messagebox("Can't see status now");
		return;
	}
    std::lock_guard<std::mutex> guard(dbguildsiege_mutex);
	sqlquery &s1=dbguildsiege;

	s1.select("count(*)");
	if(!s1.next())return;
	int n=toInt(s1[0]);
	s1.freeup();

	std::string str1;

	p->bs->cmd(p->getId(), 0xb8) << (char)4;
	*p->bs << 0x0006d9a8 << 0;
	*p->bs << 0 << n;
	s1.selectw("guildid>=0 order by money asc");
	while(s1.next())
	{
		str1=" "+s1["name"]+" " + s1["money"];
		p->bs->sndpstr(str1);
	}
	int yourguild=0;
	if(p->guild!=0)yourguild=p->guild->getsiegemoney();
	*p->bs << yourguild;
	p->greentext("teszt");
}

void t_gwapply(tplayer *p)
{
	char a;
	int b;
	if(p->cl->getclusternumber()!=0)return;
	if(p->cl->getsiegestate()!=cluster::apply)
	{
		p->messagebox("Can't apply to siege now");
		return;
	}
	if(p->guild!=0)
	{
		if(p->dbid==p->guild->getowner())
		{
			p->sr->get(a);
			if(a==2)
			{
				p->sr->get(b);
				p->guild->applysiege(b);
			}
			p->bs->cmd(p->getId(), 0xb8) << (char)1;
			*p->bs << 0 << 0 << p->guild->getsiegemoney() << 0;
		}else
		{
			p->messagebox("Only the leader can apply");
		}
	}
}

void t_setgwspawn(tplayer *p)
{
	int a;
	p->sr->get(a);
	if((a>=0)&&(a<4))p->gwspawn=a;
	else if(a==99)p->gwspawn=rnd(4);
	if(p->requested_teleportdown)
	{
		p->requested_teleportdown=false;
		if(p->guild!=0)
		{
			p->guild->player_teleportdown(p, p->gwspawn);
		}
	}
}

void initprochandlers()
{
	int a;
	for(a=0;a<65536;a++)
	{
		t_0000[a]=0;
		t_00ff[a]=0;
		t_f000[a]=0;
		t_ffff[a]=0;
	}

	t_0000[0x001a]=t_sendmail;
	t_0000[0x001b]=t_deletemail;
	t_0000[0x001c]=t_getitemfrommail;
	t_0000[0x001d]=t_checkmail;
	t_0000[0x001f]=t_getmoneyfrommail;
	t_0000[0x0020]=t_townblink;
	t_0000[0x0024]=t_markmail;
	t_0000[0xff00]=t_loadingprocess;
	t_0000[0x0f00]=t_obj_something;


	t_00ff[0x0010]=t_attack;	//str
	t_00ff[0x0011]=t_attack2;	//int
	t_00ff[0x0012]=t_attack3;	//dex
	t_00ff[0x0014]=t_attack;	//fly
	t_00ff[0x0020]=t_skilluse;
	t_00ff[0x00d5]=t_as;
	t_00ff[0x0000]=t_chat;
	t_00ff[0x000b]=t_unequip;
	t_00ff[0x0021]=t_equip;
	t_00ff[0x0023]=t_focus;
	t_00ff[0x0006]=t_moveitem;
	t_00ff[0x0007]=t_dropitem;
	t_00ff[0x0016]=t_sitdown;
	t_00ff[0x0019]=t_deleteitem;
	t_00ff[0x00b0]=t_npcchat;
	t_00ff[0x00b1]=t_npcshop;
	t_00ff[0x00b3]=t_npcbuy;
	t_00ff[0x00b4]=t_npcsell;
	t_00ff[0x00ee]=t_changeface;
	t_00ff[0x00af]=t_haircolor;
	t_00ff[0x0025]=t_blinkpool;
	t_00ff[0x00c0]=t_originalplace;
	t_00ff[0x00c1]=t_reanimate;
	t_00ff[0x00c2]=t_lodeligth;
	t_00ff[0x00c3]=t_marklodeligth;
	t_00ff[0x00c4]=t_statset;
	t_00ff[0x00a7]=t_sendtraderequest;
	t_00ff[0x00a1]=t_tradeinsertitem;
	t_00ff[0x00a3]=t_tradesetok;
	t_00ff[0x00a4]=t_tradeend;
	t_00ff[0x002f]=t_tradesetok2;
	t_00ff[0x00a5]=t_tradesetmoney;
	t_00ff[0x00a0]=t_tradeagree;
	t_00ff[0x00a9]=t_openshop;
	t_00ff[0x00aa]=t_closeshop;
	t_00ff[0x00ab]=t_itemaddtoshop;
	t_00ff[0x00ac]=t_lookinshop;
	t_00ff[0x00ad]=t_buyfromshop;
	t_00ff[0x00ae]=t_itemremovefromshop;
	t_00ff[0x0026]=t_removequest;
	t_00ff[0x0018]=t_minimapping;
	t_00ff[0x0ffd]=t_createcloack;


	t_f000[0xd008]=t_piercing1;
	t_f000[0xb025]=t_piercing2;
	t_f000[0xf114]=t_piercing2;
	t_f000[0xb024]=t_upgrade;
	t_f000[0xf113]=t_upgrade;
	t_f000[0xb055]=t_talkinshop;
	t_f000[0xb010]=t_guildlogo;
	t_f000[0xb012]=t_guildnotice;
	t_f000[0xb032]=t_guildname;
	t_f000[0xb027]=t_guildsetpay;
	t_f000[0xb026]=t_guildsetrigths;
	t_f000[0xb036]=t_guildwar;
	t_f000[0xb037]=t_acceptwar;
	t_f000[0xb047]=t_giveupwar;
	t_f000[0xb048]=t_guildpiece;
	t_f000[0xb021]=t_invtowh;
	t_f000[0xb022]=t_whtoinv;
	t_f000[0xb011]=t_moneytoguild;
	t_f000[0xf000]=t_guildleadchange;
	t_f000[0xb020]=t_wh;
	t_f000[0xd024]=t_entergw;
	t_f000[0xd022]=t_gwapply;
	t_f000[0xd028]=t_gwstatus;
	t_f000[0xd02b]=t_setgwspawn;


	t_ffff[0xff00]=t_movetopoint;
	t_ffff[0xff07]=t_follow;
	t_ffff[0xff0a]=t_removefromSslot;
	t_ffff[0xff0b]=t_addtoSslot;
	t_ffff[0xff0c]=t_addtoFslot;
	t_ffff[0xff0d]=t_removefromFslot;
	t_ffff[0xff0e]=t_addtoAslot;
	t_ffff[0xff40]=t_bank_asszem;
	t_ffff[0xff42]=t_itemtobank;
	t_ffff[0xff43]=t_moneytobank;
	t_ffff[0xff44]=t_banktoinv;
	t_ffff[0xff45]=t_moneyfrombank;
	t_ffff[0xff49]=t_banktobank;
	t_ffff[0xff1b]=t_usepartyskill;
	t_ffff[0xff01]=t_wasd;
	t_ffff[0xff02]=t_motion;
//	t_ffff[0xff05]=t_blockedmove;
	t_ffff[0xff72]=t_reachedp;
	t_ffff[0xff29]=t_flyturn;
	t_ffff[0xff06]=t_flyturn06;
	t_ffff[0xff03]=t_fly;
	t_ffff[0xff60]=t_addfriendbyid;
	t_ffff[0xff61]=t_asktobefriend;
	t_ffff[0xff62]=t_refusefriend;
	t_ffff[0xff6a]=t_deletefriend;
	t_ffff[0xff6b]=t_addfriendbyname;
	t_ffff[0xff67]=t_setmsgstate;
	t_ffff[0xff7c]=t_docheer;
	t_ffff[0xff11]=t_partyjoin;
	t_ffff[0xff12]=t_partyleave;
	t_ffff[0xff17]=t_partyinvite;
	t_ffff[0xff19]=t_partyname;
	t_ffff[0xff20]=t_partysetitem;
	t_ffff[0xff21]=t_partysetexp;
	t_ffff[0xff2f]=t_partymemberassign;
	t_ffff[0xff32]=t_guilddisband;
	t_ffff[0xff34]=t_guildleave;
	t_ffff[0xff35]=t_guildinvite;
	t_ffff[0xff3a]=t_guildpromotemember;
	t_ffff[0xff74]=t_guildclassup;
	t_ffff[0xff33]=t_guildjoin;
	t_ffff[0xff78]=t_ressed;
	t_ffff[0xff75]=t_createnick;
	t_ffff[0xff79]=t_guildnemtom;
	t_ffff[0xff08]=t_fixmobplace;
	t_ffff[0xff23]=t_duelask;
	t_ffff[0xff24]=t_duelagree;
}

void tplayer::teleportf(const char *t)
{
	if((trade!=0)||(shop.getopened()))return;
	canceldelayed();
	int x=(int)pos.x,y=(int)pos.y,z=(int)pos.z,m=mapid;
	sscanf(t, "%d %d %d %d", &x, &y, &z, &m);
	changemap(m,(float)x,(float)y,(float)z);
}

void tplayer::teleportf2(const char *t)
{
	if(accesslevel!=255)return;
	if((trade!=0)||(shop.getopened()))return;
	canceldelayed();
	float x=pos.x,z=pos.z;
	int m=mapid;
	sscanf(t, "%d %f %f", &m, &x, &z);
	changemap(m,x,pos.y,z);
}

void tplayer::movetopoint()
{
	if(shop.getopened())return;
	if(trade!=0)trade->del(this);
	trade=0;
	canceldelayed();
	float x,y,z;
	sr->forward(2);
	sr->get(x);
	sr->get(y);
	sr->get(z);
	state=1;
	state2=0;
	if(!teleportmode)
	{
		domove();
		if(this->isMoveing()==2)movein(vector3d<>(x,z,y)-pos);
		else moveto(x,y,z);
		buffer bs;
		gotoxyz(tomulticast2, id, x,y,z);
	}else{
		changemap(mapid, x,y,z);
	}

}

void tplayer::spawnitem(const char *t)
{
	int id,num=1,up=0,eletype=0,eup=0;
	sscanf(t, "%d %d %d %d %d", &id, &num, &up, &eletype, &eup);
	if(itemlist.at(id).id>0)
	{
		item targy(id);
		targy.num=num;
		targy.upgrade=up;
		targy.element=eletype;
		targy.eupgrade=eup;
		new character_item(targy, cl, mapid, pos.x, pos.y, pos.z);
	}
}

void tplayer::spawnmob(const char *t)
{
	int tp=21,a,n=1, ag=0;
	float size=1;
	sscanf(t, "%d %d %f %d", &tp, &n, &size, &ag);
	if(monsterlist.at(tp).id>0)
		for(a=0;a<n;a++)
			new character_mob(tp, cl, mapid, pos.x, pos.y, pos.z, ag!=0, size);
}


int tid=3000;
void tplayer::spawnnpc(const char *t)
{
/*
	int a=21, b=1, c, d=8, e;
//	sscanf(t, "%d %d %d", &a, &b, &d, &t2[0]);
	sscanf_s(t, "%d", &a);
	int cc=0;
	for(cc=0;(t[cc]!=32)&&(t[cc]!=0);cc++);
	if(t[cc]==32)cc++;
	buffer bs;
	for(c=0;c<b;c++)
	{
		bs.cmd(tid, 0xf0);
		bs << (char)5 << a << (char)5 << (short)a << (short)100;
		bs << (float)(pos.x + c) << pos.y << pos.z;
		bs << (short)0 << tid;
		bs << (short)0 << (char)0 << 1 << 1 << 0 << 1 << 0 << 0 << (short)0 << (char)0;
		bs.sndpstr(&t[cc]);
		for(e=0;e<d;e++)bs << (char)0;
		tid++;
	}
	gridcell->multicast2(bs);
*/
}


void tplayer::spawnobj(const char *t)
{
	int tp=21,s=100, b=1;
	sscanf_s(t, "%d %d", &tp, &s);
	character_obj *p=new character_obj(tp, cl, mapid, pos.x, pos.y, pos.z, 0, s);
}

void tplayer::spawnboxobj(const char *t)
{
	int tp=21,i=21, b=1;
	sscanf_s(t, "%d %d", &tp, &i);
	character_obj *p=new character_obj(tp, cl, mapid, pos.x, pos.y, pos.z, 0, 100, new openanddrop(i));
}

void tplayer::slashcommands(const char *t)
{
	if((t[1]=='s')&&(t[2]==' ')&&(t[3]!=0))
	{

		buffer bs;
		bs.cmd(-1, 0xd0);
		bs << id;
		bs.sndpstr(name);
		bs.sndpstr(&t[3]);
		cl->multicast(bs);
	}else if((t[1]=='p')&&(t[2]==' ')&&(t[3]!=0))
	{
		if(party!=0)party->partychat(this, name.c_str(), dbid, &t[3]);
	}else if((t[1]=='g')&&(t[2]==' ')&&(t[3]!=0))
	{
		if(guild!=0)guild->guildchat(this, guildslot, dbid, name, &t[3]);
	}else if((t[1]=='w')&&(t[2]==' '))
	{
		pwhisper(&t[3]);
	}else if(sc(&t[1], "say "))
	{
		psay(&t[5]);
	}else if(sc(&t[1], "partyinvite "))
	{
		std::string s=&t[13];
		ppartyinvite(s);
	}else if(sc(&t[1], "teleport "))
	{
		teleportf2(&t[10]);
	}else
	{
		tomulticast->cmd(id, 1);
		tomulticast->sndpstr(t);
	}
}

void tplayer::chatcommands(const char *t)
{
/*
	if(sc(&t[1], "wh"))
	{
		if((trade!=0)||(shop.getopened()))return;
		if(guild!=0)guild->showwh(this);
	}
*/
	if(sc(&t[1], "setkarma "))
	{
		int a=0;
		sscanf_s(&t[10], "%d", &a);
		pk=a;
		tomulticast->cmd(id, 0x28) << pk << pvp;
	}
	if(sc(&t[1], "cancelskill"))
	{
		this->cancelskill();
	}
	if(sc(&t[1], "str ")){
		int a;
		a = 0;
		sscanf_s(&t[5], "%d", &a);
		statset(a,0,0,0);
	}
	if(sc(&t[1], "sta ")){
		int a;
		a = 0;
		sscanf_s(&t[5], "%d", &a);
		statset(0,a,0,0);
	}
	if(sc(&t[1], "dex ")){
		int a;
		a = 0;
		sscanf_s(&t[5], "%d", &a);
		statset(0,0,a,0);
	}
	if(sc(&t[1], "int ")){
		int a;
		a = 0;
		sscanf_s(&t[5], "%d", &a);
		statset(0,0,0,a);
	}

	if(accesslevel<255)return;

	int a,b,c, d, e, f;
	float fa,fb,fc;

//	buffer *bs1;

	if(sc(&t[1], "item "))spawnitem(&t[6]);
	else if(sc(&t[1], "teleportmodeon"))teleportmode=true;
	else if(sc(&t[1], "teleportmodeoff"))teleportmode=false;
	else if(sc(&t[1], "teleport "))teleportf(&t[10]);
	else if(sc(&t[1], "mob "))spawnmob(&t[5]);
	else if(sc(&t[1], "obj "))spawnobj(&t[5]);
	else if(sc(&t[1], "box "))spawnboxobj(&t[5]);
	else if(sc(&t[1], "pos"))greentext(cl->print("%f %f %f %d", pos.x, pos.y, pos.z, mapid));
	else if(sc(&t[1], "mute "))
	{
		a=1;
		b=0;
		sscanf_s(&t[6], "%d %d", &a, &b);
		b*=1000*60*60;
		std::map<int, tplayer*>::iterator i=cl->dbidplayers.find(a);
		if(i!=cl->dbidplayers.end())
		{
			if(i->second->accesslevel<accesslevel)i->second->muted=cl->ido+b;
		}else
		{
			greentext("not found");
		}
	}
	else if(sc(&t[1], "unmute "))
	{
		a=1;
		b=0;
		sscanf_s(&t[8], "%d %d", &a, &b);
		b*=1000*60*60;
		std::map<int, tplayer*>::iterator i=cl->dbidplayers.find(a);
		if(i!=cl->dbidplayers.end())
		{
			if(i->second->accesslevel<accesslevel)i->second->muted=0;
		}else
		{
			greentext("not found");
		}
	}else if(sc(&t[1], "name2dbid "))
	{
		std::map<std::string, tplayer*>::iterator i=cl->nameplayers.find(&t[11]);
		if(i!=cl->nameplayers.end())
		{
			greentext(cl->print("%d", i->second->dbid));
		}else
		{
			greentext("not found");
		}
	}else if(sc(&t[1], "addpartyexp "))
	{
		a=0;
		sscanf_s(&t[13], "%d", &a);
		if(party!=0)
		{
			party->addexp(a, 0, 0);
		}
	}else if(sc(&t[1], "droprate ")){
		float a=0;
		sscanf_s(&t[10], "%f", &a);
		dropmult = a;
		cl->notice2(cl->print("%fx drop rate",dropmult));
	}else if(sc(&t[1], "skillexp "))
	{
		fa=(float)skillExpMultiplier;
		sscanf_s(&t[10], "%f", &fa);
		skillExpMultiplier=fa;
		cl->notice2(cl->print("%fx skillexp", (float)skillExpMultiplier));
	}else if(sc(&t[1], "partyexp "))
	{
		a=partyexpszorzo;
		sscanf_s(&t[9], "%d", &a);
		partyexpszorzo=a;
		cl->notice2(cl->print("%fx partyexp", (float)partyexpszorzo));
	}else if(sc(&t[1], "exp "))
	{
		fa=(float)expMultiplier;
		sscanf_s(&t[5], "%f", &fa);
		expMultiplier=fa;
		cl->notice2(cl->print("%fx exp", (float)expMultiplier));
	}else if(sc(&t[1], "penyarate "))
	{
		fa=(float)moneymultiplier;
		sscanf_s(&t[11], "%f", &fa);
		moneymultiplier=fa;
		cl->notice2(cl->print("%fx penya rate", (float)moneymultiplier));
	}else if(sc(&t[1], "level "))
	{
		a=level;
		sscanf_s(&t[7], "%d", &a);
		exp=0;
//		exp2=0;
//		pxp=0;
		level=a;
		setlevelexp();
		clearsp();
		HMF();
		hp=maxhp;

	}


	if(sc(&t[1], "teszt28 "))
	{
		if(getfocus()!=0)
		{
			bs->cmd(getfocus()->getId(), 0x5f) << pos.x << pos.y << pos.z << 0 << 0 << 0;
			bs->cmd(getfocus()->getId(), 0x41) << pos.x << pos.y << pos.z << -1;
		}
	}

	if(sc(&t[1], "teszt29 "))
	{
		if(getfocus()!=0)
		{
			bs->cmd(getfocus()->getId(), 0x41) << pos.x << pos.y << pos.z << -1;
		}
	}

	if(sc(&t[1], "teszt30 "))
	{
		bs->cmd(id, 0xb8) << (char)52;
		bs->sndpstr(&t[8]);

	}else if(sc(&t[1], "teszt31 "))
	{
		sscanf_s(&t[9], "%d %d %d", &a, &b, &c);

		bs->cmd(id, 0xb8) << (char)a << b << c;

	}else if(sc(&t[1], "teszt32 "))
	{
		bs->cmd(id, 0xb8) << (char)0x43;
		*bs << 0x5004 << 1 << 0x0d << -1 << 1 << 0 << 0x0280e0f2 << 0x01c9f56e;

	}else if(sc(&t[1], "teszt33 "))
	{
		sscanf_s(&t[9], "%d %x", &a, &b);

		bs->cmd(id, 0xb8) << (char)a << b;

	}else if(sc(&t[1], "teszt34 "))
	{
		sscanf_s(&t[9], "%d %x %x", &a, &b, &c);

		bs->cmd(id, 0xb8) << (char)a << b << c;

	}else if(sc(&t[1], "teszt35 "))	//individual ranks
	{
		bs->cmd(-1, 0xb8) << (char)33;
		*bs << 4;

		*bs << 1;
		bs->sndpstr("ow hai");
		*bs << 9;

		*bs << 1;
		bs->sndpstr("sup");
		*bs << 8;

		*bs << 1;
		bs->sndpstr("nothing");
		*bs << 7;

		*bs << 1;
		bs->sndpstr("kk");
		*bs << 6;

	}else if(sc(&t[1], "teszt36 "))	//hp on the left
	{
		bs->cmd(-1, 0xb8) << (char)32;
		*bs << 0;
		*bs << 2 << 81 << 6 << 86 << 6;
	}else if(sc(&t[1], "teszt37 "))	//guild ranks
	{
		bs->cmd(-1, 0xb8) << (char)34;
		*bs << 4;
		*bs << 2 << 81 << 6 << 86 << 6;
		*bs << 2 << 81 << 6 << 86 << 6;
		*bs << 2 << 81 << 6 << 86 << 6;
		*bs << 2 << 81 << 6 << 86 << 6;

	}else if(sc(&t[1], "teszt38 "))	//guild members (hp and stuff (top left)) NOT. maybe your name?
	{
		bs->cmd(-1, 0xb8) << (char)17;
		*bs << 2;
		*bs << 81 << 1 << 81 << 1;
		*bs << 86 << 1 << 86 << 2;

	}else if(sc(&t[1], "teszt80 "))
	{

		sscanf_s(&t[9], "%d", &a);
		bs->cmd(-1, 0xb2);
		for(b=0;b<a;b++)*bs << (char)0;
		*bs << (char)1;
		for(b=a+1;b<1024;b++)*bs << (char)0;

//	}else if(sc(&t[1], "teszt200 "))
//	{
//		mysql_close(cl->connections.at(5));
//		cl->connections.at(5)=mysql_init(0);
	}else if(sc(&t[1], "teszt72 "))
	{
		if(getfocus()!=0)
		{
			skilleffect(bs, getfocus()->getId(), getfocus()->getId(), 224, 20);
		}
	}else if(sc(&t[1], "teszt71 "))
	{
		HMF();
	}else if(sc(&t[1], "lineup "))
	{
		std::string str1;
		str1=&t[8];
		if(guild!=0)if(guild->addtolineup(this, str1))greentext("Lined up"); else greentext("Can't add to lineup");
	}else if(sc(&t[1], "opensiege"))
	{
		if(cl->getclusternumber()==0)
		{
			if(cl->getsiegestate()==cluster::tsnone)
			{
				cl->setsiegestate(cluster::apply);
				greentext("Siege application opened");
			}
		}
	}else if(sc(&t[1], "closesiege"))
	{
		if(cl->getclusternumber()==0)
		{
			if(cl->getsiegestate()==cluster::apply)
			{
				cl->cancelsiege(-1);
				cl->setsiegestate(cluster::tsnone);
				greentext("Siege closed");
			}
		}
	}else if(sc(&t[1], "startsiege"))
	{
		if(cl->getclusternumber()==0)
		{
			if(cl->getsiegestate()==cluster::apply)
			{
				cl->setsiegestate(cluster::prepare);
				greentext("Siege preparation started");
			}
		}
	}else if(sc(&t[1], "job "))
	{
		a=0;
		sscanf_s(&t[5], "%d", &a);
		if(a>15)a=0;
		job=a;
		jobchange();
	}else if(sc(&t[1], "restat"))restat();
	else if(sc(&t[1], "flushguilds"))tguild::saveallguilds();
	else if(sc(&t[1], "flush"))save();
	else if(sc(&t[1], "loadnpc"))
	{
		a=cl->loadnpces();
		logger.log("npces\n", a);
	}else if(sc(&t[1], "npc "))spawnnpc(&t[5]);
	else if(sc(&t[1], "loadmob"))
	{
		a=cl->loadmobspawns();
		logger.log("mobspawns\n", a);

	}else if(sc(&t[1], "search "))itemidsearch(*this, &t[8]);
	else if(sc(&t[1], "searchmob "))mobidsearch(*this, &t[11]);
	else if(sc(&t[1], "bank"))
	{
		bs->cmd(id, 0x58);
		*bs << 1 << -1 << 0;
	}else if(sc(&t[1], "id"))
	{
		if(getfocus()==0)greentext(cl->print("ID=%d", id));
		else greentext(cl->print("ID=%d", getfocus()->getId()));
	}else if(sc(&t[1], "dbid"))
	{
		greentext(cl->print("DBID=%d", dbid));
	}else if(sc(&t[1], "music "))
	{
		sscanf_s(&t[7], "%d", &a);
		music(bs, a);
		logger.log("MUSIC\n");
	}else if(sc(&t[1], "speed"))
	{
		if(getfocus()==0)greentext(cl->print("speed=%f bspeed=%f\n", (float)speed, (float)basespeed));
		else greentext(cl->print("speed=%f bspeed=%f\n", (float)getfocus()->getspeed(), (float)getfocus()->getbasespeed()));
	}else if(sc(&t[1], "summon "))
	{
		sscanf_s(&t[8], "%d", &a);
		std::map<int, tplayer*>::iterator i;
		i=cl->dbidplayers.find(a);
		if(i!=cl->dbidplayers.end())
		{
			i->second->changemap(mapid, pos.x, pos.y, pos.z);
		}

	}else if(sc(&t[1], "inccheer"))
	{
		ncheers++;
		bs->cmd(id, 0xb4) << ncheers << 0;
	}else if(sc(&t[1], "effekt "))
	{
		sscanf_s(&t[8], "%d", &a);
		effect(bs, id, a);
	}else if(sc(&t[1], "notice "))
	{
		cl->notice2(&t[8]);
	}else if(sc(&t[1], "teszt43 "))
	{
		a=state3;
		sscanf_s(&t[9], "%d", &a);
		state3=a;
		if(sp!=0)*bsstate3=a;
	}else if(sc(&t[1], "teszt42 "))
	{
		a=state2;
		sscanf_s(&t[9], "%x", &a);
		state2=a;
		if(sp!=0)*bsstate2=a;
	}else if(sc(&t[1], "teszt41 "))
	{
		a=state;
		sscanf_s(&t[9], "%x", &a);
		state=(unsigned)a;
		if(sp!=0)*bsstate=a;
	}else if(sc(&t[1], "teszt44 "))
	{
		a=state4;
		sscanf_s(&t[9], "%d", &a);
		state4=a;
		if(sp!=0)*bsstate4=a;
	}else if(sc(&t[1], "teszt61 "))
	{
		sscanf_s(&t[9], "%x %d %d %d", &a, &b, &c, &d);
		bs->cmd(id, a) << b << (short)c << d;
	}else if(sc(&t[1], "dfs "))
	{
		if(getfocus()!=0)
			if(getfocus()->type==ttnpc)
			{
				b=-1;
				sscanf_s(&t[5], "%d %d", &b, &a);
				((character_npc*)getfocus())->delfromshop(a, b-1);
			}

	}else if(sc(&t[1], "cls "))
	{
		if(getfocus()!=0)
			if(getfocus()->type==ttnpc)
			{
				b=-1;
				sscanf_s(&t[5], "%d", &b);
				((character_npc*)getfocus())->clearshop(b-1);
			}
	}else if(sc(&t[1], "ats2 "))
	{
		if(getfocus()!=0)
			if(getfocus()->type==ttnpc)
			{
				f=10000;
				sscanf_s(&t[6], "%d %d %d %d %d %d", &b, &a, &c, &d, &e, &f);
				b--;
				if((b>=0)&&(b<=3))
				{
						sqlquery &s1=cl->dbitemlist;
						if(e==1)
						{
							s1.selectw("id<"+toString(f)+" and itemjob="+toString(a) + " AND limitlevel1>=" + toString(c) + " AND limitlevel1 <=" + toString(d) + " and parts=10", "id");
						}else
						{
							s1.selectw("id<"+toString(f)+" and itemjob="+toString(a) + " AND limitlevel1>=" + toString(c) + " AND limitlevel1 <=" + toString(d) + " and parts<9", "id");
						}
						while(s1.next())
						{
							a=toInt(s1[0]);
							if(a>0)
								((character_npc*)getfocus())->addtoshop(a, b);
						}
						s1.freeup();
				}
			}
	}else if(sc(&t[1], "ats0 "))
	{
		if(getfocus()!=0)
			if(getfocus()->type==ttnpc)
			{
				f=10000;
				sscanf_s(&t[6], "%d %d %d %d %d %d", &b, &a, &c, &d, &e, &f);
				b--;
				if((b>=0)&&(b<=3))
				{
						sqlquery &s1=cl->dbitemlist;
						if(e==1)
						{
							s1.selectw("id<"+toString(f)+" and itemjob="+toString(a) + " AND limitlevel1>=" + toString(c) + " AND limitlevel1 <=" + toString(d) + " and parts=10 and id%2=0", "id");
						}else
						{
							s1.selectw("id<"+toString(f)+" and itemjob="+toString(a) + " AND limitlevel1>=" + toString(c) + " AND limitlevel1 <=" + toString(d) + " and parts<9 and id%2=0", "id");
						}
						while(s1.next())
						{
							a=toInt(s1[0]);
							if(a>0)
								((character_npc*)getfocus())->addtoshop(a, b);
						}
						s1.freeup();
				}
			}
	}else if(sc(&t[1], "ats1 "))
	{
		if(getfocus()!=0)
			if(getfocus()->type==ttnpc)
			{
				f=10000;
				sscanf_s(&t[6], "%d %d %d %d %d %d", &b, &a, &c, &d, &e, &f);
				b--;
				if((b>=0)&&(b<=3))
				{
						sqlquery &s1=cl->dbitemlist;
						if(e==1)
						{
							s1.selectw("id<"+toString(f)+" and itemjob="+toString(a) + " AND limitlevel1>=" + toString(c) + " AND limitlevel1 <=" + toString(d) + " and parts=10 and id%2=1", "id");
						}else
						{
							s1.selectw("id<"+toString(f)+" and itemjob="+toString(a) + " AND limitlevel1>=" + toString(c) + " AND limitlevel1 <=" + toString(d) + " and parts<9 and id%2=1", "id");
						}
						while(s1.next())
						{
							a=toInt(s1[0]);
							if(a>0)
								((character_npc*)getfocus())->addtoshop(a, b);
						}
						s1.freeup();
				}
			}
	}else if(sc(&t[1], "ats "))
	{
		b=1;
		a=0;
		sscanf_s(&t[5], "%d %d", &b, &a);
		if(a<1)
		{
			greentext("Wrong item id");
		}else if((b<1)||(b>4))
		{
			greentext("Wrong shop slot");
		}else if(getfocus()==0)
		{
			greentext("No npc selected");
		}else if(getfocus()->type!=ttnpc)
		{
			greentext("No npc selected");
		}else
		{
			((character_npc*)getfocus())->addtoshop(a, b-1);
		}
	}else if(sc(&t[1], "atsn "))
	{
		b=1;
		sscanf_s(&t[6], "%d", &b);
		a=6;
		while(t[a]>32)a++;
		if(t[a]!=0)
		{
			cl->dbitemlist.selectw("name like '"+toString(&t[a])+"'");
			c=0;
			std::string str1;
			if(cl->dbitemlist.next())
			{
				str1=cl->dbitemlist["name"];
				a=toInt(cl->dbitemlist["id"]);
				c++;
			}
			while(cl->dbitemlist.next())
			{
				greentext(cl->print("%d %s", a, str1.c_str()));
				str1=cl->dbitemlist["name"];
				a=toInt(cl->dbitemlist["id"]);
				c++;
			}
			if(c>1)
			{
				greentext(cl->print("%d %s", a, str1.c_str()));
				a=-1;
			}

			cl->dbitemlist.freeup();
		}else
		{
			greentext("No itemname");
			a=-1;
		}

		if(a<1)
		{
			greentext("Wrong item id");
		}else if((b<1)||(b>4))
		{
			greentext("Wrong shop slot");
		}else if(getfocus()==0)
		{
			greentext("No npc selected");
		}else if(getfocus()->type!=ttnpc)
		{
			greentext("No npc selected");
		}else
		{
			((character_npc*)getfocus())->addtoshop(a, b-1);
		}
	}else if(sc(&t[1], "effect "))
	{
		sscanf_s(&t[8], "%d", &a);
		effect(bs, id, a);
	}else if(sc(&t[1], "effect2 "))
	{
		sscanf_s(&t[9], "%d", &a);
		effectxyz(bs, id, a, pos.x, pos.y, pos.y);
	}else if(sc(&t[1], "teszt1 "))
	{
		effectxyz(bs, id, 65, pos.x, pos.y, pos.z);
	}else if(sc(&t[1], "teszt2 "))
	{
		if(getfocus()!=0)
 		{
			greentext(cl->print("%f", distance(*getfocus())));
			effectxyz(bs, id, 65, getfocus()->getx(), getfocus()->gety(), getfocus()->getz());
			effectxyz(bs, id, 63, getfocus()->getcel().x, getfocus()->getcel().y, getfocus()->getcel().z);
			effectxyz(bs, id, 64, getfocus()->getstart().x, getfocus()->getstart().y, getfocus()->getstart().z);
		}else
		{
			effectxyz(bs, id, 65, pos.x, pos.y, pos.z);
		}
	}else if(sc(&t[1], "teszt4 "))
	{
		bs->cmd(id, 0xca) << pos.x << pos.y << pos.z << 0 << 0 << 0 << 0x432bff18 << 1 << 1 << 0 << -1 << 4 << 0 << 0x0301e8ea << 0x01c9e417;
	}else if(sc(&t[1], "addstat "))
	{
		c=0x7fffffff;
		sscanf_s(&t[9], "%d %d %x", &a, &b, &c);
		addstat(tomulticast, id, a, b, c);
	}else if(sc(&t[1], "substat "))
	{
		sscanf_s(&t[9], "%d %d", &a, &b);
		substat(tomulticast, id, a, b);
	}else if(sc(&t[1], "addstat1 "))
	{
		sscanf_s(&t[9], "%d %x", &a, &b);
		addstat1(a, b);
	}else if(sc(&t[1], "substat1 "))
	{
		sscanf_s(&t[9], "%d %x", &a, &b);
		addstat1(a, -b, true);
	}else if(sc(&t[1], "maxskills")){
		maxskillexp();
	}else if(sc(&t[1], "invisible")||sc(&t[1], "inv")){
		gm_di = true;
		addbuff(id,193,20,10000000);
	}else if(sc(&t[1], "noinvisible")||sc(&t[1], "noinv")){
		gm_di = false;
		removebuff(193);
	}else if(sc(&t[1], "spd")){
		addbuff(id,195,20,100000000);
	}else if(sc(&t[1], "nospd")){
		removebuff(195);
	}else if(sc(&t[1], "teszt21"))
	{
		bs->cmd(id, 0xdf) << 0 << (char)1;
	}else if(sc(&t[1], "teszt22"))
	{
		bs->cmd(id, 0xdf) << 4 << (char)0 << 0x12c6;
	}else if(sc(&t[1], "teszt3 "))
	{
		sscanf_s(&t[8], "%f %f %f", &fa, &fb, &fc);
		if(getfocus()!=0)
		{
			bs->cmd(getfocus()->getId(), 0x5f) << pos.x << pos.y << pos.z;
			*bs << fa << fb << fc;

		}
	greentext("teszt3");

	}else if(sc(&t[1], "duel"))
	{
		if(getfocus()!=0)
		{
			if(getfocus()->type==ttplayer)
			{
				if(((tplayer*)getfocus())->duel==0)
				{
					((tplayer*)getfocus())->duel=this;
					((tplayer*)getfocus())->bs->cmd(getfocus()->getId(), 0x30) << dbid << ((tplayer*)getfocus())->dbid;
				}
			}
		}
	}else if(sc(&t[1], "teszt4 "))
	{
		sscanf_s(&t[8], "%x %f", &a, &fa);
		bs->cmd(id, 0x26) << fa << a;
	}else if(sc(&t[1], "nplayers "))
	{
		messagebox(cl->print("%d", cl->nplayers));

	}else if(sc(&t[1], "teszt31 "))
	{
		if(getfocus()!=0)
		{
			sscanf_s(&t[9], "%d %d", &a , &b);
			bs->cmd(getfocus()->getId(), 0x28) << a << b;
		}
	}else if(sc(&t[1], "setpstat "))
	{
		sscanf_s(&t[10], "%d %d %d %d", &str , &sta, &dex, &intl);
		bs->cmd(id, 0x6a) << str << sta << dex << intl << 0 << statpoints;

	}else if(sc(&t[1], "money "))
	{
		sscanf_s(&t[7], "%d", &a);
		money=a;
		setstat(bs, id, 79, a);
	}else if(sc(&t[1], "teszt5 "))
	{
		greentext(cl->print("maxhp=%d, atk=%d, def=%d\n", maxhp, (atkmax+atkmin)/2, (defmin+defmax)/2));
	}else if(sc(&t[1], "teszt6 "))
	{
		sscanf_s(&t[8], "%d %d", &a, &b);
		bs->cmd(-1, 0x41) << 52;
		bs->sndpstr("teszt22");
		*bs << 0x0001f000 << 3 << 0;
		greentext("teszt1");

	}else if(sc(&t[1], "teszt8 "))
	{
		sscanf_s(&t[8], "%d %d", &a, &b);
		bs1->cmd(id, 0x96);
		(*bs1) << a << b;
	}else if(sc(&t[1], "name"))
	{
		if(getfocus()!=0)
		{
			switch(getfocus()->type)
			{
			case character_base::ttnpc:
				greentext(monsterlist[getfocus()->getmodelid()].name.c_str());
				greentext(((character_npc*)getfocus())->name.c_str());
				break;
			case character_base::ttplayer:
				greentext(monsterlist[getfocus()->getmodelid()].name.c_str());
				greentext(((tplayer*)getfocus())->name.c_str());
				break;
			case character_base::ttmob:
				greentext(monsterlist[getfocus()->getmodelid()].name.c_str());
				break;
			}
		}
	}else if(sc(&t[1], "teszt7 "))
	{
		d=1;
		e=2;
		int c2;
		int tt=-1;
		sscanf_s(&t[8], "%x %x %x %x %d %d %x", &a, &b, &c, &c2, &d, &e, &tt);
		tomulticast->cmd(id, 0xca);//cb
		*tomulticast << pos.x << pos.y << pos.z << 0 << 0 << 0;
		*tomulticast << 0 << a << b << c << c2 << d << e << t << 0;
	}else if(sc(&t[1], "teszt71 "))
	{
		sscanf_s(&t[9], "%d %f", &a, &fa);
		bs->cmd(id, 0xdc) << a << fa;
	}else if(sc(&t[1], "attack "))
	{
		if(getfocus()!=0)
		{
			a=0x1d;
			b=0;
			c=0x00010000;
			sscanf_s(&t[8], "%d %x %x", &a, &b, &c);
			bs->cmd(id, 0xe0) << a << getfocus()->getId() << b << c;
		}
	}else if(sc(&t[1], "attack2 "))
	{
		if(getfocus()!=0)
		{
			a=0x1d;
			sscanf_s(&t[9], "%d", &a);
			bs->cmd(id, 0xe2) << a << getfocus()->getId() << 0 << 0 << 0;
		}
	}else if(sc(&t[1], "model "))
	{
		a=11+gender;
		sscanf_s(&t[7], "%d", &a);
		modelid=a;
		tomulticast->cmd(id, 0xf5) << a;

	}else if(sc(&t[1], "tp0 "))
	{
		sscanf_s(&t[5], "%x", &a);
		tomulticast->cmd(id, a);
		greentext("teszt1");
	}else if(sc(&t[1], "tp1 "))
	{
		sscanf_s(&t[5], "%x %d", &a, &b);
		tomulticast->cmd(id, a) << b;
		greentext("teszt1");
	}else if(sc(&t[1], "tp2 "))
	{
		sscanf_s(&t[5], "%x %d %d", &a, &b, &c);
		bs->cmd(id, a) << b << c;
		greentext("teszt1");
	}else if(sc(&t[1], "tp3 "))
	{
		sscanf_s(&t[5], "%x %d %d %d", &a, &b, &c, &d);
		bs->cmd(id, a);
		*bs << b << c << d;
		greentext("teszt1");
	}else if(sc(&t[1], "tp4 "))
	{
		sscanf_s(&t[5], "%x %d %d %d %d", &a, &b, &c, &d, &e);
		bs->cmd(id, a);
		*bs << b << c << d << e;
		greentext("teszt1");
	}else if(sc(&t[1], "tp5 "))
	{
		sscanf_s(&t[5], "%x %d %d %d %d %d", &a, &b, &c, &d, &e, &f);
		bs->cmd(id, a);
		*bs << b << c << d << e << f;
		greentext("teszt1");
	}else if(sc(&t[1], "dp0 "))
	{
		sscanf_s(&t[5], "%x", &a);
		bs->cmd(dbid, a);
		greentext("teszt1");
	}else if(sc(&t[1], "dp1 "))
	{
		sscanf_s(&t[5], "%x %d", &a, &b);
		bs->cmd(dbid, a);
		*bs << b;
		greentext("teszt1");
	}else if(sc(&t[1], "dp2 "))
	{
		sscanf_s(&t[5], "%x %d %d", &a, &b, &c);
		bs->cmd(dbid, a);
		*bs << b << c;
		greentext("teszt1");
	}else if(sc(&t[1], "dp3 "))
	{
		sscanf_s(&t[5], "%x %d %d %d", &a, &b, &c, &d);
		bs->cmd(dbid, a);
		*bs << b << c << d;
		greentext("teszt1");
	}else if(sc(&t[1], "dp4 "))
	{
		sscanf_s(&t[5], "%x %d %d %d %d", &a, &b, &c, &d, &e);
		bs->cmd(dbid, a);
		*bs << b << c << d << e;
		greentext("teszt1");
	}else if(sc(&t[1], "dp5 "))
	{
		sscanf_s(&t[5], "%x %d %d %d %d %d", &a, &b, &c, &d, &e, &f);
		bs->cmd(dbid, a);
		*bs << b << c << d << e << f;
		greentext("teszt1");
	}else if(sc(&t[1], "op0 "))
	{
		sscanf_s(&t[5], "%x", &a);
		*ooo << a;
		raiseooo();
	}else if(sc(&t[1], "op1 "))
	{
		sscanf_s(&t[5], "%x %d", &a, &b);
		*ooo << a << b;
		raiseooo();
	}else if(sc(&t[1], "op2 "))
	{
		sscanf_s(&t[5], "%x %d %d", &a, &b, &c);
		*ooo << a << b << c;
		raiseooo();
	}else if(sc(&t[1], "op3 "))
	{
		sscanf_s(&t[5], "%x %d %d %d", &a, &b, &c, &d);
		*ooo << a << b << c << d;
		raiseooo();
	}else if(sc(&t[1], "op4 "))
	{
		sscanf_s(&t[5], "%x %d %d %d %d", &a, &b, &c, &d, &e);
		*ooo << a << b << c << d << e;
		raiseooo();
	}else if(sc(&t[1], "op5 "))
	{
		sscanf_s(&t[5], "%x %d %d %d %d %d", &a, &b, &c, &d, &e, &f);
		*ooo << a << b << c << d << e << f;
		raiseooo();
	}else if(sc(&t[1], "tpn "))
	{
		c=0;
		sscanf_s(&t[5], "%x %d %d", &a, &b, &c);
		bs->cmd(id, a);
		for(a=0;a<b;a++)*bs << c;
		greentext("teszt1");
	}else if(sc(&t[1], "tpnc "))
	{
		c=0;
		sscanf_s(&t[6], "%x %d %d", &a, &b, &c);
		bs->cmd(id, a);
		for(a=0;a<b;a++)*bs << (char)c;
		greentext("teszt1");
	}else if(sc(&t[1], "tpxyzn "))
	{
		c=0;
		sscanf_s(&t[8], "%x %d %d", &a, &b, &c);
		bs->cmd(id, a);
		*bs << pos.x << pos.y << pos.z;
		for(a=0;a<b;a++)*bs << c;
		greentext("teszt1");
	}else if(sc(&t[1], "tpxyznc "))
	{
		c=0;
		sscanf_s(&t[9], "%x %d %d", &a, &b, &c);
		bs->cmd(id, a);
		*bs << pos.x << pos.y << pos.z;
		for(a=0;a<b;a++)*bs << (char)c;
		greentext("teszt1");
	}else if(sc(&t[1], "tq "))
	{
		b=strlen(&t[4]);
		if(b>1)
		{
			*bs << id;
			bs->inc();
			for(a=0;a<b;a+=3)
			{
				sscanf_s(&t[a+4], " %02x", &c);
				*bs << (unsigned char)c;
			}
		}

	}else if(sc(&t[1], "tw "))
	{
		b=strlen(&t[4]);
		if(b>1)
		{
//			*bs << id;
			bs->inc();
			for(a=0;a<b;a+=3)
			{
				sscanf_s(&t[a+4], " %02x", &c);
				*bs << (unsigned char)c;
			}
		}

	}else if(sc(&t[1], "teszt10 "))
	{
		a=0;
		sscanf_s(&t[9], "%d", &a);
		bs->cmd(id, 0x7b) << (char)1;
		for(b=0;b<a;b++)*bs << (char)0;
		greentext("teszt1");
	}else if(sc(&t[1], "teszt12 "))
	{
		bs->cmd(id, 0x82) << dbid << 0;
		greentext("teszt1");
	}else if(sc(&t[1], "teszt13 "))
	{
		a=0;
		b=2;
		sscanf_s(&t[9], "%d %d", &a, &b);
/*
		bs->cmd(id, 0x82) << dbid << 0;

*/
		bs->cmd(id, 0x82) << dbid;// << 0 << 0;
//		bs->sndpstr(name);
//		bs->sndpstr(name);
		*bs << b;
		*bs << 0 << 0 << b << 2 << 2 << 3 << 1 << 1 << 0 << 0 << 0 << 0 << 0;
//		*bs << 0x01ad << 0 << b << 1 << 2 << 3 << 1 << 2 << 0 << 0 << 0 << 0 << 0;
//		*bs << 0x01ad << 0 << b << level << exp << points << expmode << itemmode << 0 << 0 << 0 << 0 << 0;

		*bs << 0 << 10 << 11 << 0 << (char)0;
		bs->sndpstr("Hello");
		*bs << 1 << 10 << 11 << 0 << (char)0;
		bs->sndpstr("Hello2");
//		*bs << (char)0;

//		np=1 => b=52
//-------------------
//		c=1 => 21
		for(b=0;b<a;b++)*bs << (char)0;
		greentext("teszt1");

	}else if(sc(&t[1], "teszt14 "))
	{
		a=0;
		b=1;
		sscanf_s(&t[9], "%d %d", &a, &b);
		bs->cmd(dbid, 0x83);
/*
			bs->cmd(dbid, 0x83);
			*bs << id << level << job << (char)1;
			*bs << id << level << job << (char)1;
			bs->sndpstr(name.c_str());
			*bs << 0;
*/
		for(b=0;b<a;b++)*bs << (char)0;
		greentext("teszt1");

	}else if(sc(&t[1], "teszt11 "))
	{
		a=0;
		b=1;
		sscanf_s(&t[9], "%d %d", &a, &b);
/*
		bs->cmd(id, 0x82) << dbid << 0;

*/
		bs->cmd(id, 0x82) << dbid;// << 0 << 0;
//		bs->sndpstr(name);
//		bs->sndpstr(name);
		*bs << b;
		*bs << 0 << 0 << b << 2 << 2 << 3 << 1 << 1 << 0 << 0 << 0 << 0 << 0;
//		*bs << 0x01ad << 0 << b << 1 << 2 << 3 << 1 << 2 << 0 << 0 << 0 << 0 << 0;
//		*bs << 0x01ad << 0 << b << level << exp << points << expmode << itemmode << 0 << 0 << 0 << 0 << 0;


//		np=1 => b=52
//-------------------
//		c=1 => 21
		for(b=0;b<a;b++)*bs << (char)0;
		greentext("teszt1");
//	}else if(sc(&t[1], "guildlvlup"))
//	{
//		if(guild!=0)guild->levelup(bs, this, dbid);
	}else if(sc(&t[1], "guildcreate"))
	{
		if(guild==0)
		{
			guild=new tguild(this, dbid, level, job, name, id);
			logger.log("Guild created\n");
		}

//		getdata(*bs, dbid);
//		greentext("teszt1");
	}else if(sc(&t[1], "teszt16 "))
	{
		tomulticast->cmd(id, 0x9c) << dbid << dbid;
		tomulticast->sndpstr(name);
		*tomulticast << 0;
//		greentext("teszt1");

	}else if(sc(&t[1], "teszt17 "))
	{
		sscanf_s(&t[9], "%d", &a);
		tomulticast->cmd(id, 0x9b) << a;
//		greentext("teszt1");

	}else if(sc(&t[1], "teszt18 "))
	{

	}else if(sc(&t[1], "duelteszt1"))
	{
		tplayer *p;
		logger.log("duelteszt1\n");
		if(getfocus()!=0)
		{
			logger.log("t1\n");
			if(getfocus()->type==character_base::ttplayer)
			{
				logger.log("t2\n");
				p=(tplayer*)getfocus();
				bs->cmd(id, 0x31);
				*bs << p->dbid << 1;
				bs->cmd(id, 0x31);
				*bs << dbid << 1;

//				bs->cmd(p->id, 0xc2);
//				*bs << id << 0x40000000;
				bs->cmd(id, 0x67);
				*bs << 3;
				bs->cmd(id, 0x67);
				*bs << 2;
				bs->cmd(id, 0x67);
				*bs << 1;
				bs->cmd(id, 0x31);
				*bs << p->dbid << 0;
				bs->cmd(id, 0x31);
				*bs << dbid << 0;
				bs->cmd(id, 0x67);
				*bs << 0;
//				p->bs->cmd(id, 0xc2);
//				*p->bs << p->id << 0x40000000;
			}
		}
	}else if(sc(&t[1], "duelteszt2"))
	{
		tplayer *p;
		logger.log("duelteszt2\n");
		if(getfocus()!=0)
		{
			logger.log("t1\n");
			if(getfocus()->type==character_base::ttplayer)
			{
				logger.log("t2\n");
				p=(tplayer*)getfocus();
				bs->cmd(p->id, 0xc2);
				*bs << p->id << 0x41700000;
			}
		}
	}else if(sc(&t[1], "duelteszt3"))
	{
		tplayer *p;
		logger.log("duelteszt3\n");
		if(getfocus()!=0)
		{
			logger.log("t1\n");
			if(getfocus()->type==character_base::ttplayer)
			{
				logger.log("t2\n");
				p=(tplayer*)getfocus();
				bs->cmd(p->id, 0xc2);
				*bs << p->id << 0x41200000;
			}
		}
	}
}

void tplayer::pchat()
{
	char t[1024];
//	std::string str2;
//	sr->getpstr(str2);
//	if(str2=="")return;
	sr->getpstr(&t[0], 1023);
	if(t[0]==0)return;
//	const char *t=str2.c_str();

	if(t[0]=='>')
	{
		chatcommands(t);

	}else if(t[0]=='/')
	{
		slashcommands(t);
	}else
	{
		tomulticast->cmd(id, 1);
		tomulticast->sndpstr(t);
	}
}


void tplayer::loadingprocess()
{
	if(loadingp)return;
	loadingp=true;
	std::string username, password;
	int b,c;
	int s4;
	int cluster;

	sr->get(b);
	sr->get(pos.x); //x
	sr->get(pos.y); //y
	sr->get(pos.z); //z
	sr->get(s4);
	sr->get(b);
	sr->get(b);
	sr->get(b);
	sr->get(b);

	sr->get(cluster);
	sr->get(b);
	sr->get(c);
	for(int a=0;a<c;a++)
	{
		sr->get(b);
		sr->get(b);
	}
	sr->get(b);

	dbid=s4;

//	sr->getpstr(puffer, 1024);	//charname
//	sr->getpstr(puffer, 1024);
//	username=puffer;
//	this->username=username;
//	sr->getpstr(puffer, 1024);
//	password=puffer;
//	if(!cserver->validate(username, password, cl->getclusternumber()))
//	{
//		errorstate=true;
//		return;
//	}
logger.log("T1 ");
	int lparty=this->lparty;
//	load(lparty);
	if(errorstate)return;

logger.log("T2 ");
//	gridcell=cl->grid.get(mapid, (int)pos.x, (int)pos.z);
	if(guild!=0)if(!guild->contains(this))
	{
		guild=0;
		guildid=0;
	}
logger.log("T3 ");
	cl->dbmails.selectw("recipient="+toString(dbid));
	bool gotmail=(cl->dbmails.next());
	cl->dbmails.freeup();
logger.log("T4 ");
	if(hp<=0)
	{
		hp=1;
		townblinkns();
	}
logger.log("T5 ");
	if(!cl->grid.contains(mapid, pos.x, pos.z))townblinkns();
logger.log("T6 ");
	firstspawn(gotmail);
logger.log("T7 ");
	logincommands();
logger.log("T8 ");
	if(guild!=0)
	{
		if(!guild->login(this, guildslot, dbid, id))
		{
			guild=0;
			guildid=0;
		}
	}
logger.log("T9 ");
	int a;
/*
	bs->cmd(-1, 0xb2);
	*bs << 0x00010100 << 0x00000000 << 0x01010100 << 0x01000101;
	*bs << 0x00000000 << 0x01010000 << 0x00000001 << 0x00000000;
	for(a=32;a<1024;a++)*bs << (char)0;
*/

//	if(cl->isPK())
	{
	bs->cmd(-1, 0xb2);
	if(cl->isPK())a=0x01010100;
	else a=0x00010100;
	*bs << 0x00010100 << 0x00000000 << a          << 0x01000101;
	*bs << 0x00000000 << 0x01010000 << 0x00000001 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;

	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;

	*bs << 0x01000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;

	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	*bs << 0x00000000 << 0x00000000 << 0x00000000 << 0x00000000;
	}
logger.log("T10 ");
	addstat1type=2;
	for(b=inventory::eqoff2;b<inventory::nitems;b++)
		if(inv.getMaskedItem(b)!=0)
			inv.getMaskedItem(b)->addbonus(this);
logger.log("T11 ");
	inv.checkset();
	addstat1type=0;
//	silent=false;
	setgp();
logger.log("T12 ");
try{
	sendbdataatlogin(this->bs);
}catch(...)
{
	clearbuffs();
	raise();
}
logger.log("T13 ");
	HMF();
logger.log("T14 ");

//	gridcell->addplayer(this);
//	gridcell->update(gridcell, this, (int)pos.x, (int)pos.z);
logger.log("T15 ");
	if(!gridcell.add())townblink();
logger.log("T16 ");
	if(lparty>-1)
	{
		party=tparty::getparty(lparty);
		if(party!=0)
		{
			party=party->login(this);
		}
	}
logger.log("T17 ");
	if(guild!=0)guild->getwarstatus(this);
logger.log("T18 ");
	if(hp<=0)
	{
		hp=maxhp/3;
		reanimate();
		townblink();
	}
logger.log("T19 ");
	cl->addplayeract(this, playeractremover);
	addedact=true;
logger.log("T20\n");
}


void tplayer::npcbuy()
{
	if((trade!=0)||(shop.getopened()))return;
	short a;
	int b;
	int c;
	std::string str1;
	str1+=(char)34;
	sr->get(a);
	sr->get(a);
	sr->get(b);

	if((b<0)||(b>=(int)itemlist.size()))return;
	if(getfocus()==0)return;
	if(getfocus()->type!=ttnpc)return;
	character_npc *n=(character_npc*)getfocus();
	if(!n->shopcontains(b))return;
	item targy(b);
	if(targy.GetI()!=0)
	{
		c=a*targy.GetI()->cost;
		if(money>=c)
		{
			targy.num=a;
			if(inv.getRemainder(&targy)==0)
			{
				inv.pickup(&targy);
				money-=c;
				setstat(bs, id, 79, money);

				bs->cmd(id, 0x95);
				*bs	<< 0x0274;
				str1+=targy.GetI()->name;
				str1+=(char)34;
				bs->sndpstr(str1);
			}else bs->cmd(id, 0x55);
		}else{
			sztext(0x089f);
		}
	}
}

void tplayer::npcsell()
{
	if((trade!=0)||(shop.getopened()))return;
	int a,c;
	short b;
	a=(unsigned char)sr->get1char();	//inv id
	sr->get(b);	//num
	if(inv.getItem(a)==0)return;
	if((inv.getItem(a)->GetI()==0)||(inv.getItem(a)->getId()<=0))return;
	if(b>inv.getItem(a)->num)b=inv.getItem(a)->num;
	if(b<1)return;
	c=inv.getItem(a)->GetI()->cost*b/4;
	if(c==0)c=1;
	inv.removeitem(a, b);
	money+=c;
	setstat(bs, id, 79, money);
}

void tplayer::unequip()
{
	if((trade!=0)||(shop.getopened()))return;
	int a;
	sr->get(a);
//	sr->get(b);
	if(inv.getItem(a)!=0){if(inv.getItem(a)->getId()>0)inv.unequip(a);}
	else throw error("No item at unequip", "character_player::unequip");
}

void tplayer::equip()
{
	if((trade!=0)||(shop.getopened()))return;
	unsigned short a;
//	int b;
	sr->get(a);
	sr->get(a);

	if(inv.getItem(a)!=0)
	{
			if(inv.getItem(a)->getId()>0)
			{
				if(inv.getLookup(a)>=inventory::eqoff2)
				{
					inv.unequip(a);
					return;
				}

				if(inv.getItem(a)->GetI()!=0)
				if(inv.getItem(a)->GetI()->eqslot!=-1)
				{
					if(inv.getItem(a)->GetI()->eqslot!=13)inv.equip(a);
					else
					{
						setdelayed(da_fly, 3000);
						daflyitem=a;
					}
				}
				else
				{
					if(inv.getItem(a)!=0)
						if(inv.getItem(a)->getId()>0)
							if(inv.getItem(a)->itemuse(this))inv.removeitem(a, 1);
				}
			}
	}else throw error(string("No item at ")+toString(a), "character_player::equip");

}

void tplayer::moveitem()
{
	if((trade!=0)||(shop.getopened()))return;
	sr->forward(1);
	unsigned char a,b;
	sr->get(a);
	sr->get(b);
//	logger.log("moveitem: %d %d", (unsigned int)a, (unsigned int)b);
	if(a!=b)inv.moveitem(b,a);
}

void tplayer::deleteitem()
{
	if((trade!=0)||(shop.getopened()))return;
	int a;
	short b;
	sr->get(a);
	sr->get(b);
	if(b<1)return;
	if(inv.getItem(a)==0)return;
	if(inv.getItem(a)->getId()==0)return;
	inv.removeitem(a,b);
}

void tplayer::dropitem()
{
	if((trade!=0)||(shop.getopened()))return;
	int a;
	short b;
	item targy;
	sr->forward(4);
	sr->get(a);
	sr->get(b);

	if(inv.getItem(a)!=0)
	{
		if(inv.getItem(a)->getId()>0)
		{
			targy=*inv.getItem(a);
			if(b>targy.num)b=targy.num;
			if(b<1)return;
			targy.num=b;
			if((b>0)&&(targy.getId()>0))
			{
				new character_item(targy, cl, mapid, pos.x, pos.y, pos.z);
				inv.removeitem(a,b);
			}
		}
	}
}

void tplayer::pfollow()
{
	buffer bs1;
	int a;
	float range;
	sr->get(a);
	sr->get(range);
if(!flying)
{
	character_base *f=0;
	if(cl->playerinrange(a))f=cl->getplayer(a);
	else if(cl->mobinrange(a))f=cl->getmob(a);
	else if(cl->npcinrange(a))f=cl->getnpc(a);
	if(f!=0)setfollowed(f, range);
	else
	{
		if(cl->iteminrange(a))
		{
			character_item *targyh=cl->getitem(a);
			if(targyh!=0)
			{
				if(distanceny(*targyh)<8*8)pickup(targyh);
				else setfollowed(targyh, range);
			}
		}
		else if(cl->objinrange(a))
		{
			f=cl->getobj(a);
			if(f!=0)
			{
				if(distance(*f)<8)((character_obj*)f)->action(this);
				else setfollowed(f, range);
			}
		}

	}
}

}
void tplayer::psetfocus()
{
	int a,b;
	sr->get(a);
	b=sr->get1char();
	character_base *f=0, *oldfocus=getfocus();
	if(cl->playerinrange(a))f=cl->getplayer(a);
	else if(cl->mobinrange(a))f=cl->getmob(a);
	else if(cl->npcinrange(a))f=cl->getnpc(a);
	setfocus(f);
	if((f!=oldfocus)&&(oldfocus!=0))
	{
		if(oldfocus->type==ttmob)
		{
			if(((character_mob*)oldfocus)->playerLock==this)
			{
				((character_mob*)oldfocus)->playerLock=0;
				((character_mob*)oldfocus)->partyLock=0;
			}
		}
	}
	if((party!=0)&&(oldfocus!=getfocus()))party->setplayerfocus(this, f);
#ifdef _DEBUG
#ifdef _LOGALOT
	logger.log("focus=%p\n", f);
#endif
#endif
}

int tesztval=0;

void tplayer::wasd()
{
#ifdef _DEBUG
#ifdef _LOGALOT
	logger.log("wasd\n");
#endif
#endif
	if(shop.getopened()||flying)return;
	if(trade!=0)trade->del(this);
	trade=0;
	buffer bs2;
	float fx,fy,fz,fx2,fy2,fz2,d;
	int a,state,sp,state2;
	int b=sr->getposition();
	sr->get(fx);
	sr->get(fy);
	sr->get(fz);
	sr->get(fx2);
	sr->get(fy2);
	sr->get(fz2);
	if((fx2!=0)||(fz2!=0))	canceldelayed();
	sr->get(d);
	sr->get(state);
	sr->get(state2);
	sr->get(sp);
	if(this->sp!=0)
	{
		*bsstate=state;
		*bsstate2=state2;
	}
//	printf("%f %f %f\n%f %f %f %f\n",fx,fy,fz,fx2,fy2,fz2,d);
	dir=(int)(d*10.0);
	this->state=state;
	this->state2=state2;
//	player.speed=sp;
#ifdef _DEBUG
#ifdef _LOGALOT
	logger.log("state=%08x\n", state);
	logger.log("tp: %f %f %f %d\n", fx, fy, fz, (int)cl->ido);
#endif
#endif
	if(((fx2==0)/*&&(fy2==0)*/&&(fz2==0))||((state&0xff00)!=0))
	{
//		logger.log("WASDSTOPMOVE\n");
		stopmove();
//		pos.x=fx;
//		pos.y=fy;
//		pos.z=fz;
		setandcheckpos(fx,fy,fz);

	}else{
//		movein(fx2*10, fy2*10, fz2*10);
//		domove();
		setandcheckpos(fx,fy,fz);
		movein(vector3d<>(fx2*10, fy2*10, fz2*10));
	}

//	if(state==4)gotoxyz(&bs2, id, pos.x+fx2*10000, pos.y+fy2*10000, pos.z+fz2*10000);
//	else
	{
		bs2.cmd(id, 0xca);//cb
		bs2 << fx << fy << fz << fx2 << fy2 << fz2;
		bs2 << d << state << state2 << sp;
		for(b=sr->getposition();b<sr->length();b++)bs2 << sr->get1char();
/*
		for(b=0;b<5;b++)
		{
			sr->get(a);
			bs2 << a;
		}
*/
	}
//	bs2.cmd(id, 1);
//	bs2.sndpstr(toString(tesztval));
//	logger.log("TESZT: %d\n", tesztval);
//	tesztval++;
	tomulticast2->add(bs2);

}

void tplayer::motion()
{
#ifdef _DEBUG
#ifdef _LOGALOT
	logger.log("motion\n");
#endif
#endif
	if(shop.getopened()||flying)return;
	if(trade!=0)trade->del(this);
	trade=0;
	buffer bs2;
	canceldelayed();
	float fx,fy,fz,fx2,fy2,fz2,d;
	int a,b,state,state2,sp;
	sr->get(fx);
	sr->get(fy);
	sr->get(fz);
	sr->get(fx2);
	sr->get(fy2);
	sr->get(fz2);
	sr->get(d);
	sr->get(state);
	sr->get(state2);
	sr->get(sp);
	this->state=state;
	speed=basespeed+basespeed*stats[DST_SPEED]*0.01f;
	if(speed<0)speed=0;
	if(state2>1)speed/=3.0f;
	stance=state2;
	if(this->sp!=0)
	{
		*bsstate=state;
		*bsstate2=state2;
	}
//	player.speed=sp;
#ifdef _DEBUG
#ifdef _LOGALOT
	logger.log("%f %f %f\n%f %f %f %f\n",fx,fy,fz,fx2,fy2,fz2,d);
#endif
#endif
	dir=(int)(d*10.0);
	if((fx2==0)&&(fy2==0)&&(fz2==0))
	{
		stopmove();
//		pos.x=fx;
//		pos.y=fy;
//		pos.z=fz;
		setandcheckpos(fx,fy,fz);

	}else{
		setandcheckpos(fx,fy,fz);
		movein(vector3d<>(fx2*10, fy2*10, fz2*10));
	}

	bs2.cmd(id, 0xca);
	bs2 << fx << fy << fz << fx2 << fy2 << fz2;
	bs2 << d << state << state2 << sp;
	for(b=sr->getposition();b<sr->length();b++)bs2 << sr->get1char();
/*
	for(b=0;b<5;b++)
	{
		sr->get(a);
		bs2 << a;
	}
*/
	tomulticast2->add(bs2);
}

void tplayer::flyturn()
{
#ifdef _DEBUG
#ifdef _LOGALOT
	logger.log("flyturn\n");
#endif
#endif
	if((shop.getopened())||!flying)return;
	if(trade!=0)trade->del(this);
	trade=0;
	buffer bs2;
	float fx,fy,fz,fx2,fy2,fz2,d,vert,sp;
	int a,n1;//,n3,m1,n4,state,state2;
	int b=sr->getposition();
	sr->get(fx);
	sr->get(fy);
	sr->get(fz);
	sr->get(fx2);
	sr->get(fy2);
	sr->get(fz2);
	sr->get(d);
	sr->get(vert);
	sr->get(sp);
	sr->get(n1);
//	sr->get(state);
//	sr->get(state2);
//	sr->get(n3);
//	sr->get(m1);
//	sr->get(n4);
#ifdef _DEBUG
#ifdef _LOGALOT
	logger.log("%f %f %f\n%f %f %f %f %f %f %f\n",fx,fy,fz,fx2,fy2,fz2,d, vert, sp);
#endif
#endif
	dir=(int)(d*10.0);
//	this->state=state;
//	this->state2=state2;
	float vert1=vert;
	vert=cos(vert/p180);
	speed=sp*flyspeed*vert;
	if(speed<0)speed=0;
#ifdef _DEBUG
#ifdef _LOGALOT
	logger.log("%f %f %f %f %f\n", vert, fx2, fy2, fz2, speed);
#endif
#endif
	if((speed==0)/*&&(isMoveing())*/)
	{
		stopmove();
//		pos.x=fx;
//		pos.y=fy;
//		pos.z=fz;
		setandcheckpos(fx,fy,fz);

	}else if(speed!=0)
	{
		if((fx2==0)&&(fy2==0)&&(fz2==0))
		{
			fx2=cos(d/p180);
			fz2=sin(d/p180);
		}
//		domove();
//		movein(vector3d<>(fx2*10, fy2*10, fz2*10));
		setandcheckpos(fx,fy,fz);
		movein(vector3d<>(fx2*10, fy2*10, fz2*10));
	}
	bs2.cmd(id, 0xce);
	bs2 << fx << fy << fz << fx2 << fy2 << fz2;
	bs2 << d << vert1 << sp << n1;
	for(b=sr->getposition();b<sr->length();b++)bs2 << sr->get1char();
/*
	for(b=0;b<2;b++)
	{
		sr->get(a);
		bs2 << a;
	}
*/
	tomulticast2->add(bs2);
}

void tplayer::flyturn06()
{
#ifdef _DEBUG
#ifdef _LOGALOT
	logger.log("flyturn06\n");
#endif
#endif
	if(shop.getopened()||!flying)return;
	if(trade!=0)trade->del(this);
	trade=0;
	buffer bs2;
	float fx,fy,fz,fx2,fy2,fz2,d,vert,sp;
	int a,state,n1,state2;//,n3,m1,n4;
	int b=sr->getposition();
	sr->get(fx);
	sr->get(fy);
	sr->get(fz);
	sr->get(fx2);
	sr->get(fy2);
	sr->get(fz2);
	sr->get(d);
	sr->get(vert);
	sr->get(sp);
	sr->get(n1);
	sr->get(state);
	sr->get(state2);
	if(this->sp!=0)
	{
		*bsstate=state;
		*bsstate2=state2;
	}
	dir=(int)(d*10.0);
	this->state=state;
	this->state2=state2;
	float vert1=vert;
	vert=cos(vert/p180);
	speed=sp*flyspeed*vert;
	if(speed<0)speed=0;
#ifdef _DEBUG
#ifdef _LOGALOT
	logger.log("%f %f %f %f %f\n", vert, fx2, fy2, fz2, speed);
#endif
#endif
	if((speed==0)/*&&(isMoveing())*/)
	{
		stopmove();
		pos.x=fx;
		pos.y=fy;
		pos.z=fz;

	}else if(speed!=0)
	{
		if((fx2==0)&&(fy2==0)&&(fz2==0))
		{
			fx2=cos(d/p180);
			fz2=sin(d/p180);
		}
//		domove();
		setandcheckpos(fx,fy,fz);
		movein(vector3d<>(fx2*10, fy2*10, fz2*10));
	}

	bs2.cmd(id, 0xc9);
	bs2 << fx << fy << fz << fx2 << fy2 << fz2;
	bs2 << d << vert1 << sp << n1 << state << state2;
	for(b=sr->getposition();b<sr->length();b++)bs2 << sr->get1char();
/*
	for(b=0;b<6;b++)
	{
		sr->get(a);
		bs2 << a;
	}
*/
	tomulticast2->add(bs2);
}

void tplayer::fly()
{
#ifdef _DEBUG
#ifdef _LOGALOT
	logger.log("fly\n");
#endif
#endif
	if(shop.getopened()||!flying)return;
	if(trade!=0)trade->del(this);
	trade=0;
	buffer bs2;
	float fx,fy,fz,fx2,fy2,fz2,d,vert,sp;
	int a,state,n1,state2;//,n3,m1,n4;
	int b=sr->getposition();
	sr->get(fx);
	sr->get(fy);
	sr->get(fz);
	sr->get(fx2);
	sr->get(fy2);
	sr->get(fz2);
	sr->get(d);
	sr->get(vert);
	sr->get(sp);
	sr->get(n1);
	sr->get(state);
	sr->get(state2);
	if(this->sp!=0)
	{
		*bsstate=state;
		*bsstate2=state2;
	}
	dir=(int)(d*10.0);
	this->state=state;
	this->state2=state2;
	float vert1=vert;
	vert=cos(vert/p180);
	speed=sp*flyspeed*vert;
	if(speed<0)speed=0;
#ifdef _DEBUG
#ifdef _LOGALOT
	logger.log("%f %f %f %f %f\n", vert, fx2, fy2, fz2, speed);
#endif
#endif
	if((speed==0)/*&&(isMoveing())*/)
	{
		stopmove();
		pos.x=fx;
		pos.y=fy;
		pos.z=fz;

	}else if(speed!=0)
	{
		if((fx2==0)&&(fy2==0)&&(fz2==0))
		{
			fx2=cos(d/p180);
			fz2=sin(d/p180);
		}
//		domove();
		setandcheckpos(fx,fy,fz);
		movein(vector3d<>(fx2*10, fy2*10, fz2*10));
	}

	bs2.cmd(id, 0xcc);
	bs2 << fx << fy << fz << fx2 << fy2 << fz2;
	bs2 << d << vert1 << sp << n1 << state << state2;
	for(b=sr->getposition();b<sr->length();b++)bs2 << sr->get1char();
/*
	for(b=0;b<6;b++)
	{
		sr->get(a);
		bs2 << a;
	}

	a= sr->get1char();
	bs2 << (unsigned char)(unsigned int)a;
*/
	tomulticast2->add(bs2);
}

void itemidsearch(tplayer &player, const char *p)
{
	sqlquery &s1=player.cl->dbitemlist;
	std::string str="enabled != 0 and name like '%";
	str+=p;
	str+="%'";
	s1.selectw(str, "id, name");
	while(s1.next())player.greentext(player.cl->print("%s %s", s1[0].c_str(), s1[1].c_str()));
	s1.freeup();
}

void mobidsearch(tplayer &player, const char *p)
{
	sqlquery &s1=player.cl->dbmonsterlist;
	std::string str="name like '%";
	str+=p;
	str+="%'";
	s1.selectw(str, "id, name");
	while(s1.next())player.greentext(player.cl->print("%s %s", s1[0].c_str(), s1[1].c_str()));
	s1.freeup();
}

void tplayer::addtoFslot()
{
	save_slots=true;
	std::string str1;
	char t[1025];
	int a,b,c,d,e;
	a=(unsigned char)sr->get1char();
	b=(unsigned char)sr->get1char();
	sr->get(c);	//type
	sr->get(d);
	sr->get(e);	//type2
	if(c==8)
	{
		sr->forward(3*4);
		sr->getpstr(t, 1024);
		fslot[a][b].set(t);
	}else
	{
		fslot[a][b].set(d,c,e);
//		d=inv.getLookup(d);
//		logger.log("%d\n", d);
	}
}

void tplayer::addtoSslot()
{
	save_slots=true;
	std::string str1;
	char t[1025];
	int a,c,d,e;
	a=(unsigned char)sr->get1char();
	sr->get(c);	//type
	sr->get(d);
	sr->get(e);	//type2
	if(c==8)
	{
		sr->forward(3*4);
		sr->getpstr(t, 1024);
		sslot[a].set(t);
	}else
	{
		sslot[a].set(d,c,e);
	}
}

void tplayer::addtoAslot()
{
	save_slots=true;
	int a,b,c,d,e,f;
	sr->get(a);
	for(b=0;b<5;b++)aslot[b].set(0,0,0);
	if(a>5)throw error(std::string("Aslot number is too high: ") + toString(a), "character_player::addtoAslot");
	for(b=0;b<a;b++)
	{
        c=(unsigned char)sr->get1char();
		sr->get(d);
		sr->get(e);
		sr->get(f);
		aslot[c].set(e,d,f);
		sr->forward(3*4);
	}
}

void tplayer::removefromFslot()
{
	save_slots=true;
	int a,b;
	a=(unsigned char)sr->get1char();
	b=(unsigned char)sr->get1char();
	fslot[a][b].set(0,0,0);
}

void tplayer::removefromSslot()
{
	save_slots=true;
	int a;
	a=(unsigned char)sr->get1char();
	sslot[a].set(0,0,0);
}

enum tupgradeval {NONE=0, SUCCESS=1, FAIL=2};

tupgradeval eupgrade(int e, item *t)
{
	tupgradeval retval=NONE;
	if((t->element==0)||(t->element==e))
	{

		t->element=e;
		if(t->eupgrade>=10)retval=NONE;
		else{
			if(rnd(100)<upgradeChances[t->eupgrade])
//			if(1==1)
			{
				t->eupgrade++;
				retval=SUCCESS;
			}else
			{
				retval=FAIL;
//				t->eupgrade--;
//				if(rnd(100)<50)t->eupgrade--;
//				if(t->eupgrade<0)t->eupgrade=0;
			}
		}
	}
	return retval;
}


void tplayer::upgradeitem()
{
	if(last_upgrade_time>=cl->ido)return;
	last_upgrade_time = cl->ido+5000;

	if((trade!=0)||(shop.getopened()))return;
	int a,b,c=10,d;

	tupgradeval found=NONE;
	item *t;
			sr->get(a);
			sr->get(b);
			if((inv.getItem(a)==0)||(inv.getItem(b)==0))return;
			t=inv.getItem(a);
			switch(inv.getItem(b)->getId())
			{
			case 3205: //touch card
				found=eupgrade(1, t);
				break;
			case 3206: //flame card
				found=eupgrade(1, t);
				break;
			case 3210: //lake card
				found=eupgrade(2, t);
				break;
			case 3211: //river card
				found=eupgrade(2, t);
				break;
			case 3215: //voltage card
				found=eupgrade(3, t);
				break;
			case 3216: //generator card
				found=eupgrade(3, t);
				break;
			case 3220: //stone card
				found=eupgrade(5, t);
				break;
			case 3221: //desert card
				found=eupgrade(5, t);
				break;
			case 3225: //gale card
				found=eupgrade(4, t);
				break;
			case 3226: //cyclon card
				found=eupgrade(4, t);
				break;

			case 2034:     //shining oriaculum
				if(t->upgrade<10){
					if(rnd(100)<upgradeChancesUlt[t->upgrade])
					{
						found=SUCCESS;
						t->upgrade++;
					}else
					{
						found=FAIL;
//						t->upgrade--;
//						if(rnd(100)<50)t->upgrade--;
//						if(t->upgrade<0)t->upgrade=0;
					}
					if((t->upgrade>=6)&&(t->slots[t->upgrade-6]==65535))t->slots[t->upgrade-6]=0;
				}else
				{
					found=NONE;
				}
				break;

			case 2083:		//event moonstone
			case 2036:		//moonstone
				switch(t->getId())
				{
				case 26565:
				case 26452:
					c=5;
					d=upgradeChances[t->upgrade];
					break;
				default:
					c=20;
					d=upgradeChancesJewel[t->upgrade];
				}
				if(t->upgrade<c)
				{
					if(rnd(100)<d)
					{
						found=SUCCESS;
						t->upgrade++;
					}else
					{
						found=FAIL;
//						t->upgrade--;
//						if(rnd(100)<50)t->upgrade--;
//						if(t->upgrade<0)t->upgrade=0;
					}
				}else
				{
					found=NONE;
				}
				break;

			case 2082:    //event sunstone
			case 2035:    //sunstone
			case 3229:
			case 3230:
				if(t->upgrade<10)
				{
					if(rnd(100)<upgradeChances[t->upgrade])
					{
						found=SUCCESS;
						t->upgrade++;
					}else
					{
						found=FAIL;
//						t->upgrade--;
//						if(rnd(100)<50)t->upgrade--;
//						if(t->upgrade<0)t->upgrade=0;
					}
				}else
				{
					found=NONE;
				}
				break;
			default:
				found=NONE;
			}
			if(found!=NONE)
			{
				inv.updateitem(a);
				if(found==SUCCESS)
				{
					effect(tomulticast, id, 1714);
					sztext(0x089d);
					sound(tomulticast, 0x0e);
				}else{
					effect(tomulticast, id, 1715);
					sztext(0x089e);
					sound(tomulticast, 0x0d);
				}
				inv.removeitem(b, 1);
			}else messagebox(cl->print("Can't upgrade more than %d times", c));
}

void tplayer::banktobank()
{
	int a,b,c;
	short d;
	int e;
	item targy;
	a=(unsigned char)sr->get1char();
	if(a==1)
	{
		a=(unsigned char)sr->get1char();
		b=(unsigned char)sr->get1char();
		c=(unsigned char)sr->get1char();
		sr->get(d);
	if((a<0)||(a>2)||(b<0)||(b>2)||(b==a))return;
	if(banks[a].getItem(c)==0)
	{
		throw error("inverror", "banktobank");
//		throw error("invalid item at " + toString(b), "banktoinv");
		return;
	}
	if(banks[a].getItem(c)->getId()<=0)throw error("inverror", "banktobank");
	if(banks[a].getItem(c)->GetI()==0)throw error("inverror", "banktobank");

		if(banks[a].getItem(c)==0)return;
		targy=*banks[a].getItem(c);
		if(targy.num<d)d=targy.num;
		if(d<1)return;
		save_bank=true;
		targy.num=d;
		e=banks[b].insert(targy);
		if(d-e>0)banks[a].removeitem(c, d-e);
		if(e==d)bs->cmd(id, 0x55);
	}else if(a==0)
	{
		a=(unsigned char)sr->get1char();
		b=(unsigned char)sr->get1char();
		sr->get(c);
		if((a<0)||(a>2)||(b<0)||(b>2)||(a==b))return;
		if(bankmoney[a]>=c)
		{
			if(bankmoney[a]<c)c=bankmoney[a];
			if(c<1)return;
			save_bank=true;
			bankmoney[a]-=c;
			bankmoney[b]+=c;

			bs->cmd(id, 0x52);
			*bs << (char)a << money << bankmoney[a];
			bs->cmd(id, 0x52);
			*bs << (char)b << money << bankmoney[b];

		}
	}else logger.elog("Unknown banktobank data %d", a);
}

void tplayer::itemtobank()
{
	if((trade!=0)||(shop.getopened()))return;
	int a,b;
	short c;
	int d;
	item targy;
	a=(unsigned char)sr->get1char();	//bank slot
	b=(unsigned char)sr->get1char();	//item
	sr->get(c);	//nitems;
	if((a<0)||(a>2))throw error("Invalid bank slot "+ toString(a) + " ", "itemtobank");
	if(inv.getItem(b)==0)
	{
		throw error("inverror", "itemtobank");
		return;
	}
	if(inv.getItem(b)->getId()<=0)throw error("inverror", "itemtobank");
	if(inv.getItem(b)->GetI()==0)throw error("inverror", "itemtobank");
	targy=*inv.getItem(b);
	if(c>targy.GetI()->num)c=targy.GetI()->num;
	if(c>targy.num)c=targy.num;
	if(c<1)return;
	save_bank=true;
	targy.num=c;
	d=banks[a].insert(targy);
	if(c-d>0)inv.removeitem(b, c-d);
	if(d==c)bs->cmd(id, 0x55);
}

void tplayer::banktoinv()
{
	if((trade!=0)||(shop.getopened()))return;
	int a,b;
	short c;
	int d;
	item targy;
	a=(unsigned char)sr->get1char();	//bank slot
	b=(unsigned char)sr->get1char();	//item
	sr->get(c);	//nitems;

	if((a<0)||(a>2))return;
	if(banks[a].getItem(b)==0)
	{
		throw error("inverror", "banktoinv");
//		throw error("invalid item at " + toString(b), "banktoinv");
		return;
	}
	if(banks[a].getItem(b)->getId()<=0)throw error("inverror", "banktoinv");
	if(banks[a].getItem(b)->GetI()==0)throw error("inverror", "banktoinv");

	targy=*banks[a].getItem(b);
	targy.num=c;
	if(c>targy.GetI()->num)c=targy.GetI()->num;
	save_bank=true;

	d=inv.pickup(&targy);
	if(c-d>0)banks[a].removeitem(b,c-d);
	if(d==c)bs->cmd(id, 0x55);
}

void tplayer::blockedmove()
{
}

void tplayer::skilluse()
{
	if((silenced)||(berserk)||(stun>=cl->ido)||(nextskill>cl->ido)||(actionslotp!=0)||(shop.getopened()))
	{
		cancelskill();
		return;
	}

	short a,b;
	int c,d;
	int as=0;
	int skill, level;
	sr->get(a);
	sr->get(b);
	sr->get(c);
	sr->get(d);
	if((d>0)&&(actionslotp!=0))return;	/*toreenable*/
	if(d>0)as=1;

	skill=skills.at(job).at(b);
	level=skilllevels.at(skill);
	tskilldata *s=&skilllist.at(skill).at(level-1);

	if(s->weapontype!=0)
	{
		if((s->weapontype&sweapontype)==0)
		{
			bs->cmd(id, 0x94)<<632;
			cancelskill();
			return;
		}
	}
	if((s->cooldown>0))
	{
		if(cooldowns.at(skill)>cl->ido)
		{
			cancelskill();
			return;
		}
		else cooldowns.at(skill)=cl->ido+s->cooldown;
	}

	if(managempfp(b))
	{
		if(trade!=0)trade->del(this);
		trade=0;
		setfollowed(0, 0);
		stopmove();
		canceldelayed();
		if(di&&!gm_di)removebuff(193);
		if(doskill(skill, level, c, 0, false, (character_buffable*)getfocus()))
		{
			nextskill=cl->ido+1400;
			incskillexp(b);

			if((actionslotp==0)&&(d>0))actionslotp=new asstruct(a,c,this, (character_buffable*)getfocus(), skilllist.at(skill).at(level-1).astime+100
				, aslot[0].getId(), aslot[1].getId(), aslot[2].getId(), aslot[3].getId(), aslot[4].getId(), job);//skilllist[skill][level-1].val27);
		}
	}
}

void tplayer::blinkpool()
{
	float x,y,z;
	if((!silenced)&&(stun<cl->ido)&&(!shop.getopened()))
	{
		if(managempfp(15))
		{
			if(trade!=0)trade->del(this);
			trade=0;

			setfollowed(0, 0);
			canceldelayed();
			if(di&&!gm_di)removebuff(193);
			sr->get(x);
			sr->get(y);
			sr->get(z);
			changemap(mapid,x,y,z);
			incskillexp(15);  //blinkpool
		}else this->cancelskill();
	}else this->cancelskill();
}

void tplayer::psay(const char *puffer)
{
	std::map<std::string, tplayer*>::iterator i;
	char puffer2[256];
	int a,b,c=0;
	if(puffer[c]==0)return;
    while((puffer[c]==' ')||(puffer[c]==34))c++;
	if(puffer[c]==0)return;
	for(a=c;(puffer[a]!=' ')&&(puffer[a]!=34)&&(puffer[a]!=0);a++);
    for(b=c;b<a;b++)puffer2[b-c]=puffer[b];
    puffer2[b-c]=0;
    while((puffer[a]==' ')||(puffer[a]==34))a++;

	tplayer *p=0;
	bool found;
	int dbid1=-1;
	{
		ul m=globalplayersmutex.lock();
		i=globalnameplayers.find(&puffer2[0]);
		found=(i!=globalnameplayers.end());
		if(found)
		{
			p=i->second;
			dbid1=p->dbid;
		}
	}
	if(!found)saynotconnected();
	else
	{
		if(dbid!=dbid1)
		{
			say(name.c_str(), &puffer2[0], &puffer[a], dbid, dbid1);
			p->say(name.c_str(), &puffer2[0], &puffer[a], dbid, dbid1);
		}
	}
}

void tplayer::pwhisper(const char *puffer)
{
	std::map<std::string, tplayer*>::iterator i;
	char puffer2[256];
	int a,b,c=0;
	if(puffer[c]==0)return;
    while((puffer[c]==' ')||(puffer[c]==34))c++;
	if(puffer[c]==0)return;
	for(a=c;(puffer[a]!=' ')&&(puffer[a]!=34)&&(puffer[a]!=0);a++);
    for(b=c;b<a;b++)puffer2[b-c]=puffer[b];
    puffer2[b-c]=0;
    while((puffer[a]==' ')||(puffer[a]==34))a++;

	bool found;
	tplayer *p=0;
	int dbid1=-1;
	{
		ul m=globalplayersmutex.lock();
		i=globalnameplayers.find(&puffer2[0]);
		found=(i!=globalnameplayers.end());
		if(found)
		{
			p=i->second;
			dbid1=p->dbid;
		}
	}
	if(!found)saynotconnected();
	else
	{
		if(dbid!=dbid1)
		{
			whisper(name.c_str(), &puffer2[0], &puffer[a], dbid, dbid1);
			p->whisper(name.c_str(), &puffer2[0], &puffer[a], dbid, dbid1);
		}
	}
}

void tplayer::addfriendbyid()
{
	int a,b,c,c1,d,d1;
	int fs=friendlist.size();
	sr->get(a);
	sr->get(b);
	std::map<int, tplayer*>::iterator i;
	if(b==dbid)
	{
	    std::lock_guard<std::mutex> guard(this->asyncbuffermutex);
		for(c=0;c<fs;c++)
		{
			if(friendlist[c].dbid==a)return;
			if(friendlist[c].dbid==-1)break;
		}
		for(c1=c;c1<fs;c1++)if(friendlist[c1].dbid==a)return;
		if(c==fs)return;

		i=cl->dbidplayers.find(a);
		if(i==cl->dbidplayers.end())return;

        std::lock_guard<std::mutex> guard2(i->second->asyncbuffermutex);
		for(d=0;d<fs;d++)
		{
			if(i->second->friendlist[d].dbid==b)break;
			if(i->second->friendlist[d].dbid==-1)break;
		}

		if(i->second->friendlist[d].dbid!=b)
			for(d1=d;d1<fs;d1++)
				if(i->second->friendlist[d1].dbid==b)
				{
					d=d1;
					break;
				}
		if(d==fs)return;
		friendlist[c].dbid=i->second->dbid;
		friendlist[c].listindex=d;
		friendlist[c].p=i->second;
		i->second->friendlist[d].dbid=dbid;
		i->second->friendlist[d].listindex=c;
		i->second->friendlist[d].p=this;
		save_friendlist=true;
		i->second->save_friendlist=true;

		bs->cmd(dbid, 0x70);
		*bs << i->second->dbid << i->second->job << (char)0;
		bs->sndpstr(i->second->name);	//name

		i->second->bs->cmd(i->second->dbid, 0x70);
		*i->second->bs << dbid << job << (char)0;
		i->second->bs->sndpstr(name);	//name

		*ooo << 0xffffff67 << i->second->dbid << i->second->msgstate;
		raiseooo();
		*i->second->ooo << 0xffffff67 << dbid << msgstate;
		i->second->raiseooo();

	}
}

void tplayer::asktobefriend()
{
	int a,b;
	sr->get(a);
	sr->get(b);
	std::map<int, tplayer*>::iterator i;
	if(a==dbid)
	{
		i=cl->dbidplayers.find(b);
		if(i!=cl->dbidplayers.end())
		{
			*i->second->ooo << 0xffffff6b << dbid << b << 2 << (char)0;
			i->second->ooo->sndpstr(name);
			i->second->raiseooo();
		}else
		{
			messagebox("Can't find player");
		}
	}
}

void tplayer::refusefriend()
{
	int a,b;
	sr->get(a);
	sr->get(b);
	std::map<int, tplayer*>::iterator i;
	if(b==dbid)
	{
		i=cl->dbidplayers.find(a);
		if(i!=cl->dbidplayers.end())
			i->second->bs->cmd(i->second->dbid, 0x72);
	}
}

void tplayer::deletefriend()
{
	int a,b;
	sr->get(a);
	sr->get(b);
	if(a==dbid)
	{
		for(a=0;a<(int)friendlist.size();a++)
		{
			if(friendlist[a].dbid==b)
			{
				bs->cmd(dbid, 0x75);
				*bs << friendlist[a].dbid;
				friendlist[a].dbid=-1;
				friendlist[a].listindex=-1;
				friendlist[a].p=0;
				save_friendlist=true;
				break;
			}
		}
	}
}

void tplayer::addfriendbyname()
{
	std::string str1;
	char t[1025];
	int a;
	std::map<std::string, tplayer*>::iterator i;
	sr->get(a);
	if(a==dbid)
	{
		sr->getpstr(t, 1024);
		str1=t;
		i=cl->nameplayers.find(str1);
		if(i!=cl->nameplayers.end())
		{
			*i->second->ooo << 0xffffff6b << dbid << i->second->dbid << 2 << (char)0;
			i->second->ooo->sndpstr(name);
			i->second->raiseooo();
		}else
		{
			messagebox("Can't find player");
		}
	}
}

void tplayer::sendtraderequest()
{
	if((trade!=0)||(shop.getopened()))return;
	int a;
	sr->get(a);
	tplayer *p=cl->getplayer(a);
	if(p!=0)
	{
		if(p->shop.getopened())
		{
			messagebox("You can't trade a player with an open shop");
		}else
		{
			if(p->trade==0)p->bs->cmd(id, 0x22);
			else messagebox("That player is tradeing with someone else now");
		}
	}
}

void tplayer::tradeinsertitem()
{
	int b;
	short a,c;
	if((trade!=0)&&(!shop.getopened()))
	{
		sr->get(a);
		b=(unsigned char)sr->get1char();
		sr->get(c);
        trade->setitem(this, a,b,c);
	}
}

void tplayer::tradeagree()
{
	int a,b;
	if((trade!=0)||(shop.getopened()))return;
	sr->get(a); //other player id
	tplayer *p=cl->getplayer(a);
	if(p!=0)
	{
		if((p->trade!=0)||(p->shop.getopened()))return;
		p->bs->cmd(id, 0x07) << id;
		b=0;
		for(a=0;a<73;a++)
		{
			*p->bs << inv.getMask(a);
			if(inv.getItem(a)!=0)
				if(inv.getItem(a)->getId()!=-1)
					b++;
		}

		(*p->bs) << (unsigned char)b;

		for(b=0;b<73;b++)
			if(inv.getItem(b)!=0)
				if(inv.getItem(b)->getId()!=-1)
				{
					*p->bs << (char)b << b;
					inv.getItem(b)->snddata(p->bs);
				}

		for(a=0;a<73;a++)
			*p->bs << inv.getLookup(a);

		bs->cmd(p->id, 0x07) << p->id;
		b=0;
		for(a=0;a<73;a++)
		{
			*bs << p->inv.getMask(a);
			if(p->inv.getItem(a)!=0)
				if(p->inv.getItem(a)->getId()!=-1)
					b++;
		}

		*bs << (unsigned char)b;	//number of items

		for(b=0;b<73;b++)
			if(p->inv.getItem(b)!=0)
				if(p->inv.getItem(b)->getId()!=-1)
				{
					*bs << (char)b << b;
					p->inv.getItem(b)->snddata(bs);
				}

		for(a=0;a<73;a++)
			*bs << p->inv.getLookup(a);

		trade=new tradeclass(this, p, id, p->getId());
		p->trade=trade;
	}

}

void tplayer::openshop()
{
	return;
	if((shop.getopened())||(trade!=0)||(flying))return;
	char t[1025];
	std::string str1;
	sr->getpstr(t, 1024);    //shop name
	shop.open(t);

}

void tplayer::closeshop()
{
	return;
	if(!shop.getopened())return;
	shop.close(this);
}

void tplayer::lookinshop()
{
	return;
	if((trade!=0)||(shop.getopened()))return;
	int a;
	sr->get(a);
	tplayer *p=cl->getplayer(a);
	if(p!=0)
		if(p->shop.getopened())
			p->shop.login(this);
}

void tplayer::itemaddtoshop()
{
	return;
	if(trade==0)
	{
		short a,c;
		int b,d;
		sr->get(a);
		b=(unsigned char)sr->get1char();
		sr->get(c);
		sr->get(d);

		shop.additemtoshop(a, b,c,d);


	}
}

void tplayer::buyfromshop()
{
	return;
	if(trade==0)
	{
		int a,b,c;
		short d;
		sr->get(a);
		b=(unsigned char)sr->get1char();
		sr->get(c);
		sr->get(d);
		tplayer *p=(tplayer*)getfocus();
		if(p!=0)
		{
			if(p->getId()!=a)return;
			if(p->shop.getopened())p->shop.buyfromshop(b, this, c, d);
		}
	}
}

void tplayer::talkinshop()
{
	return;
	if(joinedshop!=0)
	{
		std::string str1;
		char t[1025];
		sr->getpstr(t, 1024);
		joinedshop->talk(t, this);
	}
}


void tplayer::checkmail()
{
	if((trade!=0)||(shop.getopened()))return;
	int nmails=0;
	int readed=0;
	int a;
	item targy;
	std::string str2, selectstr;

	selectstr=string("recipient = ")+toString(dbid) + " AND createdate > "  + toString(((int)time(0))-15*24*60*60);

	cl->dbmails.selectw(selectstr, "count(*)");
	if(cl->dbmails.next())
	{
		nmails=toInt(cl->dbmails[0]);
	}
	cl->dbmails.freeup();

#ifdef _DEBUG
#ifdef _LOGALOT
	logger.log("nmails=%d\n", nmails);
#endif
#endif

	if(nmails>0)
	{
		a=0;
		bs->cmd(id, 0xe9) << dbid << nmails;

		cl->dbmails.selectw(selectstr);
		while(cl->dbmails.next())
		{
			*bs << toInt(cl->dbmails["createdate"]) << toInt(cl->dbmails["sender"]);
			//load item here

			targy.setId(-1);
			if(cl->dbmails.getptr("item")!=0)
				targy.load(cl->dbmails.getptr("item"));
			if(targy.getId()!=-1)
			{
				*bs << (char)1 << -1;
				targy.snddata(bs);
			}else{
				*bs << (char)0;
			}
			*bs << toInt(cl->dbmails["money"]) << 5 << (char)toInt(cl->dbmails["readed"]);
			string str2, str3;
			str2=cl->dbmails["title"];
			str3=cl->dbmails["message"];
#ifdef _DEBUG
#ifdef _LOGALOT
			logger.log("title = %s\nmessage=%s\n", str2.c_str(), str3.c_str());
#endif
#endif
			bs->sndpstr(str2);
			bs->sndpstr(str3);
		}
		cl->dbmails.freeup();
	}
}

void tplayer::markmail()
{
	if((trade!=0)||(shop.getopened()))return;
	int a;
	sr->get(a);

	cl->dbmails.addupdate("readed", "1");
	cl->dbmails.update(string("recipient = " + toString(dbid) + " AND createdate = " + toString(a)));
	cl->dbmails.clearupdate();
	bs->cmd(id, 0xe7) << a << 3;
}


void tplayer::getitemfrommail()
{
	if((trade!=0)||(shop.getopened()))return;
	int a;
	sr->get(a);
	item targy;
	string str;
	str=string("recipient = ")+toString(dbid) + string(" AND createdate = ") + toString(a);

	cl->dbmails.selectw(str);
	if(cl->dbmails.next())
	{
		//targy.load(cl->dbitems);
		char *p=cl->dbmails.getptr("item");
		if(p==0)
		{
			messagebox("Cannot get item from mail");
			return;
		}
		targy.setId(-1);
		targy.load(p);
		cl->dbmails.freeup();
		if(targy.getId()!=-1)
		{
			if(inv.getRemainder(&targy)==0)
			{
				inv.pickup(&targy);
				bs->cmd(id, 0xe7) << a << 1;

				cl->dbmails.addnull("item");
				cl->dbmails.update(string("recipient = ")+toString(dbid)+string(" AND createdate=")+toString(a));
				cl->dbmails.clearupdate();
			}else bs->cmd(id, 0x55);
		}
	}else cl->dbmails.freeup();
}

void tplayer::getmoneyfrommail()
{
	if((trade!=0)||(shop.getopened()))return;
	int penya;
	int a;
	sr->get(a);
	string str;

	str=string("recipient = ")+toString(dbid) + string(" AND createdate = ") + toString(a);

	cl->dbmails.selectw(str);
	if(cl->dbmails.next())
	{
		penya=toInt(cl->dbmails["money"]);
		cl->dbmails.freeup();
		if(penya>0)
		{
			money+=penya;
			addstat(bs, id, 79, penya);
			bs->cmd(id, 0xe7) << a << 2;
			cl->dbmails.addupdate("money", "0");
			cl->dbmails.update(str);
			cl->dbmails.clearupdate();
		}
	}else cl->dbmails.freeup();


}

void tplayer::deletemail()
{
	if((trade!=0)||(shop.getopened()))return;
	int a;
	sr->get(a);
#ifdef _DEBUG
#ifdef _LOGALOT
	logger.log("delmail a=%d %08x\n", a, a);
#endif
#endif
	cl->dbmails.del(string("recipient = ")+toString(dbid)+" AND createdate = "+toString(a));
	bs->cmd(id, 0xe7) << a << 0;
}

void tplayer::sendmail()
{
	if(trade!=0)return;
	std::map<int, tplayer*>::iterator i;
	std::string cimzett, title, message;
	int penya;
	unsigned int a;
	int rid=-1,c=0;
	int crd=(int)time(0);
	item targy;
	short b;

	a=(unsigned int)(unsigned char)sr->get1char();
#ifdef _DEBUG
#ifdef _LOGALOT
	logger.log("a=%d\n", a);
#endif
#endif
	sr->get(b);
	char t[1025];
	sr->getpstr(t, 1024);
	cimzett=t;
	sr->get(penya);
	sr->getpstr(t, 1024);
	title=t;
	sr->getpstr(t, 1024);
	message=t;

	if(a!=0xff)
	{
		if(inv.getItem(a)==0)
		{
			throw error("inverror", "sendmail");
			return;
		}
		if((inv.getItem(a)->getId()<0)||(inv.getItem(a)->GetI()==0))
		{
			throw error("inverror", "sendmail");
			return;
		}
		targy.setId(-1);
		targy=*inv.getItem(a);
		targy.num=b;
		if(b<1)
		{
			errorstate=1;
			return;
		}
	}


	if(money>penya+500)
	{
		cl->dbcharacters.selectw(string("name = '")+string(&cimzett[0])+string("'"), "id");
		if(cl->dbcharacters.next())
		{
			rid=toInt(cl->dbcharacters[0]);
		}
		cl->dbcharacters.freeup();

		if(rid==-1)
		{
			messagebox("Can't find recipient!");
			return;
		}

		cl->dbmails.addupdate("recipient", toString(rid));
		cl->dbmails.addupdate("sender", toString(dbid));
		cl->dbmails.addupdate("title", string(&title[0]));
		cl->dbmails.addupdate("message", string(&message[0]));
		cl->dbmails.addupdate("money", toString(penya));
		cl->dbmails.addupdate("createdate", toString(crd));
		cl->dbmails.addupdate("readed", "0");
		if(a!=0xff)
		{
			char *i=new char[11*4*2];
			//targy.save(toString(dbid), (int)tmail, crd, cl, rid);
			targy.save(i);
			cl->dbmails.addeupdate("item", i, 11*4);
			delete[] i;
		}else
		{
			cl->dbmails.addnull("item");
		}

		cl->dbmails.insert();
		cl->dbmails.clearupdate();

		if(a!=0xff)inv.removeitem(a,b);
		money-=penya+500;
		addstat(bs, id, 79, -penya-500);
		messagebox("Sent the message.");
		i=cl->dbidplayers.find(rid);
		if(i!=cl->dbidplayers.end())i->second->gotmail();

	}
}

void tplayer::partyname()
{
	int a,b;
	std::string str1;
	if(party!=0)
	{
		sr->get(a);
		sr->get(b);
		std::string str1;
		char t[1025];
		sr->getpstr(t, 1024);
		party->setname(a, t);
	}
}

void tplayer::partyleave()
{
	int a,b;
	sr->get(a);
	sr->get(b);
	if(party!=0)party->leave(b, this);
}

void tplayer::ppartyinvite(const std::string &pname)
{
	std::map<std::string, tplayer*>::iterator i;

	tplayer *p=0;
	i=cl->nameplayers.find(pname);
	if(i!=cl->nameplayers.end())p=i->second;
	if(p!=0)
	{
		if(p->party==0)
		{
#ifdef _DEBUG
#ifdef _LOGALOT
			logger.log("sent invite\n");
#endif
#endif
			p->bs->cmd(p->dbid, 0x83);
			*p->bs << id << level << job << (char)1;
			*p->bs << p->id << p->level << p->job << (char)1;
			p->bs->sndpstr(name);
			*p->bs << 0;

		}else
		{
			bs->cmd(id, 0x95) << 647;
			bs->sndpstr(p->name);
		}
	}
}


void tplayer::partyinvite()
{
	std::map<int, tplayer*>::iterator i;
	int a,b;
	sr->get(a);	//dbid!
	sr->get(b);	//dbid!

	tplayer *p=0;
	i=cl->dbidplayers.find(b);
	if(i!=cl->dbidplayers.end())p=i->second;
	if(p!=0)
	{
		if(p->party==0)
		{
#ifdef _DEBUG
#ifdef _LOGALOT
			logger.log("sent invite\n");
#endif
#endif
			p->bs->cmd(b, 0x83);
			*p->bs << id << level << job << (char)1;
			*p->bs << p->id << p->level << p->job << (char)1;
			p->bs->sndpstr(name);
			*p->bs << 0;

		}else
		{
			bs->cmd(id, 0x95) << 647;
			bs->sndpstr(p->name);
		}
	}
}

void tplayer::partyjoin()
{
	int a,b,c,d,e,f;
	bool newparty=false;
	std::map<int, tplayer*>::iterator j,j2;

	sr->get(a);
	sr->get(b);
	sr->get(c);
	sr->get(d);
	sr->get(d);
	sr->get(e);
	sr->get(f);
#ifdef _DEBUG
#ifdef _LOGALOT
	logger.log("%d %d\n", a, d);
	logger.log("%d %d %d %d %d %d\n",a,b,c,d,e,f);
#endif
#endif

	tplayer *i=0, *i2=0;
	i=cl->getplayer(a);
	i2=cl->getplayer(d);
	if((i!=0)&&(i2==this))
		if((i2->party==0)&&(i->cl==i2->cl))
		{
			newparty=(i->party==0);
			if(i->party==0)i->party=tparty::create(i2, i);
			else i->party->join(i2);

			if(i->party!=0)
			{
				if(newparty)music(i->bs, 25);
				music(i2->bs, 25);
			}
		}
}

void tplayer::setguildnotice()
{
	if(guild!=0)
	{
		std::string str1;
		char t[1025];
		sr->getpstr(t, 1024);
		str1=t;
		guild->setnotice(str1);
	}
}

void tplayer::setguildrigths()
{
	if(guild!=0)
	{
		int b,c,d,e,g;
		sr->forward(8);
		sr->get(b);
		sr->get(c);
		sr->get(d);
		sr->get(e);
		sr->get(g);
		guild->setrigths(b,c,d,e,g);
	}
}

void tplayer::setguildname()
{
	if(guild!=0)
	{
		int a;
		sr->get(a);	//guild id?
		sr->get(a);	//owner id?
		std::string str1;
		char t[1025];
		sr->getpstr(t, 1024);
		str1=t;
		guild->setname(str1, this);
	}
}

void tplayer::challangeguild()
{
	int a;
	sr->get(a);
	if((guild!=0)&&(a==dbid))
	{
		if(a==guild->getowner())
		{
			std::string gname;
			char t[1025];
			sr->getpstr(t, 1024);
			guild->war(this, t);
		}
	}
}
void tplayer::createnick()
{
	if(guild!=0)
	{
		int a;
		sr->get(a);
		if(dbid==a)
		{
			std::string nick;
			char t[1025];
			sr->get(a);
			sr->getpstr(t, 1024);
			nick=t;
			if(nick.length()<=4)guild->setnick(this, a, nick);
		}
	}
}

void tplayer::npcchat()
{
	int c,d,e,id;
	sr->get(id);
	character_npc *p=cl->getnpc(id);
	if(p!=0)
	{
		std::string str1;
		char t[1025];
		sr->getpstr(t, 1024);
		str1=t;
		sr->get(c);
		sr->get(d);
		sr->get(e);
		p->npcchat(this,str1,c,d,e);
	}
}

void tplayer::removequest()
{
	int a;
	sr->get(a);
	std::map<int, pquestdata>::iterator i=quests.find(a);
	std::map<int, tquest*>::iterator j;
	std::string str1;
	if(i!=quests.end())
	{
		save_quests=true;
		quests.erase(i);
		bs->cmd(id, 0x3a) << -1 << a;
		{
			ul m=tquest::questsmutex.lock();
			j=tquest::quests.find(a*256);
			if(j!=tquest::quests.end())
			{
				str1="";
				str1+=(char)34;
				str1+=j->second->title;
				str1+=(char)34;
				bs->cmd(id, 0x95) << 507;
				bs->sndpstr(str1);
			}
			else
			{
				bs->cmd(id, 0x95) << 507;
				bs->sndpstr("");
			}
		}
	}
}

void tplayer::itemtowh()
{
	if((trade!=0)||(shop.getopened()))return;
	int a,b,d;
	item t;
	a=sr->get1char();	//invslot
	sr->get(b);	//nitems
//	c=sr->get1char();	//??1
	if(inv.getItem(a)!=0)if((inv.getItem(a)->getId()>0)&&(b>0))
	{
		t=*inv.getItem(a);
		t.num=b;
		d=guild->towh(this, t);
		if(d<b)
		{
			inv.removeitem(a, b-d);
			if(d>0)
			{
				bs->cmd(id, 0x55);
			}
		}
	}
}

void tplayer::whtoinv()
{
	if((trade!=0)||(shop.getopened()))return;
	int a,b,c;
	a=sr->get1char();	//whslot
	sr->get(b);	//nitems
	c=sr->get1char();	//??1
	if(c==1)guild->fromwh(this, a, b);
	else if(c==0)
	{
		money+=guild->moneyfromguild(this, b);
	}
}
