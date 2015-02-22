#include "stdafx.h"
#include "grid.h"
#include "player.h"
#include "buffer.h"
#include "error.h"

#include "player.h"
#include "monster.h"
#include "character_npc.h"
#include "cluster.h"
#include "gridremover.h"

int cellsize=64;

inline int mod(int a, int o)
{
	int r=a%o;
	if(r<0)r+=o;
	return r;
}


int tgridcell::getmapid()
{
	return owner->getmapid();
}
int tgridcell::getworldid()
{
	return owner->getworldid();
}

tgridcell::tgridcell(tworld *o, int x1, int z1)
:owner(o),nelements(0),nplayers(0),x(x1),z(z1),marked(false),nplayersaround(0)
,mobiterator(0),playeriterator(0)
{
	init();
	bs=new buffer;
}
tgridcell::~tgridcell()
{
//	if(setmarked)owner->removemark(tbr);
	deinit();
	delete bs;
	bslist.clear();
}

void tgridcell::multicast2(buffer &a)
{
	if(nplayersaround>0)
	{
		for(int b=0;b<9;b++)
			if(neighbour[b]!=0)
				neighbour[b]->addbs(a);
	}
}

void tgridcell::getmulticast(tplayer *p)
{
	if(bs->getcommandnumber()>0)p->add(*bs);
	if(!bslist.empty())
		for(std::list<buffer>::iterator j=bslist.begin();j!=bslist.end();++j)
				p->add(*j);
}

void tgridcell::domulticast(tplayer *p)
{
	if(!marked)return;
	std::list<tplayer*>::iterator i;
	bool fixplayers1=false;
	if(bs->getcommandnumber()>0)
	{
		for(i=players.begin();i!=players.end();++i)
		{
			if((*i)->validnl())
			{
//				ul mmm=(*i==p)?pmutex::dontlock():(*i)->playermutex.lock();
				(*i)->add(*bs);
			}else 
			{
				fixplayers1=true;
				tplayer *p=*i;
				i=players.erase(i);
				p->gridcell.cleargc();
				if(i==players.end())break;
			}
		}
		delete bs;
		bs=new buffer;
	}
	if(!bslist.empty())
	{
		for(i=players.begin();i!=players.end();++i)
		{
			if((*i)->validnl())
			{
//				ul mmm=(*i==p)?pmutex::dontlock():(*i)->playermutex.lock();
				for(std::list<buffer>::iterator j=bslist.begin();j!=bslist.end();++j)
					(*i)->add(*j);
			}else
			{
				fixplayers1=true;
				tplayer *p=*i;
				i=players.erase(i);
				p->gridcell.cleargc();
				if(i==players.end())break;
			}
		}
		bslist.clear();
	}
	if(fixplayers1)fixplayers();
	marked=false;
}

void tgridcell::markbc()
{
	if(!marked)
	{
//		bslist.clear();
		owner->addbc(this, broadcast_remover);
	}
	marked=true;
}

void tgridcell::init()
{
	int a,b;
	tgridcell *t;
	for(b=0;b<=2;b++)
		for(a=0;a<=2;a++)
		{
			if((a==1)&&(b==1))neighbour[4]=this;
			else
			{
				t=owner->getneighbour(x+a-1,z+b-1);
				neighbour[a+b*3]=t;
				if(t!=0)t->neighbour[2-a+(2-b)*3]=this;
			}
		}
	nplayersaround=0;
	for(a=0;a<9;a++)
		if(neighbour[a]!=0)
			nplayersaround+=neighbour[a]->nplayers;
}

void tgridcell::deinit()
{
	int a,b;
	if(owner->running)
	{
		for(b=0;b<=2;b++)
			for(a=0;a<=2;a++)
				if(neighbour[a+b*3]!=0)neighbour[a+b*3]->neighbour[2-a+(2-b)*3]=0;
		owner->removecell(this);
	}
}

std::list<tplayer*>::iterator tgridcell::addcharacter(tplayer *p)
{
	domulticast(p);

//	players.insert(std::pair<int, tplayer*>(p->id, p));
	players.push_front(p);
	nelements++;
	incplayers();
	this->multicast2(p->spawn());
	int a;
	for(a=0;a<9;a++)
	{
		if(this->neighbour[a]!=0)neighbour[a]->createallfor(p);
	}
	return players.begin();
}

