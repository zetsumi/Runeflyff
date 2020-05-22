#ifndef __party_h__
#define __party_h__

#include <string>
#include "vector2.h"
#include <list>
#include <queue>
#include "buffer.h"
#include "listremover.h"
#include "platform_threading.h"

class tplayer;
class cluster;
class character_base;
class tparty
{
	friend class cluster;
	friend class tplayer;
	struct playerdata
	{
		tplayer *pl;
		int dbid;
		int job,level;
		std::string name;
		playerdata():pl(0),dbid(-1),job(0),level(0){}
		void set(tplayer *p);
		void clear()
		{
			dbid=-1;
			pl=0;
			job=0;
			level=0;
			name="";
		}
	};
	bool advanced;
	std::string name;
	int level, points, exp, expmode, itemmode, seq,nmembers,ticket;
	static const int g_maxpartymembers=10;
	int maxpartymembers;
	std::vector2<playerdata> memberdata;
	std::vector2<playerdata*> members;
	void loginpuffer(playerdata *p);
	void sendpartydata();
	void sendpartyenddata();
	void partymulticast(buffer &bs);
	long long fctime,gbtime,linktime,stime;
	std::mutex partymutex;
	static std::mutex partycreatemutex;
	bool leave2(playerdata *pd, int a2, tplayer *p);
	long long alink, asc;
//	std::list<tparty*>::iterator alinkr, ascr;
	listremover<tparty*> alinkr, ascr;
	cluster *cl;
	int linklevel;
	character_base *leaderfocus;
	bool used;

	friend class std::vector2<tparty*>;
	tparty();

	bool create1(tplayer *p, tplayer *leader, int t);
	static std::vector2<tparty*> parties;
	static std::list<int> freeparties;
	void getpplaround_l(tplayer *p, std::list<tplayer*> &l);
	void getpplaround_l(tplayer *p, std::vector<tplayer*> &l);
public:
	~tparty();
	static void initparties(int n);
	static tparty* getparty(int i);
	static tparty* create(tplayer *p, tplayer *leader);
	bool testlink(long long ido);	//only cluster calls this
	bool testasc(long long ido);	//only cluster calls this
	bool join(tplayer *p);
	void leave(int id, tplayer *p);
	void addexp(int Aexp, int Alevel=-1, int Apoint=-1);
	bool levelto10(tplayer *p);
	void setexp(int id, int mode);
	void setitem(int id, int mode);
	void setname(int id, const std::string &name);
	void memberassign(int id, int nid, tplayer *p);
	void partychat(tplayer *p, const char *nev, int id, const char *msg);
    void managePexp(tplayer* tamado, double expval, int elevel);
	tplayer *getseq(tplayer *p);
	void gotmoney(tplayer *p, int m);
	int getticket()
	{
		return ticket;
	}

	tparty* login(tplayer *p);
	void logout(tplayer *p);

//	bool isleader(tplayer *p);
	void useskill(tplayer *p, int skill);
	void set_gwparty()
	{
	    std::lock_guard<std::mutex> partyguard(this->partymutex);
		maxpartymembers=10;
	}
	long long getfc()
	{
        std::lock_guard<std::mutex> partyguard(this->partymutex);
		return fctime;
	}
	long long getgb()
	{
        std::lock_guard<std::mutex> partyguard(this->partymutex);
		return gbtime;
	}
	long long gets()
	{
        std::lock_guard<std::mutex> partyguard(this->partymutex);
		return stime;
	}
	void setplayerfocus(tplayer *p, character_base *f);
	void getpplaround(tplayer *p, std::list<tplayer*> &l);
	void getpplaround(tplayer *p, std::vector<tplayer*> &l);
	void mmp(tplayer *o, float x, float z);
};





#endif
