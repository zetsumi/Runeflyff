#include "stdafx.h"
#include "charserver.h"
#include "network.h"
#include "logger.h"
#include "main.h"
#include "inventory.h"
#include "item.h"
#include "cluster.h"
#include "player.h"

#define ID_DCUSER 0xf03103ff
#define ID_NOGG	  0xf03103fe


//#define ConnectToLogin

int loginserverPort=23000;
const std::string& loginserverName="127.0.0.1";

charserver::charserver(int a, std::string sip)
:sck(a),endprg(false),ulong1(1),serverip(sip),hc(1),lsck(-1),startaccept(false)
{
	TimeOut.tv_sec = 0;
	TimeOut.tv_usec = 1;

	cthread = std::thread([this]() -> void { this->cmain(); });
	athread = std::thread([this]() -> void { this->amain(); });
}

charserver::~charserver()
{
	endprg=true;
	if (cthread.joinable()) {
	    cthread.join();
	}
}

void charserver::dcall()
{
	while(!mplayers.empty())
	{
		closesocket(mplayers.front()->sck);
		delete mplayers.front();
		mplayers.pop_front();
	}
}

void charserver::amain()
{
	struct sockaddr_in addr;
	int addrlen=sizeof(addr);
	TIMEVAL TimeOut;
	TimeOut.tv_sec = 0;
	TimeOut.tv_usec = 1;
	fd_set readfds;
	while(!startaccept)Sleep(1);
	int b=serversocket+1;
	int a,newsocket;
	unsigned long ulong1=1;
	char *clientip;
	while(!endprg)
	{
		FD_ZERO(&readfds);
		FD_SET((unsigned int)serversocket, &readfds);
		a=select(b, &readfds, 0,0,&TimeOut);
		if(FD_ISSET(serversocket,&readfds))
		{
			newsocket=accept(serversocket, (struct sockaddr*) &addr, (socklen_t*) &addrlen);

#ifdef WIN32
			ulong1=1;
			ioctlsocket(newsocket, FIONBIO, (unsigned long*)&ulong1);
#else
			ioctl(newsocket, O_NONBLOCK, 0);
#endif

			clientip = inet_ntoa(addr.sin_addr);
	        logger.log("Client %s:%d connected\n", clientip, newsocket);
			accepted.push(std::pair<int, std::string>(newsocket, std::string(clientip)));
		}
		Sleep(1);
	}
}

void charserver::cmain()
{
	connection.init(mysqlhost, mysqluser, mysqlpasswd, mysqldb);

	if(!startserver(&serversocket, sck))endprg=true;
	startaccept=true;
	if(!endprg)
	{
		FD_ZERO(&readfds);
//		FD_SET((unsigned int)serversocket, &readfds);
		logger.log("Characterserver started on %s:%d\n", this->serverip.c_str(), sck);
		do
		{
			try
			{
				while(!endprg)
				{
					ido=GetTickCount();
					{
					    std::lock_guard<std::mutex> guard(this->charservermutex);
						this->doselect();
					}
					Sleep(1);
				}
			}catch(error &e)
			{
				logger.log("[Char] Error: %s\n", e.what());
			}catch(std::exception &e)
			{
				logger.log("[Char] Exception: %s\n", e.what());
			}
#ifndef _DEBUG
			catch(...)
			{
				logger.elog("[Char]exception...\n");
			}
#endif

		}while(!endprg);
		dcall();
		logger.log("Characterserver ended\n");
	}

	athread.join();
}

charserver::tplayer::tplayer(charserver *o)
:insend(0),state(1),auth(false),errorstate(false),cid(-1),owner(o)
{
	loggedin=false;
	bs=new buffer;
	*bs << (char)0x5e;
	bslen=bs->insert(8);
	ido=owner->ido;
	*bs << 0 << (int)ido;

}

charserver::tplayer::~tplayer()
{
	delete bs;
	delete insend;
	if(cid!=-1)
	{
#ifdef _DEBUG
		logger.log("Marking for cluster %d %s\n", cid, name.c_str());
#endif
		(*clusters)[cid]->marklogged(this->name);
		/*12*/
	}
}

void charserver::tplayer::raise()
{
	int a=bs->length();
	if(a>5)
	{
		*bslen=a-5;
		bslist.push_back(bs);
		bs=new buffer;
		*bs << (char)0x5e;
		bslen=bs->insert(0);

	}
}

