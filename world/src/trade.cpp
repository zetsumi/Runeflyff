#include "stdafx.h"
#include "trade.h"
#include "player.h"
#include "inventory.h"

tradeclass::tradeclass(tplayer *p1, tplayer *p2, int id1, int id2)
:player1(p1),player2(p2),player1id(id1),player2id(id2),money1(0),money2(0)
,end(0),enderid(0),ok1(false),ok2(false),ok3(false),ok4(false)
{
    std::lock_guard<std::mutex> guard(this->mutex);
	inv1.resize(25, -1);
	inv2.resize(25, -1);
	num1.resize(25, 0);
	num2.resize(25, 0);
}

void tradeclass::del(tplayer *player)
{
    std::unique_lock<std::mutex> player1guard (player1->playermutex, std::defer_lock);
    std::unique_lock<std::mutex> player2guard (player2->playermutex, std::defer_lock);

    if (player == player1) {
        player2guard.lock();
    }
    else {
        player1guard.lock();
    }

	{
        std::lock_guard<std::mutex> tradeguard(this->mutex);
	}/*TODO*/
	delete this;
}

tradeclass::~tradeclass()
{
    if(player1!=0)
	{
        if((!ok3)||(!ok4))player1->bs->cmd(enderid, 0x0b) << enderid << 0;
        if(money1!=0)addstat(player1->bs, player1id, 10000, money1);
    }
    if(player2!=0)
	{
        if((!ok3)||(!ok4))player2->bs->cmd(enderid, 0x0b) << enderid << 0;
        if(money2!=0)addstat(player2->bs, player2id, 10000, money2);
    }

    if(player1!=0)player1->trade=0;
    if(player2!=0)player2->trade=0;
}

void tradeclass::setok(tplayer *player)
{
    std::lock_guard<std::mutex> tradeguard(this->mutex);
    std::unique_lock<std::mutex> player1guard (player1->playermutex, std::defer_lock);
    std::unique_lock<std::mutex> player2guard (player2->playermutex, std::defer_lock);

    if (player == player1) {
        player2guard.lock();
    }
    else {
        player1guard.lock();
    }

	int playerid;
    if(player==player1)
	{
		ok1=true;
		playerid=player1id;
	}else 
	{
		ok2=true;
		playerid=player2id;
	}

	player1->bs->cmd(playerid, 0x0a);
	player2->bs->cmd(playerid, 0x0a);
    
    if((ok1)&&(ok2))
    {
		player1->bs->cmd(-1, 0x2b);
		player2->bs->cmd(-1, 0x2b);
    }
}

bool tradeclass::setok2(tplayer *player)
{
    std::lock_guard<std::mutex> tradeguard(this->mutex);
    std::unique_lock<std::mutex> player1guard (player1->playermutex, std::defer_lock);
    std::unique_lock<std::mutex> player2guard (player2->playermutex, std::defer_lock);

    if (player == player1) {
        player2guard.lock();
    }
    else {
        player1guard.lock();
    }

	int playerid;
    if(player==player1)
	{
		ok3=true;
		playerid=player1id;
	}else 
	{
		ok4=true;
		playerid=player2id;
	}

	player->bs->cmd(playerid, 0x2c);

    if((ok3)&&(ok4))
    {

        if(completetrade())
		{
			player1->bs->cmd(-1, 0x0c);
			player2->bs->cmd(-1, 0x0c);
        }else ok3=false;
        return true;
    }
    
    return false;
}

