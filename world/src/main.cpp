#include "stdafx.h"
#include <cstdlib>
#include <iostream>
#include <conio.h>
#include <fstream>
#include <winsock2.h>
#include <windows.h>
#include <list>

#include "cluster.h"
#include "mysql.h"
#include "logger.h"
#include "network.h"
#include "stringf.h"
#include "reciever.h"
#include "buffer.h"
#include "adat.h"
#include "charserver.h"
#include "pmutex.h"

#include "quest.h"

char serverip[256]="localhost";
char mysqlhost[256]="localhost";
char mysqluser[256]="root";
char mysqlpasswd[256]="qwert";
char mysqldb[256]="flyff3";
int mysqlport=0;
std::vector2<mcon*> connections;

charserver *cserver=0;
std::map<int, tplayer*> globaldbidplayers;
std::map<std::string, tplayer*> globalnameplayers;
pmutex globalplayersmutex;

sqlquery dbaccounts;
sqlquery dbcharacters;
sqlquery dbfriendlist;
sqlquery dbitems;
sqlquery dbmails;
sqlquery dbslots;
sqlquery dbskilllist2;
sqlquery dbmonsterlist;
sqlquery dbguilds;
sqlquery dbguilds_g;
sqlquery dbcharacters_g;
sqlquery dbguildsiege;
pmutex dbguildsiege_mutex;
clustercont *clusters=0;
unsigned long ulong1=1;

void breakpoint()
{

	logger.log("");
}

int getCluster(reciever &sr)
{
	unsigned int a;
	int b,clid=-1;
	int dbid;

	sr.seek(4);
	a=0;
	sr.get(a);
	try{
		if(a==0xffffffff){
			a=0;
			sr.get(a);
#ifdef _DEBUG
#ifdef _LOGALOT
			logger.log("%x\n", a);
			sr.dump();
#endif
#endif
			if(a==0xff00)
			{
				sr.get(b);
				sr.get(b); //x
				sr.get(b); //y
				sr.get(b); //z
				sr.get(b);
				dbid=b;
				sr.get(b);
				sr.get(b);
				sr.get(b);
				sr.get(b);

				sr.get(clid);
#ifdef _DEBUG
				logger.log("Cluster=%d dbid=%d\n", clid, dbid);
#endif
				clid--;
			}
		}else
		{
#ifdef _DEBUG
#ifdef _LOGALOT
			logger.log("Command is not -1: %08x\n", a);
#endif
#endif
		}
	}catch(std::exception &e)
	{
		logger.elog("%s\n", e.what());
		clid=-1;
	}
	return clid;
}

struct miniplayer
{
	int sck;
	reciever sr;
	std::string cip;
	sendablebuffer bs;
	bool sendfinished;
	miniplayer(int sck1, char *ip):sck(sck1), cip(ip), sendfinished(false)
	{
		sr.init(sck);
		bs << (char)0x5e << 0x08 << 0 << (int)GetTickCount();
		bs.initflush(sck);
#ifdef _DEBUG
#ifdef _LOGALOT
		bs.dump();
#endif
#endif
	};
	char sendpuffer[buffer::nsize];
	void send()
	{
		if(!sendfinished)
		{
			sendfinished|=(bs.continueflush(&sendpuffer[0])==1);
#ifdef _DEBUG
#ifdef _LOGALOT
			if(sendfinished)logger.log("sent\n");
#endif
#endif
		}
	}
};

std::list<miniplayer*> mplayers;


