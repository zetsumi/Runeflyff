#include "stdafx.h"

#include "guild.h"
#include "main.h"
#include "adat.h"
#include "mysql.h"
#include "buffer.h"
#include "player.h"
#include "cluster.h"
#include "pmutex.h"
#include "main.h"

#define r_title 1
#define r_rank 2
#define r_join 4
#define r_penya 8
#define r_item 16

using namespace std;
//extern int tesztval1;
extern int tesztval;
int tesztval2=1;

int nranked[5]={1, 5, 8, 20, 9999};

pmutex tguild::globalguildmutex;
int tguild::maxguildid=0;
int tguild::maxguildownerid=0;
int tguild::nguilds=0;

pmutex guildtreemutex;
std::map<int, tguild*> guilds;
std::map<int, tguild*> owguilds;
std::map<std::string, tguild*> nameguilds;

tguild* tguild::getguild(int id)
{
	pmutex::unlocker m=guildtreemutex.lock();
	std::map<int, tguild*>::iterator i=guilds.find(id);
	if(i!=guilds.end())return i->second;
	else return 0;
}

bool tguild::war(tplayer *p, const std::string &gname)
{
	pmutex::unlocker gm=guildmutex.lock();
	if((disbanded)||(insiege)||(closetosiege))return false;
	bool found=false;
//	tguild *g;
	tplayer *q;
	std::map<int, tplayer*>::iterator j;
	int odbid;
	if(waring==0)
	{
		pmutex::unlocker m=guildtreemutex.lock();
		std::map<int, tguild*>::iterator i;
		for(i=guilds.begin();i!=guilds.end();++i)
		if(i->second!=this)
		{
			ul m=i->second->guildmutex.lock();
			if(i->second->name==gname)
			{
				if(i->second->waring!=0)
				{
					p->messagebox("The guild is already waring with another guild");
					return false;
				}
				if((i->second->insiege)||(i->second->closetosiege))
				{
					p->messagebox("Can't war a guild lined up for siege now");
					return false;
				}
				odbid=i->second->owner;
				j=p->cl->dbidplayers.find(odbid);
				if(j==p->cl->dbidplayers.end())
				{
					p->messagebox("Can't find guild's leader!");
					return false;
				}
				q=j->second;
				if((p!=q)&&(q!=0))
				{
					buffer bs;
					bs << 0xf000b036 << guildid;
					bs.sndpstr(p->name);

					pmutex::unlocker mm=q->asyncbuffermutex.lock();
					q->asyncbuffer2.push_back(buffer());
					q->asyncbuffer2.back().copy(bs);
//					if(q->asyncbuffer2==0)q->asyncbuffer2=new buffer;
//					(q->asyncbuffer2)->copy(bs);
					return true;
				}
				break;
			}
		}
		if(i==guilds.end())p->messagebox("Can't find guild");

	}
	return false;
}

void tguild::getwarpacket(buffer *bs)
{
	int a;
	for(a=0;a<(int)members.size();a++)
	{
		if(members[a].dbid!=-1)
		{
			bs->cmd(-1, 0x31) << members[a].dbid << 0;
			bs->cmd(-1, 0x31) << members[a].dbid << 1;
		}
	}
}

void tguild::getwarendpacket(buffer *bs)
{
	int a;
	for(a=0;a<(int)members.size();a++)
	{
		if(members[a].dbid!=-1)
		{
			bs->cmd(-1, 0x31) << members[a].dbid << 2;
		}
	}
}

void tguild::acceptwar(tplayer *p, int gid)
{
	{
		pmutex::unlocker gm=guildmutex.lock();
		if((disbanded)||(insiege)||(closetosiege))return;

	}
	tguild *g=getguild(gid);
	if(g!=0)
	{
		{
			pmutex::unlocker gm=guildmutex.lock();
			if(waring!=0)
			{
				p->messagebox("You can't war more than one guild at a time");
				return;
			}
		}

		ul mm=g->guildmutex.lock();
		if(g->waring==0)
		{
			ul gm=guildmutex.lock();
			waring=g;
			g->waring=this;
			/*2*/
			buffer bs;
			wid=guildid;//0x0ec6;
			g->wid=wid;

			bs << 0xf000b037 << wid << g->guildid << guildid;
//			p->cl->multicastooo(bs);
			clusters->multicastooo(bs);
//			buffer bs1;
//			bs1.cmd(members.at(0).p->getId(), 0x7a) << 0x0ec6;
//			bs1.cmd(g->members.at(0).p->getId(), 0x7a) << 0x0ec6;
//			guildmulticast3(bs1);
			buffer bs2;
			bs2 << (char)0x7a << wid;
			bs2.inc();
			guildmulticast4(bs2);
			g->guildmulticast4(bs2);

			warpacket=new buffer;
			warendpacket=new buffer;
			g->warpacket=new buffer;
			g->warendpacket=new buffer;

			g->getwarpacket(warpacket);
			g->getwarendpacket(warendpacket);

			getwarpacket(g->warpacket);
			getwarendpacket(g->warendpacket);

			if(warpacket->getcommandnumber()>0)guildmulticast3(*warpacket);
			if(g->warpacket->getcommandnumber()>0)g->guildmulticast3(*g->warpacket);
//			wid=g->guildid;
//			g->wid=wid;


		}else
		{
			p->messagebox("The guild is already waring with another guild");
		}
	}
}

void tguild::endwar(bool lost, bool won)
{
	pmutex::unlocker gm=guildmutex.lock();
	if(disbanded)return;
	waring=0;
	wid=0;

	if(won)gwwin++;
	if(lost)gwlose++;

	buffer bs2;
	bs2 << (char)0x7a << 0;
	bs2.inc();
	guildmulticast4(bs2);

	if(warendpacket!=0)
	{
		if(warendpacket->getcommandnumber()>0)
			guildmulticast3(*warendpacket);
		delete warendpacket;
		warendpacket=0;
	}
	if(warpacket!=0)
	{
		delete warpacket;
		warpacket=0;
	}

}

void tguild::giveupwar(tplayer *p, int dbid)
{
//	int wid;
	bool ended=false;
	{
		pmutex::unlocker gm=guildmutex.lock();
		if(disbanded)return;
		if((dbid==this->owner)&&(waring!=0))
		{
			/*2*/
//			wid=waring->getguildid();
			buffer bs;
			bs << 0xf000b047 << wid << p->dbid;
			bs.sndpstr(p->name);
			bs << 1;
//			p->cl->multicastooo(bs);
			clusters->multicastooo(bs);
			buffer bs1;
			bs1 << 0xf000b046 << wid << 0x27 << 0x32 << 3;
//			p->cl->multicastooo(bs1);
			clusters->multicastooo(bs1);
			ended=true;
		}
	}
	if(ended)
	{
		waring->endwar(false, true);
		endwar(true, false);
	}
}

