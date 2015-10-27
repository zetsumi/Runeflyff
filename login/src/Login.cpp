
#include "stdafx.h"
#include <cstdlib>
#include <iostream>
#include <fstream>
#include "mysql.h"
#include "network.h"
#include "buffer.h"
#include "reciever.h"
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <list>
#include <conio.h>

#include "PacketLogger.h"

PacketLogger packetLoggerText("login_packetLog.txt", PacketLogger::TEXT);
PacketLogger packetLoggerBin( "login_packetLog.bin", PacketLogger::BINARY);

int tv=20000;	//20 sec

using namespace std;
#define ID_DCUSER 0xf03103ff
#define ID_CS	  0xf03103fe

//#define ConnectToLogin

bool online=false;

#ifdef __linux__

    #define TIMEVAL timeval
    #define SOCKADDRNEV struct sockaddr*
    #define ADDRLENNEV socklen_t*

    int closesocket(int s)
    {
        int a=a=close(s);
        if(a<0){
    	perror("close()");
        }
        return a;
    }

#else

    #define TIMEVAL timeval
    #define SOCKADDRNEV LPSOCKADDR
    #define ADDRLENNEV int*
    #define sleep(a) Sleep(a)

#endif

char serverip[257];
char mysqlhost[257];
char mysqluser[257];
char mysqlpasswd[257];
char mysqldb[257];
int mysqlport=0;
MYSQL *connection=0;
MYSQL *connection3=0;
char dateval[257];

//pmutex reconnectmutex;
void reconnectsql(MYSQL **con)
{
//	ul m=reconnectmutex.lock();
	mysql_close(*con);
	*con=mysql_init(0);
	if(!mysql_real_connect(*con, mysqlhost, mysqluser, mysqlpasswd, mysqldb, mysqlport, (char*)0, 0))
		logger.log("Mysql reconnect error: %s\n", mysql_error(*con));
	else
		printf("Reconnecting to MySql!\n");
}


class tuser
{
    sendablebuffer *bs1;
    char sendpuffer[buffer::nsize];
    bool chars;
	int *bslen;
public:
    int s1;
    int sck;
    std::string ip,name, passwd;
	long long timeout;
    buffer *bs;
    reciever sr;
    bool errorstate;
    tuser()
    {
		timeout=0;
        errorstate=false;
        bs=0;
        bs1=0;
        chars=false;
    }
	void init(int s)
	{
		sck=s;
		bs=new buffer();
		*bs << (char)0x5e;
		bslen=bs->insert((int)0);

		*bs << 0 << (int)time(0);
		bs->inc();
        sr.init(sck);
	}
    ~tuser()
    {
#ifdef ConnectToLogin
		if(chars)
		{
			printf("server is offline\n");
			online=false;
		}
#endif
        delete bs;
        delete bs1;
    }
    void send()
    {
        if(bs1==0)
        {
            if(bs->getcommandnumber()>0)
            {
				*bslen=bs->length()-5;
                bs1=new sendablebuffer(*bs);
                delete bs;
                bs=new buffer();
				*bs << (char)0x5e;
				bslen=bs->insert((int)0);
	            bs1->initflush(sck);
            }
        }
        if(bs1!=0)
        {
    		if(bs1->continueflush(&this->sendpuffer[0])==1)
    		{
    			delete bs1;
    			bs1=0;
    		}
        }
    }
    void process()
    {
        int a;
		try
		{
			s1=sr.getlast4();
			sr.forward(4);
			sr.get(a);
			{
				switch(a)
				{
				case ID_CS:
#ifdef ConnectToLogin
					printf("server is online\n");
					online=true;
					chars=true;
#endif
	                break;
   				case ID_DCUSER:
	                break;
//				case 0x18:
//	                break;
				case 0xfc:
					loginprocess();
				break;
				}
	        }
		}catch(error &e)
		{
			printf("Error: %s", e.what());
			errorstate=true;
		}catch(std::exception &e)
		{
			printf("Exception %s", e.what());
			errorstate=true;
		}

        sr.clear();
    }
    void loginprocess()
    {
		char puffer[128];

#ifdef ConnectToLogin
		if(!online)
		{
			*bs << 0xfe << 0x6d;
			bs->inc();
			return;
	    }
#endif

		int a,b;
		unsigned long long t=(unsigned long long)time(0);
		t-=120;								//120 seconds
		//sr.get(a);
		//sr.forward(a);
		sr.getpstr(puffer, 128);
		string dateval=puffer;
		if(dateval != "20090727")
		{
			*bs << 0xfe << 0x6B;
			bs->inc();
			printf("Player with old version of client(0x6B)!\n");
			return;
		}

		sr.getpstr(puffer, 128);
		name=puffer;
		sr.getpstr(puffer, 128);
		{
			sqlquery s1(&connection, "accounts");
			s1.selectw("name='"+name+"'", "passwd");
			if(s1.next())
			{
				passwd=s1[0];
			}else
			{
				s1.freeup();
//				errorstate=true;
				*bs << 0xfe << 0x79;
				bs->inc();

				return;
			}
			s1.freeup();
		}

		if(passwd!=std::string(puffer))
		{
			*bs << 0xfe << 0x78;
			bs->inc();
			return;
		}

		int nserversnc=0;
		int nservers=0;

		{
			sqlquery s1(&connection, "clusters");
			s1.selectw("up>"+toString(t) + " OR cid=0", "count(*)");
			if(s1.next())
			{
				nservers=toInt(s1[0]);
			}
			s1.freeup();

			s1.selectw("cid=0", "count(*)");
			if(s1.next())
			{
				nserversnc=toInt(s1[0]);
			}
			s1.freeup();
		}

		if(nservers-nserversnc<=0)
		{
			*bs << 0xfe << 0x6d;
			bs->inc();
			return;
		}

        *bs << 0xfd << 0x5d7f1f5c << (char)1;
//        bs->sndpstr(username);
        *bs << nservers;

		sqlquery s1(&connection, "clusters");
		sqlquery s2(&connection3, "characters");
		s1.selectw("up>"+toString(t) + " OR cid=0");
		while(s1.next())
		{
			int a=toInt(s1["cid"]);
			if(a==0)
			{
				*bs << -1 << toInt(s1["sid"]);
				bs->sndpstr(s1["name"].c_str());
				bs->sndpstr(s1["ip"].c_str());
                *bs << 0 << 0 << 1 << 0;
			}else
			{
				b=0;
				int n=toInt(s1["maxplayers"]);
				if(n>0)
				{
//					s2.selectw("clusterid = " + toString(a-1) + " AND loggedin = 1", "count(*)");
//					if(s2.next())
//					{
//						b=toInt(s2[0]);
//					}
//					s2.freeup();
				}else n=41;
				logger.log("CurrentPlayers=%d MaxPlayer=%d\n", b, n);
                *bs << toInt(s1["sid"]) << a;
                bs->sndpstr(s1["name"].c_str());
			    *bs << 0 << 1 << b << 1 << n;
			}

		}
		s1.freeup();
		bs->inc();
		timeout=GetTickCount()+tv;

    }
};

