#include "stdafx.h"
#include "quest.h"
#include "mysql.h"
#include "main.h"
#include "logger.h"

#define JOB_VAGRANT 1
#define JOB_MERCENARY 2
#define JOB_ACROBAT 4
#define JOB_ASSIST 8
#define JOB_MAGICIAN 16
#define JOB_KNIGHT 32
#define JOB_BLADE 64
#define JOB_BILLPOSTER 128
#define JOB_RINGMASTER 256
#define JOB_PSYCHIKEEPER 512
#define JOB_ELEMENTOR 1024
#define JOB_JESTER 2048
#define JOB_RANGER 4096
#define JOB_PUPPETIER 8192
#define JOB_DOPPLER 16384
#define JOB_GATEKEEPER 32768

int tquest::joblookup[16]= {JOB_VAGRANT, JOB_MERCENARY, JOB_ACROBAT, JOB_ASSIST, JOB_MAGICIAN, JOB_PUPPETIER, JOB_KNIGHT, JOB_BLADE, JOB_JESTER, JOB_RANGER, JOB_RINGMASTER, JOB_BILLPOSTER, JOB_PSYCHIKEEPER, JOB_ELEMENTOR, JOB_DOPPLER, JOB_GATEKEEPER};

tquest *q=0;
std::map<int, tquest*> tquest::quests;
pmutex tquest::questsmutex;

#define QIT_BEGIN 1
#define QIT_BEGINADD 2
#define QIT_END 3
#define QIT_REMOVE 4
#define QIT_REWARD 5

