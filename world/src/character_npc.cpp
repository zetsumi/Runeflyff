#include "stdafx.h"
#include "character_npc.h"

#include "stringf.h"
#include "cluster.h"
#include "mysql.h"
#include "adat.h"
#include "logger.h"
#include "stringf.h"
#include "quest.h"

using namespace std;

void character_npc::destroy()
{
	int a;
	if(chattext!="")cl->removenpcchat(npcchatremover);
	for(a=0;a<4;a++)delete[] shops[a];
	if(pet)cl->removepet(petremover);
	cl->releasenpcid(id);
//	gridcell->removenpc(this);
	gridcell.remove();
}

bool character_npc::petpick()
{
	character_item *t=0;
	float tav, tav2=10*10;
	if(getfocus()==0)
	{
		std::list<character_item*>::iterator i;
		if(gridcell.valid())for(i=gridcell->items.begin();i!=gridcell->items.end();++i)
		{
			if((*i)->ableToPick(owner))
			{
				tav=distanceny(*(*i));
				if(tav<tav2)
				{
					t=*i;
					tav2=tav;
				}
			}
		}
		if(t!=0)
		{
			setfocus(t);
			vector3d<> v=t->getpos();
			this->moveto(v.x, v.y, v.z);
            std::lock_guard<std::mutex> guard(owner->playermutex);
			gotoid(owner->tomulticast, id, t->getId(), 0);
		}
	}else
	{
		if(!isMoveing())
		{
			character_base *f=getfocus();
			int n=1;
			if(f->type==ttitem)
			{
                std::lock_guard<std::mutex> guard(owner->playermutex);
				n=owner->pickup((character_item*)f);
			}
			setfocus(0);
			if(n==0)petpick();
			if(getfocus()==0)
			{
				if(getfollowed()==0)setfollowed(owner, md);
				if(movefollow())
				{
					buffer bs;
					gotoid(&bs, id, owner->getId(), 0);
					multicast(bs);
				}
			}
		}
	}
	return false;
}

character_npc::character_npc(cluster *c, int npctype, float x1, float y1, float z1, int m, int d, tplayer *o)
:owner(o),pet(true)
{
	type=ttnpc;
	cl=c;
	modeltype=5;
	modelid=npctype;
	name="";
	pos.x=x1;
	pos.y=y1;
	pos.z=z1;
	lastpos=pos;
	mapid=m;
	size=100;
	dir=d;
	id=cl->getnpcid(this);
	this->createremovepacket();

	sp=new buffer();
	sp->cmd(id, 0xf0);
	*sp << (char)modeltype << modelid << (char)modeltype << (short)modelid << (short)size;
	bsx=sp->insert(pos.x);
	bsy=sp->insert(pos.y);
	bsz=sp->insert(pos.z);
	*sp << (short)dir << id;
	*sp << (short)0 << (char)0 << 1 << 1 << 0 << 1 << 0 << 0 << (short)0 << (char)0;

	sp->sndpstr(name);
//	*sp << (short)0;
	*sp << 0;
//	*sp << (short)0;

//	*sp << (short)0;
//	*sp << (short)0;
//	*sp << (short)0;
//	*sp << (short)0;
/*
#ifdef _DEBUG
	sp->cmd(-1, 0xd0);
	*sp << id;
	sp->sndpstr("NPC");
	sp->sndpstr(toString(id).c_str());
#endif
*/
//	gridcell=0;
//	gridcell=cl->grid.get(mapid, (int)pos.x, (int)pos.z);
//	gridcell->addnpc(this);
	gridcell.add();

	speed=10.0f;

	int a;
	for(a=0;a<4;a++)
	{
		nshops[a]=0;
		shops[a]=0;
	}

	cl->addpet(this, petremover);
//	petpick();
	setfollowed(owner, md);
}

