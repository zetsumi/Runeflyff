#include "stdafx.h"
#include "fieldobj.h"
#include "cluster.h"
#include "grid.h"
#include "logger.h"
#include "buff.h"
#include "adat.h"
#include "player.h"

fieldobj::fieldobj(cluster *c, int m, const vector3d<> &p):cl(c), mapid(m), pos(p)
{
	gridcell=cl->grid.get(mapid, (int)pos.x, (int)pos.z);
	gridcell->fieldobjs.push_front(this);
	gridcell->nelements++;
	remover=gridcell->fieldobjs.begin();
}

void fieldobj::destroy()
{
	gridcell->fieldobjs.erase(remover);
	gridcell->nelements--;
}

f_psywall::f_psywall(cluster *c, int m, const vector3d<> &p, const vector3d<> &n, float r, tplayer *o)
	:fieldobj(c, m, vector3d<>(p.x,p.y+r,p.z)), normal(n), range(r*r),owner(o)
{
	s=&skilllist[163][owner->skilllevels[163]-1];
};

bool f_psywall::check(const vector3d<> &s, const vector3d<> &e, character_base *p)
{
//	if((!cl->isPK())&&(p->type==character_base::ttplayer))return false;
	if((p->type==character_base::ttplayer)&&(!((tplayer*)(p))->ispkable(this->owner) ))return false;

	if((p->type!=character_base::ttmob)&&(p->type!=character_base::ttplayer))return false;
	vector3d<> i=e-s;
	float a=normal*i;
	if(a==0)return false;
	float t=(normal*(pos-s))/a;
	bool retval=(t>=0)&&(t*t<=(i.sqrlength()));
	if(retval)
	{
		p->pos+=normal*5;

		buffer bs2;
		bs2.cmd(p->getId(), 0x5f) << p->pos.x << p->pos.y << p->pos.z << 0 << 0 << 0;
		gridcell->multicast2(bs2);
//		p->onChangePosition();
		owner->attack(bs2, -1, this->s->min+rnd(this->s->max-this->s->min)+(owner->intl+owner->stats[DST_INT]+owner->stats[DST_STAT_ALLUP])*this->s->rval1, this->s);
//		p->checkmove();
		;//damage here
	}
	return retval;
}

bool f_telc::check(const vector3d<> &s, const vector3d<> &e, character_base *p)
{
	if(p->type!=character_base::ttplayer)return false;
	tplayer *q=(tplayer*)p;
	bool retval=((e-pos).sqrlength()<range)&&(!q->flying);
	if(retval)
	{
		q->moveing=false;
		q->changemap(destmap, dest.x, dest.y, dest.z);
	}
	return retval;
}