void tquest::initquests()
{
	logger.log("Quest data\n");
	{
		int a,b;
		sqlquery s1(connections[0], "quests");
		s1.select("*, length(prevquests), length(killmobs), length(enditems), length(rewarditems), length(beginitems), length(toremoveitems), length(beginadditems), length(tquestitems)");

		int q_questid=s1.getColumnIndex("questid");
		int q_beginstate=s1.getColumnIndex("beginstate");
		int q_startlevel=s1.getColumnIndex("startlevel");
		int q_endlevel=s1.getColumnIndex("endlevel");
		int q_startcharacter=s1.getColumnIndex("startcharacter");
		int q_endcharacter=s1.getColumnIndex("endcharacter");
		int q_title=s1.getColumnIndex("title");
		int q_endstate=s1.getColumnIndex("endstate");
		int q_beginjob=s1.getColumnIndex("beginjob");
		int q_rewardjob=s1.getColumnIndex("rewardjob");
		int q_disquise=s1.getColumnIndex("disquise");
		int q_karmamin=s1.getColumnIndex("karmamin");
		int q_karmamax=s1.getColumnIndex("karmamax");
		int q_endkarmamin=s1.getColumnIndex("endkarmamin");
		int q_endkarmamax=s1.getColumnIndex("endkarmamax");
		int q_rewardmoneymin=s1.getColumnIndex("rewardmoneymin");
		int q_rewardmoneymax=s1.getColumnIndex("rewardmoneymax");
		int q_rewardexpmin=s1.getColumnIndex("rewardexpmin");
		int q_rewardexpmax=s1.getColumnIndex("rewardexpmax");
		int q_beginflags=s1.getColumnIndex("beginflags");
		int q_endflags=s1.getColumnIndex("endflags");
		int q_repeatable=s1.getColumnIndex("repeatable");

		while(s1.next())
		{
			a=toInt(s1[q_questid]);
			b=toInt(s1[q_beginstate]);
			q=new tquest(a);
			quests.insert(std::pair<int, tquest*>(a*256+b, q));
//			logger.log("%d %d %d\n", a, b, a*256+b);
			q->startlevel=toInt(s1[q_startlevel]);
			q->endlevel=toInt(s1[q_endlevel]);
			q->startcharacter=s1[q_startcharacter];
			q->endcharacter=s1[q_endcharacter];
			q->title=s1[q_title];
			q->beginstate=b;
			q->beginflags=toInt(s1[q_beginflags]);
			q->endflags=toInt(s1[q_endflags]);
			q->endstate=toInt(s1[q_endstate]);
			q->beginjob=toInt(s1[q_beginjob]);
			q->rewardjob=toInt(s1[q_rewardjob]);
			q->disquise=toInt(s1[q_disquise]);
			q->karmamin=toInt(s1[q_karmamin]);
			q->karmamax=toInt(s1[q_karmamax]);
			q->endkarmamin=toInt(s1[q_endkarmamin]);
			q->endkarmamax=toInt(s1[q_endkarmamax]);
			q->rewardmoneymin=toInt(s1[q_rewardmoneymin]);
			q->rewardmoneymax=toInt(s1[q_rewardmoneymax]);
			q->rewardexpmin=toInt(s1[q_rewardexpmin]);
			q->rewardexpmax=toInt(s1[q_rewardexpmax]);
			q->repeatable=(toInt(s1[q_repeatable])!=0);

			q->dialog_accept=toInt(s1["daccept"]);//5;
			q->dialog_deny=toInt(s1["ddeny"]);//6;
			q->dialog_finished=toInt(s1["dfinished"]);//7;
			q->dialog_askbefore=toInt(s1["daskbefore"]);//5
			a=toInt(s1["killmob1"]);
			if(a>0)
			{
				q->killmobs.push_back(std::pair<int, int>(a, toInt(s1["nkillmob1"])));
				a=toInt(s1["killmob2"]);
				if(a>0)
				{
					q->killmobs.push_back(std::pair<int, int>(a, toInt(s1["nkillmob2"])));
					a=toInt(s1["killmob3"]);
					if(a>0)
					{
						q->killmobs.push_back(std::pair<int, int>(a, toInt(s1["nkillmob3"])));
						a=toInt(s1["killmob4"]);
						if(a>0)
						{
							q->killmobs.push_back(std::pair<int, int>(a, toInt(s1["nkillmob4"])));
						}
					}
				}
			}

			char *i=s1.getptr("prevquests");
			b=toInt(s1["length(prevquests)"])/sizeof(int);
			if((b>0)&&(i!=0))
				for(a=0;a<b;a++)
					q->prevquests.push_back(*(int*)i);i+=sizeof(int);

		}
		s1.freeup();
	}

	{
		std::map<int, tquest*>::iterator i;
		sqlquery s1(connections[0], "questdialogs");
		s1.select();
		int q_questid=s1.getTableColumnIndex("questid");
		int q_state=s1.getTableColumnIndex("state");
		int q_dialognum=s1.getTableColumnIndex("dialognum");
		int q_str=s1.getTableColumnIndex("str");
		while(s1.next())
		{
			i=quests.find(toInt(s1[q_questid])*256+toInt(s1[q_state]));
			if(i!=quests.end())
			{
				i->second->dialogs.insert(std::pair<int, std::string>(toInt(s1[q_dialognum]), s1[q_str]));
			}else
			{
				logger.log("No quest for dialog %d %d\n", toInt(s1[q_questid]), toInt(s1[q_state]));
			}
		}
		s1.freeup();
	}

	{
		int a,b;
		sqlquery s1(connections[0], "questitems");
		std::map<int, tquest*>::iterator i;
		s1.select();
		while(s1.next())
		{
			a=toInt(s1["questid"]);
			b=toInt(s1["queststate"]);
			i=quests.find(a*256+b);
			if(i==quests.end())
			{
				logger.log("[questitems]: Quest id %d state %d not exists\n", a, b);
			}else
			{
				switch(toInt(s1["actiontype"]))
				{
				case QIT_BEGIN:
					i->second->beginitems.push_back(qitem(toInt(s1["itemid"]), toInt(s1["itemnumber"])));
					break;
				case QIT_BEGINADD:
					i->second->beginadditems.push_back(qitem(toInt(s1["itemid"]), toInt(s1["itemnumber"])));
					break;
				case QIT_END:
					i->second->enditems.push_back(qitem(toInt(s1["itemid"]), toInt(s1["itemnumber"])));
					break;
				case QIT_REMOVE:
					i->second->toremoveitems.push_back(qitem(toInt(s1["itemid"]), toInt(s1["itemnumber"])));
					break;
				case QIT_REWARD:
					i->second->rewarditems.push_back(qitem(toInt(s1["itemid"]), toInt(s1["itemnumber"])));
					break;
				default:
					logger.log("Unknown actiontype(%d) for quest item at (%d,%d)\n", toInt(s1["actiontype"]), a, b);
				}
			}
		}
		s1.freeup();
	}
	{
		sqlquery s1(connections[0], "questdrops");
		std::map<int, tquest*>::iterator i;
		s1.select();
		int a,b,c;
		while(s1.next())
		{
			a=toInt(s1["questid"]);
			b=toInt(s1["queststate"]);
			i=quests.find(a*256+b);
			if(i==quests.end())
			{
				logger.log("[questdrops]: Quest id %d state %d not exists\n", a, b);
			}else
			{
				c=toInt(s1["monsterid"]);
//				logger.log("%d %d %d %d %d:", c, a, b, i->second->questid, i->second->beginstate);
				i->second->questitems.insert(std::pair<int, tquestitems>(c, tquestitems(c, toInt(s1["itemid"]), (int)(toFloat(s1["dropchance"])*100.0f), toInt(s1["itemnumber"]) ) ));
			}
		}
		s1.freeup();
	}
}
