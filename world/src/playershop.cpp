#include "stdafx.h"
#include "playershop.h"
#include "inventory.h"
#include "player.h"
#include "grid.h"
#include "buffer.h"

playershop::playershop(tplayer *o)
:owner(o),opened(false)
{
    std::lock_guard<std::mutex> shopguard(this->shopmutex);
	itemek.resize(30, 255);
	nitemek.resize(30, 0);
	price.resize(30,0);
	nplayers=0;
}

void playershop::close(tplayer *player)
{
    std::lock_guard<std::mutex> shopguard(this->shopmutex);
	if(opened)
	{
		buffer bs;
		bs.cmd(owner->getId(), 0x43) << (char)1;
		owner->multicast(bs);

		for(std::map<int, tplayer*>::iterator i=players.begin();i!=players.end();++i)
		{
		    std::unique_lock<std::mutex> guard(i->second->playermutex, std::defer_lock);

		    if (i->second != player) {
		        guard.lock();
		    }

			i->second->joinedshop=0;
		}

		players.clear();

		itemek.clear();
		nitemek.clear();
		price.clear();
		itemek.resize(30, 255);
		nitemek.resize(30, 0);
		price.resize(30,0);
		nplayers=0;

		opened=false;
		if((owner->bs==0)||(!owner->validnl()))return;
		owner->clearsp2();
	}
}

void playershop::open(const std::string& n)
{
	{
        std::lock_guard<std::mutex> shopguard(this->shopmutex);
		if((owner->bs==0)||(!owner->validnl()))return;
		if(opened)return;
		players.clear();
		nplayers=0;
		buffer bs;
		name=n;
		bs.cmd(owner->getId(), 0x42);
		bs.sndpstr(name);
		owner->multicast(bs);
//		if(owner->party!=0)
///			if(owner->party->isleader(owner))
//				owner->messagebox("Warning: you are the party leader");
		opened=true;
	}
	login(owner);
	owner->clearsp2();
}

void playershop::login(tplayer *p)
{
    std::lock_guard<std::mutex> shopguard(this->shopmutex);
	if((owner->bs==0)||(!owner->validnl()))return;
	int b,c;
	nplayers++;
	if(p->joinedshop!=0)return;
	players.insert(std::pair<int, tplayer*>(p->getId(), p));
	p->joinedshop=this;

	if(p!=owner)
	{
			item targy;
			buffer bs;
			bs.cmd(owner->getId(), 0x45);
			for(b=0,c=0;b<(int)itemek.size();b++)
				if((itemek.at(b)!=255)&&(nitemek.at(b)>0))c++;
            bs << (char)c;
            if(c>0)
			{
                for(b=0;b<(int)itemek.size();b++)
				{
                    if((itemek.at(b)!=255)&&(nitemek.at(b)>0))
					{
						bs << (char)b << (unsigned int)itemek.at(b);
						if(owner->inv.getItem((unsigned int)itemek.at(b))==0)
						{
							logger.log("shop senddata error in playershop::login\n");
							return;
						}
                        targy=*owner->inv.getItem((unsigned int)itemek.at(b));
                        targy.num=nitemek.at(b);
						targy.snddata(&bs);
                        bs << (unsigned int)nitemek.at(b) << (int)price.at(b);
                    }
                }
            }
            bs << 1;
			p->bs->add(bs);
	}


	buffer bs;
	bs << (char)0x7b;
	bs.inc();
	bs << (char)1 << nplayers;
	for(std::map<int, tplayer*>::iterator i=players.begin();i!=players.end();++i)
	{
//		ul mm=(i->second==p)?i->second->playermutex.dontlock():i->second->playermutex.lock();
		bs << i->second->getId();
		bs.sndpstr(i->second->name);
	}
	shopmulticast(bs, p);

}
void playershop::logout(tplayer *p)
{
    std::lock_guard<std::mutex> shopguard(this->shopmutex);
	if((owner->bs==0)||(!owner->validnl()))return;
	if(p->joinedshop!=this)return;
	
	if(playershop::players.erase(p->getId())>0)nplayers--;
	p->joinedshop=0;

	buffer bs;
	bs << (char)0x7b;
	bs.inc();
	bs << (char)1 << nplayers;
	for(std::map<int, tplayer*>::iterator i=players.begin();i!=players.end();++i)
	{
		bs << i->second->getId();
		bs.sndpstr(i->second->name);
	}
	shopmulticast(bs, p);
}

