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

//	int window_size = 1024;
//	setsockopt(serverSocket, SOL_SOCKET, SO_RCVBUF,(char*)&window_size, sizeof(window_size));

	if (bind(serverSocket, (sockaddr*)&server, sizeof(server))<0) {
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
