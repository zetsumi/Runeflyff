#include "stdafx.h"
#include "player.h"
#include "network.h"
#include "logger.h"
#include "cluster.h"
#include "adat.h"
#include "grid.h"
#include "error.h"
#include "funkciok.h"
#include "stringf.h"
#include "main.h"
#include "objactions.h"
#include "charserver.h"

using namespace std;

void tplayer::init(int sck1, reciever &sr1, std::string &cip, int ticket1, cluster *cl1)
{
	{
	    std::lock_guard<std::mutex> guard(this->asyncbuffermutex);
		if(asyncbuffer!=0)
		{
			delete asyncbuffer;
			asyncbuffer=0;
		}
		asyncbuffer2.clear();
		while(!acmd.empty())acmd.pop();
	}

    std::lock_guard<std::mutex> guard(this->playermutex);
	last_upgrade_time=0;
	siege_attackable=false;
	requested_teleportdown=false;
	username="";
	gwspawn=0;
	last_active=GetTickCount();
	guild=0;
	muted=0;
	duel=0;
	duelchallanged=0;
	duelchallanger=0;
	duelcountdowntime=0;
	loadingp=false;
	addedact=false;
	state3=0;
	state4=1;
	delete sr;
	delete srr;
	sr=new reciever();
	srr=new reciever();
	sr->init(sck1);
	srr->init(sck1);
	toprocess=true;
	srready=false;
	fuel=0;
	duelbegan=false;

	activation=0;
	refresherhold=0;
	vitalx=0;

	tosendvals.clear();
	psetbonuses.clear();
	quests.clear();
	finishedquests.clear();

	lodelight=0;
	addstat1type=0;
	da=da_none;
	nexthprec=0;
	delay=0;
	duel=0;
	nextattack=0;
	nextfood=0;
	nextcheer=0;
	nextchat=0;
	nextkarmarecover=0;
	cheered=0;
	maxexp=0;
//	maxexp2=0;
	penelaty=0;
	weapontype=0;
	weapontype2=0;

	state=1;
	state2=0;
	stance=0;
	tosendvals.clear();
	delete tomulticast;
	tomulticast=new buffer;
	delete tomulticast2;
	tomulticast2=new buffer;
	tomulticast2->forbidden=this;
	((character_base*)this)->init();
	reslevel=0;
	int a;
	errorstate=false;
//	used=true;
	sck=sck1;
	ticket=ticket1;
	id=ticket1;
	inv.setplayerid(id);
	createremovepacket();
	*sr=sr1;
	sr->rewind();

	ip=cip;
	cl=cl1;
	reslevel=0;
	ncheers=0;
	nextcheer=cl->ido+60*60*1000;
	asspower=100;
	asstime=cl->ido;
	loaded=false;
	delete bs;
	delete ooo;

//	newbuffer();
	bs=new buffer;
	*bs << (char)0x5e;
	bslen=bs->insert(0);
	*bs << 0xff00 << id;
	bscmd=bs->insert((short)0);

	newooobuffer();
	gridcell.remove();
	modelid=11;
	modeltype=5;
	for(a=0;a<(int)stats.size();a++)stats[a]=0;
	for(a=0;a<(int)buffplace.size();a++)buffplace[a]=0;
	lastup=0;
	atkmin=0;
	atkmax=0;
	defmin=0;
	defmax=0;
	guildid=0;
	guild=0;
	guildslot=0;
	party=0;
//	silent=true;
	reslevel=-1;
	this->psetbonuses.clear();
	binit();
	cl->nplayers++;
}

void tplayer::removefromcluster()
{
	int a;
	handleacmd();
	{
		ul m=globalplayersmutex.lock();
		globaldbidplayers.erase(dbid);
		globalnameplayers.erase(name);
	}
	siegeremover.remove();
	if(!cl->endprg)
	{
		cserver->to_dc.push(username);
	}

	if(cl->grid.getmap(mapid)!=mapid)
	{
		mapid=1;
		pos=towns[0];
	}

	if(guild!=0)
	{
		if((guild->getowner()==dbid)&&(guild->getwaring()!=0))guild->leaderkilled();
	}

	cl->dbidplayers.erase(dbid);
	cl->nameplayers.erase(name);

	if(addedact)cl->removeplayeract(playeractremover);
	addedact=false;

	if(duel!=0)
	{
		if(duelbegan)
		{
			duel->duelwin();
			duellose(false);
		}else
		{
			duel->bs->cmd(duel->getId(), 0x3f) << dbid << 2;
			duel->bs->cmd(duel->getId(), 0x33) << 0;
		}
		duel->duel=0;
		duel=0;
	}

	removepsywalls();
	shieldskills.clear();
	weaponskills.clear();
	unsummonpet();


	{
        std::lock_guard<std::mutex> asyncbufferguard(this->asyncbuffermutex);
		for(a=0;a<(int)friendlist.size();a++)
		{
			if((friendlist[a].dbid!=-1)&&(friendlist[a].p!=0))
			{
                std::lock_guard<std::mutex> friendlistguard(friendlist[a].p->asyncbuffermutex);
				if(friendlist[a].p!=0)	//do not remove this.
				{
					friendlist[a].p->asyncbuffer2.push_back(buffer());
					friendlist[a].p->asyncbuffer2.back() << 0xffffff67 << dbid << 1;
					friendlist[a].p->friendlist[friendlist[a].listindex].p=0;
				}
			}
		}
	}

	if(trade!=0)trade->del(this);
	trade=0;
	if(shop.getopened())shop.close(this);
	if(joinedshop!=0)joinedshop->logout(this);
	joinedshop=0;
	if(actionslotp!=0)
	{
		actionslotp->player=0;
		actionslotp->endit=true;
		actionslotp=0;
	}
	if(party!=0)party->logout(this);
	if(guild!=0)guild->logout(this, guildslot, dbid);

	setfollowed(0, 0);
	clearfollowers();
	setfocus(0);
	removefocuses();
//	clearbuffs();
	stopmove();
	bremovefromcluster();
//	bdeinit();

	gridcell.remove();

}

int tplayer::deinit()
{
	if(cl->endprg)return ticket;
	int retval=ticket,a;
	tosendvals.clear();
	psetbonuses.clear();
	quests.clear();
	finishedquests.clear();
	guild=0;
	bdeinit();

	if(ticket!=-1)
	{
		closesck(sck);
		if(dbid>=0)
		{
			try{
				cl->dbcharacters.beginupdate();
				cl->dbcharacters.addupdate("loggedin", "0");
				cl->dbcharacters.update("id="+toString(dbid));
			}catch(error &e)
			{
				logger.elog("error at deinit: %s\nCharacter won't be able to log back\n", e.what());
			}
		}

		if(loaded)
		{
			{
				std::unique_lock<std::mutex> guard(this->asyncbuffermutex, std::defer_lock);
				if(guard.try_lock())
				{
					if(asyncbuffer!=0)
					{
						delete asyncbuffer;
						asyncbuffer=0;
					}
					asyncbuffer2.clear();
				}
			}
		}

		ticket=-1;
		sck=0;

		delete tomulticast;
		tomulticast=0;
		delete tomulticast2;
		tomulticast2=0;

		buffers.clear();
		delete bs1;
		bs1=0;
		delete bs;
		bs=0;
		delete ooo;
		ooo=0;
		loaded=false;
		((character_base*)this)->deinit();
	}
	dbid=-1;
	cl->nplayers--;
//	logger.log("deinited\n");
	return retval;
}

tplayer::~tplayer()
{
//	setmoveing(false);
//	deinit();
}

tplayer::tplayer()
:ticket(-1), sck(0), ooo(0), bs(0), bs1(0)
,errorstate(false),sp2(0),trade(0),shop(this),joinedshop(0)
,tomulticast(0),tomulticast2(0),party(0),asyncbuffer(0),asyncbuffer2(0)
,sr(0),srr(0),toprocess(false),maxexp(0)
{
	pet=0;
	actionslotp=0;
	dbid=-1;
//	used=false;
	cl=0;
	int a;
	inv.init(this);
	skilllevels.resize(maxskillid);
	skillexp.resize(maxskillid);
	skillenabled.resize(maxskillid);
	cooldowns.resize(maxskillid, 0);

	for(a=0;a<3;a++)banks[a].init(this, a);
	type=ttplayer;
	aslot.resize(5);
	sslot.resize(24);
	fslot.resize(4);
	friendlist.resize(maxfriends);
	for(a=0;a<4;a++)fslot[a].resize(9);
}

tplayer::tplayer(const tplayer& a2)
:ticket(-1), sck(0), ooo(0), bs(0), bs1(0), errorstate(false)
,sp2(0),trade(0),shop(this),joinedshop(0)
,tomulticast(0),tomulticast2(0),party(0),asyncbuffer(0),asyncbuffer2(0)
,sr(0),srr(0),toprocess(false),maxexp(0)
{
	pet=0;
	actionslotp=0;
	dbid=-1;
//	used=false;
	cl=0;
	int a;
	inv.init(this);
	skilllevels.resize(maxskillid);
	skillexp.resize(maxskillid);
	skillenabled.resize(maxskillid);
	cooldowns.resize(maxskillid, 0);

	for(a=0;a<3;a++)banks[a].init(this, a);
	type=ttplayer;
	aslot.resize(5);
	sslot.resize(24);
	fslot.resize(4);
	friendlist.resize(maxfriends);
	for(a=0;a<4;a++)fslot[a].resize(9);
}

void tplayer::send()
{
	if(bs1==0)
	{
        std::lock_guard<std::mutex> guard(this->playermutex);
		if(bs!=0)if((bs1==0)&&buffers.empty()&&(bs->getcommandnumber()>0))raise();
		if((bs1==0)&&(!buffers.empty()))
		{
			bs1=new sendablebuffer(buffers.front());
			buffers.pop_front();
			bs1->initflush(sck);
		}
	}
	if(bs1!=0)
	{
		if(bs1->continueflush(&this->sendpuffer[0])==1)
		{
			last_active=cl->ido;
			delete bs1;
			bs1=0;
		}
	}
}