void tgridcell::removecharacter(tplayer *p)
{
	getmulticast(p);
	std::list<tplayer*>::iterator i=p->gridcell.getplayeriterator();
/*
	if(playeriterator==0)players.erase(i);
	else if(i==*playeriterator)*playeriterator=players.erase(*playeriterator);
		 else players.erase(i);
*/
		 if(playeriterator==0)players.erase(p->gridcell.getplayeriterator());
			else try{
				if(p->gridcell.getplayeriterator()==*playeriterator)*playeriterator=players.erase(*playeriterator);
				else players.erase(p->gridcell.getplayeriterator());
			}catch (...)
			{
				players.erase(p->gridcell.getplayeriterator());
			}

	p->gridcell.cleariterator();

	nelements--;
	decplayers();
	this->multicast2(p->removepacket());
	if(nelements<=0)delete this;
}

std::list<character_item*>::iterator tgridcell::addcharacter(character_item *p)
{
//	items.insert(std::pair<int, character_item*>(p->id, p));
	items.push_front(p);
	nelements++;
	this->multicast2(p->spawn());
	return items.begin();
}

void tgridcell::removecharacter(character_item *p)
{

	//if(items.erase(p->id)!=0)
	items.erase(p->gridcell.getitemiterator());
	p->gridcell.cleariterator();
	nelements--;
	this->multicast2(p->removepacket());
	if(nelements<=0)delete this;
}

std::list<character_obj*>::iterator tgridcell::addcharacter(character_obj *p)
{
	//objs.insert(std::pair<int, character_obj*>(p->id, p));
	objs.push_front(p);
	nelements++;
	this->multicast2(p->spawn());
	return objs.begin();
}

void tgridcell::removecharacter(character_obj *p)
{
	//if(objs.erase(p->id)!=0)
	objs.erase(p->gridcell.getobjiterator());
	p->gridcell.cleariterator();
	nelements--;
	this->multicast2(p->removepacket());
	if(nelements<=0)delete this;
}

std::list<character_mob*>::iterator tgridcell::addcharacter(character_mob *p)
{
//	mobs.insert(std::pair<int, character_mob*>(p->id, p));
	mobs.push_front(p);
	nelements++;
	this->multicast2(p->spawn());
	return mobs.begin();
}

void tgridcell::removecharacter(character_mob *p)
{
/*
	std::list<character_mob*>::iterator i=p->gridcell.getmobiterator();
	if(mobiterator==0)mobs.erase(i);
	else if(*mobiterator==i)*mobiterator=mobs.erase(*mobiterator);
		 else mobs.erase(i);
*/
			if(mobiterator==0)mobs.erase(p->gridcell.getmobiterator());
			else try{
				if(*mobiterator==p->gridcell.getmobiterator())*mobiterator=mobs.erase(*mobiterator);
				else mobs.erase(p->gridcell.getmobiterator());
			}catch(...)
			{
				mobs.erase(p->gridcell.getmobiterator());
			}

	p->gridcell.cleariterator();
//	if(mobs.erase(p->id)!=0)
	nelements--;
	this->multicast2(p->removepacket());
	if(nelements<=0)delete this;
}

std::list<character_npc*>::iterator tgridcell::addcharacter(character_npc *p)
{
//	npcs.insert(std::pair<int, character_npc*>(p->id, p));
	npcs.push_front(p);
	nelements++;
	this->multicast2(p->spawn());
	return npcs.begin();
}

void tgridcell::removecharacter(character_npc *p)
{
	npcs.erase(p->gridcell.getnpciterator());
	p->gridcell.cleariterator();
//	if(npcs.erase(p->id)!=0)
	nelements--;
	this->multicast2(p->removepacket());
	if(nelements<=0)delete this;
}
void tgridcell::addbs(buffer& bs1)
{
	if(nplayers>0)
	{
		markbc();
		if(bs1.forbidden==0)bs->add(bs1);
		else bslist.push_back(bs1);
	}
}