void tguild::leaderkilled()
{
//	int wid;
	bool ended=false;
	{
		pmutex::unlocker gm=guildmutex.lock();
		if(disbanded)return;
		if(waring==0)return;
//		if((dbid==this->owner)&&(waring!=0))
		{
			/*2*/
//			wid=waring->getguildid();
			buffer bs;
//			bs << 0xf000b045 << wid << guildid;
//			bs.sndpstr(members[0].name);
//			bs << 1;
			bs.cmd(-1, 95) << 1303;
			bs.sndpstr(name + " " + waring->name + " " + waring->name + " guild leader has been killed");
//			bs.sndpstr(waring->name);
//			bs.sndpstr(waring->name);
//			bs << 0;
//			bs.cmd(-1, 0xd0) << -1;
//			bs.sndpstr("hai");
//			bs.sndpstr("hai");
			clusters->multicast(bs);
//			p->cl->multicastooo(bs);
			buffer bs1;
			bs1 << 0xf000b046 << wid << 0x27 << 0x32 << 3;
			clusters->multicastooo(bs1);
//			p->cl->multicastooo(bs1);
			ended=true;
		}
	}
	if(ended)
	{
		waring->endwar(false, true);
		endwar(true, false);
	}
}

void tguild::piece(int dbid)
{
	bool ended=false;
	{
		pmutex::unlocker gm=guildmutex.lock();
		if(disbanded)return;
		if((dbid==owner)&&(waring!=0))
		{
			/*2*/
//			int wid=waring->getguildid();
			buffer bs1;
			bs1 << 0xf000b046 << wid << 0x32 << 0 << 8;
			clusters->multicastooo(bs1);
//			p->cl->multicastooo(bs1);
	
			ended=true;
		}
	}
	if(ended)
	{
		waring->endwar(false, false);
		endwar(false, false);
	}
}
int tguild::getwaringid()
{
	pmutex::unlocker gm=guildmutex.lock();
	int r=0;
	if(disbanded)return -1;
	if(waring!=0)r=waring->getguildid();
	return r;
}

void tguild::updateglobalid()
{
	int a,b;
	{
		pmutex::unlocker gm=guildmutex.lock();
		a=guildid;
		b=owner;
	}
	{
		pmutex::unlocker gm=globalguildmutex.lock();
		if(maxguildid<guildid)maxguildid=guildid;
		if(maxguildownerid<owner)maxguildownerid=owner;
		nguilds++;
	}
}

void tguild::loadallguilds()
{
	logger.log("Guild data ");
	
	dbguilds.select();
	{
//		pmutex::unlocker gm=guildtreemutex.lock();
		while(dbguilds.next())new tguild(dbguilds);
	}
	dbguilds.freeup();
	{
		pmutex::unlocker gm=tguild::globalguildmutex.lock();
		logger.log("%d\n", tguild::nguilds);
	}
}

void tguild::saveallguilds()
{
	std::map<int,tguild*>::iterator gi;
	pmutex::unlocker gm=guildtreemutex.lock();
	for(gi=guilds.begin();gi!=guilds.end();gi++)
	{
		gi->second->save();
	}
}

void tguild::getallguildinfo(tplayer *p)
{
	int a,b;
	std::map<int, tguild*>::iterator i;
	buffer &bs=*p->bs;
	bs.cmd(-1, 0x9f);
	{
		pmutex::unlocker m=tguild::globalguildmutex.lock(); 
		a=tguild::maxguildid;
		b=tguild::nguilds;
	}
	bs << a << b;
	pmutex::unlocker m=guildtreemutex.lock();
	for(i=guilds.begin();i!=guilds.end();++i)
	{
		bs << i->second->guildid;
		bs << i->second->owner;
		bs << i->second->logo;
		bs.sndpstr(i->second->name);
		bs << i->second->logo << 0 << i->second->gwwin << i->second->gwlose << i->second->gwgiveup;
	}
	bs << 1;
	bs << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0;

return;
/*
	{
		pmutex::unlocker m=guildtreemutex.lock();
		for(i=guilds.begin();i!=guilds.end();++i)
		{
			{
				pmutex::unlocker gm=i->second->guildmutex.lock();
				p->bs->cmd(-1, 0x9c) << i->second->owner << i->second->guildid;
				p->bs->sndpstr(i->second->members.at(0).name);
				*p->bs << 0;
				if(i->second->logo!=0)p->bs->cmd(-1, 0xfb) << i->second->guildid << i->second->logo;
				if(i->second->name!="")
				{
					*p->ooo << 0xf000b032 << i->second->guildid;
					p->ooo->sndpstr(i->second->name);
					p->raiseooo();
				}
			}

		}
	}
*/
}

pmutex guildsavemutex;
void tguild::save()
{
	{
		pmutex::unlocker gm=guildmutex.lock();
		if(disbanded)return;
	}

	ul mmmm=guildsavemutex.lock();

	pmutex::unlocker gm=guildmutex.lock();
	if(disbanded)return;
	sqlquery &s4=dbguilds_g;
	s4.addupdate("ownerid", toString(owner));
	s4.addupdate("name", name);
	s4.addupdate("notice", gnotice);
	s4.addupdate("exp", toString(exp));
	s4.addupdate("level", toString(level));
	s4.addupdate("logo", toString(logo));
	s4.addupdate("gwwin", toString(gwwin));
	s4.addupdate("gwlose", toString(gwlose));
	s4.addupdate("gwgiveup", toString(gwgiveup));
	s4.addupdate("bankmoney", toString(bankmoney));
	s4.addupdate("nmembers", toString(nmembers));
	s4.addupdate("r1", toString(this->rigths.at(0)));
	s4.addupdate("r2", toString(this->rigths[1]));
	s4.addupdate("r3", toString(this->rigths[2]));
	s4.addupdate("r4", toString(this->rigths[3]));
	s4.addupdate("r5", toString(this->rigths[4]));
	s4.addupdate("m1", toString(this->pay.at(0)));
	s4.addupdate("m2", toString(this->pay[1]));
	s4.addupdate("m3", toString(this->pay[2]));
	s4.addupdate("m4", toString(this->pay[3]));
	s4.addupdate("m5", toString(this->pay[4]));
	s4.addupdate("cwtime", toString(this->cwtime));


	char *i=new char[wh.getnitems()*item::itemdatasize];
	wh.save(i);
	s4.addeupdate("guildbank", i, wh.getnitems()*item::itemdatasize);
	delete[] i;


	s4.update(string("id=")+toString(guildid));
	int a;
	for(a=0;a<(int)members.size();a++)
		if(members.at(a).dbid!=-1)
		{
			sqlquery &s3=dbcharacters_g;
			s3.addupdate("contrib", toString(members.at(a).contrib));
			s3.addupdate("moneycontrib", toString(members.at(a).moneycontrib));
			s3.addupdate("surrender", toString(members.at(a).surrender));
			s3.addupdate("rank", toString(members.at(a).rank));
			s3.addupdate("rank2", toString(members.at(a).rank2));
			s3.addupdate("nick", members.at(a).nick);
			s3.update("id="+toString(members.at(a).dbid));
		}
}