void playershop::talk(const std::string &p, tplayer *player)
{
    std::lock_guard<std::mutex> shopguard(this->shopmutex);
	if((owner->bs==0)||(!owner->validnl()))return;
	if(player->joinedshop!=this)return;
    buffer bs;
    bs << (char)0x7b << (char)4 << player->getId();
    bs.sndpstr(p);
    bs.inc();
    shopmulticast(bs, player);
}

void playershop::shopmulticast(buffer &bs, tplayer *player)
{
	for(std::map<int, tplayer*>::iterator i=players.begin();i!=players.end();++i)
	{
	    std::unique_lock<std::mutex> guard(i->second->playermutex, std::defer_lock);

	    if (i->second != player) {
	        guard.lock();
	    }

		*i->second->bs << i->second->getId();
		i->second->add(bs);
	}
}

void playershop::shopmulticast2(buffer &bs, tplayer *player)
{
	for(std::map<int, tplayer*>::iterator i=players.begin();i!=players.end();++i)
	{
        std::unique_lock<std::mutex> guard(i->second->playermutex, std::defer_lock);

        if (i->second != player) {
            guard.lock();
        }

		i->second->add(bs);
	}
}

bool playershop::removeitem(int a)
{
    std::lock_guard<std::mutex> shopguard(this->shopmutex);
	if((owner->bs==0)||(!owner->validnl()))return false;
	if(!opened)
	{
		bool retval=(itemek.at(a)!=-1);
		itemek.at(a)=-1;
		return retval;
	}
	return false;
}

void playershop::additemtoshop(int a, int b, int c, int d)
{
    std::lock_guard<std::mutex> shopguard(this->shopmutex);
	if((owner->bs==0)||(!owner->validnl()))return;
	if(!opened)
	{
		if((owner->inv.getItem(b)==0)||(d<1)||(c<1))return;
		if(owner->inv.getItem(b)->getId()<=0)return;
		if(owner->inv.getItem(b)->GetI()==0)return;
		if(owner->inv.getItem(b)->num<c)c=owner->inv.getItem(b)->num;
		if(c<1)return;
		itemek.at(a)=b;
		nitemek.at(a)=c;
		price.at(a)=d;

		owner->bs->cmd(owner->getId(), 0x44) << (short)a << (char)b << (short)c << d;
	}
}


bool playershop::buyfromshop(int b, tplayer *player, int c, int d)
{
	if((owner->bs==0)||(!owner->validnl()))return false;
	if(player==owner)return false;
	if(player->joinedshop!=this)return false;

	buffer bs;
	std::lock_guard<std::mutex> shopguard(this->shopmutex);
	{
	    std::lock_guard<std::mutex> playerguard(owner->playermutex);

		item targy, *t;
		int e;

		if(owner->inv.getItem(itemek.at(b))!=0)
		{
			t=owner->inv.getItem(itemek.at(b));
			if((t->getId()==c)&&(nitemek.at(b)>=d)&&(player->money>=price.at(b)*d))
			{
	            targy=*t;
				if(d>t->num)return false;
				targy.num=d;
				if(player->inv.getRemainder(&targy)==0)
				{
					targy=*t;
					targy.num=d;
					player->inv.pickup(&targy);
					e=price.at(b)*d;
					player->money-=e;
					owner->money+=e;

					setstat(player->bs, player->getId(), 79, player->money);
					setstat(owner->bs, owner->getId(), 79, owner->money);

					nitemek.at(b)-=d;

					bs.cmd(owner->getId(), 0x46) << (char)b << (short)nitemek.at(b);
					owner->inv.removeitem(itemek.at(b), d);

				}else 
				{
					player->bs->cmd(player->getId(), 0x55);	//inventory is full
					return false;
				}
			}
		}
	}
	bool retval=bs.getcommandnumber()>0;
	if(retval)shopmulticast2(bs, player);
	return retval;
}