void tgridcell::removeallfrom(tplayer *p)
{
	std::list<tplayer*>::iterator i1;
	std::list<character_item*>::iterator i2;
	std::list<character_mob*>::iterator i3;
	std::list<character_obj*>::iterator i4;
	std::list<character_npc*>::iterator i5;
	for(i1=players.begin();i1!=players.end();++i1)
		if((*i1)!=p)p->bs->cmd((*i1)->getId(), 0xf1);
	for(i2=items.begin();i2!=items.end();++i2)
		p->bs->cmd((*i2)->getId(), 0xf1);
	for(i3=mobs.begin();i3!=mobs.end();++i3)
		p->bs->cmd((*i3)->getId(), 0xf1);
	for(i4=objs.begin();i4!=objs.end();++i4)
		p->bs->cmd((*i4)->getId(), 0xf1);
	for(i5=npcs.begin();i5!=npcs.end();++i5)
		p->bs->cmd((*i5)->getId(), 0xf1);
}
void tgridcell::createallfor(tplayer *p)
{
	ul m=p->asyncbuffermutex.lock();
	if(p->asyncbuffer==0)p->asyncbuffer=new buffer();
	std::list<tplayer*>::iterator i1;
	std::list<character_item*>::iterator i2;
	std::list<character_mob*>::iterator i3;
	std::list<character_obj*>::iterator i4;
	std::list<character_npc*>::iterator i5;
	for(i1=players.begin();i1!=players.end();++i1)
		if((*i1)!=p)p->asyncbuffer->add((*i1)->spawn());
	for(i2=items.begin();i2!=items.end();++i2)
		p->asyncbuffer->add((*i2)->spawn());
	for(i3=mobs.begin();i3!=mobs.end();++i3)
		p->asyncbuffer->add((*i3)->spawn());
	for(i4=objs.begin();i4!=objs.end();++i4)
		p->asyncbuffer->add((*i4)->spawn());
	for(i5=npcs.begin();i5!=npcs.end();++i5)
		p->asyncbuffer->add((*i5)->spawn());
}




