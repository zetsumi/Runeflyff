#ifndef __charserver_h__
#define __charserver_h__

#include "mysql.h"
#include <list>
#include <string>
#include "buffer.h"
#include "reciever.h"
#include "m_queue.h"
#include "platform_threading.h"

class charserver
{
	int hc,lsck;
	long long ulong1;

	int sck;
	void senddcmsg(std::string &name);

	charserver(const charserver& a){};
	mcon connection;
    std::thread cthread, athread;
	void cmain();
	void amain();
	void dcall();
	struct tplayer
	{
		int sck;
		std::string ip, name, password;
		buffer *bs;
		reciever sr;
		tplayer(charserver *o);
		~tplayer();
		void send();
		bool process();
		charserver *owner;
		bool errorstate;
		int cid;
		long long ido;
		bool loggedin;
	private:
		char sendpuffer[buffer::nsize];
		std::list<buffer*> bslist;
		void raise();
		void sendcharinfo();
		void chardel();
		void dologin();
		void createchar();
		void f6funkc();
		int *bslen, s1, state, accid;
		bool auth;
		sendablebuffer *insend;
	};
	std::list<tplayer*> mplayers;
	m_queue<std::pair<int, std::string> > accepted;
	int serversocket;
	void doselect();
	void getchars(int client, char *puffer, int n);
	int connecttologinserver(char* loginserverName, int loginserverPort);
	bool startaccept;
	TIMEVAL TimeOut;
	fd_set readfds;
	std::string serverip;
	std::mutex charservermutex;
public:
	bool endprg;
	m_queue<std::string> to_dc;
	bool validate(const std::string n, const std::string p, int cid);	//BYVAL!
	charserver(int a, std::string sip);
	~charserver();
	long long ido;

};

#endif