fd_set mreadfds;
int hc=1;
void doselect(int serverSocket)
{
	int newsocket,d,a;
	struct sockaddr_in addr;
	int addrlen=sizeof(addr);

	TIMEVAL TimeOut;
	TimeOut.tv_sec = 0;
	TimeOut.tv_usec = 50000;
	miniplayer* newconnection;
	char *clientip;
	std::list<miniplayer*>::iterator i;
	fd_set readfds=mreadfds;
	bool recalc=false;
	select(hc, &readfds, 0, 0, &TimeOut);

	if(FD_ISSET(serverSocket,&readfds))
	{
		newsocket=accept(serverSocket, (struct sockaddr*) &addr, (socklen_t*) &addrlen);
#ifdef WIN32
        ioctlsocket(newsocket, FIONBIO, (unsigned long*)&ulong1);
#else
        ioctl(newsocket, O_NONBLOCK, 0);
#endif
		clientip = inet_ntoa(addr.sin_addr);
//#ifdef _DEBUG
        if(strcmp(clientip, "127.0.0.1")!=0)logger.log("Client %s:%d connected\n", clientip, newsocket);
//#endif
        newconnection = new miniplayer(newsocket, clientip);
		mplayers.push_back(newconnection);
		FD_SET((unsigned int)newsocket, &mreadfds);
	}
	for(i=mplayers.begin();i!=mplayers.end();++i)
	{
		(*i)->send();

		if(FD_ISSET((*i)->sck,&readfds))
		{ 
			a=(*i)->sr.recieve();
			if(a==3)
			{
				d=getCluster((*i)->sr);
				if((d>=0)&&(d<(int)clusters->size()))(*clusters)[d]->handower((*i)->sck, (*i)->sr, (*i)->cip);//handower
				delete *i;
				i=mplayers.erase(i);
				recalc=true;
				if(i==mplayers.end())break;
				else continue;
			}else if((a==1)||(a==2))
			{
#ifdef _DEBUG
				logger.log("recv dc\n");
#endif
				closesocket((*i)->sck);
				delete *i;
				i=mplayers.erase(i);
				recalc=true;
				if(i==mplayers.end())break;
				else continue;
			}
		}
	}
	if(recalc)
	{
		FD_ZERO(&mreadfds);
		FD_SET((unsigned int)serverSocket, &mreadfds);
		std::list<std::list<miniplayer*>::iterator> toerase;
		hc=serverSocket;
		for(i=mplayers.begin();i!=mplayers.end();++i)
		{
			if((*i)->sck>hc)hc=(*i)->sck;
			FD_SET((unsigned int)(*i)->sck, &mreadfds);
		}
		hc++;
	}
}
/*
pmutex reconnectmutex;
void reconnectsql(MYSQL **con)
{
	ul m=reconnectmutex.lock();
	for(int a=0;a<128;a++)
	{
		mysql_close(*con);
		*con=mysql_init(0);
		if(!mysql_real_connect(*con, mysqlhost, mysqluser, mysqlpasswd, mysqldb, mysqlport, (char*)0, 0))
		{
			if(a==127)throw std::range_error("Couldnt reconnect to mysql!");
			else 
			{
				logger.log("reconnect try %d\n", a);
				Sleep(1);
			}
		}
			
	}
}
*/
int servernumber=0;