void tguild::deleteguild(tplayer *pl)
{
	buffer bs;
	{
		pmutex::unlocker gm=guildmutex.lock();
		if((insiege)||(closetosiege))return;
		if(disbanded)return;
		if(waring)return;
		/*TODO: multicastolni mindenkinek*/
		for(int a=0;a<(int)members.size();a++)
		{
			if((members.at(a).dbid!=-1)&&(members.at(a).p!=0))
			{
				ul mmm=members.at(a).p->asyncbuffermutex.lock();
				members.at(a).p->acmd.push(tplayer::ac_quit_guild);
			}
		}
		bs.cmd(-1, 0x9d) << 0 << guildid;
		clusters->multicast(bs);

		sqlquery &s4=dbguilds;
		s4.del("id="+toString(guildid));

		sqlquery &s3=dbcharacters;
		s3.addupdate("guild", "0");
		s3.update("guild="+toString(guildid));
		{
			pmutex::unlocker m=guildtreemutex.lock();
			guilds.erase(guildid);
			owguilds.erase(owner);
			if(name!="")nameguilds.erase(name);
		}
		{
			pmutex::unlocker m=globalguildmutex.lock();
			nguilds--;
			if(this->guildid==maxguildid)
			{
				int a=-1;
				pmutex::unlocker m=guildtreemutex.lock();
				for(std::map<int, tguild*>::iterator i=guilds.begin();i!=guilds.end();++i)
					if(i->first>a)a=i->first;
				maxguildid=-1;
			}
			if(this->owner==maxguildownerid)
			{
				int a=-1;
				pmutex::unlocker m=guildtreemutex.lock();
				for(std::map<int, tguild*>::iterator i=owguilds.begin();i!=owguilds.end();++i)
					if(i->first>a)a=i->first;
				maxguildownerid=-1;
			}
		}

	}
//	delete this;
	if(siegemoney>0)
	{
		ul m=dbguildsiege_mutex.lock();
		dbguildsiege.del("guildid = " + toString(guildid));
	}
	disbanded=true;
	
}

tguild::tguild(tplayer *p, int dbid, int plevel, int pjob, std::string &pname, int id)
:level(1), exp(0),logo(0),gwwin(0),gwlose(0),gwgiveup(0)
,nmembers(0),bankmoney(0),nloggedin(0),waring(0),cwing(false),cwtime(0),warpacket(0),warendpacket(0),siegemoney(0)
,disbanded(false),insiege(false),closetosiege(false),siegecluster(0)
{
	{
		pmutex::unlocker gm=guildmutex.lock();
		members.resize(80);

		rigths.resize(5, 0x1f);
		pay.resize(5, 0);
		nranks.resize(5, 0);

		guildid=p->dbid;
		owner=p->dbid;
		buffer bs;
		created(&bs, p, id , pname);
		clusters->multicast(bs);
		created(p->bs, p, id, pname);

		guildid=owner;
		join(p, dbid, plevel, pjob, pname);

		sqlquery &s4=dbguilds;
		s4.addupdate("id", toString(guildid));
		s4.addupdate("ownerid", toString(owner));
		s4.addupdate("name", name);
		s4.addupdate("notice", gnotice);
		s4.addupdate("exp", toString(exp));
		s4.addupdate("level", toString(level));
		s4.addupdate("logo", toString(logo));
		s4.addupdate("gwwin", toString(gwwin));
		s4.addupdate("gwlose", toString(gwlose));
		s4.addupdate("gwgiveup", toString(gwgiveup));
		s4.addnull("guildbank");
		s4.addupdate("bankmoney", toString(bankmoney));
		s4.addupdate("nmembers", toString(nmembers));
		s4.addupdate("r1", "31");
		s4.addupdate("r2", "31");
		s4.addupdate("r3", "31");
		s4.addupdate("r4", "31");
		s4.addupdate("r5", "31");
		s4.addupdate("m1", "0");
		s4.addupdate("m2", "0");
		s4.addupdate("m3", "0");
		s4.addupdate("m4", "0");
		s4.addupdate("m5", "0");
		s4.addupdate("cwtime", "0");
		s4.insert();
		s4.freeup();

		{
			pmutex::unlocker gm=guildtreemutex.lock();
//			tguild *ii=this;
			guilds.insert(std::pair<int, tguild*>(guildid, this));
			owguilds.insert(std::pair<int, tguild*>(owner, this));
			if(name!="")nameguilds.insert(std::pair<std::string, tguild*>(name, this));
		}
	}
	updateglobalid();

	
}

tguild::tguild(sqlquery &s4)
:nloggedin(0),nmembers(0),waring(0),cwing(false),disbanded(false),siegemoney(0),insiege(false),closetosiege(false),siegecluster(0)
{
	{
		pmutex::unlocker gm=guildmutex.lock();

		rigths.resize(5, 0x1f);
		pay.resize(5, 0);
		nranks.resize(5, 0);
		members.resize(80);

		guildid=toInt(s4["id"]);
		owner=toInt(s4["ownerid"]);
		level=toInt(s4["level"]);
		exp=toInt(s4["exp"]);
		logo=toInt(s4["logo"]);
		gwwin=toInt(s4["gwwin"]);
		gwlose=toInt(s4["gwlose"]);
		gwgiveup=toInt(s4["gwgiveup"]);
		name=s4["name"];
		gnotice=s4["notice"];
		bankmoney=toInt(s4["bankmoney"]);
		//nmembers;
		rigths.at(0)=toInt(s4["r1"]);
		rigths[1]=toInt(s4["r2"]);
		rigths[2]=toInt(s4["r3"]);
		rigths[3]=toInt(s4["r4"]);
		rigths[4]=toInt(s4["r5"]);
		pay.at(0)=toInt(s4["m1"]);
		pay[1]=toInt(s4["m2"]);
		pay[2]=toInt(s4["m3"]);
		pay[3]=toInt(s4["m4"]);
		pay[4]=toInt(s4["m5"]);
		cwtime=toInt(s4["cwtime"]);
		{
			pmutex::unlocker gm=guildtreemutex.lock();
			guilds.insert(std::pair<int, tguild*>(guildid, this));
			owguilds.insert(std::pair<int, tguild*>(owner, this));
			if(name!="")nameguilds.insert(std::pair<std::string, tguild*>(name, this));
		}
		{
			ul m=dbguildsiege_mutex.lock();
			sqlquery &s1=dbguildsiege;
			s1.selectw("guildid=" + toString(guildid), "money");
			if(s1.next())
			{
				siegemoney=toInt(s1[0]);
			}
			s1.freeup();
		}


		char *i=s4.getptr("guildbank");
		if(i!=0)wh.load(i);
		sqlquery &s3=dbcharacters;
		s3.selectw("guild="+toString(guildid), "guildslot, id, name, job, level, rank, rank2, nick, contrib, moneycontrib, surrender");
		int a;
		while(s3.next())
		{
			a=toInt(s3[0]);
			members.at(a).dbid=toInt(s3[1]);
			members.at(a).name=s3[2];
			members.at(a).job=toInt(s3[3]);
			members.at(a).level=toInt(s3[4]);
			members.at(a).rank=toInt(s3[5]);
			members.at(a).rank2=toInt(s3[6]);
			if((a>0)&&(members[a].rank==0))
			{
				members[a].rank=4;
				members[a].rank2=0;
			}
			nranks[members.at(a).rank]++;
			members.at(a).nick=s3[7];
			members.at(a).contrib=toInt(s3[8]);
			members.at(a).moneycontrib=toInt(s3[9]);
			members.at(a).surrender=toInt(s3[10]);
			nmembers++;
		}
	}
	updateglobalid();
}


