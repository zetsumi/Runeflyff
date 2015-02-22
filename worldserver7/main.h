#ifndef __main_h__
#define __main_h__

#include "mysql.h"
#include "pmutex.h"
#include "vector2.h"

extern std::vector2<mcon*> connections;
extern sqlquery dbaccounts;
extern sqlquery dbcharacters;
extern sqlquery dbfriendlist;
extern sqlquery dbitems;
extern sqlquery dbmails;
extern sqlquery dbslots;
extern sqlquery dbskilllist2;
extern sqlquery dbmonsterlist;
extern sqlquery dbguilds;
extern sqlquery dbguilds_g;
extern sqlquery dbcharacters_g;


extern char mysqlhost[256];
extern char mysqluser[256];
extern char mysqlpasswd[256];
extern char mysqldb[256];
extern int mysqlport;

class cluster;
class clustercont;
extern clustercont *clusters;

class charserver;
extern charserver *cserver;
class tplayer;
extern std::map<int, tplayer*> globaldbidplayers;
extern std::map<std::string, tplayer*> globalnameplayers;
extern pmutex globalplayersmutex;
extern int servernumber;


#endif