pmutex savemutex;
void tplayer::save(bool logout)
{
    std::lock_guard<std::mutex>(cl->savemutex);
	int a;
#ifdef _DEBUG
	unsigned long long t1=GetTickCount();
#endif

for(int w=0;w<3;w++)
{
	sqlquery *savequery;
	sqlquery *savebankquery;
	savequery=0;
	savebankquery=0;
	try
	{
	if(!loaded)return;
	savequery = new sqlquery(cl->saveconn);
	savebankquery = new sqlquery(cl->saveconn);
	savequery->settable("characters");
	savebankquery->settable("accounts");
	sqlquery &s1=*savequery;//cl->dbcharacters;
	s1.beginupdate();
	if(pos.x<0)pos.x=0;
	if(pos.y<0)pos.y=0;
	if(pos.z<0)pos.z=0;
	s1.addupdate("x", toString(pos.x));
	s1.addupdate("y", toString(pos.y));
	s1.addupdate("z", toString(pos.z));
	s1.addupdate("mapid", toString(mapid));
	s1.addupdate("hp", toString(hp));
	s1.addupdate("mp", toString(mp));
	s1.addupdate("fp", toString(fp));
	s1.addupdate("msgstate", toString(msgstate));
	s1.addupdate("exp1", toString(exp));
//	s1.addupdate("exp2", toString(exp2));
	s1.addupdate("pxp1", toString(pxp));
	s1.addupdate("money", toString(money));
	s1.addupdate("maxexp", toString(maxexp));
//	s1.addupdate("maxexp2", toString(maxexp2));
	if((stance<0)||(stance>100))stance=0;
	s1.addupdate("stance", toString(stance));
	s1.addupdate("face", toString(face));
	s1.addupdate("hair", toString(hair));
	s1.addupdate("haircolor", toString(haircolor));
	s1.addupdate("flylevel", toString(flylevel));
	s1.addupdate("flyexp", toString(flyexp));
	s1.addupdate("str", toString(str));
	s1.addupdate("sta", toString(sta));
	s1.addupdate("dex", toString(dex));
	s1.addupdate("intl", toString(intl));
	s1.addupdate("level", toString(level));
	s1.addupdate("job", toString(job));
	s1.addupdate("gender", toString(gender));
	s1.addupdate("pk", toString(pk));
	s1.addupdate("disp", toString(disp));
	s1.addupdate("pvp", toString(pvp));
	s1.addupdate("lodelight", toString(lodelight));
	s1.addupdate("statpoints", toString(statpoints));
	a=muted;
	if(a!=0)a-=cl->ido;
	s1.addupdate("muted", toString(a));

	a=-1;
	if(party!=0)a=party->getticket();
	s1.addupdate("party", toString(a));
	if(accesslevel==0xff)a=1;
	else a=0;
	s1.addupdate("accesslevel", toString(a));
	if(guild==0)guildid=0;
	s1.addupdate("guild", toString(guildid));
	s1.addupdate("guildslot", toString(guildslot));
	char *i;
	char *j;

	i=new char[skilllevels.size()*9];
	j=i;
	for(a=0;a<(int)skilllevels.size();a++)
	{
		*(int*)j=skilllevels[a];
		j+=4;
		*(int*)j=skillexp[a];
		j+=4;
		*j=skillenabled.at(a);
		j++;
	}
	s1.addeupdate("skills", i, skilllevels.size()*9);

	delete[] i;


//	if(save_inv)
	{
		i=new char[73*item::itemdatasize];
		inv.save(i);
		s1.addeupdate("inventory", i, 73*item::itemdatasize);
		delete[] i;
	}

	i=new char[100*12];
	a=savebuffs((int*)i);
	s1.addeupdate("buffs", i, a);
	delete[] i;

//	if(save_friendlist)
	{
		i=new char[friendlist.size()*8];
		{
		    std::lock_guard<std::mutex> guard(this->asyncbuffermutex);
			for(a=0;a<(int)friendlist.size();a++)
			{
				*(int*)(&i[a*8+0])=friendlist[a].dbid;
				*(int*)(&i[a*8+4])=friendlist[a].listindex;
			}
		}
		s1.addeupdate("friendlist", i, friendlist.size()*8);
		delete[] i;
	}

	int a,b,c,d;

//	if(save_quests)
	{
		a=quests.size();
		i=new char[4+sizeof(pquestdata)*a];
		*(int*)i=a;
		a=4;
		for(std::map<int, pquestdata>::iterator j=quests.begin();j!=quests.end();++j)
		{
			*(int*)(&i[a])=j->second.id;a+=4;
			*(int*)(&i[a])=j->second.state;a+=4;
			*(int*)(&i[a])=j->second.flags;a+=4;
			*(int*)(&i[a])=j->second.n;a+=4;
		}
		s1.addeupdate("quests", i, a);
		delete[] i;

		a=finishedquests.size();
		i=new char[4+sizeof(int)*a];
		*(int*)i=a;
		a=4;
		for(std::set<int>::iterator j=finishedquests.begin();j!=finishedquests.end();++j)
		{
			*(int*)(&i[a])=*j;a+=4;
		}
		s1.addeupdate("finishedquests", i, a);
		delete[] i;
	}

	if(save_slots)
	{
		std::vector2<char> ss;
		ss.reserve((aslot.size()+fslot.size()*fslot[0].size()+sslot.size())*20);
		for(a=0;a<(int)aslot.size();a++)
		{
			i=aslot[a].save(b, inv);
			for(c=0;c<b;c++)ss.push_back(i[c]);
			delete[] i;
		}
		for(a=0;a<(int)sslot.size();a++)
		{
			i=sslot[a].save(b, inv);
			for(c=0;c<b;c++)ss.push_back(i[c]);
			delete[] i;
		}
		for(d=0;d<(int)fslot.size();d++)
			for(a=0;a<(int)fslot[d].size();a++)
			{
				i=fslot[d][a].save(b, inv);
				for(c=0;c<b;c++)ss.push_back(i[c]);
				delete[] i;
			}

		s1.addeupdate("slots", &ss[0], ss.size());
	}

#ifdef _DEBUG
	logger.log("Basic data in %d\n", (unsigned int)((unsigned)GetTickCount()-t1));
#endif

//	s1.update("id="+toString(dbid));

//	if(save_bank)
	{
		sqlquery &s2=*savebankquery;//cl->dbaccounts;
		s2.beginupdate();

		i=new char[banks[0].getnitems()*item::itemdatasize];
		banks[0].save(i);
		s2.addeupdate("bank1", i, banks[0].getnitems()*item::itemdatasize);
		delete[] i;
		i=new char[banks[1].getnitems()*item::itemdatasize];
		banks[1].save(i);
		s2.addeupdate("bank2", i, banks[1].getnitems()*item::itemdatasize);
		delete[] i;
		i=new char[banks[2].getnitems()*item::itemdatasize];
		banks[2].save(i);
		s2.addeupdate("bank3", i, banks[2].getnitems()*item::itemdatasize);
		delete[] i;
		s2.addupdate("bank1money", toString(bankmoney[0]));
		s2.addupdate("bank2money", toString(bankmoney[1]));
		s2.addupdate("bank3money", toString(bankmoney[2]));
//		s2.update("id="+toString(accid));
	}
	save_inv=true;
	save_bank=false;
	save_slots=false;
	save_friendlist=false;
	save_quests=false;

	cluster::charsavedata s3;
	s3.dbid=dbid;
	s3.accid=accid;
	s3.s1=savequery;
	s3.s2=savebankquery;
	s3.p=0;
	if(logout)s3.p=this;
	cl->charsave_queue.push(s3);

	goto saved;
	}catch(error &e)
	{
		logger.log("Error at player save: %s\n", e.what());
		delete savequery;
		delete savebankquery;
	}catch(std::exception &e)
	{
		logger.log("Error at player save: %s\n", e.what());
		delete savequery;
		delete savebankquery;
	}

}
	logger.log("character save error. dbid %d\n", dbid);

saved:;

#ifdef _DEBUG
	t1=(unsigned)GetTickCount()-t1;
	logger.log("Saved in %d\n", (unsigned int)t1);
#endif
}

int p_x,p_y,p_z,p_mapid,p_hp,p_mp,p_fp,p_exp1, p_exp2, p_pxp1, p_money, p_msgstate;
int p_accid, p_name, p_flylevel, p_flyexp, p_str, p_sta, p_dex, p_intl, p_level, p_job, p_gender;
int p_hair, p_haircolor, p_face, p_pvp, p_pk, p_disp, p_maxexp, p_maxexp2, p_stance, p_statpoints;
int p_inventory, p_party, p_guild, p_guildslot, p_skills, p_quests, p_finishedquests;
int p_slots, p_friendlist, p_buffs, p_accslot, p_lodelight, p_accesslevel, p_muted;

void initplayerdbvals(sqlquery &s1)
{
	p_accid=s1.getTableColumnIndex("accid");
	p_name=s1.getTableColumnIndex("name");

	p_x=s1.getTableColumnIndex("x");
	p_y=s1.getTableColumnIndex("y");
	p_z=s1.getTableColumnIndex("z");
	p_mapid=s1.getTableColumnIndex("mapid");
	p_hp=s1.getTableColumnIndex("hp");
	p_mp=s1.getTableColumnIndex("mp");
	p_fp=s1.getTableColumnIndex("fp");

	p_exp1=s1.getTableColumnIndex("exp1");
//	p_exp2=s1.getTableColumnIndex("exp2");
	p_pxp1=s1.getTableColumnIndex("pxp1");

	p_money=s1.getTableColumnIndex("money");
	p_msgstate=s1.getTableColumnIndex("msgstate");

	p_flylevel=s1.getTableColumnIndex("flylevel");
	p_flyexp=s1.getTableColumnIndex("flyexp");
	p_str=s1.getTableColumnIndex("str");
	p_sta=s1.getTableColumnIndex("sta");
	p_dex=s1.getTableColumnIndex("dex");
	p_intl=s1.getTableColumnIndex("intl");
	p_level=s1.getTableColumnIndex("level");
	p_job=s1.getTableColumnIndex("job");
	p_gender=s1.getTableColumnIndex("gender");

	p_hair=s1.getTableColumnIndex("hair");
	p_haircolor=s1.getTableColumnIndex("haircolor");
	p_face=s1.getTableColumnIndex("face");

	p_pvp=s1.getTableColumnIndex("pvp");
	p_pk=s1.getTableColumnIndex("pk");
	p_disp=s1.getTableColumnIndex("disp");

	p_maxexp=s1.getTableColumnIndex("maxexp");
//	p_maxexp2=s1.getTableColumnIndex("maxexp2");
	p_stance=s1.getTableColumnIndex("stance");
	p_statpoints=s1.getTableColumnIndex("statpoints");

	p_inventory=s1.getTableColumnIndex("inventory");
	p_party=s1.getTableColumnIndex("party");
	p_guild=s1.getTableColumnIndex("guild");
	p_guildslot=s1.getTableColumnIndex("guildslot");

	p_skills=s1.getTableColumnIndex("skills");
	p_quests=s1.getTableColumnIndex("quests");
	p_finishedquests=s1.getTableColumnIndex("finishedquests");
	p_slots=s1.getTableColumnIndex("slots");
	p_friendlist=s1.getTableColumnIndex("friendlist");
	p_buffs=s1.getTableColumnIndex("buffs");
	p_accslot=s1.getTableColumnIndex("accslot");
	p_lodelight=s1.getTableColumnIndex("lodelight");
	p_accesslevel=s1.getTableColumnIndex("accesslevel");
	p_muted=s1.getTableColumnIndex("muted");
}