std::list<tuser> users;

int loadsqlserverip(const std::string& filename)
{
    ifstream f1;

	char puffer[257];
	int a;
	f1.open(filename.c_str());
	if(f1.fail()){
		printf("error opening %s!\n", filename.c_str());
		exit(1);
	}
	else
		printf("INFO: Loaded server.txt!\n");
    f1.getline(mysqlhost,256);
    f1.getline(mysqluser,256);
    f1.getline(mysqlpasswd,256);
    f1.getline(mysqldb,256);
    f1.getline(puffer,256);
    sscanf(puffer, "%d", &mysqlport);
    f1.getline(puffer,256);
    sscanf(puffer, "%d", &a);
	f1.getline(dateval,256);
    f1.close();
    return a;

}

int main(int argc, char *argv[])
{
	int serverid=loadsqlserverip("server.txt");
#ifndef __linux__
	WSAData wsaData;
#endif
	TIMEVAL TimeOut;
	struct sockaddr_in server;
	int port=23000;
	fd_set readfds;

	struct sockaddr_in addr;
	int addrlen=sizeof(addr);

	TimeOut.tv_sec = 1;
	TimeOut.tv_usec = 0;
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = INADDR_ANY;

#ifndef __linux__
	if (WSAStartup(MAKEWORD(2,2), &wsaData)<0) {
		printf("Winsock hiba\n");
		exit(1);
	}
#endif
	connection=mysql_init(0);
	if(connection==0)
    {
		printf("mysql_init fail\n");
		exit(1);
	}
	connection3=mysql_init(0);
	if(connection3==0)
    {
		printf("mysql_init fail\n");
		exit(1);
	}

	if(!mysql_real_connect(connection, mysqlhost, mysqluser, mysqlpasswd, mysqldb, mysqlport, (char*)0, 0))
	    printf("error connecting to mysql database! %s\n", mysql_error(connection));
	else
		printf("INFO: Connected to Mysql(connection)!\n");
	if(!mysql_real_connect(connection3, mysqlhost, mysqluser, mysqlpasswd, mysqldb, mysqlport, (char*)0, 0))
	    printf("error connecting to mysql database! %s\n", mysql_error(connection3));
		else
		printf("INFO: Connected to Mysql(connection3)!\n");


    int starttime=time(0);
    int serverSocket;
    int a,c;
    if(startserver(&serverSocket, port))
    {
        while(!kbhit())
        {
            FD_ZERO(&readfds);
            c=serverSocket;
            FD_SET(serverSocket, &readfds);
            for(std::list<tuser>::iterator i=users.begin();i!=users.end();++i)
            {
                FD_SET((*i).sck, &readfds);
                if((*i).sck>c)c=(*i).sck;
            }
            select(c, &readfds, 0, 0, &TimeOut);
            if(FD_ISSET(serverSocket, &readfds))
            {
                users.push_back(tuser());
                users.back().init(accept(serverSocket, (struct sockaddr*) &addr, (socklen_t*) &addrlen));
                users.back().ip=inet_ntoa(addr.sin_addr);
            }
            for(std::list<tuser>::iterator i=users.begin();i!=users.end();++i)
            {
                i->send();
				a=0;
                if(FD_ISSET((*i).sck, &readfds))
                {
        			a=(*i).sr.recieve();
        			if(a==3)(*i).process();
                }
       			if((a==1)||(a==2)||(i->errorstate))
       			{
       				closesocket((*i).sck);
       				i=users.erase(i);
       				if(i==users.end())break;
      				else continue;
                }
				if(i->timeout>0)
				{
					if(GetTickCount()>i->timeout)
					{
						i->errorstate=true;
						i->timeout=0;
						logger.log("Connection time out! Player disconnected!\n");
					}
				}
            }
        }
    }

    return EXIT_SUCCESS;
}