character_npc::character_npc(cluster *c, int npctype, string n, float x1, float y1, float z1, int m, int d, char *shop1, char *shop2, char *shop3, char *shop4, int l1, int l2, int l3, int l4, int dbid1, const std::string &ct, const std::string &gt)
:chattext(ct),pet(false),greetingtext(gt)
{
	cl=c;
	dbid=dbid1;
	type=ttnpc;
	int *p, a;
	modeltype=5;
	modelid=npctype;
	name=n;
	pos.x=x1;
	pos.y=y1;
	pos.z=z1;
	lastpos=pos;
	mapid=m;
//	size=100;
	size=monsterlist[modelid].size;
	if(size<1)size=100;
	dir=d;
//	hp=-1;
	id=cl->getnpcid(this);
	this->createremovepacket();

//	logger.log("%d %d %s %d %08x %d\n", modeltype, modelid, name.c_str(), size, id, dbid);

	bsx=0;
	bsy=0;
	bsz=0;
	sp=new buffer();
	sp->cmd(id, 0xf0);
	*sp << (char)modeltype << modelid << (char)modeltype << (short)modelid << (short)size;
//	*sp << pos.x << pos.y << pos.z;
	bsx=sp->insert(pos.x);
	bsy=sp->insert(pos.y);
	bsz=sp->insert(pos.z);
	*sp << (short)dir << id;
//	*sp << (short)0 << (char)0 << 1 << 1 << 0 << 1 << 0 << 0 << (short)0 << (char)0;
	*sp << (short)0 << (char)0 << -1 << 0 << 0 << 1 << 0 << 0 << (short)0 << (char)0;

	sp->sndpstr(name);
	*sp << 0;

//	gridcell=0;
//	gridcell=cl->grid.get(mapid, (int)pos.x, (int)pos.z);
//	gridcell->addnpc(this);
	gridcell.add();

	
	nshops[0]=l1/4;
	nshops[1]=l2/4;
	nshops[2]=l3/4;
	nshops[3]=l4/4;
	for(a=0;a<4;a++)
	{
		shops[a]=0;
		if(nshops[a]>100)nshops[a]=100;
	}
	if((shop1!=0)&&(nshops[0]>0))
	{
		shops[0]=new int[100];
		p=(int*)shop1;
		for(a=0;a<nshops[0];a++)shops[0][a]=p[a];
	}
	if((shop2!=0)&&(nshops[1]>0))
	{
		shops[1]=new int[100];
		p=(int*)shop2;
		for(a=0;a<nshops[1];a++)shops[1][a]=p[a];
	}
	if((shop3!=0)&&(nshops[2]>0))
	{
		shops[2]=new int[100];
		p=(int*)shop3;
		for(a=0;a<nshops[2];a++)shops[2][a]=p[a];
	}
	if((shop4!=0)&&(nshops[3]>0))
	{
		shops[3]=new int[100];
		p=(int*)shop4;
		for(a=0;a<nshops[3];a++)shops[3][a]=p[a];
	}



	{
		ul m=tquest::questsmutex.lock();
		int a=0,b=0;
		std::map<int, tquest*>::iterator i;
		for(i=tquest::quests.begin();i!=tquest::quests.end();++i)
		{
//		ul mm=i->second->questmutex.lock();
			if(i->second->startcharacter==name){qbegin.push_back(i->second);a++;}
			if(i->second->endcharacter==name){qend.push_back(i->second);b++;}
		}
	}
	if(chattext!="")cl->addnpcchat(this, npcchatremover);
	sqlquery s1(cl->connections[0], "npcdialogs");
	s1.selectw("npcid="+toString(dbid));
	tnpcdialog nd;
	while(s1.next())
	{
		nd.link=s1["linktext"];
		nd.dialogs[0]=s1["dt1"];
		nd.dialogs[1]=s1["dt2"];
		nd.dialogs[2]=s1["dt3"];
		nd.dialogs[3]=s1["dt4"];
		nd.dialogs[4]=s1["dt5"];
		npcdialogs.push_back(nd);
	}
	s1.freeup();
}

void character_npc::sndshop(buffer *bs)
{
	item t;
	int b,c;
	bs->cmd(id, 0x14);
	for(b=0;b<4;b++)
	{
		for(c=0;c<100;c++)*bs << c;
		*bs << (char)nshops[b];
//		logger.log("nshops%d = %d\n", b, nshops[b]);
		for(c=0;c<nshops[b];c++)
		{
			t.setId(shops[b][c]);
			t.num=9999;
			*bs << (char)c << c;
			t.snddata(bs);
		}
		for(c=0;c<100;c++)*bs << c;
	}
}

void character_npc::clearshop(int slot)
{
	if((slot>=0)&&(slot<4))
	{
		nshops[slot]=0;
		this->save();
	}
}