int tguild::getsiegemoney()
{
	pmutex::unlocker gm=guildmutex.lock();
	return siegemoney;
}

bool tguild::applysiege(int addmoney)
{
	pmutex::unlocker gm=guildmutex.lock();
	if(addmoney<siegemoney+1)return false;
	int a=addmoney-siegemoney;
	if(bankmoney<a)return false;
	bankmoney-=a;
	siegemoney=addmoney;
	ul m=dbguildsiege_mutex.lock();
	sqlquery &s1=dbguildsiege;
	s1.addupdate("guildid", toString(guildid));
	s1.addupdate("name", name);
	s1.addupdate("money", toString(siegemoney));
	s1.replace();
	return true;
}

void tguild::getwarstatus(tplayer *p)
{
	if(disbanded)return;
	if(waring!=0)
	{	
		*p->ooo << 0xf000b037 << 0x0ec6 << waring->guildid << guildid;
		p->raiseooo();
		p->bs->cmd(p->getId(), 0x7a) << wid;
	}
}

bool tguild::login(tplayer *p, int guildslot, int dbid, int id)
{
	bool send_gw=false;
	bool retval=false;
	{
		pmutex::unlocker gm=guildmutex.lock();
		if(disbanded)return false;
		if(dbid==members[guildslot].dbid)
		{
			members[guildslot].p=p;
			members[guildslot].id=id;
			sendlogin(p);
	
			send_gw=(insiege)&&(p->cl==siegecluster)&&(members[guildslot].linedup);
			nloggedin++;
	
			retval=true;
		}
	}
	if(send_gw)player_gw_login(p);
	return retval;
}

void tguild::logout(tplayer *p, int guildslot, int dbid)
{
	bool teleportup=false;
	{
		pmutex::unlocker gm=guildmutex.lock();
		if(disbanded)return;
		if(dbid==members[guildslot].dbid)
		{
			members[guildslot].p=0;
			nloggedin--;
			teleportup=(insiege)&&(members[guildslot].teleported_down);
		}
	}
	if(teleportup)player_teleportup(p);
}

bool tguild::join(tplayer *p, int dbid, int plevel, int pjob, std::string &pname)
{
//	if(p->guildid!=0)return false;
	int a,i=-1;
	if(disbanded)return false;
	for(a=0;a<(int)members.size();a++)
	{
		if((i==-1)&&(members.at(a).dbid==-1))i=a;
		if(members.at(a).dbid==dbid)return false;
	}
	if(i!=-1)
	{
		a=i;
		members.at(a).dbid=dbid;
		members.at(a).contrib=0;
		members.at(a).moneycontrib=0;
		members.at(a).surrender=0;
		members.at(a).level=plevel;
		members.at(a).job=pjob;
		members.at(a).name=pname;
		members.at(a).nick="";
		members.at(a).p=p;
		members.at(a).rank2=0;

		if(i==0)members.at(a).rank=0;
		else members.at(a).rank=4;
		nranks[members.at(a).rank]++;

		p->guildid=guildid;
		p->guild=this;
		p->guildslot=a;
		p->clearsp();
		p->save();	//


		nmembers++;
		nloggedin++;
		sendlogin(p);
		return true;
	}
	return false;
}

bool tguild::invited(tplayer *p, int dbid, int plevel, int pjob, std::string &pname, int id)
{
	ul mm=guildmutex.lock();
	if(insiege)return false;
	if(disbanded)return false;
	if(this->waring!=0)return false;
	if((nmembers>=nguildmembers[level-1])||(nmembers>=(int)members.size()))return false;
//	if(p->guild!=0)return false;
	/*TODO*/
//	buffer bs;
//	bs << 0xffffff33 << dbid << 0 << guildid;/*1*/
//	bs.sndpstr(pname);
//	guildmulticast2(bs);

	buffer bs;
	bs << (char)0x95 << 687;
	bs.sndpstr(p->name);
	bs.inc();

	guildmulticast4(bs);

	p->bs->cmd(id, 0x9b) << guildid;
	join(p, dbid, plevel, pjob, pname);
	for(int a=0;a<(int)members.size();a++)
	{
		if((members.at(a).dbid!=-1)&&(members.at(a).dbid!=dbid)&&(members.at(a).p!=0))
		{
			sendlogin(members.at(a).p);
		}
	}
	return true;
}

void tguild::promote(tplayer *p, int dbid, int rank)
{
	ul mmm=guildmutex.lock();
	if(disbanded)return;
	if(((rigths[members[p->guildslot].rank]&r_rank)!=0)&&(rank>=0)&&(rank<=4)&&(rank>members[p->guildslot].rank))
	{
		if(nranks[rank]<nranked[rank])
			for(int a=1;a<(int)members.size();a++)
			{
				if(members.at(a).dbid==dbid)
				{
					if(members.at(a).rank>members[p->guildslot].rank)
					{
						nranks[members.at(a).rank]--;
						members.at(a).rank=rank;/*1*/
						nranks[rank]++;
						members.at(a).rank2=0;
						buffer bs;
						bs << 0xffffff3a << dbid << rank;
						guildmulticast2(bs);
					}
					break;
				}
			}
	}
}