void charserver::tplayer::send()
{
	raise();
	if((insend==0)&&(!bslist.empty()))
	{

		insend=new sendablebuffer(*bslist.front());
//		logger.log("sck: %d\n", sck);
		delete bslist.front();
		bslist.pop_front();
//		insend->dump();
		insend->initflush(sck);
	}
	if(insend!=0)
	{
		if(insend->continueflush(&this->sendpuffer[0])==1)
		{
			delete insend;
			insend=0;
		}
	}
}

void charserver::getchars(int client, char *puffer, int n)
{
	int a;
	int c=0;

	while(c<n){
		a=recv(client, &puffer[c], n-c, 0);
		if(a<0){perror("recv()");return;}
		if(a==0){return;}
		c+=a;

	}
}
void charserver::senddcmsg(std::string &name)
{
/*
	char sendpuffer[buffer::nsize];
	sendablebuffer bs;
	int a=name.length()+4;
	bs << (char)0x5e << 12+a << 8+a;
	bs << 0 << (int)ID_DCUSER;
	bs.sndpstr(name);
	bs << 0;
	bs.initflush(lsck);
	while(!bs.continueflush(&sendpuffer[0]))Sleep(1);
*/
}

int charserver::connecttologinserver(char* loginserverName, int loginserverPort)
{
	char sendpuffer[buffer::nsize];
	char puffer[20];
	int loginserverSocket;
	struct sockaddr_in server;
	struct hostent *host_entry;

	loginserverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (loginserverSocket<0) {
		logger.log("Error opening socket for login server!\n");
		return -1;
	}
	if ((host_entry = gethostbyname(loginserverName)) == NULL) {
		logger.log("Cant find loginserver at: %s!\n", loginserverName);
		return -1;
	}

	server.sin_family = AF_INET;
	server.sin_port = htons(loginserverPort);
	server.sin_addr.s_addr = *(unsigned long*) host_entry->h_addr;

	if (connect(loginserverSocket, (sockaddr*)&server, sizeof(server))<0) {
		logger.log("Can't connect to login server!\n");
		return -1;
	}

	getchars(loginserverSocket,puffer,13);

	sendablebuffer bs;
	bs << (char)0x5e << 16 << 12;
	bs << 0 << (int)ID_NOGG << 0;
	bs << 0;
	bs.initflush(loginserverSocket);
	while(!bs.continueflush(&sendpuffer[0]))Sleep(1);
	if(loginserverSocket!=-1)logger.log("connected to loginserver\n");

	return loginserverSocket;
}

void charserver::doselect()
{
	int newsocket,a;
	bool recalc=false;

	tplayer* newconnection;
	char *clientip;
	std::list<tplayer*>::iterator i;
#ifdef ConnectToLogin
	if(lsck==-1)
	{
		lsck=connecttologinserver(&loginserverName[0], loginserverPort);
		if(lsck!=-1)
		{
			FD_SET((unsigned int)lsck, &readfds);
			if(lsck>hc-1)hc=lsck+1;
		}
	}
#endif
	fd_set lreadfds;
	std::pair<int, std::string> newplayerpair;

	std::list<tplayer*> recvmark, rtemp;
	std::list<tplayer*>::iterator j=mplayers.begin();
	for(;j!=mplayers.end();)
	{
		rtemp.clear();
		int	c=-1;
		FD_ZERO(&lreadfds);
		for(int a=0;(j!=mplayers.end())&&(a<FD_SETSIZE-2);a++,++j)
		{
			if((*j)->sck>c)c=(*j)->sck;
			FD_SET((unsigned int)(*j)->sck, &lreadfds);
			rtemp.push_back(*j);
		}
		if(c>-1)
		{
			select(c+1, &lreadfds, 0, 0, &TimeOut);
			for(std::list<tplayer*>::iterator i=rtemp.begin();i!=rtemp.end();++i)
			{
				if(FD_ISSET((*i)->sck,&lreadfds))recvmark.push_back(*i);
			}
		}
	}

//	select(hc, &lreadfds, 0, 0, &TimeOut);
#ifdef ConnectToLogin
	if(FD_ISSET(lsck, &lreadfds))
	{
		lsck=-1;
		recalc=true;
	}
#endif
//	if(FD_ISSET(serversocket,&lreadfds))
	if(!accepted.empty())
	{
		newplayerpair=accepted.pop();
		newsocket=newplayerpair.first;

		newconnection = new tplayer(this);
		newconnection->sck=newsocket;
		newconnection->ip=newplayerpair.second;
		newconnection->sr.init(newsocket);
		mplayers.push_back(newconnection);
	}
//	if(!mplayers.empty())logger.log(".");

	for(i=recvmark.begin();i!=recvmark.end();++i)
	{

		if(!(*i)->errorstate)
		{
			a=(*i)->sr.recieve();
			if(a==3)(*i)->process();
			else if((a==1)||(a==2))
			{
				(*i)->errorstate=true;
			}
		}

	}

	recvmark.clear();

	for(i=mplayers.begin();i!=mplayers.end();++i)
	{
		(*i)->send();
		if((*i)->ido<ido-2*60*1000)(*i)->errorstate=true;
		if((*i)->errorstate)
		{
			closesocket((*i)->sck);
			delete *i;
			i=mplayers.erase(i);
			if(i==mplayers.end())break;
			else continue;
		}
	}

	while(!to_dc.empty())
	{
		std::string str1=to_dc.pop();
		for(i=mplayers.begin();i!=mplayers.end();++i)
		{
			if((*i)->name==str1)
			{
				(*i)->errorstate=true;
				break;
			}
		}
	}
}