bool character_npc::addtoshop(int itemid, int slot)
{
	if((slot<0)||(slot>3))return false;
	if(this->nshops[slot]>99)return false;
	if((nshops[slot]==0)&&(this->shops[slot]==0))shops[slot]=new int[100];
	shops[slot][nshops[slot]]=itemid;
	logger.log("added %d to slot %d\n", itemid, slot);
	nshops[slot]++;
	this->save();
	return true;
}

bool character_npc::delfromshop(int itemid, int slot)
{
	if((slot<0)||(slot>3))return false;
	bool retval;
	int a,b=0;
	int *nshop=new int[100];

	for(a=0;a<nshops[slot];a++)
	if(shops[slot][a]!=itemid)
	{
		nshop[b]=shops[slot][a];
		b++;
	}
	retval=(b!=nshops[slot]);
	nshops[slot]=b;
	delete shops[slot];
	shops[slot]=nshop;
	this->save();
	return retval;
}

void character_npc::save()
{
	sqlquery s4(cl->connections[0], "npcdata");
	s4.addupdate("type", toString(modelid));
	s4.addupdate("name", name);
	s4.addupdate("x", toString(pos.x));
	s4.addupdate("y", toString(pos.y));
	s4.addupdate("z", toString(pos.z));
	s4.addupdate("mapid", toString(mapid));
	s4.addupdate("direction", toString(dir));
	s4.addupdate("funkc", "0");
	s4.addupdate("messages", "");
	s4.addupdate("buttons", "");
	s4.addupdate("enabled", "1");
	s4.addupdate("size", toString(size));
	s4.addeupdate("shop1", (char*)&shops[0][0], nshops[0]*4);
	s4.addeupdate("shop2", (char*)&shops[1][0], nshops[1]*4);
	s4.addeupdate("shop3", (char*)&shops[2][0], nshops[2]*4);
	s4.addeupdate("shop4", (char*)&shops[3][0], nshops[3]*4);
	s4.update(string("id=")+toString(dbid));

}

bool character_npc::beginthequest(tquest *q, tplayer *p)
{
	std::map<int, pquestdata>::iterator j=p->quests.find(q->questid);
	if(j!=p->quests.end())if(j->second.state==0)return false;
	std::set<int>::iterator j2=p->finishedquests.find(q->questid);
	if(j2!=p->finishedquests.end())return false;

	if(q->beginaddgold!=0)
	{
		p->money+=q->beginaddgold;
		setstat(p->bs, p->getId(), 79, p->money);
	}
	//item numbers must be <= than their stacking values
	int a,b;
	std::vector2<tquest::qitem>::iterator i;
	item targy;

	a=q->beginadditems.size();
	for(b=0;(b<inventory::eqoff)&&(a>0);b++)
	{
		if(p->inv.getMaskedItem(b)!=0)
			if(p->inv.getMaskedItem(b)->getId()==-1)
				a--;
	}
	if(a>0)
	{
		p->messagebox("You need more free spots in your inventory");
		return false;
	}

	for(i=q->beginadditems.begin();i!=q->beginadditems.end();++i)
	{
		targy.setId(i->id);
		targy.num=i->num;
		p->inv.pickup(&targy);
	}



	return true;
}

