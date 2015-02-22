#include "stdafx.h"
#include "slot.h"
#include "inventory.h"
#include "logger.h"
#include "adat.h"

int slot::load(char *p, inventory &inv, int job)
{
	int r;
	type=*(int*)p;
	text="";
	if(type!=8)
	{
		type2=*(int*)(&p[4]);
		id=*(int*)(&p[8]);
		r=12;
		if(type==5)
		{
			if(inv.getItem(id)->getId()<=0)
			{
				type=0;
				id=0;
				type2=0;
			}
		}else if(type==6)
		{
			if(skills[job][id]<1)
			{
				type=0;
				id=0;
				type2=0;
			}
		}
	}else
	{
		type2=0;
		id=0;
		r=*(int*)(&p[4]); 
		for(int a=0;a<r;a++)text+=p[a+8];
		r+=8;
	}
	return r;
}

char* slot::save(int &h, inventory &inv)
{
	char *r;
	if(type!=8)
	{
		h=12;
		if(type==5)
		{
			id=inv.getLookup(id);
			if(id==-1)
			{
				id=0;
				type=0;
				type2=0;
			}
		}
		r=new char[12];
		*(int*)r=type;
		*(int*)(&r[4])=type2;
		*(int*)(&r[8])=id;
	}else
	{
		h=text.length()+8;
		r=new char[h];
		*(int*)r=type;
		*(int*)(&r[4])=h-8;
		memcpy(&r[8], text.c_str(), h-8);
	}
	return r;
}
