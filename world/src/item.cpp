#include "stdafx.h"
#include "item.h"
#include "network.h"
#include "player.h"
#include "adat.h"
#include "mysql.h"
#include <map>
#include "cluster.h"

#include "adat.h"


int getitempos(int id)
{
	int retval=-1;
	if((id>=0)&&(id<(int)itemlist.size()))retval=id;
	return retval;
}

void item::setId(int id1)
{
	id=id1;
	getItemData(id1);
}
item::item(int id1)
:id(id1),num(1),durability(0),upgrade(0),element(0),eupgrade(0),itemstat(0)
{
	slots.resize(nslots);
	for(int a=0;a<nslots;a++)slots[a]=65535;
	getItemData(id);
	
}
item::item(const item &a):id(a.id),num(a.num),durability(a.durability),upgrade(a.upgrade),element(a.element),eupgrade(a.eupgrade),itemstat(a.itemstat),i(a.i)
{
	slots.resize(nslots);
	for(int b=0;b<nslots;b++)
		slots[b]=a.slots[b];
}
item& item::operator =(const item &a)
{
	int b;
	if(&a!=this)
	{
		id=a.id;
		num=a.num;
		durability=a.durability;
		upgrade=a.upgrade;
		element=a.element,
		eupgrade=a.eupgrade;
		for(b=0;b<nslots;b++)slots[b]=a.slots[b];
		i=a.i;
	}
	return *this;
}


void item::addbonus(tplayer *player)
{

	int str=0,sta=0,dex=0,intl=0,a;
	itemdata *i2;

	getstat(&str,&sta,&dex,&intl);
	if(str>0)player->addstat1(1, str);
	if(dex>0)player->addstat1(2, dex);
	if(intl>0)player->addstat1(3, intl);
	if(sta>0)player->addstat1(4, sta);

	if((id!=-1)&&(i!=0))
	{
		if(element!=0)
		{
			a=eupgrade*5/2;
			if(i->eqslot==10)player->stats.at(stlknl[element])+=a;
			else player->stats.at(dstlknl[element])+=a;
		}
		if(i->destparam1!=-1)player->addstat2(i->destparam1, i->adjparamval1);
		if(i->destparam2!=-1)player->addstat2(i->destparam2, i->adjparamval2);
		if(i->destparam3!=-1)player->addstat2(i->destparam3, i->adjparamval3);
		for(a=0;a<(int)slots.size();a++)
			if((this->slots[a]!=0)&&(this->slots[a]!=65535))
			{
				i2=&itemlist.at(slots[a]);
				if(i2->destparam1!=-1)player->addstat1(i2->destparam1, i2->adjparamval1);
			}
	}

}

void item::rembonus(tplayer *player)
{

	int str=0,sta=0,dex=0,intl=0,a;
	itemdata *i2;

	getstat(&str,&sta,&dex,&intl);
	if(str>0)player->addstat1(1, -str);
	if(dex>0)player->addstat1(2, -dex);
	if(intl>0)player->addstat1(3, -intl);
	if(sta>0)player->addstat1(4, -sta);

	if((id!=-1)&&(i!=0))
	{
		if(element!=0)
		{
			a=eupgrade*5/2;
			if(i->weapontype>0)player->stats.at(stlknl[element])-=a;
			else player->stats.at(dstlknl[element])-=a;
		}
		if(i->destparam1!=-1)player->addstat2(i->destparam1, -i->adjparamval1);
		if(i->destparam2!=-1)player->addstat2(i->destparam2, -i->adjparamval2);
		if(i->destparam3!=-1)player->addstat2(i->destparam3, -i->adjparamval3);
		for(a=0;a<(int)slots.size();a++)
			if((this->slots[a]!=0)&&(this->slots[a]!=65535))
			{
				i2=&itemlist.at(slots[a]);
				if(i2->destparam1!=-1)player->addstat1(i2->destparam1, -i2->adjparamval1);
			}
	}

}


void item::getItemData(int id1)
{
	int a;
	i=0;
	if(id>0)
	{
		a=getitempos(id1);
		if(a!=-1)i=&itemlist.at(a);
	}

}
unsigned int item::getstat()
{
	return itemstats.at(itemstat);
}

void item::getstat(int *str, int *sta, int *dex, int *intl)
{
	unsigned int a=itemstats.at(itemstat);
	*str=a&0xff;
	*sta=(a/0x100)&0xff;
	*dex=(a/0x10000)&0xff;
	*intl=(a/0x1000000)&0xff;
}

