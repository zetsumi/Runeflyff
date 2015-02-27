#include "stdafx.h"
#include "character_base.h"
#include "grid.h"
#include "cluster.h"
#include "adat.h"
#include "stringf.h"
#include "spawns.h"

character_item::character_item(item t, cluster *c, int m, float x1, float y1, float z1, void *o, spawner *sw1, void *o2)
	:targy(t),owner(o),owner2(o2),sw(sw1),ir(0)
{
	type=ttitem;
	pos.x=x1;
	pos.y=y1;
	pos.z=z1;
	lastpos=pos;
	mapid=m;
	cl=c;
	id=cl->getitemid(this);
	spawntime=cl->ido+60*1000;
	if(owner!=0)picktime=cl->ido+30*1000;
	else picktime=0;
	modelid=targy.getId();
	modeltype=4;
	createremovepacket();

	sp=new buffer;
	sp->cmd(id, 0xf0);
	*sp << (char)modeltype << modelid << (char)modeltype << (short)(modelid) << (short)size;
	bsx=sp->insert(pos.x);
	bsy=sp->insert(pos.y);
	bsz=sp->insert(pos.z);
	*sp << (short)0 << id << -1;
	targy.snddata(sp);

	gridcell.add();
//	gridcell=0;
//	gridcell=cl->grid.get(mapid, (int)pos.x, (int)pos.z);
//	gridcell->additem(this);

	if(sw==0)
	{
		ir=new itemremover(this);
		cl->additem(ir);
	}
	abletomove=1;
}

bool character_item::ableToPick(void *p)
{
	return (owner==0)||(p==owner)||(p==owner2)||(picktime<cl->ido);
}

void character_item::pickedup()
{
	if(ir!=0)ir->zeroit();
}

void character_item::setnum(int n)
{
	if((n>=0)&&(n<this->targy.num))
	{
		targy.num=n;
		if(n==0)
		{
			this->pickedup();
			delete this;
		}
	}
}

void character_item::removethis()
{
	if(ir!=0)ir->zeroit();
	cl->releaseitemid(id);
	gridcell.remove();
	if(sw!=0)sw->unregister(this);
}



void character_base::movein(const vector3d<>& i)
{
	float len;
//	if(abletomove<=0)
	{
		len=i.length();
		if(len==0)
		{
			stopmove();
			return;
		}
//		stopmove();
		movestart=cl->ido-90;
		lastmove=cl->ido-90;
		start=pos;
		irany=i;
		movelength=99999999999999999999999999999.9f;
		irany/=len;
		cel=irany*900+start;
		if(!moverremover.isset())cl->addmover(this, moverremover);
		movetype=1;
		moveing=true;
		delete spb;
		spb=0;
		domove();
		startmove();
	}
}

void character_base::moveto(float x1, float y1, float z1)
{
	float len;
//	if(abletomove<=0)
	{
//		stopmove();
		movestart=cl->ido-90;
		lastmove=cl->ido-90;
		start=pos;
		cel.set(x1,y1,z1);
		irany=cel-start;
		len=irany.length();
		if(len==0)
		{
			stopmove();
			return;
		}
		irany/=len;
		movelength=(cel-start).sqrlength();
		if(!moverremover.isset())cl->addmover(this, moverremover);
		movetype=0;
		moveing=true;
		delete spb;
		spb=0;
		domove();
		startmove();
	}
}

bool character_base::movefollow()
{
//	float len;
	if(getfollowed()==0)
	{
		stopmove();
		return false;
	}
//	if((abletomove<=0)&&(distance(*followed)>followdistance))
	{
//		stopmove();
		movestart=cl->ido;
		lastmove=cl->ido;
		if(!moverremover.isset())cl->addmover(this, moverremover);
		movetype=2;
		moveing=true;
		delete spb;
		spb=0;
		startmove();
		return true;
	}
}