void tplayer::load(int &lparty)
{
#ifdef _DEBUG
	unsigned long long t1=GetTickCount();
#endif
	save_bank=false;
	save_inv=true;
	save_slots=false;
	save_friendlist=false;
	save_quests=false;
	try
	{
	sqlquery &s1=cl->dbcharacters4load;
	char *p;
//	int *i;
//	logger.log("dbid=%d\n", dbid);
	s1.selectw("id="+toString(dbid), "*, length(friendlist), length(slots), length(inventory), length(skills), length(buffs), length(quests), length(finishedquests)");
	if(!s1.next())
	{
		s1.freeup();
		this->errorstate=true;
		return;
	}
	int a,b;
/*
	for(a=0;a<400;a++)
	{
		skilllevels[a]=1;
		skillexp[a]=0;
		skillenabled.at(a)=true;
	}
*/
	accid=toInt(s1[p_accid]);
	slotnum=toInt(s1[p_accslot]);
	name=s1[p_name].c_str();
	mapid=toInt(s1[p_mapid]);
//	logger.log("mapid=%d\n", mapid);
	maxhp=1000;
	maxmp=1000;
	maxfp=1000;
	pos.x=toFloat(s1[p_x]);
	pos.y=toFloat(s1[p_y]);
	pos.z=toFloat(s1[p_z]);
	lastpos=pos;
	hp=toInt(s1[p_hp]);
	mp=toInt(s1[p_mp]);
	fp=toInt(s1[p_fp]);
	exp=toUnsignedLongLong(s1[p_exp1]);
	expbeforeko=exp;
//	exp2=toInt(s1[p_exp2]);
	pxp=toInt(s1[p_pxp1]);
	money=toInt(s1[p_money]);
	if(money<0)money=0;
	msgstate=toInt(s1[p_msgstate]);
	bankmoney[0]=0;
	bankmoney[1]=0;
	bankmoney[2]=0;
	flylevel=toInt(s1[p_flylevel]);
	flyexp=toInt(s1[p_flyexp]);
	str=toInt(s1[p_str]);
	sta=toInt(s1[p_sta]);
	dex=toInt(s1[p_dex]);
	intl=toInt(s1[p_intl]);
	level=toInt(s1[p_level]);
	gender=toInt(s1[p_gender]);
	job=toInt(s1[p_job]);
	hair=toInt(s1[p_hair]);
	haircolor=toInt(s1[p_haircolor]);
	muted=toInt(s1[p_muted]);
	if(muted>0)muted+=cl->ido;
	pk=toInt(s1[p_pk]);
	disp=toInt(s1[p_disp]);
	pvp=toInt(s1[p_pvp]);
	face=toInt(s1[p_face]);
	lodelight=toInt(s1[p_lodelight]);
	maxexp=toUnsignedLongLong(s1[p_maxexp]);
//	maxexp2=toInt(s1[p_maxexp2]);
	stance=toInt(s1[p_stance]);
	a=toInt(s1[p_accesslevel]);
	if(a<1)accesslevel=0x46;
	else accesslevel=0xff;
	state2=stance;

	statpoints=toInt(s1[p_statpoints]);
	inv.load(s1.getptr(p_inventory));
	party=0;
	lparty=toInt(s1[p_party]);
	guildid=toInt(s1[p_guild]);
	guildslot=toInt(s1[p_guildslot]);
	guild=tguild::getguild(guildid);
	if(guild==0)guildid=0;

	p=s1.getptr(p_skills);
	if(((s1["length(skills)"])!="NULL")&&(p!=0))
	{
		for(a=0;a<(int)skilllevels.size();a++)
		{
			skilllevels[a]=*(int*)p;
			p+=4;
			skillexp[a]=*(int*)p;
			p+=4;
			skillenabled.at(a)=(*p!=0);
			p++;
		}
	}else
	{
		for(a=0;a<(int)skilllevels.size();a++)
		{
			skilllevels[a]=1;
			skillexp[a]=0;
			skillenabled.at(a)=true;
		}
	}

	p=s1.getptr(p_quests);
	if(((s1["length(quests)"])!="NULL")&&(p!=0))
	{
		a=*(int*)p;p+=4;
		if(a<1000)
		for(b=0;b<a;b++)
		{
			quests.insert(std::pair<int, pquestdata>(*(int*)(p), pquestdata(*(int*)(p), *(int*)(p+4), *(int*)(p+8), *(int*)(p+12))));
			p+=16;
		}
	}

	p=s1.getptr(p_finishedquests);
	if(((s1["length(finishedquests)"])!="NULL")&&(p!=0))
	{
		a=*(int*)p;p+=4;
		if(a<1000)
		for(b=0;b<a;b++)
		{
			finishedquests.insert(*(int*)p);
			p+=4;
		}
	}

	p=s1.getptr(p_slots);
	if(((s1["length(slots)"])!="NULL")&&(p!=0))
	{
		for(a=0;a<(int)aslot.size();a++)p+=aslot[a].load(p, inv, job);
		for(a=0;a<(int)sslot.size();a++)p+=sslot[a].load(p, inv, job);
		for(b=0;b<(int)fslot.size();b++)
			for(a=0;a<(int)fslot[b].size();a++)p+=fslot[b][a].load(p, inv, job);
	}else
	{
		for(a=0;a<(int)aslot.size();a++)aslot[a].clear();
		for(a=0;a<(int)sslot.size();a++)sslot[a].clear();
		for(b=0;b<(int)fslot.size();b++)
			for(a=0;a<(int)fslot[b].size();a++)fslot[b][a].clear();

	}
	p=s1.getptr(p_friendlist);
	if(((s1["length(friendlist)"])!="NULL")&&(p!=0))
	{
        std::lock_guard<std::mutex> guard(this->asyncbuffermutex);
		for(a=0;a<(int)friendlist.size();a++)
		{
			friendlist[a].dbid=*(int*)(&p[a*8+0]);
			friendlist[a].listindex=*(int*)(&p[a*8+4]);
			friendlist[a].p=0;
		}
	}else
	{
        std::lock_guard<std::mutex> guard(this->asyncbuffermutex);
		for(a=0;a<(int)friendlist.size();a++)
		{
			friendlist[a].dbid=-1;
			friendlist[a].listindex=-1;
			friendlist[a].p=0;
		}
	}

	p=s1.getptr(p_buffs);
	if(((s1["buffs"])!="NULL")&&(p!=0))loadbuffs((int*)p);
	sqlquery &s2=cl->dbaccounts4load;
	s2.selectw("id="+toString(accid), "bank1money, bank2money, bank3money, bank1, bank2, bank3, length(bank1), length(bank2), length(bank3)");

	if(s2.next())
	{
		this->bankmoney[0]=toInt(s2[0]);
		this->bankmoney[1]=toInt(s2[1]);
		this->bankmoney[2]=toInt(s2[2]);
		if(bankmoney[0]<0)bankmoney[0]=0;
		if(bankmoney[1]<0)bankmoney[1]=0;
		if(bankmoney[2]<0)bankmoney[2]=0;
		if((s2[6])!="NULL")banks[0].load(s2.getptr(3));
		else banks[0].clear();
		if((s2[7])!="NULL")banks[1].load(s2.getptr(4));
		else banks[1].clear();
		if((s2[8])!="NULL")banks[2].load(s2.getptr(5));
		else banks[2].clear();
	}
	s2.freeup();

//	state=1;
//	state2=0;
//	speed=6.2f;
//	flying=false;
	flying=(inv.getMask(inventory::eqoff2+13)!=-1);

	mpconsumption=0;
	fpconsumption=0;
	berserk=false;
	silence=false;
	sitting=false;

	cl->dbidplayers.insert(std::pair<int, tplayer*>(dbid, this));
	cl->nameplayers.insert(std::pair<std::string, tplayer*>(name, this));
	{
		ul m=globalplayersmutex.lock();
		globaldbidplayers.insert(std::pair<int, tplayer*>(dbid, this));
		globalnameplayers.insert(std::pair<std::string, tplayer*>(name, this));
	}



	updatesp();
	delete sp2;
	sp2=0;
//	*bs << 0;

//	for(a=0;a<54;a++)*bs << 0;
//--
//	*bs << 0;

		loaded=true;
		s1.beginupdate();
		s1.addupdate("loggedin", "1");
		s1.addupdate("clusterid", toString(cl->getclusternumber()));
		s1.update("id="+toString(dbid));
	}
	catch(error &e)
	{
		logger.log("%s\n", e.what());
		this->errorstate=true;
	}
	catch(std::exception &e)
	{
		logger.log("%s\n", e.what());
		this->errorstate=true;
	}
	teleportmode=false;
	lastsave=cl->ido;

#ifdef _DEBUG
	t1=(unsigned)GetTickCount()-t1;
	logger.log("Loaded in %d\n", (unsigned int)t1);
#endif
}

void tplayer::clearsp2()
{
	delete sp2;
	sp2=0;
}
void tplayer::clearsp()
{
	delete sp;
	sp=0;
	bsx=0;
	bsy=0;
	bsz=0;
	bsdir=0;
	bshp=0;
	bsstate=0;
	bsstate2=0;
	bspk=0;
	bspvp=0;
	bsdisp=0;
	bslevel=0;
}


void tplayer::onHpChanged()
{
	if(hp<=0)
	{
		state=0x08000000;
		state3=8;
		removepsywalls();
	}
	else
	{
		state=0;
		state3=0;
	}
	if(sp!=0)
	{
		*bshp=hp;
		*bsstate3=state3;
	}
}

void tplayer::updatesp()
{
	int a;
	delete sp;
	sp=new buffer;
	sp->forbidden=this;
	sp->cmd(id, 0xf0);
	a=modelid;
	if(a==11)a+=gender;
	*sp << (char)modeltype << (short)(a) << (short)0 << (char)modeltype << (short)(a) << (short)size;
	bsx=sp->insert(pos.x);
	bsy=sp->insert(pos.y);
	bsz=sp->insert(pos.z);
	bsdir=sp->insert((short)dir);
	*sp << id;

	bsstate3=sp->insert(state3);
//	*sp << (unsigned short)8
//	bsstate4=sp->insert(state4);
	*sp << (char)1;	//5 1
	bshp=sp->insert(hp);
	bsstate=sp->insert(state);
	bsstate2=sp->insert(state2);
//	*sp << (unsigned char)1;		// 1
	bsstate4=sp->insert(state4);

	sp->sndpstr(name);
	*sp << (char)gender << (char)0 << (char)hair << haircolor << (char)face << dbid << (char)job << (short)str << (short)sta << (short)dex << (short)intl;
	bslevel=sp->insert((short)level);
//	*sp << 0x40b0;
//	*sp << 0x0bb8;
	*sp << 0x04b0;
//	*sp << 0x0bb8;
	*sp<< 0;
	if(guild!=0)
	{
		*sp << (char)1 << guildid << 0;// << 0xc60b;
	}else
	{
		*sp << (char)0;
	}
	*sp << 0;
//	*sp << (char)0;
	*sp << (char)1 << 0x046f << 0;
	*sp << (unsigned char)0x46;//; << 0x00020000 << 0 <<0x09e9;// << 0x01f8;//<< 0x12c5; //<< 0x1f6;
	*sp << (char)0 << (char)0 << (char)2 << (char)0;
//	*sp << 0 << 0x0af7;
//	*sp << 0 << 0x469c << (short)0x08e9;
	*sp << 0 << 0x0505; //<< (short)0;
	bsdisp=sp->insert((short)disp);
	bspk=sp->insert(pk);
	bspvp=sp->insert(pvp);
	*sp << 0x1980 << (short)0;
//	*sp << 0x40b0 << 0 << (char)0 << 0 << (char)0 << (unsigned char)0x46 << 0x00020000 << 0 << 0x12c5; //<< 0x1f6;
//	bspk=sp->insert((short)1);	//pk??
//	bsdisp=sp->insert(2);	//disp??
//	bspvp=sp->insert(3);	//pvp??
	*sp << (short)1 << (char)0;// << 0 << 0;
}

buffer& tplayer::spawn()
{
	int a,a1,b;

	if(sp==0)
	{
		delete sp2;
		sp2=0;
		updatesp();
	}
	if(sp2==0)
	{
		sp2=new buffer;
		sp2->add(*sp);

		for(a=inventory::eqoff;a<inventory::nitems;a++)
		{
			if(inv.getMaskedItem(a)!=0)
			{
				if(inv.getMaskedItem(a)->getId()>0)
				{
					*sp2 << (short)inv.getMaskedItem(a)->upgrade << (char)inv.getMaskedItem(a)->element << (char)inv.getMaskedItem(a)->eupgrade;
				}else *sp2 << 0;//b=0;
			}else *sp2 << 0;//b=0;
	    }
		for(a=0;a<23;a++)*sp2 << 0;

		if(shop.getopened())sp2->sndpstr(shop.getname());	//playershop
		else *sp2 << 0;

		b=0;
		for(a=inventory::eqoff;a<inventory::nitems;a++)
			if(inv.getMaskedItem(a)!=0)if(inv.getMaskedItem(a)->getId()>0)b++;
		*sp2 << (char)b;


		for(a1=inventory::eqoff;a1<inventory::nitems;a1++)
		{
			if(inv.getMaskedItem(a1)!=0)if(inv.getMaskedItem(a1)->getId()>0)
				*sp2 << (char)(a1-inventory::eqoff+2) << (short)inv.getMaskedItem(a1)->getId() << (char)inv.getMaskedItem(a1)->upgrade; //inv.itemek[a].upgrade
		}
		*sp2 << -1;

		sp2->forbidden=this;
	}

	delete sp9;
	sp9=new buffer(*sp2);

	spawnbufficons(sp9);

	if(this->isMoveing()>0)gotoxyz(sp9, id, cel.x, cel.y, cel.z);
	if(sitting)
	{
		sp9->cmd(id, 0x98);
		*sp9 << 4;
	}


	std::map<int, bonus>::iterator vi;
	for(vi=tosendvals.begin();vi!=tosendvals.end();++vi)
	{
		if(vi->second.val>0)addstat(sp9, id, vi->second.stat, vi->second.val);
		else if(vi->second.val<0)substat(sp9, id, vi->second.stat, -vi->second.val);
	}
	if(abletomove>0)
	{
		addstat(sp9, id, 11, 0);
		addstat(sp9, id, 64, 4096);
	}

	if(hp<=0)sp9->cmd(id, 0xc7) << id << 0;

	sp9->forbidden=this;
	return *sp9;
}