bool charserver::tplayer::process()
{
	try
	{
		this->ido=owner->ido;
		s1=sr.getlast4();
		int a;
		sr.get(a);
		sr.get(a);
		if(a==-1)
		{
			sr.get(a);
			switch(a)
			{
			case 0x0b:
					*bs << 0x11 << 0x29dd954e << 0xdb553ace << 0x97adb620 << 0x1cc05ca6;
					raise();
					state++;
				break;
			case 0x11:
//					if(!auth)
//					{
//						*bs << 0x14 << s1;
//						raise();
//					}
				break;
			case 0x14:
					/*if(state<3)*/*bs << 0x0b << s1 << 0xa8aec5ce << 0x01c85d7f;
//					else *bs << 0x14 << s1;
					raise();
				break;
			case 0xf6:
					f6funkc();
				break;
			case 0xf4:
					createchar();
				break;
			case 0xf5:
					chardel();
				break;
			case 0xff05:
					dologin();
				break;
			}
		}
	}catch(error &e)
	{
		logger.log("[Char] account=%d %s %s:%d Error: %s\n", accid, name.c_str(), ip.c_str(), sck, e.what());
	}catch(std::exception &e)
	{
		logger.log("[Char] account=%d %s %s:%d Exception: %s\n", accid, name.c_str(), ip.c_str(), sck, e.what());
	}
#ifndef _DEBUG
	catch(...)
	{
		logger.elog("[Char_process]exception...\n");
	}
#endif

	sr.clear();
	return false;
}

void charserver::tplayer::f6funkc()
{
	sqlquery s1(&owner->connection, "accounts");
	int a;

	sr.get(a);
	sr.forward(a);	//skip date

	sr.get(this->s1);
	char t[1025];
	sr.getpstr(&t[0], 1024);
	name=t;

	sr.getpstr(t, 1024);
	password=t;


	s1.selectw("name='"+name+"'", "name, passwd, id");
	if(s1.next())
	{
		accid=toInt(s1["id"]);
	}else
	{
		errorstate=true;
		s1.freeup();
		return;
	}
	s1.freeup();

	for(std::list<tplayer*>::iterator i=owner->mplayers.begin();i!=owner->mplayers.end();++i)
	{
		if(((*i)->accid==accid)&&((*i)!=this))
		{
			errorstate=true;
			return;
		}
	}


	auth=true;

	owner->senddcmsg(this->name);

	*bs << 0xf2;
	bs->sndpstr(owner->serverip);
	raise();

	sendcharinfo();

	state++;
}