void character_npc::missingstuff(tquest *q, tplayer *p)
{
//	buffer &bs=*p->bs;
	int pid=p->getId();
	std::string str2;
	bool retval=true;
	int a,b;
	item *t;
	item targy;
	std::vector2<tquest::qitem>::iterator i;
	std::map<int, pquestdata>::iterator j=p->quests.find(q->questid);
	if(j==p->quests.end())return;
//	retval&=(p->pk>=q->karmamin)&&(p->pk<=q->karmamax);
//	if(p->pk<q->karmamin)str2+="You don't have enough karma\n";
//	if(p->pk>q->karmamax)str2+="You have too much karma\n";

	if(q->beginflags!=0)
	{
		if((q->beginflags&QBF_GUILDLESS)!=0)str2+="You must be guildless\n";
	}

/*TODO*/
//if(q->endstate==0)	//!
{
//	if(j->second.state!=q->beginstate)str2+="Quest state error\n";

	if((!q->enditems.empty()))
	{
		for(i=q->enditems.begin();(i!=q->enditems.end())&&(retval);++i)
		{
			b=i->num;
			for(a=0;a<inventory::eqoff;a++)
			{
				t=p->inv.getMaskedItem(a);
				if((t!=0)&&(t->getId()==i->id))
				{
					b-=t->num;
					if(b<=0)break;
				}
			}
			if(a>=inventory::eqoff)str2+="You don't have enough " + itemlist[i->id].name + "\n";
		}
	}
}
/*
	if((!q->toremoveitems.empty()))
	{
		for(i=q->toremoveitems.begin();(i!=q->toremoveitems.end())&&(retval);++i)
		{
			b=i->num;
			for(a=0;a<inventory::eqoff;a++)
			{
				t=p->inv.getMaskedItem(a);
				if((t!=0)&&(t->getId()==i->id))
				{
					b-=t->num;
					if(b<=0)break;
				}
			}
			if(a>=inventory::eqoff)str2+="You don't have enough " + itemlist[i->id].name + "\n";
		}
	}
*/
//	if(retval)
	{
		a=q->rewarditems.size();
		for(b=0;(b<inventory::eqoff)&&(a>0);b++)
		{
			if(p->inv.getMaskedItem(b)!=0)
				if(p->inv.getMaskedItem(b)->getId()==-1)
					a--;
		}
		if(a>0)str2+="Your inventory is full\n";
	}
	a=0;
	for(std::vector2<std::pair<int, int> >::iterator i=q->killmobs.begin();i!=q->killmobs.end();++i)
	{
		if(j->second[a]<i->second)str2+="You need to kill more "+monsterlist[i->first].name;
	}
	if(str2!="")
	{
		p->bs->cmd(pid, 0x24) << (short)0x12;
		p->bs->sndpstr(str2);
		*p->bs << 0;
	}

}

bool character_npc::endthequest(tquest *q, tplayer *p)
{
	bool retval=true;
	int a,b;
	item *t;
	item targy;
	std::vector2<tquest::qitem>::iterator i;
	std::map<int, pquestdata>::iterator j=p->quests.find(q->questid);
	if(j==p->quests.end())return false;

//	retval&=(p->pk>=q->endkarmamin)&&(p->pk<=q->endkarmamax);

	if(q->beginflags!=0)
	{
		if((q->beginflags&QBF_GUILDLESS)!=0)retval&=(p->guild==0);
	}

/*TODO*/
//if(q->endstate==0)	//!
{
	retval&=j->second.state==q->beginstate;


	if((!q->enditems.empty())&&(retval))
	{
		for(i=q->enditems.begin();(i!=q->enditems.end())&&(retval);++i)
		{
			b=i->num;
			for(a=0;a<inventory::eqoff;a++)
			{
				t=p->inv.getMaskedItem(a);
				if((t!=0)&&(t->getId()==i->id))
				{
					b-=t->num;
					if(b<=0)break;
				}
			}
			retval&=(a<inventory::eqoff);
		}
	}
}
	if((!q->toremoveitems.empty())&&(retval))
	{
		for(i=q->toremoveitems.begin();(i!=q->toremoveitems.end())&&(retval);++i)
		{
			b=i->num;
			for(a=0;a<inventory::eqoff;a++)
			{
				t=p->inv.getMaskedItem(a);
				if((t!=0)&&(t->getId()==i->id))
				{
					b-=t->num;
					if(b<=0)break;
				}
			}
			retval&=(a<inventory::eqoff);
		}
	}

	if(retval)
	{
		a=q->rewarditems.size();
		for(b=0;(b<inventory::eqoff)&&(a>0);b++)
		{
			if(p->inv.getMaskedItem(b)!=0)
				if(p->inv.getMaskedItem(b)->getId()==-1)
					a--;
		}
		retval&=(a<1);
		if(retval)
		{
			a=0;
			for(std::vector2<std::pair<int, int> >::iterator i=q->killmobs.begin();i!=q->killmobs.end();++i)
			{
				retval&=(j->second[a]>=i->second);
			}
		}
	}

	return retval;
}