void tplayer::firstspawn(bool gotmail)
{
    int a,b,c,d;
//	buffer *bs=&getbs();
//	int pk=0;

	bs->cmd(id, 0xf0);
	a=modelid;
	if(a==11)a+=gender;
	*bs << (char)modeltype << a << (char)modeltype << (short)(a) << (short)size;
	*bs << pos.x << pos.y << pos.z << (short)0 << id;
	if(!flying)state2=stance;
	*bs << (short)0 << (char)1 << hp << state << state2 << (char)1;
	bs->sndpstr(name);
	*bs << (char)gender << (char)0 << (char)hair << haircolor << (char)face << dbid << (char)job;
	*bs << (short)str  << (short)sta  << (short)dex  << (short)intl << (short)level;
	*bs << 0x04b0 << 0;

	if(guild!=0)
	{
		*bs << (char)1 << guildid << 0;
	}else
	{
		*bs << (char)0;
	}

	*bs << 0 << (char)0 << (char)accesslevel;
	*bs << (char)0;
	a=0;
	if(gotmail)a=0x80;
	*bs	<< (unsigned char)(unsigned int)a;

	*bs << (char)2 << (char)0;
	*bs << 0 << 0x12c5;

	*bs << (short)disp << pk << pvp;
	*bs << 0 << 0;
	*bs  << (char)4;
	for(a=0;a<29;a++)*bs << 0;
	for(a=0;a<23;a++)*bs << 0;

	*bs << (short)mp << (short)fp << (short)flylevel << flyexp << money;
	*bs << exp;
	*bs << pxp;
	*bs << 0 << 0x0272 << 0 << 0x15;

/**/
	for(a=0;a<16;a++)*bs << 0;
	*bs << cl->grid.getmap(mapid) << pos.x << pos.y << pos.z;

	d=quests.size();
	(*bs) << (char)d;
	for(std::map<int, pquestdata>::iterator i=quests.begin();i!=quests.end();++i)
	{
		*bs << i->second.state << (short)i->first << i->second.n;
	}
	c=finishedquests.size();
	*bs << (char)c;
	for(std::set<int>::iterator i=finishedquests.begin();i!=finishedquests.end();++i)
	{
		*bs << ((short)*i);
	}

	*bs << 50000 << (short)0 << (short)0;

	for(a=0;a<31;a++)*bs << 0xffffffff;
	sendskills();

	*bs << (char)0 << 0x0036ee80;
//	*bs << (char)20 << -1;
	*bs << (char)slotnum; //slot
	*bs << bankmoney[0] << bankmoney[1] << bankmoney[2] << dbid;
	*bs << 0 << 2 << 0;
//	*bs << (unsigned char)255 << (unsigned char)255;
	*bs << (unsigned char)255 << (char)0 << (char)0 << 0;// << (char)0 << (char)0 << 0;

	b=0;
	for(a=0;a<inventory::nitems;a++)
	{
		(*bs) << inv.getMask(a);
		if(inv.getItem(a)->getId()>0)b++;
	}
	*bs << (unsigned char)b; //nitems

	for(b=0;b<inventory::nitems;b++)
	{
		if(inv.getItem(b)->getId()>0)
		{
			*bs << (char)b << b;
			inv.getItem(b)->snddata(bs);
		}
	}

	for(a=0;a<inventory::nitems;a++)*bs << inv.getLookup(a);
	for(c=0;c<3;c++)
	{
		b=0;
	    for(a=0;a<42;a++)
		{
			(*bs) << a;
             if(banks[c].getItem(a)->getId()>0)b++;
        }
        (*bs) << (unsigned char)b;
        for(a=0;a<42;a++)
		if(banks[c].getItem(a)->getId()>0)
		{
			*bs << (char)a << a;
			banks[c].getItem(a)->snddata(bs);
		}
    	for(a=0;a<42;a++)(*bs) << a;
    }



	*bs << -1 << (char)1;
	*bs << 0 << 1 << 2 << 3 << 4 << 5;
	*bs << (char)0;
	*bs << 0 << 1 << 2 << 3 << 4 << 5;
	*bs << 0 << 0 << 0 << 0 << (short)0;
raise();
return;

}

void tplayer::logincommands()
{
	int a,b,c;
	sqlquery &s4=cl->dbcharacters;

	bs->cmd(id, 0x97);

	c=0;
	for(a=0;a<(int)sslot.size();a++)
		if(sslot[a].getType()!=0)c++;
	*bs << c;	//S slot
	for(a=0;a<(int)sslot.size();a++)
		if(sslot[a].getType()!=0)
		{
			*bs << a << sslot[a].getType() << sslot[a].getId() << sslot[a].getType2() << a << 0 << 0;
			if(sslot[a].getType()==8)bs->sndpstr(sslot[a].getText());
		}

	c=0;
	for(b=0;b<(int)fslot.size();b++)
		for(a=0;a<(int)fslot[b].size();a++)
			if(fslot[b][a].getType()!=0)c++;
	*bs << c;	//F slot
	for(b=0;b<(int)fslot.size();b++)	//4-8
		for(a=0;a<(int)fslot[b].size();a++)
			if(fslot[b][a].getType()!=0)
			{
				*bs << b << a << fslot[b][a].getType() << fslot[b][a].getId() << fslot[b][a].getType2() << a << 0 << 1;
				if(fslot[b][a].getType()==8)bs->sndpstr(fslot[b][a].getText());
			}

	c=0;
	for(a=0;a<(int)aslot.size();a++)
		if(aslot[a].getType()!=0)c++;
	*bs << c;	//A slot
	for(a=0;a<(int)aslot.size();a++)
		if(aslot[a].getType()!=0)
		{
			*bs << a << aslot[a].getType() << aslot[a].getId() << aslot[a].getType2() << a << 0 << 2;
			if(aslot[a].getType()==8)bs->sndpstr(aslot[a].getText());
		}


	*bs << 0x64;
	raise();

	*ooo << 0xffffff67 << dbid << msgstate;
	raiseooo();

	bs->cmd(id, 0x68);
	bs->sndpstr("AdvPartyName");

	std::map<int, tplayer*>::iterator i;
	c=0;
//	logger.log("FRIENDS\n");
	{
        std::lock_guard<std::mutex> guard(this->asyncbuffermutex);
		for(a=0;a<(int)friendlist.size();a++)
		{
			if(friendlist[a].dbid!=-1)
			{
				c++;
				bool found;
				{
					{
						ul mm=globalplayersmutex.lock();
						i=globaldbidplayers.find(friendlist[a].dbid);
					}
					found=(i!=globaldbidplayers.end());
					if(found)
					{
						tplayer *p=i->second;
						friendlist[a].p=p;
						{
                            std::lock_guard<std::mutex> playerguard(p->asyncbuffermutex);
							if(p->friendlist[friendlist[a].listindex].dbid!=dbid)
							{
								friendlist[a].dbid=-1;
								friendlist[a].listindex=-1;
							}else
							{
								p->friendlist[friendlist[a].listindex].p=this;

								bs->cmd(dbid, 0x70);
								*bs << friendlist[a].dbid << p->job << (char)0;
								bs->sndpstr(p->name);
								*ooo << 0xffffff67 << friendlist[a].dbid << friendlist[a].p->msgstate;
								raiseooo();
								p->asyncbuffer2.push_back(buffer());
								p->asyncbuffer2.back() << 0xffffff67 << dbid << msgstate;
							}

						}
						i->second->say(name.c_str(), i->second->name.c_str(), "has logged in", dbid, i->second->dbid);
					}
				}

				if(!found)
				{
					s4.selectw("id="+toString(friendlist[a].dbid), "name, job");
					if(s4.next())
					{
						bs->cmd(dbid, 0x70);
						*bs << friendlist[a].dbid << toInt(s4[1]) << (char)0;
						bs->sndpstr(s4[0]);
					}else
					{
						friendlist[a].dbid=-1;
						friendlist[a].listindex=-1;
					}
					s4.freeup();
				}
			}
		}
	}

//	logger.log("nfriends=%d\n", c);
//	tomulticast->cmd(id, 0x28) << pk << pvp;
//	tomulticast->cmd(id, 0x40) << disp;
	settime(bs, id, cl->getclienttime());
	tguild::getallguildinfo(this);
	gwprepare();
//	greentext("tesztloginguild\n");

}

void tplayer::greentext(const std::string &c)
{
	logger.log("[SYS]: %s\n", c.c_str());
	bs->cmd(-1, 0xd0);
	*bs << id;
	bs->sndpstr("SYS");
	bs->sndpstr(c);
}
void tplayer::messagebox(const std::string &c)
{
	bs->cmd(id, 0xb7);
	bs->sndpstr(c);
}

void tplayer::amessagebox(const std::string &c)
{
    std::lock_guard<std::mutex> guard(this->asyncbuffermutex);
	if(asyncbuffer==0)asyncbuffer=new buffer;
	asyncbuffer->cmd(id, 0xb7);
	asyncbuffer->sndpstr(c);
}


int wtlookup[22]={0, w_sword, w_axe, w_stick, w_knuckle, w_staff, w_wand, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, w_yoyo, w_bow};
long double tplayer::calcWeaponAtk(int weapon)
{
	switch (weapon)
	{
/*sword*/    	case  1: return (str+stats[DST_STR]+stats[DST_STAT_ALLUP] - 12) * 4.5 + level * 1.1;
/*axe  */    	case  2: return (str+stats[DST_STR]+stats[DST_STAT_ALLUP] - 12) * 5.5 + level * 1.2;
/*stick*/    	case  3: return (str+stats[DST_STR]+stats[DST_STAT_ALLUP] - 10) * 3.0 + level * 1.3;
/*knuck*/    	case  4: return (str+stats[DST_STR]+stats[DST_STAT_ALLUP] - 10) * 5.0 + level * 1.2;
/*wand */    	case  6: return (intl+stats[DST_INT]+stats[DST_STAT_ALLUP] - 10) * 6.0 + level * 1.2;
/*staff*/    	case  5: return (str+stats[DST_STR]+stats[DST_STAT_ALLUP] - 10) * 0.8 + level * 1.1;

/*yoyo */    	case  20: return (str+stats[DST_STR]+stats[DST_STAT_ALLUP] - 12) * 4.2 + level * 1.1;
/*bow  */		case  21: return ((dex+stats[DST_DEX]+stats[DST_STAT_ALLUP] - 14) * 4.0 + (str+stats[DST_STR]+stats[DST_STAT_ALLUP]) * 0.2 + level * 1.3) * 0.7;
    	default:
        	return (str+stats[DST_STR]+stats[DST_STAT_ALLUP] - 10) * 0.8 + level * 1.1;
	}
}

int tplayer::calcAtk(int weapon, int baseAtk, int refine, int adjAtk)
{
    if(refine>10)refine=10;
    if(refine<0)refine=0;
	long double d = ((baseAtk * 2) + calcWeaponAtk(weapon) + adjAtk) * ((enctable[refine] + 100) * 0.01) + pow(refine, 1.5);
	return (int)d;
}

void tplayer::calcdef(int a, float *min, float *max)
{
	item *t=inv.getMaskedItem(a);
    float sz;
    if(t!=0){
    	if(t->GetI()!=0){
            sz=(float)((enctable[t->upgrade] + 100) * 0.01);
            *min+=t->GetI()->min*sz;
            *max+=t->GetI()->max*sz;
        }
    }
}


