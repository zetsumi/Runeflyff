#include "stdafx.h"
#include "buffer.h"

#include "mysql.h"
#include <list>
using namespace std;

void buffer::dump()
{
#ifdef _DEBUG
#ifdef _LOGALOT
	bpcont *p;
	int a, b=0, c=0;;
	std::list<bpcont*>::iterator i;
	logger.log("BS:\n");
	for(i=plist.begin();i!=plist.end();++i)
	{
		c++;
		p=*i;
		logger.log("PLIST %d %d\n", p->pos, nsize);
		for(a=0;a<p->pos;a++)
		{
			logger.log("%02x ", (int)(unsigned char)p->puffer[a]);
		}
		if(p->getn()<1)b++;
		logger.log("\n");
	}
	logger.log("END\n");
//	logger.log("\nnp=%d\n", c);
	if(b>0)logger.elog("NULL PUFFERS! %d\n", b);
#endif
#endif
}


sendablebuffer::sendablebuffer()
:sck(0),insendpos(0)
{
	insend=plist.end();
}

sendablebuffer::sendablebuffer(const buffer &a)
:buffer(a),sck(0),insendpos(0)
{
	insend=plist.end();
}


void sendablebuffer::initflush(int sck)
{
	this->sck=sck;
    insend=plist.begin();
    insendpos=0;
}

int sendablebuffer::continueflush(char *q)
{
    int retval=0;
    int a=0;
	bool sw=(insendpos!=0);
    while(insend!=plist.end())
	{
		if((*insend)->pos>insendpos)
		{
			if(sw)
			{
#ifdef use_s
				a=s_send::send(sck, &q[insendpos], (*insend)->pos-insendpos,0);
#else
				a=send(sck, &q[insendpos], (*insend)->pos-insendpos,0);
#endif
			}else
			{
#ifdef use_s
				a=s_send::send(sck, &(*insend)->puffer[insendpos], (*insend)->pos-insendpos,0);
#else
				a=send(sck, &(*insend)->puffer[insendpos], (*insend)->pos-insendpos,0);
#endif
			}
		}
		if(a>0)
		{
			insendpos+=a;
			if(insendpos>=(*insend)->pos)
			{
				++insend;
				insendpos=0;
				sw=false;
			}else
			{
//				if(!sw&&((*insend)->haspointer))memcpy(&q[insendpos], &(*insend)->puffer[insendpos], (*insend)->pos-insendpos);
				if(!sw)memcpy(&q[insendpos], &(*insend)->puffer[insendpos], (*insend)->pos-insendpos);
				break;
			}
		}else break;
    }
    if(insend==plist.end())retval=1;

    return retval;
}