void character_npc::addrewards(tquest *q, tplayer *p)
{
	int a;
	std::vector2<tquest::qitem>::iterator i;
	item targy;
	if(q->rewardjob!=-1)
	{
		for(a=0;a<16;a++)if(tquest::joblookup[a]==q->rewardjob)break;
		p->job=a;
		p->jobchange();
	}

	long double expMultiplier1=expMultiplier;
	expMultiplier=1.0f;
	p->addexp(q->rewardexpmin+rnd(q->rewardexpmax-q->rewardexpmin), p->level);
	expMultiplier=expMultiplier1;

	if(q->endkarmamax!=0)p->setkarma(p->pk+q->endkarmamax);

	a=q->rewardmoneymin+rnd(q->rewardmoneymax-q->rewardmoneymin);
	p->money+=a;
	if(a!=0)setstat(p->bs, p->getId(), 79, p->money);

	for(i=q->toremoveitems.begin();i!=q->toremoveitems.end();++i)
	{
		p->inv.removeitemid(i->id, i->num);
	}


	for(i=q->rewarditems.begin();i!=q->rewarditems.end();++i)
	{
		targy.setId(i->id);
		targy.num=i->num;
		p->inv.pickup(&targy);
	}

	if(q->endflags!=0)
	{
		if((q->endflags&QEF_CREATEGUILD)!=0)
		{
			p->guild=new tguild(p, p->dbid, p->level, p->job, p->name, p->getId());
		}
	}

}

bool character_npc::abletobegin(tquest *q, tplayer *p)
{
	item *t;
	std::vector2<tquest::qitem>::iterator i;
	int a,b;
	bool retval=true;
	std::map<int, pquestdata>::iterator j=p->quests.find(q->questid);
	if(j!=p->quests.end())return false;
	std::set<int>::iterator j2=p->finishedquests.find(q->questid);
	if(j2!=p->finishedquests.end())return false;

	if(q->beginflags!=0)
	{
		if((q->beginflags&QBF_GUILDLESS)!=0)retval&=(p->guild==0);
	}

	retval&=q->beginstate==0;
	retval&=((p->level>=q->startlevel)&&(p->level<=q->endlevel));
	retval&=(q->disquise==0)||(p->getmodelid()==q->disquise);
	retval&=((p->pk<=q->karmamax)&&(p->pk>=q->karmamin));
//	for(a=0;a<16;a++)if(tquest::joblookup[a]==q->beginjob)
	retval&=(q->beginjob==0)||((tquest::joblookup[p->job]&q->beginjob)!=0);

	if(retval)for(std::vector2<int>::iterator k=q->prevquests.begin();(k!=q->prevquests.end())&&(retval);++k)
	{
		j2=p->finishedquests.find(*k);
		retval&=(j2!=p->finishedquests.end());
	}

	if((!q->beginitems.empty())&&(retval))
	{
		for(i=q->beginitems.begin();(i!=q->beginitems.end())&&(retval);++i)
		{
			b=i->num;
			for(a=0;a<inventory::eqoff;a++)
			{
				t=p->inv.getMaskedItem(a);
				if((t!=0)&&(t->getId()==i->id))
				{
					b-=t->num;
					if(b<=0)break;
				}
			}
			retval&=(a<inventory::eqoff);
		}
	}
	return retval;
}

void character_npc::questinformation(tplayer *p, tquest *q, buffer &bs)
{
	int pid=p->getId();
	std::string str1;
	str1="NO TEXT\nStart npc: " + q->startcharacter + "\nEnd npc: " + q->endcharacter;
	str1+="\nLevel: " + toString(q->startlevel) + " - " + toString(q->endlevel);
	str1+="\nBeginstate: " + toString(q->beginstate);
	str1+="\nEndstate: " + toString(q->endstate);
	bs.cmd(pid, 0x24) << (short)0x12;
	bs.sndpstr(str1);
	bs << 0;

	str1="";
	if(!q->killmobs.empty())
	{
		str1="Kill mobs: ";
		for(std::vector2<std::pair<int, int> >::iterator j=q->killmobs.begin();j!=q->killmobs.end();++j)
		{
			str1+=monsterlist[(*j).first].name + ": " + toString(j->second) + " ";
		}
		str1+="\n";
	}
	if(!q->enditems.empty())
	{
		str1="Get items: ";
		for(std::vector2<tquest::qitem>::iterator j=q->enditems.begin();j!=q->enditems.end();++j)
		{
			str1+=itemlist[j->id].name + ": " + toString(j->num) + " ";
		}
		str1+="\n";
	}

	bs.cmd(pid, 0x24) << (short)0x12;
	bs.sndpstr(str1);
	bs << 0;
					
}

