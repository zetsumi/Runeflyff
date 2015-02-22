#ifndef __inventory_h__
#define __inventory_h__

#include "item.h"

#include "vector2.h"

class cluster;
class buffer;
class tplayer;

class inventory
{
public:
	static const int eqoff=44;
	static const int eqoff2=42;
	static const int nitems=73;
	void fillmask();
	void filllookup();
private:	
	bool unequip2(int a);
	void allocate(int b1);
	void remupbonus();
	std::vector2<int> mask;
	std::vector2<item> itemek;
	std::vector2<int> lookup;
	int playerid;
	void checksetbonus();
public:
	void setplayerid(int id)
	{
		playerid=id;
	}
	item* getItem(int a);
	item* getMaskedItem(int a);
	int getMask(int a);
	int getLookup(int a);
	void updateitem(int a);
	void updateitemammount(int a);
	tplayer *player;
	inventory(tplayer *ch=0);
	void init(tplayer *ch);
	inventory(const inventory& a);
	void load(char *q);
	void save(char *q);
	int removeitem(int place,int num);
	int removeitemid(int id,int num);
	bool unequip(int a);
	bool equip(int a);
	int pickup(item *targy, bool silent=false);
	int getCapacity(item *targy);
	int getRemainder(item *targy, int cap);
	int getRemainder(item *targy);
	void moveitem(int a, int b);
	void dump();
	void checkset();
	bool contains(int id);
	bool decsuppliment(int id);
};

void initsetbonuses();

#endif