void item::snddata(buffer *bs)
{
	int c,d;
	*bs << getId() << -1 << 0 << (short)num << (char)0;
	*bs << this->GetI()->duration;

	*bs << (char)0;
	*bs << upgrade << 0 << (char)element << eupgrade << 0;
	c=0;
	for(d=0;d<5;d++)if(slots[d]!=65535)c++;
	*bs << (char)c;
	for(d=0;d<5;d++)if(slots[d]!=65535)*bs << (short)slots[d];
	*bs << 0 << 0 << 0;
}

bool item::itemuse(tplayer *player)
{
	if(GetI()!=0)if(i->funkc!=0)
	{
		bool r=i->funkc(player, i);
		if(r)effect(player->tomulticast, player->getId(), i->sfx);
		return r;
	}
	return false;
}

bool i_addhp(tplayer *p, itemdata *i)
{
	if(p->cl->ido>p->nextfood)
	{
		p->alterhp(i->adjparamval1);
//		effect(p->tomulticast, p->getId(), 62);
		setstat(p->tomulticast, p->getId(), 38, p->gethp());

		p->nextfood=p->cl->ido+3000;
		return true;
	}else
	{
		return false;
	}
}

bool i_addmp(tplayer *p, itemdata *i)
{
	p->altermp(i->adjparamval1);
//	effect(p->tomulticast, p->getId(), 65);
	setstat(p->bs, p->getId(), 39, p->getmp());
	return true;
}

bool i_addfp(tplayer *p, itemdata *i)
{
	p->alterfp(i->adjparamval1);
//	effect(p->tomulticast, p->getId(), 62);
	setstat(p->bs, p->getId(), 40, p->getfp());
	return true;
}

bool i_summonpet1(tplayer *p, itemdata *i)
{
	p->summonpet(i->id-(21000-720));
	return false;
}

bool i_userestat(tplayer *p, itemdata *i)
{
	p->restat();
	return true;
}

bool i_usereskill(tplayer *p, itemdata *i)
{
//	p->usereskill();
	return false;
}

bool i_bw_flaris(tplayer *p, itemdata *i)
{
	if(p->getmapid()!=2)p->setdelayed(tplayer::da_flarisblink, 10000);
	return false;
}

bool i_bw_sm(tplayer *p, itemdata *i)
{
	if(p->getmapid()!=2)p->setdelayed(tplayer::da_smblink, 10000);
	return false;
}
bool i_bw_drakon(tplayer *p, itemdata *i)
{
	if(p->getmapid()!=2)p->setdelayed(tplayer::da_drakonblink, 10000);
	return false;
}
bool i_bw_town(tplayer *p, itemdata *i)
{
	if(p->getmapid()!=2)p->setdelayed(tplayer::da_townblinkwing, 10000);
	return false;
}

bool i_accelorfuel(tplayer *p, itemdata *i)
{
	if(p->flying)
	{
		setfuel(p->bs, p->getId(), 12000);
		p->fuel=12000;
	}
	return p->flying;
}

bool i_upcut(tplayer *p, itemdata *i)
{
	p->addbuff(p->getId(), -16, 0, 30*60000);
	return true;
}

bool i_bullh(tplayer *p, itemdata *i)
{
	p->addbuff(p->getId(), -9, 0, 15*60000);
	return true;
}

bool i_grilled(tplayer *p, itemdata *i)
{
	p->addbuff(p->getId(), -10, 0, 15*60000);
	return true;
}

bool i_refresherhold(tplayer *p, itemdata *i)
{
	p->addbuff(p->getId(), -7, 0, 20*60000);
	if(p->maxmp>p->mp)
	{
		addstat(p->bs, p->getId(), 39, p->maxmp-p->mp);
		p->mp=p->maxmp;
	}
	return true;
}

bool i_vitalx(tplayer *p, itemdata *i)
{
	p->addbuff(p->getId(), -8, 0, 20*60000);
	if(p->maxfp>p->fp)
	{
		addstat(p->bs, p->getId(), 40, p->maxfp-p->fp);
		p->fp=p->maxfp;
	}
	return true;
}

bool i_activation(tplayer *p, itemdata *i)
{
	p->addbuff(p->getId(), -11, 0, 10*60000);
	p->bs->cmd(p->getId(), 0xc5) << 0;
	return true;
}
bool i_velocity(tplayer *p, itemdata *i)
{
	p->addbuff(p->getId(), -22, 0, 3*60000);
	return true;
}

