#ifndef __item_h__
#define __item_h__

#include "vector2.h"
#include <string>
#include "error.h"

class buffer;
class tplayer;
class sqlquery;
int getitempos(int id);

void inititemfunkcs();

struct itemdata;
typedef bool (*ITEMFUNKC)(tplayer*, itemdata*);
struct itemdata
{
	int id;
	std::string name;
	int gender;
	int num;
	int min;
	int max;
	int eqslot;
	int eqslot2;
	int level;
	int job;
	int cost;
	int duration;
	int handed; 
	int weapontype;
	float aspd;
	int destparam1;
	int destparam2;
	int destparam3;
	int adjparamval1;
	int adjparamval2;
	int adjparamval3;
	float flightspeed;
	bool valid;
	__int8 gendrop;
	int sfx;
	int mobid;	//for mdrops
	ITEMFUNKC funkc;
	itemdata()
	{
		valid=false;
		funkc=0;
		mobid=0;
		gendrop=0;
	}
};

class cluster;
class item
{
	int id;
	itemdata *i;
public:
	static const int itemdatasize=45;
	bool itemuse(tplayer *player);
	itemdata* GetI()
	{
//#ifdef _DEBUG
		if(i==0)throw error(std::string("Incorrect itemdata! id=")+toString(id), "item::GetI");
//#endif
		return i;
	}
	bool valid()
	{
		return (i!=0);
	}
	static const int nslots=5;
	void addbonus(tplayer *player);
	void rembonus(tplayer *player);
	void getstat(int *str, int *sta, int *dex, int *intl);
	unsigned int getstat();
	void snddata(buffer *bs);
	int getId() const{return id;};
	void setId(int id1);
	item(int id1=-1);
	item(const item &a);
	item& operator =(const item &a);
	void getItemData(int id1);
	int num;
	int durability;
	int upgrade;
	int element;
	int eupgrade;
	std::vector2<int> slots;
	unsigned char itemstat;
	void load(char *q);
	void save(char *q);
};

#endif
