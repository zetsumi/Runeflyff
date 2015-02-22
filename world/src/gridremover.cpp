#include "grid.h"
#include "cluster.h"

void tgridcellremover::addcharacter()
{
	if(!added)
	{
		if(i!=0)
		{
			delete i;
			i=0;
		}
		switch(p->type)
		{
		case character_base::ttplayer:
			i=new tplayer_p(owner->addcharacter((tplayer*)p));
			break;
		case character_base::ttmob:
			i=new character_mob_p(owner->addcharacter((character_mob*)p));
			break;
		case character_base::ttnpc:
			i=new character_npc_p(owner->addcharacter((character_npc*)p));
			break;
		case character_base::ttobject:
			i=new character_obj_p(owner->addcharacter((character_obj*)p));
			break;
		case character_base::ttitem:
			i=new character_item_p(owner->addcharacter((character_item*)p));
			break;
//#ifdef _DEBUG
		default:
			throw error("mover type is not set", "gridremover::addcharacter");
//#endif
		};
		added=true;
	}
}

bool tgridcellremover::add()
{
	owner=p->cl->grid.get(p->getmapid(), (int)p->pos.x, (int)p->pos.z);
	addcharacter();
	purposely_added=true;
	return (p->cl->grid.contains(p->getmapid(), (int)p->pos.x, (int)p->pos.z));
}

void tgridcellremover::check(buffer &bs)
{
	if((!added)&&(purposely_added))add();
	if((purposely_added)&&(owner!=0))owner=owner->update(p, (int)p->pos.x, (int)p->pos.z, bs);
}

void tgridcellremover::check()
{
	if((!added)&&(purposely_added))add();
	if((purposely_added)&&(owner!=0))owner=owner->update(p, (int)p->pos.x, (int)p->pos.z);
}


void tgridcellremover::removecharacter()
{
	if(added)
	{
	tgridcell *o=owner;
	added=false;
	owner=0;
	switch(p->type)
	{
	case character_base::ttplayer:
		o->removecharacter((tplayer*)p);
		break;
	case character_base::ttmob:
		o->removecharacter((character_mob*)p);
		break;
	case character_base::ttnpc:
		o->removecharacter((character_npc*)p);
		break;
	case character_base::ttobject:
		o->removecharacter((character_obj*)p);
		break;
	case character_base::ttitem:
		o->removecharacter((character_item*)p);
		break;
//#ifdef _DEBUG
	default:
		throw error("mover type is not set", "gridremover::removecharacter");
//#endif
	};
	}
}

int tgridcellremover::playersaround()
{
	if(owner!=0)return owner->playersaround();
	return 0;
}

void tgridcellremover::multicast2(buffer &bs1)
{
	if(owner!=0)owner->multicast2(bs1);
}
/*
std::list<tplayer*>::iterator tgridcellremover::begin()
{
	if(owner!=0)return owner->players.begin();
	return players1.begin();
}
std::list<tplayer*>::iterator tgridcellremover::end()
{
	if(owner!=0)return owner->players.end();
	return players1.end();
}
*/
tgridcell* tgridcellremover::getsz(int a)
{
	if(owner!=0)return owner->getsz(a);
	return 0;
}

bool tgridcellremover::checkmove(const vector3d<> &s, const vector3d<> &e, character_base *p)
{
	if(owner!=0)return owner->checkmove(s, e, p);
	return false;
}