void tradeclass::setitem(tplayer *player, int a, int b, int c)
{
	if((a<0)||(a>=(int)inv1.size()))return;
	if(player->inv.getItem(b)==0)return;
	if(player->inv.getItem(b)->getId()<=0)return;
	if(player->inv.getItem(b)->num<c)return;

    std::lock_guard<std::mutex> tradeguard(this->mutex);
    std::unique_lock<std::mutex> player1guard (player1->playermutex, std::defer_lock);
    std::unique_lock<std::mutex> player2guard (player2->playermutex, std::defer_lock);

    if (player == player1) {
        player2guard.lock();
    }
    else {
        player1guard.lock();
    }

	int playerid;
    if(player==player1)
	{
        inv1.at(a)=b;
        num1.at(a)=c;
		playerid=player1id;
    }else{
        inv2.at(a)=b;
        num2.at(a)=c;
		playerid=player2id;
    }
	player1->bs->cmd(playerid, 0x08) << (short)a << (char)b << (short)c;
	player2->bs->cmd(playerid, 0x08) << (short)a << (char)b << (short)c;
}

void tradeclass::setmoney(tplayer *player, int a)
{
	if((a<0)||(a>player->money))return;

    std::lock_guard<std::mutex> tradeguard(this->mutex);
    std::unique_lock<std::mutex> player1guard (player1->playermutex, std::defer_lock);
    std::unique_lock<std::mutex> player2guard (player2->playermutex, std::defer_lock);

    if (player == player1) {
        player2guard.lock();
    }
    else {
        player1guard.lock();
    }

	int playerid;
    if(player1==player)
	{
		money1=a;
		playerid=player1id;
	}else 
	{
		money2=a;
		playerid=player2id;
	}
    
	player1->bs->cmd(playerid, 0x20) << a;
	player2->bs->cmd(playerid, 0x20) << a;
}

bool tradeclass::completetrade()
{
    int a;
    bool retval=true;
    inventory *inv1, *inv2;
    inv1=new inventory(player1->inv);
    inv2=new inventory(player2->inv);
	inv1->player=0;
	inv2->player=0;
    item targy;
    for(a=0;a<25;a++)
		if(this->inv1.at(a)!=-1)
			if(player1->inv.getItem(this->inv1.at(a))!=0)
			{
				targy=*player1->inv.getItem(this->inv1.at(a));
				targy.num=this->num1.at(a);
			    if(inv2->pickup(&targy)>0)
				{
					retval=false;
					break;
				}
			}

    if(retval)
		for(a=0;a<25;a++)
			if(this->inv2.at(a)!=-1)
				if(player2->inv.getItem(this->inv2.at(a))!=0)
				{
					targy=*player2->inv.getItem(this->inv2.at(a));
					targy.num=this->num2.at(a);
					if(inv1->pickup(&targy)>0)
					{
						retval=false;
						break;
					}
				}
    
    if(!retval)
	{
        delete inv1;
        delete inv2;
        return false;
    }

    player1->inv.player=0;
    player2->inv.player=0;

    for(a=0;a<25;a++)
		if(this->inv1.at(a)!=-1)
			player1->inv.removeitem(this->inv1.at(a), this->num1.at(a));

    for(a=0;a<25;a++)
		if(this->inv2.at(a)!=-1)
			player2->inv.removeitem(this->inv2.at(a), this->num2.at(a));

    for(a=0;a<25;a++)
		if(this->inv1[a]!=-1)
			if(inv1->getItem(this->inv1.at(a))!=0)
			{
				targy=*inv1->getItem(this->inv1.at(a));
				if(targy.getId()!=-1)
				{
					targy.num=this->num1.at(a);
					player2->inv.pickup(&targy);
				}
			}


    for(a=0;a<25;a++)
		if(this->inv2[a]!=-1)
			if(inv2->getItem(this->inv2[a])!=0)
			{
		        targy=*inv2->getItem(this->inv2[a]);
				if(targy.getId()!=-1)
				{
					targy.num=this->num2[a];
					player1->inv.pickup(&targy);
				}
			}

    player1->inv.player=player1;
    player2->inv.player=player2;
    
    delete inv1;
    delete inv2;
    
    if(money1>0)
	{
        player1->altermoney(-money1);
        player2->altermoney(+money1);
    }
    if(money2>0)
	{
        player1->altermoney(+money2);
        player2->altermoney(-money2);
    }

    money1=0;
    money2=0;
    return true;

}