void tplayer::HMF()
{
    long double ld,a,b,c,d,e;
	float defmin=0, defmax=0;
//	int i1;
	item *t;
    int job=this->job;
    if(job>=24)job-=8;
    if(job>=16)job-=10;
//	int str=this->str+stats[DST_STAT_ALLUP]+stats[DST_STR];
	int sta=this->sta+stats[DST_STAT_ALLUP]+stats[DST_STA];
//	int dex=this->dex+stats[DST_STAT_ALLUP]+stats[DST_DEX];
	int intl=this->intl+stats[DST_STAT_ALLUP]+stats[DST_INT];

//	greentext("addatk: %d, addpatk: %d", addatk, addpatk);

	a = level * hpj[job] * 0.5;
	b = (level + 1.0) * 0.25;
	c = a * b;
	d = (sta) * 0.02 + 1.0;
	e = d * c + (sta) * 10.0 + 80.0;
    maxhp=(int)(((int)(e) + stats[DST_HP_MAX]) * (1 + stats[DST_HP_MAX_RATE] / 100.0));
	maxhp+=stats[DST_HPDMG_UP];


//	ld = ((sta*addsta) * 0.02 + 1.0) * ((level * hpj[job] * 0.5) * ((level + 1.0) * 0.25)) + (sta+addsta) * 10.0 + 80.0;
//    maxhp=(int)(((int)(ld) + addhp) * (1 + addPhp / 100.0));

    ld = (level + level + (intl) * 8.0) * mpj[job] + 22.0 + (intl) * mpj[job];
    maxmp=(int)(((int)(ld) + stats[DST_MP_MAX]) * (1 + stats[DST_MP_MAX_RATE] / 100.0));

    ld = (level + level + (sta) * 6.0) * fpj[job] + (sta) * fpj[job];
    maxfp=(int)(((int)(ld) + stats[DST_FP_MAX]) * (1 + stats[DST_FP_MAX_RATE] / 100.0));

    atkmin=0;
    atkmax=0;

	weapontype=0;
	weapontype2=0;
	sweapontype=0;
	t=inv.getMaskedItem(11+inventory::eqoff2);
    if(t!=0)
		if((t->GetI()!=0)&&(t->GetI()->eqslot==11))
		{
			sweapontype|=w_shield;
		}

	t=inv.getMaskedItem(10+inventory::eqoff2);
    if(t!=0)
		if((t->GetI()!=0)&&(t->GetI()->eqslot==10))
		{
			weapontype=t->GetI()->weapontype;
			sweapontype|=w_weapon;
			if(weapontype>0)sweapontype|=wtlookup[weapontype];
			atkmin=calcAtk(t->GetI()->weapontype,t->GetI()->min,t->upgrade, 0);
			atkmax=calcAtk(t->GetI()->weapontype,t->GetI()->max,t->upgrade, 0);
			switch(t->GetI()->weapontype)
			{
    			case  1:
					atkmin+=stats[DST_SWD_DMG];
					atkmax+=stats[DST_SWD_DMG];
					break;
    			case  2:
					atkmin+=stats[DST_AXE_DMG];
					atkmax+=stats[DST_AXE_DMG];
					break;
    			case  4:
					atkmin+=stats[DST_KNUCKLE_DMG];
					atkmax+=stats[DST_KNUCKLE_DMG];
					break;
    			case  20:
					atkmin+=stats[DST_YOY_DMG];
					atkmax+=stats[DST_YOY_DMG];
					break;
				case  21:
					atkmin+=stats[DST_BOW_DMG];
					atkmax+=stats[DST_BOW_DMG];
					break;
			}
		}
	t=inv.getMaskedItem(9+inventory::eqoff2);
    if(t!=0)
		if((t->GetI()!=0)&&(t->GetI()->eqslot==10))
		{
			weapontype2=t->GetI()->weapontype;
			atkmin+=calcAtk(t->GetI()->weapontype,t->GetI()->min,t->upgrade, 0)*3/4;
			atkmax+=calcAtk(t->GetI()->weapontype,t->GetI()->max,t->upgrade, 0)*3/4;
		}

//	atkmin+=;
//	atkmax+=;
	atkmin=atkmin*(stats[DST_ATKPOWER_RATE]+100)/100;
	atkmax=atkmax*(stats[DST_ATKPOWER_RATE]+100)/100;
	atkmin+=stats[DST_CHR_DMG]+stats[DST_HPDMG_UP];
	atkmax+=stats[DST_CHR_DMG]+stats[DST_HPDMG_UP];


/*
    i1=inv.mask[9+eqoff2];
    logit("i=%d ",i);
    if(i1>=0){

        logit("id=%d ", inv[i1].id);
    	i=checkweapon(inv[i1].id);
        logit("i=%d\n",i);
    	if(i!=-1){
            atkmin+=calcAtk(weaponlist[i][3],weaponlist[i][4],inv[i1].upgrade, 0)/2;
            atkmax+=calcAtk(weaponlist[i][3],weaponlist[i][5],inv[i1].upgrade, 0)/2;
        }
    }
*/

    if((atkmin==0)&&(atkmax==0))
    {
        atkmin=(int)(((0 * 2) + calcWeaponAtk(8) + 0) * ((0 + 100) * 0.01) + pow(0, 1.5));
        atkmax=atkmin+5;
    }


	calcdef(2+inventory::eqoff2, &defmin, &defmax);
	calcdef(4+inventory::eqoff2, &defmin, &defmax);
	calcdef(5+inventory::eqoff2, &defmin, &defmax);
	calcdef(6+inventory::eqoff2, &defmin, &defmax);
	calcdef(11+inventory::eqoff2, &defmin, &defmax);

    this->defmin=(int)(defmin/4+(level*leveldef[job])+((sta)*stadef[job])-defbase[job]);
    this->defmax=(int)(defmax/4+(level*leveldef[job])+((sta)*stadef[job])-defbase[job]);
    this->defmin+=stats[DST_ADJDEF]+stats[DST_DEFHITRATE_DOWN];
    this->defmax+=stats[DST_ADJDEF]+stats[DST_DEFHITRATE_DOWN];
    this->defmin=(int)(this->defmin*(stats[DST_ADJDEF_RATE]/100.0+1));
    this->defmax=(int)(this->defmax*(stats[DST_ADJDEF_RATE]/100.0+1));

	if(!flying)
	{
		speed=basespeed+basespeed*stats[DST_SPEED]*0.01f;
		if(speed<0)speed=0;
		if(state2==2)speed/=3.0;
	}
//	greentext(cl->print("atk %d %d", atkmin, atkmax));
//	greentext(cl->print("def %d %d", this->defmin, this->defmax));
//	greentext(cl->print("hp %d mp %d fp %d", maxhp, maxmp, maxfp));

}



void tplayer::sendskills()
{
    int a,b,skill;
    for(a=0;a<43;a++)
	{
		skill=skills[job][a];
		*bs << skill;
		if(skills[job][a]>=0)
		{
			if(skillenabled.at(skill))b=1; else b=0;
			*bs << skillexp[skill]-skillacc[skilllevels[skill]-1];
			*bs << (int)(b);
			*bs << skilllevels[skill];
		}
		else
		{
			*bs << 0 << 0 << 0;
		}
	}
}

void tplayer::sendskills2(int e)
{
    int a,b,skill;
    for(a=0;a<43+e;a++)
	{
		skill=skills[job][a];
		*bs << skill;
		if(skills[job][a]>=0)
		{
			if(skillenabled.at(skill))b=1; else b=0;
			*bs << skillexp[skill]-skillacc[skilllevels[skill]-1];
			*bs << (int)(b);
			*bs << skilllevels[skill];
		}
		else
		{
			*bs << 0 << 0 << 0;
		}
	}
}

void tplayer::raise()
{
	if(bs->getcommandnumber()>0)
	{
		*bslen=bs->length()-5;
		*bscmd=bs->getcommandnumber();
		buffers.push_back(*bs);
		delete bs;
		newbuffer();
	}
}

void tplayer::raiseooo()
{
	int a=ooo->length();
	if(a>5)
	{
		*ooolen=a-5;
		raise();
		buffers.push_back(*ooo);
		delete ooo;
		newooobuffer();
	}
}

void tplayer::newbuffer()
{
	bs=new buffer;
	*bs << (char)0x5e;
	bslen=bs->insert(0);
	*bs << 0xffffff00 << id;
	bscmd=bs->insert((short)0);
}

void tplayer::newooobuffer()
{
	ooo=new buffer();
	*ooo << (char)0x5e;
	ooolen=ooo->insert(0);
}



void tplayer::changemap2(int a, float fx, float fy, float fz)
{
	if(!cl->grid.validworld(a))return;

	int lmapid=mapid;
		clearfollowers();
		setfollowed(0, 0);
		buffer bs1;
		stopmove();

		unsummonpet();
		if(party!=0)party->logout(this);
		if(guild!=0)guild->logout(this, guildslot, dbid);
		gridcell.remove();
		bs->cmd(-1, 0xf2);
		*bs << cl->grid.getmap(a) << fx << fy << fz;
		mapid=a;
		pos.x=fx;
		pos.y=fy;
		pos.z=fz;
		state=1;
		state2=0;
		clearsp();

		if(sp!=0)
		{
			*bsx=pos.x;
			*bsy=pos.y;
			*bsz=pos.z;
//			*bsstate=state;
//			*bsstate2=state2;
		}

//		gridcell=0;
//		gridcell=cl->grid.get(mapid, (int)pos.x, (int)pos.z);

		firstspawn();

//		addstat1type=2;
//		for(int b=inventory::eqoff2;b<73;b++)
//			if(inv.getMaskedItem(b)!=0)
//				inv.getMaskedItem(b)->addbonus(this);
//		inv.checkset();
//		addstat1type=0;
		setgp();
		sendbdata(this->bs);
//		else spawnbufficons(this->bs);

//		gridcell->addplayer(this);
//		gridcell->update(gridcell, this, (int)pos.x, (int)pos.z);
		gridcell.add();
		onChangePosition();

		if(party!=0)party->login(this);
		if(guild!=0)
		{
			if(!guild->login(this, guildslot, dbid, id))
			{
				guild=0;
				guildid=0;
			}
		}
		if(guild!=0)guild->getwarstatus(this);

}
void tplayer::changemap(int a, float fx, float fy, float fz, bool forcedchange)
{
	int pi1=-1,pi2=-1;
	if((a==mapid)&&(!forcedchange))
	{
		clearfollowers();
		setfollowed(0, 0);
		buffer bs1;
		stopmove();

		pos.x=fx;
		pos.y=fy;
		pos.z=fz;
		state=1;
		state2=0;
		teleport(&bs1,id,pos.x,pos.y,pos.z);
		if(sp!=0)
		{
			*bsx=pos.x;
			*bsy=pos.y;
			*bsz=pos.z;
//			*bsstate=state;
//			*bsstate2=state2;
		}

//		gridcell->update(gridcell, this, (int)pos.x, (int)pos.z, bs1);
		gridcell.check(bs1);
		onChangePosition();
	}else{
		int b=mapid;
		changemap2(a,fx,fy,fz);
		if(a!=b)
		{
			if(b==202)siegeremover.remove();
			else cl->addtosiegemap(siegeremover, this);
		}
	}
}



int tplayer::pickup(item *targy)
{
	int c=targy->num;
	if(targy->getId() < 16)
	{
		if(party!=0)party->gotmoney(this, c);
		else
		{
			money+=c;
			addstat(bs, id, 79, c);
			bs->cmd(id, 0x95) << 0x0274;
			bs->sndpstr(catclaws(cl->print("%d penya", c)));
			c=0;
		}
	}else{
		c=inv.pickup(targy);
		if(c<targy->num)
		{
			bs->cmd(id, 0x95) << 0x0274;
			bs->sndpstr(targy->GetI()->name);
		}
	}
	return c;
}

