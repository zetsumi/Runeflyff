#ifndef __funkciok_h__
#define __funkciok_h__

int rnd(int a);

class buffer;
void sound(buffer *bs, int a);
void music(buffer *bs, int a);
void effect(buffer *bs, int id, int a);
void effectxyz(buffer *bs, int id, int a, float fx, float fy, float fz);
void setstat(buffer *bs, int playerid, int stat, int val);
void addstat(buffer *bs, int playerid, int stat, int val, int val2=-1);
void substat(buffer *bs, int playerid, int stat, int val);
void gotoxyz(buffer *bs, int id, float x, float y, float z);
void gotoid(buffer *bs, int id, int toid, float range);
void skilleffect(buffer *bs, int id, int cel, int skill, int skilllevel, int as=0, int n3=-1);
void bufficon(buffer *bs, int id1, int id2, int a, int b, int c, int e);
void changeface(buffer *bs, int dbid, int face);
void settime(buffer *bs, int id, float time);
void teleport(buffer *bs, int id, float x, float y, float z);
void motion2(buffer *bs, int id, float x, float y, float z, int mot);
void setfuel(buffer *bs, int id, int ammount, int magic=1);


#endif
