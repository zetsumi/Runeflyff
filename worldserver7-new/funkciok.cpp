#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include "player.h"
#include "buffer.h"

int rnd(int a)
{
	a=(int)((rand()/(RAND_MAX+1.0))*a);
	return a;
}

void effect(buffer *bs, int id, int a)
{
	bs->cmd(id, 0x0f) << a << 0 << 0 << 0 << 0;
}

void effectxyz(buffer *bs, int id, int a, float fx, float fy, float fz)
{
	bs->cmd(id, 0x0f) << a << fx << fy << fz << 0;
}

void skilleffect(buffer *bs, int id, int cel, int skill, int skilllevel, int as, int n3)
{
	if(n3==-1)
	{
		if(as==0)n3=3;
		else if(as==1)n3=0x0d;
		else n3=0;
	}
	bs->cmd(id, 0x19) << skill << skilllevel << cel << as << n3;
}

void bufficon(buffer *bs, int id1, int id2, int a, int b, int c, int e)
{
	bs->cmd(id1, 0x4c) << id2 << (short)a << (short)b << c << e;
}

void setstat(buffer *bs, int playerid, int stat, int val)
{
	bs->cmd(playerid, 0x1e) << stat << val;
}

void addstat(buffer *bs, int playerid, int stat, int val, int val2)
{
	bs->cmd(playerid, 0x1c) << stat << val << val2;
}

void substat(buffer *bs, int playerid, int stat, int val)
{
	bs->cmd(playerid, 0x1d) << stat << val;
}

void sound(buffer *bs, int a)
{
	bs->cmd(-1, 0xd2) << (char)0 << a;
}

void music(buffer *bs, int a)
{
	bs->cmd(-1, 0xd1) << a;
}

void gotoid(buffer *bs, int id, int toid, float range)
{
	bs->cmd(id, 0xc2) << toid << range;
}

void gotoxyz(buffer *bs, int id, float x, float y, float z)
{
	bs->cmd(id, 0xc1) << x << y << z << (char)1;
}

void changeface(buffer *bs, int dbid, int face)
{
	bs->cmd(-1, 0x4d) << dbid << face;
}

void settime(buffer *bs1, int id, float fx)
{
	unsigned int b,c;
	float a;
	a=(float)((unsigned int)(fx*60.0));
	b=(unsigned int)(a/27.3125);
	a-=((float)b)*27.3125f;
	c=(unsigned int)a+0xb6;
	b&=0xff;
	c&=0xff;
	bs1->cmd(id, 0x96);
//	(*bs1) << (char)0x96;
	(*bs1) << 0 << (unsigned char)c << (unsigned char)b << (unsigned char)0x95 << (unsigned char)0x41;
//	bs1->inc();

}

void chat(buffer *bs, int id, char *p)
{
	bs->cmd(id, 1);
	bs->sndpstr(p);
}

void teleport(buffer *bs, int id, float x, float y, float z)
{
	bs->cmd(id, 0x10) << x << y << z;
}

void motion2(buffer *bs, int id, float x, float y, float z, int mot)
{
	bs->cmd(id, 0xca);//cb
	*bs << x << y << z << 0 << 0 << 0;
	*bs << 0 << 1 << 21 << mot << mot << 2 << 2 << -1 << 0;
}

void setfuel(buffer *bs, int id, int ammount, int magic)
{
	bs->cmd(id, 0x4b) << magic << ammount;
}
