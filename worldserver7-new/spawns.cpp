#include "stdafx.h"
#include "spawns.h"
#include "character_obj.h"
#include "monster.h"
#include "character_base.h"
#include "item.h"
#include "cluster.h"
#include "objactions.h"

flarisdroom::tfdp flarisdroom::fdp[7]={{1190, 80, 553}, {1193, 80, 543}, {1190, 80, 532}, {1183, 80, 529}, {1176, 80, 535}, {1174, 80, 541}, {1167, 80, 550}};

void itemspawn::timer()
{
	if(cl->ido>=lastrespawn+respawnrate)
	{
		lastrespawn=cl->ido;
		for(;n<num;n++)
			new character_item(item(type), cl, mapid, x+rnd(rx*2)-rx,y,z+rnd(ry*2)-ry, 0, this);
	}
}

void mobspawn::timer()
{
	if(cl->ido>=lastrespawn+respawnrate)
	{
		lastrespawn=cl->ido;
		for(;nnormal<normal;nnormal++)new character_mob(type, cl, mapid, x+rnd(rx*2)-rx, y, z+rnd(ry*2)-ry, false, 1, this);
		for(;nagro<agro;nagro++)new character_mob(type, cl, mapid, x+rnd(rx*2)-rx, y, z+rnd(ry*2)-ry, true, 1, this);
	}
}

void mobspawn::unregister(character_base *q)
{
	character_mob *p=(character_mob*)q;
	if(p->agro)nagro--;
	else nnormal--;
}


void spawner::removethis()
{
	cl->removeobjspawner(remover);
}

spawner::spawner(cluster *c):cl(c)
{
	cl->addobjspawner(this, remover);
}


void flarisdroom::timer()
{
	character_obj *o;
	int t,m,n;
	while(!torespawn.empty())
	{
		t=torespawn.front();
		n=10;
		switch(t)
		{
		case 0:
			m=661;
			break;
		case 6:
		case 1:
			m=666;
			break;
		case 2:
		case 3:
		case 4:
		case 5:
			n=1;
			if(rnd(100)<40)m=666;
			else
			{
				if(rnd(100)<10)m=665;
				else m=664;
			}
			break;
		}
		o=new character_obj(26, cl, 200, fdp[t].x, fdp[t].y, fdp[t].z, rnd(3600), 100, new openandspawn(m,n,true), this);
		o->tag=t;
		torespawn.pop_front();
	}
}

void flarisdroom::unregister(character_base *o)
{
	torespawn.push_front(((character_obj*)o)->tag);
}

void gobjspawner::timer()
{
	for(;torespawn>0;torespawn--)
	{
		new character_obj(id, cl, mapid, x+rnd(rx*2)-rx, y, z+rnd(ry*2)-ry, rnd(3600), 100, oaf(), this);
	}
}