void tplayer::jobchange()
{
//	packet size is wrong...

	bs->cmd(id, 0xa7);
	*bs << job;
	sendskills();
	raise();

//	changemap2(mapid, pos.x, pos.y, pos.z);

//	clearsp();
	music(bs, 24);
	HMF();
}

void tplayer::setgp()
{
//	bs->cmd(id, 0xa6);
//    *bs << statpoints << 0;
	bs->cmd(id, 0x6a);
	*bs << str << sta << dex << intl << 0 << statpoints;
}

void tplayer::setlevelexp()
{
	bs->cmd(id, 0x12);
	*bs << exp << pxp << 0 << (short)level << 0 << 0 << (short)65535;
}

void tplayer::sztext(int a)
{
	bs->cmd(id, 0x94);
	*bs << a;
}

void tplayer::reanimate()
{
	if(hp<=0)
	{
		isDead = false;
		tomulticast->cmd(id, 0xa2);
		hp=maxhp/3;
		if(sp!=0)*bshp=hp;
	}
}

void tplayer::incskillexp(int b)
{
    int skill=skills.at(job).at(b);
	if(skill<0)return;
	tskilldata *s=&skilllist.at(skill).at(skilllevels.at(skill)-1);
	if((s->skilltype==13)&&(party==0))return;
	unsigned int pval=s->pxp;

    if((skilllevels.at(skill)<skillmaxlevel.at(job).at(b))&&(pxp>=pval))
	{
		pxp-=pval;
        this->skillexp.at(skill)+=skillExpMultiplier;
        if(this->skillexp[skill]>=skillacc[skilllevels[skill]])
		{
            while((skilllevels[skill]<skillmaxlevel[job][b])&&(skillexp[skill]>=skillacc[skilllevels[skill]]))skilllevels[skill]++;
			bs->cmd(id, 0x6b);
			*bs << skill << skilllevels[skill];
        }
		bs->cmd(id, 0x25) << 1 << b << pxp << 0 << skillexp[skill]-skillacc[skilllevels[skill]-1];
    }
}

bool tplayer::managempfp(int b)	//mp fp and arrows and posters
{
	if((berserk)||(silence))return false;
    int skill=skills.at(job).at(b);
    int level=skilllevels.at(skill);
//#ifdef _DEBUG
	if((level<1)||(level>20))throw error(std::string("Skill level is out of range ")+toString(level), "character_player::managempfp");
//#endif
	if(this->mp<0)this->mp=0;
	if(this->fp<0)this->fp=0;
    int mp=skilllist.at(skill).at(level-1).mpreq*(100-stats[DST_MP_DEC_RATE])/100;//[skill*20+level-1][9];
    int fp=skilllist.at(skill).at(level-1).fpreq*(100-stats[DST_FP_DEC_RATE])/100;//[skill*20+level-1][10];
	if(refresherhold>=cl->ido)mp=0;
	if(vitalx>=cl->ido)fp=0;
    if(this->mp<mp)
	{
		bs->cmd(id, 0x94);
		*bs << 616;
        cancelskill();
		return false;
    }
    if(this->fp<fp)
	{
		bs->cmd(id, 0x94);
		*bs << 615;
        cancelskill();
        return false;
    }
	int sup=skillsupliment.at(skill);
	if(sup!=0)
	{
		if(!inv.decsuppliment(sup))
		{
			cancelskill();
			return false;
		}
	}
    if(mp!=0)
	{
		this->altermp(-mp);
		addstat(bs, id, 39, -mp);
	}
    if(fp!=0)
	{
		this->alterfp(-fp);
		addstat(bs, id, 40, -fp);
	}
    return true;
}

void tplayer::statset(int a, int b, int c, int d)
{
//	greentext("statpoints: %d %d %d %d %d", statpoints, a, b, c, d);
	if(a+b+c+d<=statpoints && a>=0 && b>= 0 && c >= 0 && d >=0)
	{
		str+=a;
		sta+=b;
		dex+=c;
		intl+=d;
		statpoints-=a+b+c+d;
		bs->cmd(id, 0x6a);
		*bs << str << sta << dex << intl << 0 << statpoints;
//		setgp();
		HMF();
		clearsp();
	}
}

void tplayer::sitdown()
{
//	buffer bs;
	int a=4;
	if(sitting)a++;
	sitting=!sitting;
//	bs.cmd(id, 0x98);
//	bs << a;
//	gridcell->multicast2(bs);
	tomulticast->cmd(id, 0x98) << a;
	clearsp2();
}

void tplayer::gotmail()
{
	say("Mail system", name.c_str(), "Mail arrived", -1, dbid);
//  bs->cmd(id, 0xd3);
//	*bs << 0x00028000;
}
/*
void tplayer::restat(int a)
{
	if(a==-1)
	{
		if(job<16)a=level*2-2;
		if(job>16)a=120*2-2-100+(level-61)*3;
	}
//	buffer bs1;
	if(str-15!=0)substat(tomulticast, id, 1, str-15);
	if(sta-15!=0)substat(tomulticast, id, 4, sta-15);
	if(dex-15!=0)substat(tomulticast, id, 2, dex-15);
	if(intl-15!=0)substat(tomulticast, id, 3, intl-15);
	str=15;
	sta=15;
	dex=15;
	intl=15;
	statpoints=a;

	effect(tomulticast, id, 107);
//	gridcell->multicast2(bs1);
	bs->cmd(id, 0x24);
	*bs << (short)0x1a << a;
	HMF();
	clearsp();
}
*/

void tplayer::say(const std::string &nev, const std::string &cimzett, const std::string &message, int kuldoid, int cimzettid)
{
    std::lock_guard<std::mutex> guard(this->asyncbuffermutex);
	asyncbuffer2.push_back(buffer());
	asyncbuffer2.back() << 0x00ff00e0;
	asyncbuffer2.back().sndpstr(nev);  //küldő
	asyncbuffer2.back().sndpstr(cimzett);            //címzett
	asyncbuffer2.back().sndpstr(message);       //üzenet
	asyncbuffer2.back() << kuldoid << cimzettid << 0;
}
void tplayer::whisper(const std::string &nev, const std::string &cimzett, const std::string &message, int kuldoid, int cimzettid)
{
    std::lock_guard<std::mutex> guard(this->asyncbuffermutex);
	asyncbuffer2.push_back(buffer());
	asyncbuffer2.back() << 0x00ff00d4;
	asyncbuffer2.back().sndpstr(nev);  //küldő
	asyncbuffer2.back().sndpstr(cimzett);            //címzett
	asyncbuffer2.back().sndpstr(message);       //üzenet
	asyncbuffer2.back() << kuldoid << cimzettid << 0;
}
void tplayer::saynotconnected()
{
	*ooo << 0x00ff00e0 << 0 << 0 << 0  << 0x00057a36 << dbid << 1;
	raiseooo();
}

void tplayer::onChangePosition()
{
	if(sp!=0)
	{
		*bsx=pos.x;
		*bsy=pos.y;
		*bsz=pos.z;
		*bsstate=state;
		*bsstate2=state2;
	}
//	if(gridcell!=0)gridcell->update(gridcell, this, (int)pos.x, (int)pos.z);
	gridcell.check();
	if(duel!=0)
	{
		vector3d<> pos(this->pos.x, 0, this->pos.z);
		if((pos-arenapos).sqrlength()>arenarange)
		{
			duellose();
			duel->duelwin();
			duel->duel=0;
			duel=0;
		}
	}
}

void tplayer::townblinkns()
{
    int b,c;
    float t1, t;
	if(pk<0)
	{
		mapid=2;
		pos.set(1271,113,1333);
	}else
    switch(mapid)
    {
        case 1: //madrigal
			t=(pos-towns[0]).sqrlength();
            b=0;
            for(c=1;c<3;c++)
			{
				t1=(pos-towns[c]).sqrlength();
                if(t1<t)
				{
                    t=t1;
                    b=c;
                }
            }

//            teleport2(user, towns[b][0],  towns[b][1], towns[b][2]);
			mapid=1;
			pos=towns[b];
        break;
		case 2:	//kebaras
//			mapid=2;
//			pos.set(1271, 113,1333);
			mapid=1;
			pos=towns[0];
		break;
        case 200: //flaris dungeon
			mapid=1;
			pos=towns[0];
        break;
        case 201: //drakon dungeon
			mapid=1;
			pos=towns[2];
        break;
        case 202: //guildwar
			mapid=1;
			pos=towns[0];
        break;
        case 204: //sm dungeon
			mapid=204;
			pos.set(1036.994019f, 5.0f, 539.900146f);
        break;
        default:    //everything else
			mapid=1;
			pos=towns[2];
    }
}

void tplayer::townblink()
{
    int b,c;
    float t1, t;
	if(pk<0)
	{
        changemap(2, 1271, 113,1333, forcedchangeonblink);
	}else
    switch(mapid)
    {
        case 1: //madrigal
			t=(pos-towns[0]).sqrlength();
            b=0;
            for(c=1;c<3;c++)
			{
				t1=(pos-towns[c]).sqrlength();
                if(t1<t)
				{
                    t=t1;
                    b=c;
                }
            }

//            teleport2(user, towns[b][0],  towns[b][1], towns[b][2]);
            changemap(1, towns[b].x, towns[b].y, towns[b].z, forcedchangeonblink);
        break;
		case 2: //kebaras
//            changemap(2, 1271, 113,1333, forcedchangeonblink);
            changemap(1, towns[0].x, towns[0].y, towns[0].z, forcedchangeonblink);

		break;
        case 200: //flaris dungeon
            changemap(1, towns[0].x, towns[0].y, towns[0].z, forcedchangeonblink);
        break;
        case 201: //drakon dungeon
            changemap(1, towns[2].x, towns[2].y, towns[2].z, forcedchangeonblink);
        break;
        case 202: //guildwar
            changemap(1, towns[0].x, towns[0].y, towns[0].z, forcedchangeonblink);
        break;
        case 204: //sm dungeon
            changemap(204, 1036.994019f, 5.0f, 539.900146f, forcedchangeonblink);
        break;
        default:    //everything else
            changemap(1, towns[2].x, towns[2].y, towns[2].z, forcedchangeonblink);
    }
}

void tplayer::addstat1(int stat, int val, bool forcesub)
{
	int val2=-1;
	if(stat!=64)stats.at(stat)+=val;
	else
	{
		if(val>0)stats.at(stat)|=val;
		else stats.at(stat)&=~(-val);
		if(val==1024)val2=0x7fffffff;
	}
	buffer *bs1=tomulticast;
	if(addstat1type==1)bs1=tomulticast2;

	if(addstat1type<=1)
	{
		if((val>=0)&&(!forcesub))addstat(bs1, id, stat, val, val2);
		else substat(bs1, id, stat, -val);
	}

	if(((stat<38)||(stat>40))&&(((stat==11)&&(val!=0))||(stat!=11)))
	{
		std::map<int, bonus>::iterator i=tosendvals.find(stat);
		if(i!=tosendvals.end())
		{
			if(i->second.val+val==0)tosendvals.erase(i);
			else i->second.val+=val;
		}else
		{
			tosendvals.insert(std::pair<int, bonus>(stat, bonus(stat,val)));
		}
	}
}

void tplayer::addstat3(int stat, int val)
{
	if(stat!=64)stats.at(stat)+=val;
	else
	{
		if(val>0)stats.at(stat)|=val;
		else stats.at(stat)&=~(-val);
	}

	std::map<int, bonus>::iterator i=tosendvals.find(stat);
	if(i!=tosendvals.end())
	{
		if(i->second.val+val==0)tosendvals.erase(i);
		else i->second.val+=val;
	}else
		tosendvals.insert(std::pair<int, bonus>(stat, bonus(stat,val)));

}

