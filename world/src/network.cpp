#include "stdafx.h"
#include "network.h"
#include "logger.h"
#include <winsock2.h>


bool startserver(int *s, int port)
{
    int serverSocket;
	struct sockaddr_in server;
#ifdef WIN32
	WSAData wsaData;
	if (WSAStartup(2*256+2, &wsaData)<0) {
		logger.elog("Winsock hiba\n");
		return false;
	}
#endif

	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = INADDR_ANY;

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (bind(serverSocket, (sockaddr*)&server, sizeof(server))<0) 
	{
		logger.elog("Bind error\n");
		perror("bind()");
		closesocket(serverSocket);
		return false;
	}

	if (listen(serverSocket, 5)<0) {
	    logger.elog("Listen error\n");
		perror("listen()");
		closesocket(serverSocket);
		return false;
	}

    *s=serverSocket;
    return true;
}

void closeserver(int serverSocket)
{
	shutdown(serverSocket,2);
	closesocket(serverSocket);
#ifdef WIN32
	WSACleanup();
#endif
}

void closesck(int sck)
{
	closesocket(sck);
}

s_sender::s_sender()
:all(0),curr(0),peak(0)
{
	starttime=GetTickCount();
}


s_reciever::s_reciever()
:all(0),curr(0),peak(0)
{
	starttime=GetTickCount();
}

void s_sender::printstats()
{
	long long t=GetTickCount();
	long long p,a;
	{
		ul m=mutex.lock();
		p=peak;
		a=all;
	}
	p/=1024;
	a/=1024*1024;
	logger.log("All: %dMB\n Peak: %dKB\n", (int)a, (int)p);
}

void s_reciever::printstats()
{
	long long t=GetTickCount();
	long long p,a;
	{
		ul m=mutex.lock();
		p=peak;
		a=all;
	}
	p/=1024;
	a/=1024*1024;
	logger.log("All: %dMB\n Peak: %dKB\n", (int)a, (int)p);
}

s_sender s_send;
s_reciever s_recv;