void tguild::classup(tplayer *p, int dbid, int rank2)
{
	ul mmm=guildmutex.lock();
	if(disbanded)return;
	if(((rigths[members[p->guildslot].rank]&r_rank)!=0)&&(rank2>=0)&&(rank2<=1))
	{
		int r=rank2;
		if(r==0)r=-1;
		for(int a=1;a<(int)members.size();a++)
		{
			if(members.at(a).dbid==dbid)
			{
				if(members.at(a).rank>members[p->guildslot].rank)
				{
					members.at(a).rank2+=r;
					if(members.at(a).rank2>2)members.at(a).rank2=2;
					else if(members.at(a).rank2<0)members.at(a).rank2=0;
					else
					{
						buffer bs;
						bs << 0xffffff74 << dbid << members.at(a).rank2;
						guildmulticast2(bs);
					}
				}
				break;
			}
		}
	}
}

int tguild::leave(tplayer *r, int dbid)
{
	buffer bs;
	ul mm=guildmutex.lock();
	if(insiege)return -1;
	if(disbanded)return -1;
	if(dbid==members.at(0).dbid)return -1;
	if((dbid!=r->dbid)&&(r->dbid!=members.at(0).dbid))return 0;
	for(int a=0;a<(int)members.size();a++)
	{
		if(members.at(a).dbid==dbid)
		{
			if((r!=members.at(0).p)&&(r!=members.at(a).p))return 0;
			if(members.at(a).p!=0)
			{
				{
					ul mm=members.at(a).p->asyncbuffermutex.lock();
					members.at(a).p->acmd.push(tplayer::ac_quit_guild);
/*
					ul mm=(r==members.at(a).p)?pmutex::dontlock():guildmutex.relockwhen(members.at(a).p->playermutex);
					if(members.at(a).p!=0)
					{
						members.at(a).p->bs->cmd(members.at(a).p->getId(), 0x95) << 693;
						members.at(a).p->bs->sndpstr(name);
						members.at(a).p->tomulticast->cmd(members.at(a).p->getId(), 0x9b) << 0;
						members.at(a).p->guildid=0;
						members.at(a).p->guild=0;
					}
*/
				}
				nloggedin--;
			}else
			{
				/*1*///SAVE CHAR DATA!
				sqlquery &s3=r->cl->dbcharacters;
				s3.addupdate("guild", "0");
				s3.update("id="+toString(members.at(a).dbid));
			}
//			buffer bs.cmd(members.at(a).dbid, 0x9b) << 0;
			buffer bs;
			bs << (char)0x95 << 692;
			bs.sndpstr(members.at(a).name);
			bs.inc();

			nranks[members.at(a).rank]--;
			nmembers--;
			members.at(a).dbid=-1;
			members.at(a).p=0;

			guildmulticast4(bs);

			for(int a=0;a<(int)members.size();a++)
			{
				if((members.at(a).dbid!=-1)&&(members.at(a).p!=0))
				{
					sendlogin(members.at(a).p);
				}
			}
			return 1;
		}
	}
	return 0;
}


void tguild::sendlogin(tplayer *p)
{
//		ul mmm=guildmutex.lock();
		if(bankmoney<0)
		{
			logger.log("guildmoney is less than 0!\n");
			bankmoney=0;
		}
		if(exp<0)
		{
			logger.log("guild exp is less than 0!\n");
			exp=0;
		}
		int a;
		ul mmm=p->asyncbuffermutex.lock();
		if(p->asyncbuffer==0)p->asyncbuffer=new buffer;
		buffer *bs=p->asyncbuffer;
		bs->cmd(-1, 0x9e) << owner << owner;
		*bs << owner;	//gleader dbid
		*bs << 1;
		bs->sndpstr(name);	/*11*/
		*bs << logo;	//logo
		*bs << 0;//bankmoney;	//penya
		*bs << gwwin;	//win
		*bs << gwlose;	//lose
		*bs << gwgiveup;	//surrender
		for(a=0;a<5;a++)*bs<<rigths.at(a);
		for(a=0;a<5;a++)*bs<<pay.at(a);
		bs->sndpstr(gnotice);
		*bs << 0;//exp;	//guild exp
		*bs << level;	//guild level
		*bs << 0;

		*bs	<< (short)nmembers;	//nusers

		tguildmember *m;
		int b;
		for(a=0;a<(int)members.size();a++)
		{
			if(members.at(a).dbid!=-1)
			{
				m=&members.at(a);
				*bs << m->dbid;	//dbid
				*bs	<< 0xffffff9a;
				*bs << m->contrib;	//penya contrib
				*bs << m->moneycontrib;	//contrib
				*bs << (short)0;
				*bs << (short)m->surrender;	//number of times surrendered
				*bs << (char)m->rank;	//rank
	
				*bs << 0;	//some status icons... 
				*bs << m->job;	//job 
				*bs << m->level;	//level
				b=0;
				if(m->p!=0)b=1;
				*bs << b;	//online

				*bs << 0 << (short)0 << 0;
				bs->sndpstr(m->nick);	//nick

			}
		}

		*bs << (char)0 << (short)0;
		for(a=0;a<(int)members.size();a++)
		{
			if(members.at(a).dbid!=-1)
			{
				p->asyncbuffer2.push_back(buffer());
				p->asyncbuffer2.back() << 0xffffff74 << members.at(a).dbid << members.at(a).rank2;
			}
		}
		if((waring!=0)&&(warpacket!=0))if(warpacket->getcommandnumber()>0)bs->add(*warpacket);
}

void tguild::contribution(tplayer *contributor, int dbid, int questcontrib, int moneycontrib)
{
	bool tosave=false;
	{
	ul mm=guildmutex.lock();
	if(disbanded)return;
	bool levelled=false;
	if((contributor->guild==this)&&(members[contributor->guildslot].dbid==dbid))
	{
		exp+=questcontrib;
		bankmoney+=moneycontrib;

		while(level<50)
		{
			if((guildexp[level].mdrop<=exp)&&(guildexp[level].money<=bankmoney))
			{
				exp-=guildexp[level].mdrop;
				bankmoney-=guildexp[level].money;
				level++;
				levelled=true;
			}else break;
		}
		tosave=levelled;
//		bs->cmd(-1, 0xfc) << guildid << contributor << 1 << 2 << 3 << 4 << (short)level;

		members[contributor->guildslot].contrib+=questcontrib;
		members[contributor->guildslot].moneycontrib+=moneycontrib;
		buffer bs;
		bs.cmd(-1, 0xfc) << guildid << dbid;
		bs << questcontrib << moneycontrib << exp << bankmoney << (short)level;
		guildmulticast3(bs);
	}
	}
	if(tosave)this->save();
}


void tguild::created(buffer *bs, tplayer *p, int id, std::string &pname)
{
	*bs << id << (char)0x9c << guildid << owner;	//xy formed the guild message
	bs->sndpstr(pname);
	*bs << 0;
	bs->inc();
}

