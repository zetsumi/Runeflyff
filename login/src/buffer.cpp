#include "buffer.h"

#include "mysql.h"
#include <list>
using namespace std;

void buffer::dump()
{
#ifdef _DEBUG
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
}

void buffer::addbytes(char *p, int step)
{
	int a,l=strlen(p);
	char ch,dh;
	for(a=0;a<l;a+=step)
	{
		ch=p[a];
		if((ch>='0')&&(ch<='9'))ch-='0';
		else if((ch>='A')&&(ch<='F'))ch=ch-'A'+10;
		else if((ch>='a')&&(ch<='f'))ch=ch-'a'+10;
		else break;
		dh=p[a+1];
		if((dh>='0')&&(dh<='9'))dh-='0';
		else if((dh>='A')&&(dh<='F'))dh=dh-'A'+10;
		else if((dh>='a')&&(dh<='f'))dh=dh-'a'+10;
		else break;
		dh=dh*16+ch;
		*this << dh;
	}
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
	bool sw=insendpos!=0;
    while(insend!=plist.end())
	{
		if((*insend)->pos>insendpos)
		if(sw)
		{
			packetLoggerText.logPacket(&q[insendpos], (*insend)->pos-insendpos);
			a=send(sck, &q[insendpos], (*insend)->pos-insendpos,0);
		}else
		{
			packetLoggerText.logPacket(&(*insend)->puffer[insendpos], (*insend)->pos-insendpos);
			a=send(sck, &(*insend)->puffer[insendpos], (*insend)->pos-insendpos,0);
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
				if(!sw)memcpy(&q[insendpos], &(*insend)->puffer[insendpos], (*insend)->pos-insendpos);
				break;
			}
		}
    }
    if(insend==plist.end())retval=1;

    return retval;
}