void charserver::tplayer::sendcharinfo()
{
	int nchars=0;
	char *i, *p;
	int a,b,e,e2,neq;
	sqlquery s1(&owner->connection, "characters");
	s1.selectw("accid="+toString(accid),"count(*)");
	if(s1.next())nchars=toInt(s1[0]);
	s1.freeup();
	int ids[3];
	int accslots[3]={-1, -1, -1};
/*
	s1.selectw("accid="+toString(accid)+" AND loggedin!=0");
	this->loggedin=s1.next();
	s1.freeup();
*/
	if(nchars==0)
	{
		*bs << 0xf3 << this->s1 << 0 << 0;
		raise();
	}else
	{
		*bs << 0xf3 << this->s1 << nchars;
		s1.selectw("accid="+toString(accid), "*, length(inventory)");
		while(s1.next())
		{
			e=(int)toInt(s1["accslot"]);
			accslots[e]=e;

			*bs << e;
			if(!loggedin)*bs << 1; else *bs << 0;

			*bs << toInt(s1["mapid"]);
			*bs << 0x0b+toInt(s1["gender"]);
			bs->sndpstr(s1["name"]);
			*bs << toFloat(s1["x"]);
			*bs << toFloat(s1["y"]);
			*bs << toFloat(s1["z"]);
			e2=toInt(s1["id"]);
			ids[e]=e2;
			*bs << e2;

			*bs << 0 << 0 << 0 << 0;
			*bs << toInt(s1["hair"]);
			*bs << toInt(s1["haircolor"]);
			*bs << toInt(s1["face"]);
			*bs << (char)toInt(s1["gender"]);
			*bs << toInt(s1["job"]);
			*bs << toInt(s1["level"]);
			*bs << 0 << toInt(s1["str"]) << toInt(s1["sta"]) << toInt(s1["dex"]) << toInt(s1["intl"]);
			*bs << 0 << 0;
			i=s1.getptr("inventory");
			neq=0;
			p=i;
			if(toInt(s1["length(inventory)"])<=0)i=0;
			if(i!=0)
			{
				i+=item::itemdatasize*inventory::eqoff;
				b=toInt(s1["length(inventory)"]);
				b/=item::itemdatasize;

				for(a=inventory::eqoff;a<b;a++)
				{
					if((*((int*)i))>0)neq++;
					i+=item::itemdatasize;
				}
			}
//			logger.log("neq: %d\n", neq);
			*bs << neq;
			if((p!=0)&&(neq>0))
			{
				i=p;
				i+=item::itemdatasize*inventory::eqoff;
				for(a=inventory::eqoff;a<b;a++)
				{
					if((*((int*)i))>0)*bs << *(int*)i;
					i+=item::itemdatasize;
				}
			}

		}
		s1.freeup();
		*bs << nchars;
		for(b=0;b<nchars;b++)
		{
			if(accslots[b]!=-1)
			{
				*bs << (char)b << 0 << 0;
//				*bs << ids[b] << 0 << 0;
			}
		}
		raise();
	}

}

void charserver::tplayer::chardel()
{
	std::string str1;
	char t[1025];
	int a;
//	sr.getpstr(str1);
	sr.getpstr(t, 1024);
	str1=t;
	if(name!=str1)
	{
		errorstate=true;
		return;
	}
//	sr.getpstr(str);
	sr.get(a);
	sr.forward(a);
	sr.getpstr(t, 1024);
	str1=t;
	sr.get(a);
	sqlquery s1(&owner->connection, "characters");
	s1.del("id="+toString(a));
	s1.freeup();
	sendcharinfo();
}

void charserver::tplayer::dologin()
{
	std::string str1;
	char t[1025];
	sr.getpstr(t, 1024);	//name
	str1=t;
	if(name!=str1)
	{
		errorstate=1;
		return;
	}
//	sr.getpstr(puffer, 1024);	//charname
	if((!loggedin)&&(state>0))
	{
		*bs << 0xff05;
		raise();
	}
}