void tplayer::addstat2(int stat, int val)
{
	if(stat!=64)stats.at(stat)+=val;
	else
	{
		if(val>=0)stats.at(stat)|=val;
		else stats.at(stat)&=~(-val);
	}
	if(addstat1type==0)
	{
		if(val>=0)addstat(tomulticast, id, stat, val);
		else substat(tomulticast, id, stat, -val);
	}
//	if(silent)bs1.forbidden=this;
//	gridcell->multicast2(bs1);
}

void tplayer::restat()
{
	if(str-15!=0)substat(tomulticast2, id, 1, str-15);
	if(sta-15!=0)substat(tomulticast2, id, 4, sta-15);
	if(dex-15!=0)substat(tomulticast2, id, 2, dex-15);
	if(intl-15!=0)substat(tomulticast2, id, 3, intl-15);
	statpoints=level*2-2;
	str=15;
	sta=15;
	dex=15;
	intl=15;
	bs->cmd(id, 0x6a);
	*bs << str << sta << dex << intl << 0 << statpoints;
	HMF();
	clearsp();
	effect(tomulticast, id, 107);
}

void tplayer::usereskill()
{
	int a,skill,val=0;
	for(a=0;a<43;a++)
	{
		skill=skills[job][a];
		if(skill>=0)
		{
			val+=skillexp[skill];
		}
	}
	bs->cmd(id, 0x7c);
	*bs << val;
//	buffer bs1;
	effect(tomulticast, id, 107);
//	gridcell->multicast2(bs1);

}

void tplayer::reskill()
{
	int a;
	int b,c;
	for(a=0;a<42;a++)
	{
		sr->get(b);
		sr->get(c);
		if(b>-1)
		{
			skilllevels[b]=c;
			skillexp[b]=skillacc[c];
		}
	}
	bs->cmd(id, 0x7d);
	sendskills();
	*bs << 0;
}


void tplayer::onKo(character_buffable *p)
{
	tplayer *d=0;
	if(p->type==ttplayer)d=(tplayer*)p;
	if(shop.getopened())shop.close(this);
	if(trade!=0)trade->del(this);
	trade=0;
	bool explose=(level>=15)&&(job>0);
	vector3d<> pos(this->pos.x, 0, this->pos.z);
	explose&=((pos-arenapos).sqrlength()>arenarange);
	if(d!=0)
	{
		if(guild!=0)
		{
			if((d->guild==guild->getwaring())&&(d->guild!=0))explose=false;
			if((guild->getowner()==dbid)&&(guild->getwaring()!=0))
			{
				guild->leaderkilled();
			}
		}
	}
	if(explose)
	{
//		if((duel==0)||((duel!=0)&&(duel!=d)))
		{
			pxp=0;
			expbeforeko=exp;
			unsigned long long b=explist[level-1];
			b*=explose;
			b/=100;

			if(b<=exp)exp-=b;
			else exp=0;

			setlevelexp();
			if(pk<=-2000)drop();
		}
	}
	if((duel==d)&&(duel!=0))
	{
		duel->duelwin();
		duellose();
		duel->duel=0;
		duel=0;
	}
	music(bs, 22);
	clearbuffs();
}

void tplayer::manageexp(long long e1, int l)
{
	if(party!=0)party->managePexp(this, e1, l);
	else addexp(e1, l);
}

void tplayer::addexp(long long e1, int l)
{
	if((bs==0)||(hp<=0))return;
	if((e1==0)||((job==0)&&(level>=15))||((job<6)&&(level>=60))||(level>=explist.size()))return;
	bool levelled=false;
	long double p,m=expMultiplier;
	unsigned int p1;
	if(exp<maxexp)m*=1.5;
	if(cheered>cl->ido)m*=1.05;
	l=iabs(level-l);
	if(l>5)
	{
		l=100-l*4;
		if(l<0)l=0;
		p=l;
		p/=100.0;
		m*=p;
	}


	e1*=m;

	if(pxp<pxplist.at(level-1))
	{
		p1=pxplist[level-1]-pxp;
		if(p1>e1)p1=e1;
		e1-=p1;
		pxp+=p1;
	}
//	a+=e;
	exp=exp+e1;

	while((level-1<explist.size()-1)&&(exp>=explist.at(level-1)))
	{
		exp-=explist[level-1];
		level++;
		statpoints+=2;
		levelled=true;
	}

	if(exp>explist.at(level-1)-1)exp=explist.at(level-1)-1;

	if(exp>maxexp)maxexp=exp;

	setlevelexp();
	if(levelled)
	{
		effect(tomulticast, id, 42);
		tomulticast2->cmd(id, 0x11) << (short)level;
		setgp();
		HMF();
		hp=maxhp;
		mp=maxmp;
		fp=maxfp;
		pxp=pxplist[level-1];
		if((level==15)||(level==60))music(bs, 26);
		else music(bs, 23);
		setstat(bs, id, 80, pxp);
	}

}

void tplayer::removesetbonuses()
{
	while(!psetbonuses.empty())
	{
		addstat1(psetbonuses.front().stat, -psetbonuses.front().val);
//		logger.log("Removed bonus %d %d\n", psetbonuses.front().stat, psetbonuses.front().val);
		psetbonuses.pop_front();
	}
}

void tplayer::addsetbonus(const bonus& b)
{
	addstat1(b.stat, b.val);
	psetbonuses.push_front(b);
//	logger.log("Added bonus %d %d\n", b.stat, b.val);
}

void tplayer::docheer()
{
	int a;
	sr->get(a);

	tplayer *p=cl->getplayer(a);
	if((p!=(tplayer*)getfocus())||(p==0))return;
	if(p->type!=ttplayer)return;
	ncheers--;
	bs->cmd(id, 0xb4) << ncheers << 0;
	p->cheered=cl->ido+600000;
	effect(tomulticast, id, 0x06b5);
	effect(tomulticast, p->getId(), 0x06b6);
	tomulticast->cmd(id, 0xcb) << pos.x << pos.y << pos.z << 0 << 0 << 0 << 0x4303fbf8 << 1 << 33 << 145 << 145 << 1 << 2 << 0xd693cbdc << 0x01c9d464;
	tomulticast->cmd(p->getId(), 0x4c) << p->getId() << (short)0 << (short)0x28cd << 0 << 600000;

	/*TODO*/
}

asstruct::asstruct(int a1, int c1, tplayer *p, character_buffable *f, long long td1, int s0, int s1, int s2, int s3, int s4, int pj)
:a(a1),c(c1),pos(1),player(p),initialFocus(f),td(td1),endit(false),pjob(pj)
{
	askills[0]=s0;
	askills[1]=s1;
	askills[2]=s2;
	askills[3]=s3;
	askills[4]=s4;
	nexttime=td+p->cl->ido;//player->cl->ido;
	p->cl->addas(this, remover);
}

asstruct::~asstruct()
{
//	logger.log("asstruct ended\n");
	if(player!=0)
	{
        std::lock_guard<std::mutex> guard(player->playermutex);
		player->actionslotp=0;
		player->cancelskill();
	}
}

int asspos[]={-10, 0, 10, 25, 50, 100};
//int asspow[]={5,5,8,12,25}
//float assplus[]={2.0f, 2.0f, 1.875f, 2.083333f, 2.0f};

void tplayer::updateas()
{
	long long t=cl->ido-asstime;
	asstime=cl->ido;
	t/=1500;
/*
	t/=3000;
	int a,b;
	for(b=1;(b<6)&&(t>0);b++)
	{
		if(asspower<asspow[b])
		{
			a=asspow[b]-asspower;
			if(t<a)a=t;
			t-=a;
			asspower+=a*2;
		}
	}
*/
	asspower+=t;
	if(asspower>100)asspower=100;

}


bool asstruct::process()
{
	if(player==0)return true;
	if(!player->validnl())return true;
	if(player!=0)
	{
		if(player->cl->ido<nexttime)
		{
//			if(endit)player->cancelskill();
			return endit;
		}
	}
	else
	{
//		player->cancelskill();
		return true;
	}

    std::lock_guard<std::mutex> guard(player->playermutex);

	if(player->activation<player->cl->ido)
	{
		player->updateas();
		int asspower=player->asspower;
		if(asspower<asspos[pos])
		{
//			player->cancelskill();
			return true;
		}
		asspower-=10;
		if(asspower<-10)asspower=-10;
		if(player->bs!=0)player->bs->cmd(player->getId(), 0xc5) << (int)asspower;
		player->asspower=asspower;
	}

	nexttime+=td;
	if(initialFocus!=(character_buffable*)player->getfocus())pos=5;
	else if(askills[pos]==0)pos=5;
	else if(player->managempfp(askills[pos]))
	{
		int skill=skills.at(pjob).at(askills[pos]);
		int level=player->skilllevels.at(skill);
		if(!player->doskill(skill, level, c, pos, true, this->initialFocus))pos=5;
		else player->incskillexp(askills[pos]);
	} else pos=5;
	pos++;
	endit|=(pos>=5);
	return endit;
}

void tplayer::domulticast()
{
    std::lock_guard<std::mutex> guard(this->playermutex);
	if(!tbs.empty())
	{
		while(tbs.top().t<=cl->ido)
		{
			tomulticast->add(*tbs.top().bs);
			delete tbs.top().bs;
			tbs.pop();
			if(tbs.empty())break;
		}
	}

	if(gridcell.valid())
	{
		if(tomulticast->getcommandnumber()>0)
		{
			gridcell.multicast2(*tomulticast);
			delete tomulticast;
			tomulticast=new buffer;
		}
		if(tomulticast2->getcommandnumber()>0)
		{
			gridcell.multicast2(*tomulticast2);
			delete tomulticast2;
			tomulticast2=new buffer;
			tomulticast2->forbidden=this;
		}
	}
}

void tplayer::getasyncbuffer()
{
	{
        std::lock_guard<std::mutex> playerguard(this->playermutex);
		handleacmd();
        std::lock_guard<std::mutex> asyncbufferguard(this->asyncbuffermutex);
		if(asyncbuffer!=0)
		{
			this->add(*this->asyncbuffer);
			delete asyncbuffer;
			asyncbuffer=0;
		}
		while(!asyncbuffer2.empty())
		{
			ooo->add(asyncbuffer2.front());
			raiseooo();
			asyncbuffer2.pop_front();
		}
	}
}

bool tplayer::testLock(character_mob *p)
{
	if(p->getmodelid()==164)return true;
	if((p->playerLock==this)||(p->playerLock==0))
	{
		p->playerLock=this;
		p->partyLock=party;
	}
	return (p->playerLock==this)||(p->partyLock==party);
}

void tplayer::summonpet(int id)
{
	if(pet!=0)
	{
		if(id==pet->getmodelid())
		{
			unsummonpet();
			return;
		}
	}
	unsummonpet();
	if(!flying)pet=new character_npc(cl, id, pos.x, pos.y, pos.z, mapid, dir, this);
}

void tplayer::unsummonpet()
{
	if(pet==0)return;
	delete pet;
	pet=0;
}

void tplayer::stopmove()
{
//	if(pet!=0)pet->stopmove();
	character_base::stopmove();
}

int tplayer::pickup(character_item *targyh)
{
	int a=targyh->getitem().num;
	tplayer *pl;
	if(targyh->ableToPick(this)||targyh->ableToPick(party))
	{
		if(party==0)pl=this;
		else pl=party->getseq(this);
		item tgtItem = targyh->getitem();
		a=pl->pickup(&tgtItem);
		if((a==0)||(targyh->getitem().getId()<19))/*1*/
		{
			targyh->pickedup();
			delete targyh;
		}else
		{
			targyh->setnum(a);
		}
	}
	return a;
}