bool character_base::domove()
{
	if(!moverremover.isset())return true;
	if((cl->ido-lastmove<=2)||(abletomove>0)||((movetype==2)&&(followed==0)))return false;
	float t=(cl->ido-lastmove)/1000.0f;
	lastmove=cl->ido;
	t*=speed;
	lastpos=pos;

	if(movetype!=2)
	{
		vector3d<> p(pos);
		p+=irany*t;
		if((p-start).sqrlength()>=movelength)
		{
			pos=cel;
			moveing=false;
			delete spb;
			spb=0;
			checkmove();
			onChangePosition();
			arrived();
		}else
		{
			pos=p;
			checkmove();
			onChangePosition();
		}
	}else
	{
		bool stm=false;
		stm=(distance(*followed)<=followdistance+0.05);
		if(!stm)
		{
			vector3d<> v(followed->getpos());
			cel=v;
			v-=pos;
			if(v.sqrlength()<=t*t)
			{
				v*=0.999f;
			}else
			{
				v/=v.length();
				v*=t;
			}
			pos+=v;
			checkmove();
			onChangePosition();

			stm=(followed==0)||((followed!=0)&&(distance(*followed)<=followdistance+0.05));
			if(stm)
			{
				moveing=false;
				delete spb;
				spb=0;
				arrived();
			}
		}else
		{
			moveing=false;
			delete spb;
			spb=0;
			arrived();
		}
	}
//	gridcell->update(gridcell, this, (int)pos.x, (int)pos.z);
	gridcell.check();
	return !moverremover.isset();
}

void character_base::checkmove()
{
	gridcell.checkmove(lastpos, pos, this);
}

void character_base::processfollowers(bool stm)
{
	if(!followers.empty())
	{
		if(!stm)
		{
			buffer bs;
			for(std::list<std::pair<character_base*, float> >::iterator i=followers.begin();i!=followers.end();++i)
			{
				if((!i->first->isMoveing())&&(distance(*i->first)>i->second*i->second))
				{
					i->first->movefollow();
					gotoid(&bs, i->first->getId(), id, i->second);
				}
			}
			if(bs.getcommandnumber()>0)multicast(bs);
		}else
		{
			followersonstop();
		}
	}
}

void character_base::followersonstop()
{
	if(!followers.empty())
	{
		buffer bs;
		for(std::list<std::pair<character_base*, float> >::iterator i=followers.begin();i!=followers.end();++i)
		{
			if(distance(*i->first)>i->second*i->second)
			{
				vector3d<> v=i->first->getpos();
				v-=getpos();
				v/=v.length();

				v*=i->second;
				v+=getpos();

				i->first->moveto(v.x, v.y, v.z);
				gotoid(&bs, i->first->getId(), id, i->second);
			}
		}
		if(bs.getcommandnumber()>0)multicast(bs);
	}
}

void character_base::stopmove()
{
	if(moverremover.isset())
	{
		cl->removemover(moverremover);
		followersonstop();
	}
	moveing=false;
}

void character_base::setfocus(character_base *f)
{
	if(focus!=0)
	{
		focus->focuslist.erase(focusremover);
		focus=0;
	}
	if(f!=0)
	{
		focus=f;
		focusremover=focus->addfocus(this);
	}
}

void character_base::removefocuses()
{
	while(!focuslist.empty())
	{
		focuslist.front()->setfocus(0);
//		focuslist.pop_front();
	}
}

std::list<character_base*>::iterator character_base::addfocus(character_base *f)
{
	focuslist.push_front(f);
	return focuslist.begin();
}


void character_base::onChangePosition()
{
	*bsx=pos.x;
	*bsy=pos.y;
	*bsz=pos.z;
//	gridcell->update(gridcell, this, (int)pos.x, (int)pos.z);
	gridcell.check();
}

void character_base::multicast(buffer &bs)
{
	if(bs.getcommandnumber()>0)gridcell.multicast2(bs);
}

int character_base::getdir(const vector3d<> &v2)
{
	vector3d<> v=v2/v2.length();
	float f;
	f=acos(-v.z)*(180.0f/3.1415f);

	return (int)(f*10);
}

void character_base::chat(const char *t)
{
	buffer bs;
	bs.cmd(id, 1);
	bs.sndpstr(t);
	multicast(bs);
}

void character_base::setandcheckpos(float fx,float fy,float fz)	/*11*/
{
	vector3d<> v(fx,fy,fz);
	if(v!=pos)
	{
		lastpos=pos;
		pos.set(fx,fy,fz);
		checkmove();
		onChangePosition();
		tgridcell *c=gridcell.ggc();
//		gridcell->update(gridcell, this, (int)pos.x, (int)pos.z);
		gridcell.check();
		if(c!=gridcell.ggc())checkmove();
	}
}

character_base::character_base()
	:rs(0),sp(0),spb(0),abletomove(0),moveing(false),type(ttnone),size(100),
	state(1),state2(0),speed(6.00f),basespeed(6.00f),focus(0),dir(rnd(3600)),sp9(0),lsp9(0),
	spmovepacket(false),followed(0),gridcell(this)
{}
