#include "stdafx.h"
#include "character_obj.h"
#include "grid.h"
#include "cluster.h"
#include "objactions.h"
#include "spawns.h"

void objaction::destroy()
{
	while(!guards.empty())
	{
		guards.front()->clear();
		guards.pop_front();
	}
}


character_obj::character_obj(int t, cluster *c, int m, float x1, float y1, float z1, int d, int s, objaction *oa1, spawner* os1)
{
	type=ttobject;
	oa=oa1;
	os=os1;
	cl=c;
	pos.x=x1;
	pos.y=y1;
	pos.z=z1;
	lastpos=pos;
	size =s;
	mapid=m;
	id=cl->getobjid(this);

	if(oa!=0)
	{
		modeltype=2;
		oa->setowner(this);
	}else
	{
		modeltype=0;
		if(t==33)modeltype=2;
	}
	modelid=t;
	createremovepacket();
	dir=d;

	sp=new buffer;
	sp->cmd(id, 0xf0);
	*sp << (char)modeltype << modelid << (char)modeltype << (short)(modelid) << (short)size;
	bsx=sp->insert(pos.x);
	bsy=sp->insert(pos.y);
	bsz=sp->insert(pos.z);
	*sp << (short)dir << id;

	sp->cmd(-1, 0xd0);
	*sp << id;
	sp->sndpstr("OBJ");
	sp->sndpstr("OBJ");

/*1*/

//	gridcell=0;
//	gridcell=cl->grid.get(mapid, (int)pos.x, (int)pos.z);
//	gridcell->addobj(this);
	gridcell.add();
	abletomove=1;
	type=ttobject;
}


void character_obj::removethis()
{
	cl->releaseobjid(id);
//	gridcell->removeobj(this);
	gridcell.remove();
	if(oa!=0)delete oa;
	if(os!=0)os->unregister(this);
}
