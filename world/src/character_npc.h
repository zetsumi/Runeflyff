#ifndef __character_npc_h__
#define __character_npc_h__

#include "character_base.h"
#include <string>

#include "scheduler.h"

class tquest;
class tgridcell;
class tplayer;
class character_npc:public character_base
{
	void destroy();
	int nshops[4];
	int *shops[4];
	int dbid;
	int dir;
	std::list<tquest*> qbegin, qend;
	bool abletobegin(tquest *q, tplayer *p);
	bool beginthequest(tquest *q, tplayer *p);
	bool endthequest(tquest *q, tplayer *p);
	void addrewards(tquest *q, tplayer *p);
	void missingstuff(tquest *q, tplayer *p);
	void questinformation(tplayer *p, tquest *q, buffer &bs);
	bool pet;
	tplayer *owner;
	struct tnpcdialog
	{
		std::string link;
		std::string dialogs[5];
	};
	std::vector2<tnpcdialog> npcdialogs;
	schedulerl<character_npc*>::sremover npcchatremover;
	std::string greetingtext;
public:
	bool shopcontains(int itemid);
	schedulerl<character_npc*>::sremover petremover;
	std::string chattext;
	std::string name;
	bool petpick();
	character_npc(cluster *c, int type, std::string n, float x1, float y1, float z1, int m, int d, char *shop1, char *shop2, char *shop3, char *shop4, int l1, int l2, int l3, int l4, int dbid, const std::string &ct, const std::string &gt);
	character_npc(cluster *c, int type, float x1, float y1, float z1, int m, int d, tplayer *o);
	virtual ~character_npc()
	{
		destroy();
	}
	void sndshop(buffer *bs);
	bool addtoshop(int itemid, int slot);
	bool delfromshop(int itemid, int slot);
	void clearshop(int slot);
	void save();
	void npcchat(tplayer *p, std::string &b, int c, int d, int e);
};

#endif