void character_npc::npcchat(tplayer *p, std::string &b, int c, int d, int e)
{
	if(pet)return;
//	buffer &bs=*p->bs;
	int pid=p->getId();
	int a,a2,f;
	std::string str1;
	std::list<tquest*>::iterator i;
	std::map<int, std::string>::iterator j;

	if(b=="exit")
	{
		p->bs->cmd(pid, 0x24) << (short)0x16;
		p->setfocus(0);
		return;
	}

	p->bs->cmd(pid, 0x24) << (short)0x1d;
	if(b=="")
	{
		for(i=qend.begin();i!=qend.end();++i)
		{
			if(!endthequest(*i, p))missingstuff(*i, p);
		}
		for(i=qend.begin();i!=qend.end();++i)
		{
			if(endthequest(*i, p))
			{
				addrewards(*i, p);
				p->save_quests=true;
				
				{				
					j=(*i)->dialogs.find((*i)->dialog_finished);
					if(j!=(*i)->dialogs.end())
					{
						p->bs->cmd(pid, 0x24) << (short)0x12;
						p->bs->sndpstr(j->second);
						*p->bs << 0;
					}else if((*i)->endstate==(*i)->beginstate)
					{
						std::string str1;
						str1="NO TEXT\n Quest completed " + (*i)->title;
						p->bs->cmd(pid, 0x24) << (short)0x12;
						p->bs->sndpstr(str1);
						*p->bs << 0;
					}
					if((*i)->endstate==(*i)->beginstate)
					{
						music(p->bs, 20);
						p->bs->cmd(pid, 0x95) << 0x01fa;
						str1="";
						str1+=(char)34;
						str1+=(*i)->title;
						str1+=(char)34;
						p->bs->sndpstr(str1);
						if(!(*i)->repeatable)p->bs->cmd(pid, 0xb0) << 0x0e << (short)(*i)->questid << 0;
						else p->bs->cmd(pid, 0x3a) << -1 << (*i)->questid;

						if(!(*i)->repeatable)p->finishedquests.insert((*i)->questid);
						p->quests.erase((*i)->questid);

					}else 
					{
//						bs.sndpstr(cl->print("QSI_%d %d", (*i)->questid, (*i)->endstate));
						if(!p->quests.find((*i)->questid)->second.setstate((*i)->endstate))
						{
							logger.log("quest has no %d state! quest erased\n", (*i)->endstate);
							p->quests.erase((*i)->questid);
							p->save_quests=true;
						}else
						{
							a=(*i)->questid;
							a2=(*i)->endstate;
							goto nextpart;
						}
					}
					goto sfc;
				}

			}
		}

		p->bs->cmd(pid, 0x24) << (short)0x12;
		p->bs->sndpstr(greetingtext);
		*p->bs << 0;
	}else if(b.substr(0, 3)=="ND_")
	{
		sscanf(&b.c_str()[3], "%d", &a);
		tnpcdialog &nd=npcdialogs.at(a);
		for(a2=0;a2<5;a2++)
		{
			if(nd.dialogs[a2]!="")
			{
				p->bs->cmd(pid, 0x24) << (short)0x12;
				p->bs->sndpstr(nd.dialogs[a2]);
				*p->bs << 0;
			}
		}
	}else if(b.substr(0, 4)=="QBY_")
	{
		sscanf(&b.c_str()[4], "%d %d", &a, &a2);
nextpart:
		for(i=qbegin.begin();i!=qbegin.end();++i)
		{
			if(((*i)->questid==a)&&((*i)->beginstate==a2))
			{

						/*1*/
				if(beginthequest(*i, p))
				{
//					p->greentext(cl->print("%d--%d %s\n", a, (*i)->questid, (*i)->title.c_str()));
					p->quests.insert(std::pair<int, pquestdata>((*i)->questid, pquestdata((*i)->questid, 0, 0, 0)));
					p->save_quests=true;
					p->bs->cmd(pid, 0xb0) << (*i)->beginstate << (short)(*i)->questid << 0;	/*9*/
					music(p->bs, 19);
					p->bs->cmd(pid, 0x95) << 0x01f9;
					str1="";
					str1+=(char)34;
					str1+=(*i)->title;
					str1+=(char)34;
					p->bs->sndpstr(str1);
					if((*i)->beginstate==0)
					{
						j=(*i)->dialogs.find((*i)->dialog_accept);
						if(j!=(*i)->dialogs.end())
						{
							p->bs->cmd(pid, 0x24) << (short)0x12;
							p->bs->sndpstr(j->second);
							*p->bs << 0;
						}
					}else
					{
						j=(*i)->dialogs.begin();
						if(j==(*i)->dialogs.end())
						{
							questinformation(p, *i, *p->bs);
						}else for(;j!=(*i)->dialogs.end();++j)
						{
							p->bs->cmd(pid, 0x24) << (short)0x12;
							p->bs->sndpstr(j->second);
							*p->bs << 0;
						}
					}

					p->bs->cmd(pid, 0x24) << (short)0x13;
					p->bs->sndpstr("__OK__");
					p->bs->sndpstr("");
					*p->bs << 0;
					*p->bs << (*i)->questid;
				}
				break;
			}
		}
	}else if(b.substr(0, 4)=="QBN_")
	{
		sscanf(&b.c_str()[4], "%d %d", &a, &a2);
		for(i=qbegin.begin();i!=qbegin.end();++i)
		{
			if(((*i)->questid==a)&&((*i)->beginstate==a2))
			{

				j=(*i)->dialogs.find((*i)->dialog_deny);
				if(j!=(*i)->dialogs.end())
				{
					if(j!=(*i)->dialogs.end())
					{
						p->bs->cmd(pid, 0x24) << (short)0x12;
						p->bs->sndpstr(j->second);
						*p->bs << 0;
					}
				}
				break;
			}
		}
	}else if(b.substr(0, 4)=="QSI_")
	{
		sscanf(&b.c_str()[4], "%d %d", &f, &a2);
		for(i=qbegin.begin();i!=qbegin.end();++i)
		{	
			if(((*i)->questid==f)&&((*i)->beginstate==a2))
			{
//				bool foundq=false;
				bool qby=false,qbn=false;
				j=(*i)->dialogs.begin();
				if(j==(*i)->dialogs.end())
				{
					questinformation(p, *i, *p->bs);
					p->bs->cmd(pid, 0x24) << (short)0x13;
					p->bs->sndpstr("__YES__");
					p->bs->sndpstr(cl->print("QBY_%d %d", (*i)->questid, (*i)->beginstate));
					*p->bs << 0;
					*p->bs << (*i)->questid;
				}
				else
				{
					while(j!=(*i)->dialogs.end())
					{
						if(j->first<(*i)->dialog_askbefore)
						{
							if(j!=(*i)->dialogs.end())
							{
								p->bs->cmd(pid, 0x24) << (short)0x12;
								p->bs->sndpstr(j->second);
								*p->bs << 0;
							}
						}else break;

						j++;
					}
					p->bs->cmd(pid, 0x24) << (short)0x13;
					p->bs->sndpstr("__YES__");
					p->bs->sndpstr(cl->print("QBY_%d %d", (*i)->questid, (*i)->beginstate));
					*p->bs << 0;
					*p->bs << (*i)->questid;
					p->bs->cmd(pid, 0x24) << (short)0x13;
					p->bs->sndpstr("__NO__");
					p->bs->sndpstr(cl->print("QBN_%d %d", (*i)->questid, (*i)->beginstate));
					*p->bs << 0;
					*p->bs << (*i)->questid;
				}

				break;
			}
		}
	}

	for(i=qbegin.begin();i!=qbegin.end();++i)
	{
		if(abletobegin(*i, p))
		{
			p->bs->cmd(pid, 0x24) << (short)0x10;
			p->bs->sndpstr((*i)->title);
			p->bs->sndpstr(cl->print("QSI_%d %d", (*i)->questid, (*i)->beginstate));
			*p->bs << 0 << 0;

		}
	}

	for(size_t i=0;i<npcdialogs.size();++i)
	{
		p->bs->cmd(pid, 0x24) << (short)0x10;
		p->bs->sndpstr(npcdialogs[i].link);
		p->bs->sndpstr(cl->print("ND_%d", i));
		*p->bs << 0 << 0;
	}

sfc:
	p->bs->cmd(pid, 0x24) << (short)0x10;
	p->bs->sndpstr("Farewell");
	p->bs->sndpstr("exit");
	*p->bs << 0 << 0;
}


bool character_npc::shopcontains(int itemid)
{
	int a,b;
	for(a=0;a<4;a++)
	{
		for(b=0;b<nshops[a];b++)
		{
			if(shops[a][b]==itemid)return true;
		}
	}
	return false;
}

