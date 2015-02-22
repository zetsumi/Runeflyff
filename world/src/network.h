#ifndef __network_h__
#define __network_h__

#include "pmutex.h"

//#define use_s

#ifndef WIN32
#ifdef _WIN32
#define WIN32
#endif
#endif

#ifdef __linux__

	#include <sys/socket.h>
	#include <netdb.h>
	#include <sys/fcntl.h>
	#define closesocket(a) close(a)
	#define Sleep(a) sleep(a)
#endif

#ifdef WIN32
    #include <winsock2.h>
	#ifndef scoklen_t
		#define socklen_t int
	#endif
	#ifndef WSAData
		#define WSAData WSADATA
	#endif
#endif

bool startserver(int *s, int port);
void closeserver(int serverSocket);
void closesck(int sck);


class s_sender
{
	long long all,curr;
	long long peak;
	long long starttime;
	pmutex mutex;
public:
	s_sender();
	int s_sender::send(SOCKET s, const char *buf, int len, int flags)
	{
		int r=::send(s, buf, len, flags);
		if(r>0)
		{
			ul m=mutex.lock();
			all+=r;
			curr+=r;
		}
	}
	long long getcurrent()
	{
		ul m=mutex.lock();
		long long r=curr;
		if(curr>peak)peak=curr;
		curr=0;
		return r;
	}
	void printstats();
};

class s_reciever
{
	long long all,curr,peak;
	long long starttime;
	pmutex mutex;
public:
	s_reciever();
	int recv(SOCKET s, char *buf, int len, int flags)
	{
		int r=::recv(s, buf, len, flags);
		if(r>0)
		{
			ul m=mutex.lock();
			all+=r;
			curr+=r;
		}
	}
	long long getcurrent()
	{
		ul m=mutex.lock();
		long long r=curr;
		if(curr>peak)peak=curr;
		curr=0;
		return r;
	}
	void printstats();
};


extern s_sender s_send;
extern s_reciever s_recv;


#endif
