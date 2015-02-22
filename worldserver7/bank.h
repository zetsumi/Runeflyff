#ifndef __bank_h__
#define __bank_h__

#include "inventory.h"

#include "vector2.h"

class cluster;
class tplayer;
class tguild;
class bank
{
	friend class tguild;
	int nitems;
	std::vector<bool> usedslots;
protected:
	virtual void insertpacket(item &targy);
	virtual void removepacket(unsigned int a, int b);
	std::vector2<item> itemek;
public:

	void clear();
	int getnitems()
	{
		return nitems;
	}
	tplayer *player;
	int banknum;
	item *getItem(int a);
	void init(tplayer *ch, int b=0, int n=42);
	bank(tplayer *ch=0, int b=0, int n=42);
	virtual ~bank()
	{
	}
	int insert(item &i);
	int removeitem(int a, int b);
	void load(char *p);
	void save(char *p);
};

class backpack:public bank
{
	void insertpacket(item &targy);
	void removepacket(unsigned int a, int b);
public:
	backpack(tplayer *ch=0, int b=0, int n=24):bank(ch,b,n){}
};

class warehouse:public bank
{
	void insertpacket(item &targy);
	void removepacket(unsigned int a, int b);
	buffer *bs;
public:
	warehouse(int n=0x2a):bank(0,0,n){}
	void setplayer(tplayer *p, buffer *b)
	{
		player=p;
		bs=b;
	}
};


#endif
