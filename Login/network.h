#ifndef __network_h__
#define __network_h__

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


#endif