int smain()
try{
	int a;
	char ch;
	int serversocket;	

	try
	{
		initprochandlers();
		void init11();
		init11();
		initsetbonuses();
		//mysql_library_init(-1, 0, 0);
		mysql_init(0);
		if(mysql_thread_safe()!=1)throw std::runtime_error("Mysql server is not threadsafe!");
		if(CHAR_BIT!=8)throw std::runtime_error("Only 8bit char platforms supported!");
		if(sizeof(short)!=2)throw std::runtime_error("Short is not 2 bytes!");
		if(sizeof(int)!=4)throw std::runtime_error("Int is not 4 bytes!");
		if(sizeof(long long)!=8)throw std::runtime_error("Long long is not 8 bytes!");
		if(sizeof(float)!=4)throw std::runtime_error("Float is not 4 bytes!");
		logger.log("Rune 0.75\n");
#ifdef WIN32
		logger.log("Windows");
#endif
#ifdef linux
		logger.log("Linux");
#endif

#ifdef _CLR
		logger.log(" CLR");
#endif

#ifdef _DEBUG
		logger.log(" Debug");
#else
		logger.log(" Relase");
#endif
		logger.log(" build\n");
		srand((unsigned)time(0));
		std::ifstream fl1;
		fl1.open("server.txt");
		fl1.getline(&mysqlhost[0], 256);
		fl1.getline(&mysqluser[0], 256);
		fl1.getline(&mysqlpasswd[0], 256);
		fl1.getline(&mysqldb[0], 256);
		fl1 >> mysqlport >> servernumber;
		fl1.close();
		mysqlport = 3306;

		logger.log("mysql connections\n");
		connections.resize(2);
		for(a=0;a<(int)connections.size();a++)
		{
			connections[a]=new mcon();
			connections[a]->init(mysqlhost, mysqluser, mysqlpasswd, mysqldb);
		}

		dbaccounts.init(connections[0], "accounts");
		dbcharacters.init(connections[0], "characters");
		dbmails.init(connections[0], "mails");
		dbskilllist2.init(connections[0], "skilllist2");
		dbmonsterlist.init(connections[0], "monsterlist");
		dbguilds.init(connections[0], "guilds");

		dbguilds_g.init(connections[0], "guilds");
		dbcharacters_g.init(connections[0], "characters");
		dbguildsiege.init(connections[0], "guildsiege");

		dbmails.del("createdate < " + toString(((int)time(0))-15*24*60*60));

		tquest::initquests();
		datainit();
		inititemfunkcs();
		initplayerdbvals(dbcharacters);

		dbcharacters.addupdate("loggedin", "0");
		dbcharacters.addupdate("party", "-1");
		dbcharacters.update("id>=0");
		sqlquery s1(connections[0], "guilds");
		dbcharacters.query("select g.id from guilds g, characters c where g.ownerid=c.id and c.guild!=g.id;");
		while(dbcharacters.next())
		{
			s1.del("id="+dbcharacters[0]);
		}
		dbcharacters.freeup();
		
		logger.log("Quests\n");

	}
	catch(error &e)
	{
		logger.elog("%s", e.what());
		_getch();
		return 1;
	}
	catch(std::exception &e)
	{
		logger.elog("%s", e.what());
		_getch();
		return 1;
	}


	tguild::loadallguilds();
	int nclusters=0;
	{
		sqlquery s1(connections.at(0), "clusters");
		s1.selectw(std::string("sid=")+toString(servernumber), "count(*)");
		nclusters=0;
		if(s1.next())nclusters=toInt(s1[0])-1;
		s1.freeup();
		clusters=0;
	}
	if(nclusters<=0)logger.elog("No clusters!");
	
	else if(startserver(&serversocket, 15400))
	{
		tparty::initparties(nclusters*cluster::nmaxplayers/2);
		{
			sqlquery s1(connections.at(0), "clusters");
			s1.selectw("sid="+toString(servernumber)+" AND cid=0");
			if(s1.next())
			{
				sprintf_s(serverip, 256, "%s", s1["ip"].c_str());
			}else
			{
				logger.log("Error: Couldn't load serverip from database!(set to localhost)");
			}
			s1.freeup();
		}
		cserver=new charserver(28000, serverip);
		logger.log("Worldserver started on %d\n", 15400);
		logger.log("Starting clusters\n");
		{
			clusters=new clustercont(connections.at(0), nclusters, servernumber);
		}
		FD_ZERO(&mreadfds);
		FD_SET((unsigned int)serversocket, &mreadfds);
		logger.log("Ready\n");
		for(;;)
		{
			try
			{
				for(;;)
				{
					if(_kbhit())
					{
						ch=_getch();
						if(ch==27)goto mainloop_end;
					}
					doselect(serversocket);
					Sleep(1);
				}
			}
			catch(error &e){logger.elog("%s", e.what());}
			catch(std::exception &e){logger.elog("%s", e.what());}
#ifndef _DEBUG
			catch(...){logger.elog("[Main]exception...\n");}
#endif
		}
mainloop_end:

		closeserver(serversocket);
		cserver->endprg=true;
	}

	
	void printft();
	printft();

	tguild::saveallguilds();

	logger.log("Closeing clusters\n");
	pthread_t **cj=0;
	if(clusters!=0)
	{
		cj=new pthread_t*[nclusters];
		for(a=0;a<(int)clusters->size();a++)
		{
			cj[a]=(*clusters)[a]->cthread;
			(*clusters)[a]->endprg=true;
		}
		logger.log("Clusters closed\n");

	}

	logger.log("Closeing mysql\n");
	dbaccounts.freeup();
	dbcharacters.freeup();
	dbmails.freeup();
	dbskilllist2.freeup();
	dbmonsterlist.freeup();
	dbguilds.freeup();

	dbguilds_g.freeup();
	dbcharacters_g.freeup();

	dbguildsiege.freeup();

	if(clusters!=0)
	{
		logger.log("Waiting for clusterthreads to finish\n");
		for(a=0;a<nclusters;a++)pthread_join(*cj[a],NULL);
		logger.log("threads finished\n");
		delete[] cj;
		if(clusters!=0)delete clusters;
		clusters=0;
	}
	delete cserver;

	//mysql_library_end();


	logger.log("Quit...\n");
	return 0;

}catch(std::bad_alloc&)
{
	fprintf(stderr, "Out of memory!\n");
	exit(1);
}