void tplayer::setdelayed(tdelayedaction da1, long long d)
{
	if(da!=da_none)
	{
		tomulticast->cmd(id, 0xdf) << 0 << (char)1;
	}
//	if(!isMoveing())
	{
		da=da1;
	//	delaystart=cl->ido;
		delay=cl->ido+d;
		//add here;
		if(da==da_fly)
		{
			tomulticast->cmd(id, 0xdf) << 4 << (char)0 << 0x13f0;
		}else if(da==da_ttnt)
		{
			tomulticast->cmd(id, 0xdf) << 4 << (char)0 << 0x28c3;
		}else	//blinkwings
		{
			tomulticast->cmd(id, 0xdf) << 4 << (char)0 << 0x12c6;
		}
	}
}

void tplayer::canceldelayed()
{
	if(da!=da_none)
	{
		da=da_none;
		delay=0;
		tomulticast->cmd(id, 0xdf) << 0 << (char)1;
	}
}

int tt=690;

void tplayer::timer()
{
	int a;
	item *t;
	if((cl->ido>duelcountdowntime)&&(duelcountdowntime!=0))
	{
		if(duel==0)
		{
			duelcountdowntime=0;
		}else
		{
			bs->cmd(id, 0x67) << duelcountdown;
			duel->bs->cmd(duel->getId(), 0x67) << duelcountdown;
			duelcountdown--;
			if(duelcountdown<0)
			{
				duelcountdowntime=0;
				duelbegan=true;
				duel->duelbegan=true;
				bs->cmd(id, 0x31) << duel->dbid << 0;
				bs->cmd(id, 0x31) << duel->dbid << 1;
				duel->bs->cmd(duel->getId(), 0x31) << dbid << 0;
				duel->bs->cmd(duel->getId(), 0x31) << dbid << 1;
			}else
			{
				duelcountdowntime=cl->ido+1000;
			}
		}


	}
	if(cl->ido>nextcheer)
	{
		nextcheer+=60*60*1000;
		ncheers++;
		bs->cmd(id, 0xb4) << ncheers << 0;
	}
	if((pk>-2000)&&(pk<0))
	{
		if(cl->ido>=nextkarmarecover)
		{
			pk++;
			tomulticast->cmd(id, 0x28) << pk << pvp;
			bs->cmd(id, 0x95) << 666;
			bs->sndpstr("1");
			if(pk<-5)nextkarmarecover=cl->ido+15000;
			else nextkarmarecover=cl->ido+15000*3;
		}
	}
	if((da!=da_none)&&(delay<=cl->ido))
	{
		tdelayedaction da1=da;
		canceldelayed();
		switch(da1)
		{
		case da_townblinkwing:
			if(shop.getopened())break;
			if(trade!=0)trade->del(this);
			trade=0;
			inv.removeitemid(4805, 1);
			unsummonpet();
			townblink();
			break;
		case da_ttnt:
			if(shop.getopened())break;
			if(trade!=0)trade->del(this);
			trade=0;
			unsummonpet();
			townblink();
			break;
		case da_flarisblink:
			if(shop.getopened())break;
			if(trade!=0)trade->del(this);
			trade=0;
			unsummonpet();
			inv.removeitemid(4803, 1);
			changemap(1, 7161, 100, 3264, forcedchangeonblink);
		break;
		case da_smblink:
			if(shop.getopened())break;
			if(trade!=0)trade->del(this);
			trade=0;
			unsummonpet();
			inv.removeitemid(4804, 1);
			changemap(1, 8321, 100, 3720, forcedchangeonblink);
			break;
		case da_drakonblink:
			if(shop.getopened())break;
			if(trade!=0)trade->del(this);
			trade=0;
			unsummonpet();
			inv.removeitemid(4806, 1);
			changemap(1, 3896, 59, 4378, forcedchangeonblink);
			break;
		case da_fly:
			t=inv.getItem(daflyitem);
			if(t!=0)
				if(t->GetI()!=0)
					if(t->GetI()->eqslot==13)
					{
						if(shop.getopened())break;
						if(trade!=0)trade->del(this);
						trade=0;
						unsummonpet();
						inv.equip(daflyitem);
						fuel=0;
					}
			break;
		}
	}
	if((hp>0)&&(nexthprec<=cl->ido))
	{
		nexthprec=cl->ido+5000;
		a=60;
		if(sitting)a/=2;
		if(party!=0)if(party->gets()>cl->ido)a/=2;
		if(hp<maxhp)
		{
			alterhp(maxhp/a);
//			hp+=maxhp/a;
//			if(hp>maxhp)hp=maxhp;
			setstat(tomulticast, id, 38, hp);
			/*TODO bshp*/
		}
		if(mp<maxmp)
		{
			mp+=maxmp/a;
			if(mp>maxmp)mp=maxmp;
			setstat(tomulticast, id, 39, mp);
		}
		if(fp<maxfp)
		{
			fp+=maxfp/a;
			if(fp>maxfp)fp=maxfp;
			setstat(tomulticast, id, 40, fp);
		}
	}
	if(lastsave+300000<cl->ido)
	{
		lastsave=cl->ido;
		save();
	}
	if((fuel>0)&&(flying)&&(isMoveing())&&(speed>0))
	{
		fuel-=1000;
		if(fuel<0)fuel=0;
		setfuel(bs, id, fuel);
	}
/*
	bs->cmd(id, 0xa7);
	*bs << job;
//	sendskills2(b);
	for(a=0;a<tt;a++)*bs << (char)0;
	greentext(cl->print("jobteszt %d", tt));
	tt++;
*/
}

void tplayer::removepsywalls()
{
	while(!psywalls.empty())
	{
		psywalls.front()->clear();
	}
}

void tplayer::OnShieldUnequip()
{
	for(std::set<int>::iterator i=shieldskills.begin();i!=shieldskills.end();++i)
	{
		removebuff(*i);
	}
	shieldskills.clear();
}

void tplayer::OnWeaponUnequip()
{
	for(std::set<int>::iterator i=weaponskills.begin();i!=weaponskills.end();++i)
	{
		removebuff(*i);
	}
	weaponskills.clear();
}
void tplayer::AddWeaponSkill(int skill)
{
	std::set<int>::iterator i=weaponskills.find(skill);
	if(i==weaponskills.end())weaponskills.insert(skill);
}

void tplayer::AddShieldSkill(int skill)
{
	std::set<int>::iterator i=shieldskills.find(skill);
	if(i==shieldskills.end())shieldskills.insert(skill);
}

void tplayer::handleacmd()
{
    std::lock_guard<std::mutex> guard(this->asyncbuffermutex);
	while(!acmd.empty())
	{
		switch(acmd.front())
		{
		case ac_quit_party:
			party=0;
			break;
		case ac_quit_guild:
			tomulticast2->cmd(id, 0x9b) << 0;
			bs->cmd(id, 0x9b) << 0;
			bs->cmd(id, 0x9e);
			for(int a=0;a<24;a++)*bs << 0;
			*bs << (short)0;
			guild=0;
			guildid=0;
			guild=0;
			break;
		case ac_sitdown:
			sitting=true;
			break;
		};
		acmd.pop();
	}
}
bool tplayer::ispkable(character_buffable *p)
{
	if(p->type!=ttplayer)return true; //(cl->isPK())&&(job>0)&&(level>=15)&&(mapid!=2);
	else
	{
		tplayer *q=(tplayer*)p;
		return ((cl->isPK()||(iswaring(q)))&&(q!=this)&&(job>0)&&(level>=15)&&(mapid!=2)&&((q->party!=party)||(party==0))&&((q->guild!=guild)||(guild==0))||(siege_attackable&&q->siege_attackable));
	}
}

bool tplayer::iswaring(tplayer *p)
{
	bool r=(duel==p->duel)&&(p!=this)&&(duel!=0);
	if((guild!=0)&&(p->guild!=0)&&(p!=this))
	{
		r|=((guild->getwaring()==p->guild)&&(p->guild->getwaring()==guild)&&(guild->getwaring()!=0));
	}
	return r;
}

void tplayer::managekarma(character_buffable *q)
{
	vector3d<> pos(this->pos.x, 0, this->pos.z);
	if((cl->isPK())&&(q->type==ttplayer)&&(q!=this)&&((pos-arenapos).sqrlength()>arenarange))
	{
		tplayer *p=(tplayer*)q;
		if((guild!=0)&&(p->guild!=0))
		{
			if(guild->getwaring()==p->guild)return;
		}
		if(p->gethp()>0)
		{
			if(p->pk>=0)
			{
				if(pk>-50)setkarma(-50);
			}
		}else
		{
			if(p->pk>=0)
			{
				if(pk>-2000)setkarma(-2000);
				else setkarma(pk-10);
			}
		}
	}
}

void tplayer::drop()
{
	if(accesslevel>0x46)return;
	if(rnd(100)<50)
	{
		int a=rnd(inventory::eqoff2);
		item *t=inv.getMaskedItem(a);
		if(t!=0)
		{
			if(t->getId()>0)
			{
				new character_item(*t, cl, mapid, pos.x,pos.y,pos.z);
				inv.removeitem(inv.getMask(a), t->num);
			}
		}
	}
}

void tplayer::duelwin()
{
	duelbegan=false;
	if(duel==0)return;
	int a=duel->level-level+20;
	if(a<0)a=0;
	if(a>40)a=40;
	pvp+=a;
	messagebox(cl->print("You have won the duel against %s", duel->name.c_str()));
	bs->cmd(id, 0x31) << duel->dbid << 2;
	tomulticast->cmd(id, 0x40) << pvp;
}

void tplayer::duellose(bool tosend)
{
	duelbegan=false;
	if(duel==0)return;
	int a=duel->level-level+20;
	if(a<0)a=0;
	if(a>40)a=40;
	pvp-=a;
	if(pvp<0)pvp=0;
	if(tosend)
	{
		messagebox(cl->print("You have lost the duel against %s", duel->name.c_str()));
		bs->cmd(id, 0x31) << duel->dbid << 2;
		tomulticast->cmd(id, 0x40) << pvp;
	}
}

void tplayer::maxskillexp(){

	int skill;
	for(int a=0;a<43;a++)
	{
		skill=skills.at(job).at(a);
		if(skill<0)continue;
		tskilldata *s=&skilllist.at(skill).at(skilllevels.at(skill)-1);
		//if((s->skilltype==13)&&(party==0))continue;

		if((skilllevels.at(skill)<skillmaxlevel.at(job).at(a)))
		{
			this->skillexp.at(skill)+=skillExpMultiplier;
			if(this->skillexp[skill]>=skillacc[skilllevels[skill]])
			{
				while((skilllevels[skill]<skillmaxlevel[job][a]))skilllevels[skill]++;
				bs->cmd(id, 0x6b);
				*bs << skill << skilllevels[skill];
			}
			bs->cmd(id, 0x25) << 1 << a << pxp << 0 << skillexp[skill]-skillacc[skilllevels[skill]-1];
		}
	}
}

void tplayer::addmoney(int a)
{
	if(a>0)
	{
		int m=money;
		money+=a;
		if(money<0)money=m;
		else
		{
			bs->cmd(id, 0x95) << 0x0274;
			bs->sndpstr(catclaws(cl->print("%d penya", a)));
		}
		setstat(bs, id, 79, money);
	}
}

void tplayer::submoney(int a)
{
	if(a>0)
	{
		int m=money;
		money-=a;
		if((money<0)||(money>m))money=0;
	}
}

void tplayer::gwprepare()
{
	if(cl->getsiegetimertype()!=0)
	{
		int a=cl->getsieget()-cl->ido;
		if(a>1)
		{
            std::lock_guard<std::mutex> guard(this->asyncbuffermutex);
			if(this->asyncbuffer==0)asyncbuffer=new buffer;
			asyncbuffer->cmd(id, 0xb8) << (char)49 << a << cl->getsiegetimertype();
		}
	}
}

void tplayer::siege_teleportdown()
{
	if(!requested_teleportdown)
	{
//		gwspawn=-1;
		requested_teleportdown=true;
		bs->cmd(id, 0xb8) << (char)6 << 69 << 10;

	}
}
