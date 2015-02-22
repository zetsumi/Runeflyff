#ifndef __quest_h__
#define __quest_h__

#include "pmutex.h"
#include <string>
#include <map>
#include "vector2.h"
#include <list>

#define QBF_GUILDLESS 1
#define QBF_NO_ACROBAT 2
#define QBF_CWQUEST 4
#define QEF_CREATEGUILD 1
#define QEF_CWQUEST 4

class tquest
{
public:
	static int joblookup[16];
	static pmutex questsmutex;
	static std::map<int, tquest*> quests;

	struct qitem
	{
		int id;
		int num;
		qitem(int a,int b):id(a),num(b){};
	};
	struct tquestitems
	{
		int monsterid;
		int dropid;
		int dropchance;
		int number;
		tquestitems(int a, int b, int c, int n)
			:monsterid(a), dropid(b), dropchance(c), number(n){}
	};
	std::map<int, tquestitems> questitems;
	int dialog_accept, dialog_deny, dialog_finished, dialog_askbefore;
	int questid;
	bool repeatable;
	int beginflags,endflags;
	std::string title;
	std::string startcharacter, endcharacter;
	int startlevel, endlevel;	//beginlevelmin, beginlevelmax
	int beginjob;
	int endstate,beginstate;
	int beginaddgold;
	int karmamin, karmamax, endkarmamin, endkarmamax;
	int rewardmoneymin, rewardmoneymax;
	int disquise;
	int rewardjob;
	long long rewardexpmin, rewardexpmax;
	std::vector2<qitem> enditems, rewarditems, beginitems, toremoveitems,beginadditems;
	std::vector2<int> prevquests;
	std::vector2<std::pair<int, int> > killmobs;
	std::map<int, std::string> dialogs;

	tquest(int i)
	{
		questid=i;
		beginjob=-1;
		startlevel=0;
		endlevel=0;
		beginjob=0;
		endstate=0;
		rewardmoneymin=0;
		rewardmoneymax=0;
		rewardexpmin=0;
		rewardexpmax=0;
		karmamin=-2000000000;
		karmamax=2000000000;
		endkarmamin=-2000000000;
		endkarmamax=2000000000;
		beginaddgold=0;
		disquise=0;
		rewardjob=-1;
	}

	static void initquests();
};

#endif
