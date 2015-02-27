#ifndef __guild_h__
#define __guild_h__

#include "pmutex.h"
#include <string>
#include <list>
#include <map>
#include "vector2.h"

#include "bank.h"

class tplayer;

class sqlquery;
class buffer;
class tguild
{
	friend class cluster;
	void updateglobalid();

	struct tguildmember
	{
		int dbid;
		int id;
		int contrib;
		int moneycontrib;
		int surrender;
		int rank;
		int rank2;
		int level,job;
		int lives;
		bool teleported_down;
		std::list<tguildmember*>::iterator siegelistremover;
		std::string nick, name;
		bool linedup;
		bool siege_line_toend;
		int point;
		tplayer *p;
		tguildmember():p(0),dbid(-1),rank(0),rank2(0),level(0),job(0),linedup(false),teleported_down(false){}
	};
	std::vector2<tguildmember> members;
	int nmembers;
	std::string name, gnotice;
	int exp, level,logo;
//	int rigths[5];
//	int pay[5];
	std::vector2<int> nranks;
	std::vector2<int> rigths;
	std::vector2<int> pay;
	int gwwin, gwlose,gwgiveup,bankmoney;
	int nloggedin;
	buffer *warpacket, *warendpacket;

	bool join(tplayer *p, int dbid, int plevel, int pjob, std::string &pname);

	int owner,guildid;
	static pmutex globalguildmutex;
	static int maxguildid;
	static int maxguildownerid;
	static int nguilds;

	pmutex guildmutex;

	void guildmulticast(buffer &bs);
	void guildmulticast2(buffer &bs2);
	void guildmulticast3(buffer &bs);
	void guildmulticast31(buffer &bs, tplayer *toskip);
	void guildmulticast4(buffer &bs);
	void save();
	void created(buffer *bs, tplayer *p, int id, std::string &pname);
	tguild *waring;
	int wid;	//warid
	warehouse wh;
	long long cwtime;
	bool cwing;
	void getwarpacket(buffer *bs);
	void getwarendpacket(buffer *bs);
	bool disbanded;
	int siegemoney;
	std::list<tguildmember*> siegeline;
	bool insiege, closetosiege;
	int siegepoint, siege_players_down, requested_players_down;
	cluster *siegecluster;
	void sendsiegedata(tplayer *p);
public:
	void sendsiegepacket(tplayer *p);
	tplayer* getmaster();
	void siege_createline_start();
	bool siege_createline_finish(cluster *cl);
	bool siege_createline_auto();
	void siege_teleportin();
	void siege_teleportdown();
	void player_teleportdown(tplayer *p, int pos);
	void player_teleportup(tplayer *p);
	void player_gw_login(tplayer *p);
	void siege_end(bool won);
	tguild *getwaring()
	{
		ul m=guildmutex.lock();
		return waring;
	}
	static void loadallguilds();
	static void saveallguilds();
	static void getallguildinfo(tplayer *p);
	static tguild* getguild(int id);
	int getowner();
	int getguildid();
	void changelead(tplayer *p, int id1, int id2);
	void createcloack(tplayer *p);
	void showwh(tplayer *p);
	int towh(tplayer *p, item &t);
	void fromwh(tplayer *p, int a, int b);
	void contribution(tplayer *contributor, int dbid, int questcontrib, int moneycontrib);
	int moneyfromguild(tplayer *p, int a);

	tguild(tplayer *master, int dbid, int plevel, int pjob, std::string &pname, int id);
	tguild(sqlquery &s4);
	void deleteguild(tplayer *pl);

	void sendlogin(tplayer *p);
	void setnotice(std::string &n);
	void setname(std::string &n, tplayer *p);
	void setlogo(int a);
	void setpay(int a,int b);
	void setrigths(int a, int b, int c, int d, int e);
	void promote(tplayer *p, int dbid, int rank);
	void classup(tplayer *p, int dbid, int rank2);
	bool login(tplayer *p, int guildslot, int dbid, int id);
	void logout(tplayer *p, int guildslot, int dbid);
	void guildchat(tplayer *p, int guildslot, int dbid, const std::string &name, const char *text);
	int leave(tplayer *r, int dbid);
	bool invited(tplayer *p, int dbid, int plevel, int pjob, std::string &pname, int id);
	bool war(tplayer *p, const std::string &gname);
	void acceptwar(tplayer *p, int gid);
	void giveupwar(tplayer *p, int dbid);
	void leaderkilled();
	void endwar(bool lost, bool won);
	void piece(int dbid);
	bool addtolineup(tplayer *p, const std::string &n);
	int getwaringid();
	void getwarstatus(tplayer *p);
	bool contains(tplayer *p);
	void setnick(tplayer *p, int id, const std::string &nick);
	bool applysiege(int money);
	int getsiegemoney();
/*
	void setcw(time_t t);
	void cwstart();
	void cwend();
	bool iscwing()
	{
		return cwing;
	}
	void cwteleportout(cluster *cl);
*/
};

extern std::map<int, tguild*> guilds;
extern std::map<int, tguild*> owguilds;
extern pmutex guildtreemutex;


#endif