tgridcell* tgridcell::update(character_base* p, int x, int z)
{
	int a;

	bool tip=false;

	x/=cellsize;
	z/=cellsize;
	if((x!=this->x)||(z!=this->z))
	{
		tgridcell *r=0;
		if(!isneighbour(x,z))r=&owner->get(x*cellsize, z*cellsize);
		else
		{
			r=neighbour[x-this->x+1 + (z-this->z+1)*3];
			if(r==0)r=owner->createneighbour(x, z);
		}
//#ifdef _DEBUG
		if(r==0)throw error("r is 0", "tgridcell");
		if(r==this)throw error("r is this", "tgridcell");
//#endif
		nelements--;
		switch(p->getType())
		{
		case character_base::ttplayer:
//			players.erase(((tplayer*)p)->ggcr());
			if(playeriterator==0)players.erase(p->gridcell.getplayeriterator());
			else try{
				if(p->gridcell.getplayeriterator()==*playeriterator)*playeriterator=players.erase(*playeriterator);
				else players.erase(p->gridcell.getplayeriterator());
			}catch (...)
			{
				players.erase(p->gridcell.getplayeriterator());
			}
			p->gridcell.cleariterator();
			decplayers();
//			r->players.push_front(((tplayer*)p));
//			((tplayer*)p)->sgcr(r->players.begin());
//			r->incplayers();
			r->players.push_front(((tplayer*)p));
			p->gridcell.setandadd(r, r->players.begin());
			r->incplayers();
			tip=true;
			break;
		case character_base::ttmob:
//			mobs.erase(((character_mob*)p)->gridcellremover);
			if(mobiterator==0)mobs.erase(p->gridcell.getmobiterator());
			else try{
				if(*mobiterator==p->gridcell.getmobiterator())*mobiterator=mobs.erase(*mobiterator);
				else mobs.erase(p->gridcell.getmobiterator());
			}catch(...)
			{
				mobs.erase(p->gridcell.getmobiterator());
			}
			p->gridcell.cleariterator();
			r->mobs.push_front(((character_mob*)p));
			p->gridcell.setandadd(r, r->mobs.begin());
			break;
		case character_base::ttitem:
			items.erase(p->gridcell.getitemiterator());
			p->gridcell.cleariterator();
			r->items.push_front((character_item*)p);
			p->gridcell.setandadd(r, r->items.begin());
			break;
		case character_base::ttobject:
			objs.erase(p->gridcell.getobjiterator());
			p->gridcell.cleariterator();
			r->objs.push_front((character_obj*)p);
			p->gridcell.setandadd(r, r->objs.begin());
			break;
		case character_base::ttnpc:
			npcs.erase(p->gridcell.getnpciterator());
			p->gridcell.cleariterator();
			r->npcs.push_front((character_npc*)p);
			p->gridcell.setandadd(r, r->npcs.begin());
			break;
		break;
//#ifdef _DEBUG
		default:
			throw error("Mover type is not set. Go and cry in a corner.", "tgridcell");
//#endif
		}

		r->nelements++;

		for(a=0;a<9;a++)
		{
			if(neighbour[a]!=0)neighbour[a]->mark=0;
			if(r->neighbour[a]!=0)r->neighbour[a]->mark=0;
		}
		for(a=0;a<9;a++)
		{
			if(neighbour[a]!=0)neighbour[a]->mark++;
			if(r->neighbour[a]!=0)r->neighbour[a]->mark+=2;
		}

		if(tip)
		{
			for(a=0;a<9;a++)
			{
				if(neighbour[a]!=0)if(neighbour[a]->mark==1)
				{
					neighbour[a]->getmulticast((tplayer*)p);
					neighbour[a]->removeallfrom((tplayer*)p);
				}
				if(r->neighbour[a]!=0)if(r->neighbour[a]->mark==2)
				{
					r->neighbour[a]->domulticast((tplayer*)p);
					r->neighbour[a]->createallfor((tplayer*)p);
				}
			}

		}
		buffer &spawnbs=p->spawn();
		for(a=0;a<9;a++)
		{
			if(neighbour[a]!=0)if(neighbour[a]->mark==1)
			{
				neighbour[a]->addbs(p->removepacket());
			}
			if(r->neighbour[a]!=0)if(r->neighbour[a]->mark==2)
			{
				r->neighbour[a]->addbs(spawnbs);
			}
		}
		if(nelements<=0)
		{
//#ifdef _DEBUG
			if(r==this)
				throw error("gridcell suicided without an update", "tgridcell::update");;
//#endif
			if(r!=this)delete this;
			else logger.log("gridcell is the same");
		}
		return r;
	}
	return this;
}