bool i_showbank(tplayer *p, itemdata *i)
{
//	p->bs->cmd(p->getId(), 0x58) << 1 << -1 << 0;
//	return true;
	return false;
}

bool i_party(tplayer *p, itemdata *i)
{
	if(p->party!=0)
	{
		return p->party->levelto10(p);
	}
	return false;
}

bool i_forgive(tplayer *p, itemdata *i)
{
	if(p->pk<0)
	{
		p->pk++;
		p->tomulticast->cmd(p->getId(), 0x28) << p->pk << p->pvp;
		p->bs->cmd(p->getId(), 0x95) << 666;
		p->bs->sndpstr("1");
		return true;
	}
	return false;
}

bool i_res(tplayer *p, itemdata *i)
{
	if(p->gethp()<=0)
	{
		p->reanimate();
		p->exp=p->expbeforeko;

		p->setlevelexp();
		return true;
	}
	return false;
}

bool i_nodisquise(tplayer *p, itemdata *i)
{
	if(p->modelid!=11)
	{
		p->modelid=11;
		p->tomulticast->cmd(p->getId(), 0xf6);
		return true;
	}
	return false;
}
void inititemfunkcs()
{
	int a;
	itemlist.at(2898).funkc=i_addmp;
	itemlist.at(2899).funkc=i_addfp;

	itemlist.at(15220).funkc=i_showbank;
	itemlist.at(10427).funkc=i_forgive;
	itemlist.at(10428).funkc=i_party;
	itemlist.at(10432).funkc=i_velocity;
	itemlist.at(10431).funkc=i_res;

	itemlist.at(10207).funkc=i_refresherhold;
	itemlist.at(10208).funkc=i_vitalx;
	itemlist.at(10209).funkc=i_bullh;
	itemlist.at(10210).funkc=i_grilled;
	itemlist.at(10270).funkc=i_upcut;
	itemlist.at(10218).funkc=i_activation;

	itemlist.at(18167).funkc=i_addhp;
	itemlist.at(26334).funkc=i_addhp;
	itemlist.at(26337).funkc=i_addhp;
	itemlist.at(26402).funkc=i_addhp;
	itemlist.at(30026).funkc=i_addhp;
	itemlist.at(30027).funkc=i_addhp;


	for(a=2800;a<2831;a++)itemlist.at(a).funkc=i_addhp;
	for(a=2892;a<2898;a++)itemlist.at(a).funkc=i_addhp;

	for(a=2530;a<2540;a++)itemlist.at(a).funkc=i_addmp;
	for(a=2544;a<2553;a++)itemlist.at(a).funkc=i_addfp;

	for(a=21000;a<=21006;a++)itemlist.at(a).funkc=i_summonpet1;

	itemlist.at(10211).funkc=i_userestat;
	itemlist.at(10434).funkc=i_usereskill;
	itemlist.at(4803).funkc=i_bw_flaris;
	itemlist.at(4804).funkc=i_bw_sm;
	itemlist.at(4805).funkc=i_bw_town;
	itemlist.at(4806).funkc=i_bw_drakon;
	itemlist.at(5902).funkc=i_accelorfuel;
	
	itemlist.at(25999).funkc=i_nodisquise;
//	itemlist.at(26002).funkc=i_;


}

void item::load(char *q)
{
	int *p=(int*)q;
	int id1=p[0];
	if(id1<1)setId(-1);
	else
	{
		this->setId(id1);
		num=p[1];
		durability=p[2];
		upgrade=p[3];
		eupgrade=p[4];
		element=p[5]&0xff;
		itemstat=(p[5]>>8)&0xff;
		slots[0]=p[6];
		slots[1]=p[7];
		slots[2]=p[8];
		slots[3]=p[9];
		slots[4]=p[10];
	}
}

void item::save(char *q)
{
	int *p=(int*)q;
	
	if(id>0)
	{
		p[0]=id;
		p[1]=num;
		p[2]=durability;
		p[3]=upgrade;
		p[4]=eupgrade;
		p[5]=element+(((int)itemstat)<<8);
		p[6]=slots[0];
		p[7]=slots[1];
		p[8]=slots[2];
		p[9]=slots[3];
		p[10]=slots[4];
	}else
	{
		p[0]=-1;
	}
}