void tguild::setnotice(string &n)
{
	{
		ul gm=guildmutex.lock();
		if(disbanded)return;
		gnotice =n;
		buffer bs;
		bs.cmd(-1, 0xfd) << guildid;
		bs.sndpstr(gnotice);
		guildmulticast3(bs);
	}
	this->save();
}

void tguild::setname(string &n, tplayer *p)
{
	bool tosave=false;
	{
	ul gm=guildmutex.lock();
	if(disbanded)return;
	if(name=="")
	{
		if(n.length()<18)
		{
			bool found;
			{
				pmutex::unlocker m=globalguildmutex.lock();
				std::map<std::string, tguild*>::iterator i=nameguilds.find(n);
				found=(i!=nameguilds.end());
			}
			if(!found)
			{
				name=n;
				buffer bs;
				bs << 0xf000b032 << guildid;
				bs.sndpstr(name);
				clusters->multicastooo(bs);
				tosave=true;
			}else p->messagebox("Guild name already in use");
		}else p->messagebox("Guild name is too long");
	}
	}
	if(tosave)this->save();
}

void tguild::setlogo(int a)
{
	bool tosave=false;
	{
	ul gm=guildmutex.lock();
	if(disbanded)return;
	if(logo==0)
	{
		logo=a;
		buffer bs;
		bs.cmd(-1, 0xfb) << guildid << logo;
		clusters->multicast(bs);
		tosave=true;
	}
	}
	if(tosave)this->save();
}

void tguild::setpay(int a, int b)
{
	ul gm=guildmutex.lock();
	if(disbanded)return;
	if((a>=0)&&(a<5))
	{
		pay.at(a)=b;
		buffer bs;
		bs.cmd(-1, 0xff) << a << b;
		guildmulticast3(bs);
	}
}

void tguild::setrigths(int a, int b, int c, int d, int e)
{
	ul gm=guildmutex.lock();
	if(disbanded)return;
	a=31;
	rigths.at(0)=a;
	rigths[1]=b;
	rigths[2]=c;
	rigths[3]=d;
	rigths[4]=e;
	buffer bs;
	bs.cmd(-1, 0xfe) << a << b << c << d << e;
	guildmulticast3(bs);
}

void tguild::guildchat(tplayer *p, int guildslot, int dbid, const std::string &name, const char *text)
{
	ul gm=guildmutex.lock();
	if(disbanded)return;
	if((members[guildslot].dbid==dbid))
	{
//		buffer bs;
//		bs << 0xffffff39 << dbid;
//		bs.sndpstr(name);
//		bs.sndpstr(text);
//		guildmulticast2(bs);
		buffer bs2;
		bs2 << 0xffffff39 << p->getId();
		bs2.sndpstr(name);
		bs2.sndpstr(text);
		guildmulticast2(bs2);
	}
}

void tguild::guildmulticast(buffer &bs)
{
	int a,b;
	tguildmember *p;
	for(a=0,b=0;(a<(int)members.size());a++)
		if(members.at(a).dbid!=-1)
		{
			b++;
			p=&members.at(a);
			if(p->p!=0)
			{
				{
					pmutex::unlocker m=p->p->asyncbuffermutex.lock();
					if(p->p->asyncbuffer==0)p->p->asyncbuffer=new buffer;
					*p->p->asyncbuffer << p->dbid;
					(p->p->asyncbuffer)->copy(bs);
				}
			}
		}
}

void tguild::guildmulticast3(buffer &bs)
{
	int a,b;
	tguildmember *p;
	for(a=0,b=0;(a<(int)members.size());a++)
		if(members.at(a).dbid!=-1)
		{
			b++;
			p=&members.at(a);
			if(p->p!=0)
			{
				{
					pmutex::unlocker m=p->p->asyncbuffermutex.lock();
					if(p->p->asyncbuffer==0)p->p->asyncbuffer=new buffer;
					(p->p->asyncbuffer)->copy(bs);
				}
			}
		}
}

void tguild::guildmulticast31(buffer &bs, tplayer *toskip)
{
	int a,b;
	tguildmember *p;
	for(a=0,b=0;(a<(int)members.size());a++)
		if(members.at(a).dbid!=-1)
		{
			b++;
			p=&members.at(a);
			if((p->p!=0)&&(p->p!=toskip))
			{
				{
					pmutex::unlocker m=p->p->asyncbuffermutex.lock();
					if(p->p->asyncbuffer==0)p->p->asyncbuffer=new buffer;
					(p->p->asyncbuffer)->copy(bs);
				}
			}
		}
}

void tguild::guildmulticast4(buffer &bs)
{
	int a,b;
	tguildmember *p;
	for(a=0,b=0;(a<(int)members.size());a++)
		if(members.at(a).dbid!=-1)
		{
			b++;
			p=&members.at(a);
			if(p->p!=0)
			{
				{
					pmutex::unlocker m=p->p->asyncbuffermutex.lock();
					if(p->p->asyncbuffer==0)p->p->asyncbuffer=new buffer;
					*p->p->asyncbuffer << p->id;
					(p->p->asyncbuffer)->copy(bs);
				}
			}
		}
}

void tguild::guildmulticast2(buffer &bs)
{
	int a,b;
	tguildmember *p;
	for(a=0,b=0;(a<(int)members.size());a++)
		if(members.at(a).dbid!=-1)
		{
			b++;
			p=&members.at(a);
			if(p->p!=0)
			{
				{
					pmutex::unlocker m=p->p->asyncbuffermutex.lock();
					p->p->asyncbuffer2.push_back(buffer());
					p->p->asyncbuffer2.back().copy(bs);
//				if(p->p->asyncbuffer2==0)p->p->asyncbuffer2=new buffer;
//					(p->p->asyncbuffer2)->copy(bs);
				}
			}
		}
}


int tguild::getowner()
{
	ul mm=guildmutex.lock();
	if(disbanded)return -1;
	return owner;
}

int tguild::getguildid()
{
	ul mm=guildmutex.lock();
	if(disbanded)return -1;
	return guildid;
}

bool tguild::contains(tplayer *p)
{
	ul mm=guildmutex.lock();
	if(disbanded)return false;
	return (p->dbid==members[p->guildslot].dbid);
}

void tguild::setnick(tplayer *p, int dbid, const std::string &nick)
{
	ul mm=guildmutex.lock();
	if(disbanded)return;
	int a;

	if((this->rigths[members[p->guildslot].rank]&r_title)==0)return;

	for(a=0;a<(int)members.size();a++)
	{
		if(members.at(a).dbid==dbid)
		{
			members.at(a).nick=nick;
			buffer bs;
			bs << 0xffffff75 << dbid;
			bs.sndpstr(nick);
			guildmulticast2(bs);
			break;
		}
	}
}

