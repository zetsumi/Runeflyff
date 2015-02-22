#include "stdafx.h"
#include "bank.h"
#include "network.h"
#include "player.h"
#include "logger.h"

bank::bank(tplayer *ch, int b, int n):player(ch),banknum(b),nitems(n)
{
	usedslots.resize(n, false);
	itemek.resize(n);
}


int bank::insert(item &i)
{
#ifdef _DEBUG
#ifdef _LOGALOT
	logger.log("in=%d\n", i.num);
#endif
#endif
	item targy=i;
	int nstack=1;
	int a;
	if(i.GetI()==0)a=nitems;
	else{
		nstack=i.GetI()->num;
		if(nstack<2)a=nitems;
	}
#ifdef _DEBUG
#ifdef _LOGALOT
	logger.log("nstack %d\n", nstack);
#endif
#endif
	if(nstack>1)for(a=0;(a<nitems)&&(i.num>0);a++)
		if((usedslots.at(a))&&(itemek.at(a).getId()==i.getId())&&(itemek.at(a).num<nstack))
			if(nstack-itemek.at(a).num>i.num)
			{
				itemek.at(a).num+=i.num;
				i.num=0;
			}else{
				i.num-=nstack-itemek.at(a).num;
				itemek.at(a).num=nstack;
			}

	if(i.num>0)
	{
		for(a=0;(a<nitems);a++)
			if(!usedslots.at(a))
			{
				usedslots.at(a)=true;
				itemek.at(a)=i;
				i.num=0;

				break;
			}
	}
#ifdef _DEBUG
#ifdef _LOGALOT
	logger.log("tn=%d in=%d\n", targy.num, i.num);
#endif
#endif
	targy.num-=i.num;
	if(targy.num>0)
	{
#ifdef _DEBUG
#ifdef _LOGALOT
		logger.log("insertpacket called");
#endif
#endif
		insertpacket(targy);
	}

	return i.num;
}
int bank::removeitem(int a, int b)
{
	if(b>0)
	{
		if((a>=nitems)||(a<0))throw std::range_error("Incorrect item slot");//error(string("incorrect item slot")+toString(a), "bank::removeitem");
		if(b>itemek.at(a).num)b=itemek.at(a).num;
		itemek.at(a).num-=b;
		if(itemek.at(a).num<=0)
		{
			itemek.at(a).setId(0);
			usedslots.at(a)=false;
		}
		removepacket(a, b);
		return b;
	}
	return 0;
}


void bank::init(tplayer *ch, int b, int n)
{
	player=ch;
	banknum=b;
	nitems=n;
	usedslots.resize(n, false);
	itemek.resize(n);
}
void bank::clear()
{
	for(int a=0;a<nitems;a++)
	{
		usedslots[a]=false;
		itemek[a].setId(-1);
	}
}

void bank::insertpacket(item &targy)
{
	if(player!=0)
	{
		player->bs->cmd(-1, 0x50);
		*player->bs << (char)banknum << -1;
		targy.snddata(player->bs);
	}
}

void bank::removepacket(unsigned int a, int b)
{
	if(player!=0)
	{
		player->bs->cmd(player->getId(), 0x54);
		*player->bs << (char)banknum << (short)a << itemek.at(a).num;
	}
}

void backpack::insertpacket(item &targy)
{
/*
	if(player!=0)
	{
		buffer bs;
		bs << -1 << (short)0x0201 << banknum << -1;
		targy.snddata(&bs);
		bs.inc();
		player->add(&bs);
	}
*/
}

void backpack::removepacket(unsigned int a, int b)
{
/*
	if(player!=0)
	{
		*player->bs << -1 << (short)0x0202 << banknum << a << (short)b;
		player->bs->inc();
	}
*/
}


void warehouse::insertpacket(item &targy)
{

	if(player!=0)
	{
		player->bs->cmd(player->getId(), 0xef);
		*player->bs << (char)1 << -1;
		targy.snddata(player->bs);
	}
	if(bs!=0)
	{
		bs->cmd(player->getId(), 0xef);
		*bs << (char)3 << -1;
		targy.snddata(bs);
	}

}

void warehouse::removepacket(unsigned int a, int b)
{
/*
	if(player!=0)
	{
//		player->bs->cmd(player->getId(), 0x54);
//		*player->bs << (char)banknum << (short)a << itemek.at(a).num;
	}
*/
}

item* bank::getItem(int a)
{
	item *retval=0;
	if((a<nitems)&&(a>=0))retval=&itemek.at(a);
	else throw std::range_error("Incorrect item slot");//error(string("Incorrect item slot ") + toString(a), "bank::getItem");
	return retval;
}


void bank::load(char *q)
{
	int a;
	if(q!=0)
		for(a=0;a<nitems;a++)
		{
			itemek.at(a).load(&q[a*item::itemdatasize]);
			usedslots.at(a)=(itemek.at(a).getId()>0);
		}
}

void bank::save(char *q)
{
	int a;
	item dummy;
	if(q!=0)
	{
//		for(a=0;a<nitems;a++)
//			*(int*)&q[a*item::itemdatasize]=-1;
		for(a=0;a<nitems;a++)
		{
			if(usedslots.at(a))itemek.at(a).save(&q[a*item::itemdatasize]);
			else dummy.save(&q[a*item::itemdatasize]);
		}
	}
	
}