void charserver::tplayer::createchar()
{
	std::string str1;
	char t[1025];
	sr.getpstr(t, 1024);
	str1=t;
	std::string charname;
	int slot, a;
	if(name!=str1)
	{
		errorstate=true;
		return;
	}
	sr.getpstr(t, 1024);
	str1=t;
	if(password!=str1)
	{
		errorstate=true;
		return;
	}

	slot=(int)sr.get1char();

	sr.getpstr(t, 1024);
	charname=t;

	{
		int b=charname.length();
		for(a=0;a<b;a++)
		{
			if((t[a]==34)||(t[a]==37))
			{
				errorstate=true;
				return;
			}
		}
	}
	{
		sqlquery s2(&owner->connection, "accounts");
		s2.selectw("name='"+name+"'");
		if(s2.next())
		{
			a=toInt(s2["id"]);
		}else
		{
			errorstate=true;
			return;
		}
		s2.freeup();
		if(a!=accid)
		{
			errorstate=true;
			return;
		}
	}
	sqlquery s1(&owner->connection, "characters");
	s1.selectw("name = '"+charname+"'", "id");
	if(s1.next())
	{
		*bs << 0xfe << 0x0524;
		raise();
		return;
	}
	s1.freeup();

	s1.addupdate("level", "1");
	s1.addupdate("name", charname);
	s1.addupdate("str", "15");
	s1.addupdate("sta", "15");
	s1.addupdate("dex", "15");
	s1.addupdate("intl", "15");
	s1.addupdate("statpoints", "0");
//	s1.addupdate("modeltype", "5");
	s1.addupdate("money", "0");
	s1.addupdate("job", "0");
	s1.addupdate("mapid", "1");
	s1.addupdate("x", "6966");
	s1.addupdate("y", "100");
	s1.addupdate("z", "3333");
	s1.addupdate("hp", "100");
	s1.addupdate("mp", "10");
	s1.addupdate("fp", "10");
	s1.addupdate("exp1", "0");
	s1.addupdate("skillpoints", "0");
	s1.addupdate("flylevel", "1");
	s1.addupdate("flyexp", "0");
	s1.addupdate("pvp", "0");
	s1.addupdate("pk", "0");
	s1.addupdate("disp", "0");
	sr.forward(3);
	s1.addupdate("hair", toString((int)sr.get1char()));
	sr.get(a);
	s1.addupdate("haircolor", toString(a));
	int gender=(int)sr.get1char();
	s1.addupdate("gender", toString(gender));
	sr.forward(1);
	s1.addupdate("face", toString((int)sr.get1char()));

	s1.addupdate("accid", toString(accid));
	s1.addupdate("accslot", toString(slot));
	s1.addupdate("msgstate", "0");
	s1.addupdate("size", "100");
	s1.addupdate("party", "0");
	s1.addupdate("guild", "0");
	s1.addupdate("pxp1", "0");
	s1.addupdate("pxp2", "0");
	char *inv=new char[73*item::itemdatasize];
	for(a=0;a<73*item::itemdatasize;a++)inv[a]=0;
	item targy;
	targy.num=1;
	targy.setId(20);
	targy.save(&inv[item::itemdatasize*(8+inventory::eqoff)]);
	if(gender!=0)
	{
		targy.setId(0x01f8);
		targy.save(&inv[item::itemdatasize*(0+inventory::eqoff)]);
		targy.setId(0x01fc);
		targy.save(&inv[item::itemdatasize*(2+inventory::eqoff)]);
		targy.setId(0x0200);
		targy.save(&inv[item::itemdatasize*(3+inventory::eqoff)]);
	}else
	{
		targy.setId(0x01f6);
		targy.save(&inv[item::itemdatasize*(0+inventory::eqoff)]);
		targy.setId(0x01fa);
		targy.save(&inv[item::itemdatasize*(2+inventory::eqoff)]);
		targy.setId(0x01fe);
		targy.save(&inv[item::itemdatasize*(3+inventory::eqoff)]);
	}

	s1.addeupdate("inventory", inv, 73*item::itemdatasize);
	delete inv;
	char *skills=new char[character_buffable::maxskillid*9];
	for(a=0;a<character_buffable::maxskillid*9;a+=9)
	{
		*(int*)(&skills[a])=1;
		*(int*)(&skills[a+4])=0;
		skills[a+8]=1;
	}
	s1.addeupdate("skills", skills, character_buffable::maxskillid*9);
	delete[] skills;
	a=0;
	s1.addeupdate("quests", (char*)&a, 4);
	s1.addeupdate("finishedquests", (char*)&a, 4);
	char *friends=new char[::tplayer::maxfriends*8];
	for(a=0; a< ::tplayer::maxfriends*8; a+=8)
	{
		*(int*)(&friends[a])=-1;
		*(int*)(&friends[a+4])=-1;
	}
	s1.addeupdate("friendlist", friends, ::tplayer::maxfriends*8);
	delete[] friends;


	char *slots=new char[3*4*(5+24+4)];
	for(a=0;a<3*4*(5+24+4);a++)slots[a]=0;

	s1.addeupdate("slots", slots, 3*4*(5+24+4));
	delete[] slots;
	s1.insert();
	s1.freeup();
	sendcharinfo();
}

bool charserver::validate(const std::string n, const std::string p, int cid)
{
	bool retval=false;
	std::lock_guard<std::mutex> guard(this->charservermutex);

    std::list<tplayer*>::iterator i;
	for(i=mplayers.begin();i!=mplayers.end();++i)
	{
		if((*i)->name==n)
		{
			(*i)->cid=cid;
			retval=true;
			break;
		}
	}
	return retval;
}