void tguild::showwh(tplayer *p)
{
	int a,b;
	ul mm=guildmutex.lock();
	if(disbanded)return;
	buffer &bs=*p->bs;
	bs.cmd(p->getId(), 0xfa) << 0 << bankmoney;
	for(a=0;a<0x2a;a++)bs << a;
	b=0;
	for(a=0;a<0x2a;a++)if(wh.usedslots.at(a))b++;
	bs << (char)b;
	
	for(a=0;a<0x2a;a++)
	{
		if(wh.usedslots.at(a))
		{
			bs << (char)a << a;
			wh.itemek.at(a).snddata(&bs);
		}
	}
	for(a=0;a<0x2a;a++)bs << a;
}

int tguild::towh(tplayer *p, item &t)
{
	ul mm=guildmutex.lock();
	if(disbanded)return 0;
	buffer bs;
	int a=t.num;
	/*if(nloggedin>1)*/wh.setplayer(p, &bs);
//	else wh.setplayer(p, 0);
	int r=wh.insert(t);	

	/*if(nloggedin>1)*/if(r<a)guildmulticast31(bs,p);
	return r;
}

void tguild::fromwh(tplayer *p, int a, int b)
{
	int c;
	ul mm=guildmutex.lock();
	if(disbanded)return;
	item t;

	if((this->rigths[members[p->guildslot].rank]&r_item)==0)return;

	if(wh.usedslots.at(a))
		if(wh.itemek.at(a).num>=b)
		{
			t=wh.itemek.at(a);
			t.num=b;
			c=p->inv.pickup(&t, true);
			if(c<b)
			{
//				item t=wh.itemek.at(a);
				t.num=wh.removeitem(a, b-c);
				p->bs->cmd(p->getId(), 0xd4) << (char)1 << a;
				t.snddata(p->bs);
//				if(nloggedin>1)
				{
					buffer bs;
					bs.cmd(p->getId(), 0xd4) << (char)3 << a;
					t.snddata(&bs);
					guildmulticast31(bs,p);/*9*/
				}
			}
		}
}

int tguild::moneyfromguild(tplayer *p, int a)
{
	ul mm=guildmutex.lock();
	if(disbanded)return 0;

	if((this->rigths[members[p->guildslot].rank]&r_penya)==0)return 0;

	if(bankmoney<a)a=bankmoney;

	bankmoney-=a;
	if(a>0)
	{
		members[p->guildslot].moneycontrib-=a;
		buffer bs;
		bs.cmd(-1, 0xfc) << guildid << p->dbid;
		bs << 0 << -a << exp << bankmoney << (short)level;
//		bs.cmd(p->getId(), 0xd4);
//		bs << (char)0 << a << members[p->guildslot].dbid << (char)0;
		guildmulticast3(bs);

	}

	return a;
}

void tguild::createcloack(tplayer *p)
{
	ul mm=guildmutex.lock();
	if(disbanded)return;
	if(logo==0)
	{
		p->messagebox("Must set guild logo first!");
		return;
	}
//	if(bankmoney)
	item t(4601+logo);
	if(p==members.at(0).p)
	{
		t.num=1;
	
		buffer bs;
		if(bankmoney>1000)
		{
			bs.cmd(p->getId(), 0xd4);
			bs << (char)0 << 1000 << members[p->guildslot].dbid/*-1*/ << (char)0;

			buffer bs;
			wh.setplayer(p, &bs);
			if(wh.insert(t)>0)
			{
				p->bs->cmd(p->getId(), 0x55);
			}else 
			{
				bankmoney-=1000;
				guildmulticast31(bs, p);
			}
		}
	}
}

void tguild::changelead(tplayer *p, int id1, int id2)
{
	return;
/*
	ul mm=guildmutex.lock();
	if((members.at(0).id==id1)&&(members.at(0).p==p))
	{
		int a;
		for(a=0;a<(int)members.size();a++)
		{
			if(members.at(a).dbid==id2)
			{
				tguildmember b=members.at(0);
				members.at(0)=members.at(a);
				members.at(a)=b;
				members.at(a).rank=members.at(0).rank;
				members.at(a).rank2=members.at(0).rank2;
				members.at(0).rank=0;
				members.at(0).rank2=0;
				return;
			}
		}
	}
*/
}
/*
void tguild::cwstart()
{
	ul mm=guildmutex.lock();
	cwing=true;
}

void tguild::cwend()
{
	ul mm=guildmutex.lock();
	cwing=false;
}

void tguild::setcw(time_t t)
{
	ul mm=guildmutex.lock();
	cwtime=t;
}

void tguild::cwteleportout(cluster *cl)
{
	int a,b;
	vector3d<> v(3892, 0, 3961), v2;

	tguildmember *p;
	for(a=0,b=0;(a<(int)members.size())&&(b<nloggedin);a++)
		if(members.at(a).dbid!=-1)
		{
			b++;
			p=&members.at(a);
			if(p->p!=0)
			{
				if(p->p->cl==cl)	//only ppl on the same cluster
				{
					ul m=p->p->playermutex.lock();	//only lock the whole player on the same cluster (for reciever thread)
					if(p->p->getmapid()==1)
					{
						v2=p->p->getpos();
						v2.y=0;
						v2-=v;
						if(v2.sqrlength()<=110*110)p->p->townblink();
					}
//					pmutex::unlocker m=p->p->asyncbuffermutex.lock();
//					if(p->p->asyncbuffer==0)p->p->asyncbuffer=new buffer;
//					*p->p->asyncbuffer << p->id;
//					(p->p->asyncbuffer)->copy(bs);
				}
			}
		}
}
*/



void tguild::siege_createline_start()
{
	bool iswaring=false;
	{
		ul mm=guildmutex.lock();
		closetosiege=true;
		iswaring=waring!=0;
		for(int a=0;a<(int)members.size();++a)
		{
			members[a].linedup=false;
		}
		siegeline.push_back(&members[0]);
		members[0].linedup=true;
	}
	if(iswaring)this->piece(owner);

}

bool tguild::siege_createline_finish(cluster *cl)
{
	ul mm=guildmutex.lock();
	if(siegeline.size()>0)
	{
		insiege=true;
		for(std::list<tguildmember*>::iterator i=siegeline.begin();i!=siegeline.end();++i)
		{
			(*i)->lives=3;
			(*i)->point=0;
			cl->addtosiege(*i);
		}
		siegepoint=0;
		siegecluster=cl;
		return true;
	}
	closetosiege=false;
	return false;
}