tgridcell* tgridcell::update(character_base* p, int x, int z, buffer &bs1)	//used for teleport only
{
//	tgridcell *r=this;
	bool tip=false;
	bool m3=false;
	int x1=x,z1=z;
	x/=cellsize;
	z/=cellsize;
	if((x!=this->x)||(z!=this->z))
	{
		tgridcell *r=0;
		int a;
		if(!isneighbour(x,z))r=&owner->get(x1, z1);
		else
		{
			r=neighbour[x-this->x+1 + (z-this->z+1)*3];
			if(r==0)
			{
				r=owner->createneighbour(x, z);
			}
		}
//#ifdef _DEBUG
		if(r==0)throw error("r is 0", "tgridcell");
		if(r==this)throw error("r is this", "tgridcell");
//#endif
		nelements--;
		switch(p->getType())
		{
		case character_base::ttplayer:
//			players.erase(((tplayer*)p)->ggcr());
/*
			if(playeriterator==0)players.erase(p->gridcell.getplayeriterator());
			else if(p->gridcell.getplayeriterator()==*playeriterator)*playeriterator=players.erase(*playeriterator);
				 else players.erase(p->gridcell.getplayeriterator());
*/
			if(playeriterator==0)players.erase(p->gridcell.getplayeriterator());
			else try{
				if(p->gridcell.getplayeriterator()==*playeriterator)*playeriterator=players.erase(*playeriterator);
				else players.erase(p->gridcell.getplayeriterator());
			}catch (...)
			{
				players.erase(p->gridcell.getplayeriterator());
			}

			p->gridcell.cleariterator();
			decplayers();
//			r->players.push_front(((tplayer*)p));
//			((tplayer*)p)->sgcr(r->players.begin());
//			r->incplayers();
			r->players.push_front(((tplayer*)p));
			p->gridcell.setandadd(r, r->players.begin());
			r->incplayers();
			tip=true;
			break;
		case character_base::ttmob:
//			mobs.erase(((character_mob*)p)->gridcellremover);
/*
			if(mobiterator==0)mobs.erase(p->gridcell.getmobiterator());
			else if(*mobiterator==p->gridcell.getmobiterator())*mobiterator=mobs.erase(*mobiterator);
				 else mobs.erase(p->gridcell.getmobiterator());
*/
			if(mobiterator==0)mobs.erase(p->gridcell.getmobiterator());
			else try{
				if(*mobiterator==p->gridcell.getmobiterator())*mobiterator=mobs.erase(*mobiterator);
				else mobs.erase(p->gridcell.getmobiterator());
			}catch(...)
			{
				mobs.erase(p->gridcell.getmobiterator());
			}

			p->gridcell.cleariterator();
			r->mobs.push_front(((character_mob*)p));
			p->gridcell.setandadd(r, r->mobs.begin());
			break;
		case character_base::ttitem:
			items.erase(p->gridcell.getitemiterator());
			p->gridcell.cleariterator();
			r->items.push_front((character_item*)p);
			p->gridcell.setandadd(r, r->items.begin());
			break;
		case character_base::ttobject:
			objs.erase(p->gridcell.getobjiterator());
			p->gridcell.cleariterator();
			r->objs.push_front((character_obj*)p);
			p->gridcell.setandadd(r, r->objs.begin());
			break;
		case character_base::ttnpc:
			npcs.erase(p->gridcell.getnpciterator());
			p->gridcell.cleariterator();
			r->npcs.push_front((character_npc*)p);
			p->gridcell.setandadd(r, r->npcs.begin());
			break;
//#ifdef _DEBUG
		default:
			throw "Mover type is not set. Go and cry in a corner.";
//#endif
		}

		r->nelements++;


		for(a=0;a<9;a++)
		{
			if(neighbour[a]!=0)neighbour[a]->mark=0;
			if(r->neighbour[a]!=0)r->neighbour[a]->mark=0;
		}
		for(a=0;a<9;a++)
		{
			if(neighbour[a]!=0)neighbour[a]->mark++;
			if(r->neighbour[a]!=0)r->neighbour[a]->mark+=2;
		}
//----
		if(tip)
		{
			for(a=0;a<9;a++)
			{
				if(neighbour[a]!=0)if(neighbour[a]->mark==1)
				{
					neighbour[a]->getmulticast((tplayer*)p);
					neighbour[a]->removeallfrom((tplayer*)p);
				}
			}
		}
		for(a=0;a<9;a++)
		{
			if(neighbour[a]!=0)if(neighbour[a]->mark==1)
			{
				neighbour[a]->addbs(p->removepacket());
			}
		}

//----

		if(tip)((tplayer*)p)->raise();
//		int n=0;
		for(a=0;a<9;a++)
		{
			if(r->neighbour[a]!=0)
				if(r->neighbour[a]->mark==3)
				{
//					n++;
					r->neighbour[a]->addbs(bs1);
					r->neighbour[a]->domulticast((tplayer*)p);
				}
		}
		if((r->mark!=3)&&(p->type==character_base::ttplayer))
			((tplayer*)p)->add(bs1);

		if(tip)((tplayer*)p)->raise();
//----
		if(tip)
		{
			for(a=0;a<9;a++)
			{
				if(r->neighbour[a]!=0)if(r->neighbour[a]->mark==2)
				{
					r->neighbour[a]->domulticast((tplayer*)p);
					r->neighbour[a]->createallfor((tplayer*)p);
				}
			}
		}

		buffer &spawnbs=p->spawn();
		for(a=0;a<9;a++)
		{
			if(r->neighbour[a]!=0)if(r->neighbour[a]->mark==2)
			{
				r->neighbour[a]->addbs(spawnbs);
			}
		}

		if(nelements<=0)
		{
//#ifdef _DEBUG
			if(r==this)throw error("gridcell suicided without an update", "tgridcell::update");
//#endif
			if(r!=this)delete this;
			else logger.log("gridcell is the same");
		}
		return r;
	}else
	{
		this->multicast2(bs1);
	}
	return this;
}