bool tguild::siege_createline_auto()
{
	ul mm=guildmutex.lock();
	int b=0;
	for(int a=0;(a<(int)members.size());a++)
	{
		if(members.at(a).dbid!=-1)
		{
			if(members[a].p!=0)
			{
				if(members[a].p->cl->getclusternumber()==0)
				{
					siegeline.push_back(&members[a]);
					b++;
					if(b==10)break;
				}
			}
		}
	} 
	return (b>0);
}
void tguild::siege_teleportin()
{
	std::list<tplayer *> tosend;
	{
		ul mm=guildmutex.lock();
		siege_players_down=0;
		requested_players_down=0;
		for(std::list<tguildmember*>::iterator i=siegeline.begin();i!=siegeline.end();++i)
		{
			(*i)->teleported_down=false;
			if((*i)->p!=0)
			{
				if((*i)->p->cl==siegecluster)
				{
//					(*i)->p->changemap(202, 1366, 105, 1280);
//					sendsiegedata((*i)->p);
//					(*i)->p->gwprepare();
					tosend.push_back((*i)->p);
				}else
				{
					(*i)->p->amessagebox("You have been lined up for guild siege, please relog to channel 1 and join siege");
				}
			}
		}
	}
	if(!tosend.empty())
	{
		for(std::list<tplayer*>::iterator i=tosend.begin();i!=tosend.end();++i)
		{
			(*i)->changemap(202, 1366, 105, 1280);
			sendsiegedata(*i);
			(*i)->gwprepare();
		}
		tosend.clear();
	}
}

void tguild::siege_teleportdown()
{
	ul mm=guildmutex.lock();
	int n=0;
	if(siege_players_down<5)
	{
		for(std::list<tguildmember*>::iterator i=siegeline.begin();(requested_players_down+siege_players_down<5)&&(i!=siegeline.end());++i)
		{
			bool sent=false;
			(*i)->siege_line_toend=false;
			if(((*i)->p!=0)&&((*i)->lives>0)&&(!(*i)->teleported_down))
			{
				if((*i)->p->cl==siegecluster)
				{
					(*i)->siegelistremover=i;
					(*i)->teleported_down=false;
					(*i)->p->siege_teleportdown();
					(*i)->p->gwprepare();
					requested_players_down++;
					sent=true;
				}
			}
			if((!sent)&&(!(*i)->teleported_down))
			{
				if(((*i)->lives>0))(*i)->lives--;
				(*i)->siege_line_toend=true;
				n++;
			}
		}
		if(n>0)
		{
			std::list<tguildmember*> toend;
			for(std::list<tguildmember*>::iterator i=siegeline.begin();i!=siegeline.end();++i)
			{
				if((*i)->siege_line_toend)
				{
					(*i)->siege_line_toend=false;
					toend.push_back(*i);
					i=siegeline.erase(i);
					if(i==siegeline.end())break;
				}
			}
			for(std::list<tguildmember*>::iterator i=toend.begin();i!=toend.end();++i)
			{
				siegeline.push_back(*i);
			}
			toend.clear();
		}

	}
}

void tguild::player_gw_login(tplayer *p)
{
	bool tosend=false;
	{
		ul mm=guildmutex.lock();
		if((requested_players_down+siege_players_down<5)&&(members[p->guildslot].linedup)&&(members[p->guildslot].lives>0)&&(p->cl==siegecluster))
		{
			for(std::list<tguildmember*>::iterator i=siegeline.begin();i!=siegeline.end();++i)
			{
				if((*i)->p==p)
				{
					tosend=true;
					(*i)->siegelistremover=i;
					p->siege_teleportdown();
					requested_players_down++;
					break;
				}
			}
		}
	}
	if(tosend)sendsiegedata(p);
}

void tguild::sendsiegedata(tplayer *p)
{
	if(p->cl==siegecluster)
	{
		siegecluster->sendsiegedata(p);
		p->bs->cmd(-1, 0xb8) << (char)32 << 0 << siegeline.size();
		for(std::list<tguildmember*>::iterator i=siegeline.begin();i!=siegeline.end();++i)
		{
			*p->bs << (*i)->dbid << 1;
		}
	}
}

void tguild::sendsiegepacket(tplayer *p)
{
	ul mm=guildmutex.lock();
	*p->bs << guildid << siegeline.size();
	for(std::list<tguildmember*>::iterator i=siegeline.begin();i!=siegeline.end();++i)
	{
		*p->bs << (*i)->dbid << 1;
	}
}

void tguild::player_teleportdown(tplayer *p, int pos)
{
	bool teleport=false;
	{
		ul mm=guildmutex.lock();
		teleport=(pos>=0)&&(pos<4)&&(siege_players_down<5)&&(!members[p->guildslot].teleported_down);
		
	}
	if(teleport)
	{
		p->changemap(202, gwtele[pos].x, gwtele[pos].y, gwtele[pos].z, false);
		ul mm=guildmutex.lock();
		requested_players_down--;
		siege_players_down++;
		members[p->guildslot].teleported_down=true;
		siegecluster->gw_attackable();
	}
}

void tguild::player_teleportup(tplayer *p)
{
	bool teled_up=false;
	{
		ul mm=guildmutex.lock();
		if(members[p->guildslot].teleported_down)
		{
			teled_up=true;
			members[p->guildslot].teleported_down=false;
			members[p->guildslot].lives--;
			siegeline.erase(members[p->guildslot].siegelistremover);
			siegeline.push_back(&members[p->guildslot]);
			siege_players_down--;
		}
	}
	if(teled_up)
	{
		p->changemap(202, 1366, 105, 1280);
		siege_teleportdown();
		siegecluster->gw_attackable();
	}
}

void tguild::siege_end(bool won)
{
	ul mm=guildmutex.lock();
	closetosiege=false;
	insiege=false;
	for(std::list<tguildmember*>::iterator i=siegeline.begin();i!=siegeline.end();++i)
	{
		(*i)->teleported_down=false;
		if((*i)->linedup)(*i)->point+=(*i)->lives;
		if((*i)->p!=0)if((*i)->p->getmapid()==202)(*i)->p->changemap(1, 6968.384766f, 100.0f, 3328.863037f);
	}
	siegeline.clear();

}

bool tguild::addtolineup(tplayer *p, const std::string &n)
{
	ul mm=guildmutex.lock();
	if(!closetosiege)return false;
	for(int a=0;(a<(int)members.size());a++)
	{
		if(members.at(a).dbid!=-1)
		{
			if(members[a].name==n)
			{
				if(members[a].linedup)return false;
				siegeline.push_back(&members[a]);
				members[a].linedup=true;
				return true;
			}
		}
	}
	return false;
}

tplayer *tguild::getmaster()
{
	ul mm=guildmutex.lock();
	tplayer *r=0;
	if(members[0].p!=0)r=members[0].p;
	return r;
}