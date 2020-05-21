#include "stdafx.h"
#include "cluster.h"
#include <windows.h>
#include <mysql.h>
#include "logger.h"
#include "error.h"
#include "main.h"
#include "objactions.h"
#include "charserver.h"
#include "io/file/csv.h"

const int desiredlatency=100;
const int desiredlatency23=desiredlatency*2/3;

bool playeractfunkc(tplayer *p)
{
	p->timer();
	return false;
}

bool npcchatfunkc(character_npc *p)
{
	p->chat(p->chattext.c_str());
	return false;
}


bool objspawnerfunkc(spawner *p)
{
	p->timer();
	return false;
}

bool dotfunkc(character_buffable *p)
{
	bool r=p->dot();
	if(r)p->dotremover.clear();
	return r;
}

bool chkbuffsfunkc(character_buffable *p)
{
	ul m=(p->type==character_base::ttplayer)?((tplayer*)p)->playermutex.lock():pmutex::dontlock();
	bool r=p->checkbuffs();
	if(r)p->buffedremover.clear();
	return r;
}

bool petpickfunkc(character_npc *p)
{
	bool r=p->petpick();
	if(r)p->petremover.clear();
	return r;
}

bool mobagrofunkc(character_mob *p)
{
	bool r=p->agrocheck();
	if(r)p->agromobremover.clear();
	return r;
}

bool mobattackfunkc(character_mob *p)
{
	bool r=p->mobattack();
	if(r)p->mobattackremover.clear();
	return r;
}

bool moverfunkc(character_base *p)
{
	bool r;
	{
		ul m=(p->type==character_base::ttplayer)?((tplayer*)p)->playermutex.lock():pmutex::dontlock();
		r=p->domove();
		if(r)p->moverremover.clear();
	}
	p->processfollowers(r);
	return r;
}

bool assfunkc(asstruct *p)
{
	bool r=p->process();
	if(r)p->remover.clear();
	return r;
}

bool mobactfunkc(character_mob *p)
{
	bool r=p->act();
	if(r)p->mobremover.clear();
	return r;
}

bool cluster::loadplayer(tplayer *p)
{
	std::string username, password;
	int b,c;
	int s4;
	int cluster;
	p->sr->rewind();
	p->sr->forward(12);
	p->sr->get(b);
	p->sr->get(p->pos.x); //x
	p->sr->get(p->pos.y); //y
	p->sr->get(p->pos.z); //z
	p->sr->get(s4);
	p->sr->get(b);
	p->sr->get(b);
	p->sr->get(b);
	p->sr->get(b);

	p->sr->get(cluster);
	p->sr->get(b);
	p->sr->get(c);
	for(int a=0;a<c;a++)
	{
		p->sr->get(b);
		p->sr->get(b);
	}
	p->sr->get(b);

//	sr->dump();

	p->dbid=s4;

	{
		ul m=globalplayersmutex.lock();
		std::map<int, tplayer*>::iterator i=globaldbidplayers.find(p->dbid);
		if(i!=globaldbidplayers.end())
		{
			p->seterrorstatenl();
			p->sr->rewind();
			logger.log("player already logged in\n");
			return false;
		}
	}

//	p->sr->getpstr(str1);	//charname

	p->sr->get(b);
	p->sr->forward(b);
	char t[1025];
	p->sr->getpstr(t, 1024);
	username=t;
	p->username=username;
	p->sr->getpstr(t, 1024);
	password=t;
	if(!cserver->validate(username, password, getclusternumber()))
	{
		p->seterrorstatenl();
		p->sr->rewind();
		logger.log("player validate error\n");
		return false;
	}
	p->load(p->lparty);
	p->sr->rewind();

#ifdef _DEBUG
#ifdef _LOGALOT
	logger.log("Loaded\n");
#endif
#endif
	return true;
}

void cluster::doselect()
{
	std::list<tplayer*> dsplayers;

	int a;
	reciever *sval;
	struct sockaddr_in addr;
	int addrlen=sizeof(addr);

	bool recalc=false;
	tplayer *p;
	std::list<tplayer*>::iterator i;
	fd_set lreadfds, readfds;
	int hc2, hc=-1;
	FD_ZERO(&readfds);
	TIMEVAL TimeOut;


while(!endprg)
{
try{
while(!endprg)
{
	Sleep(1);

	while(!startrecv.empty())
	{
		tplayer *p=startrecv.pop();
		dsplayers.push_front(p);
	}

	lreadfds;

	TimeOut.tv_sec = 0;
	TimeOut.tv_usec = 1;

	std::list<tplayer*> rtemp;
	std::list<tplayer*>::iterator j=dsplayers.begin();
	for(;j!=dsplayers.end();)
	{
		(*j)->markedforrecv=false;
		rtemp.clear();
		int	c=-1;
		FD_ZERO(&lreadfds);
		for(int a=0;(j!=dsplayers.end())&&(a<FD_SETSIZE-2);a++,++j)
		{
			if((*j)->sck>c)c=(*j)->sck;
			FD_SET((unsigned int)(*j)->sck, &lreadfds);
			rtemp.push_back(*j);
		}
		if(c>-1)
		{
			select(c+1, &lreadfds, 0, 0, &TimeOut);
			for(std::list<tplayer*>::iterator i=rtemp.begin();i!=rtemp.end();++i)
			{
				if(FD_ISSET((*i)->sck,&lreadfds))(*i)->markedforrecv=true;
			}
		}
	}


//	select(hc2, &lreadfds, 0, 0, &TimeOut);

	for(i=dsplayers.begin();i!=dsplayers.end();++i)
	{
		{
			p=(*i);
			a=0;
			if(p->srready&&!p->toprocess)
			{
					sval=p->sr;
					p->sr=p->srr;
					p->srr=sval;
					p->srready=false;
					p->toprocess=true;
					toprocess.push(p);
			}
			if((!p->srready)&&(p->markedforrecv)&&(!p->geterrorstate()))
			{
				p->markedforrecv=false;
				a=p->srr->recieve();
				if(a==3)
				{
					p->last_active=GetTickCount();
					p->srready=true;

					if(!p->toprocess)
					{
						sval=p->sr;
						p->sr=p->srr;
						p->srr=sval;
						p->srready=false;
						p->toprocess=true;
						toprocess.push(p);
					}
					continue;
				}
			}
			if((a==1)||(a==2)||p->geterrorstate())
			{
				recalc=true;
				cmain_toerase.push(p);
				i=dsplayers.erase(i);
				if(i==dsplayers.end())break;
			}
		}
	}

	while(!doselect_toerase.empty())
	{
		tplayer *p=doselect_toerase.pop();
		{
			ul m=p->playermutex.lock();
			p->save(true);
		}
	}
	while(!logged_and_saved.empty())
	{
		tplayer *p=logged_and_saved.pop();
		{
			ul m=p->playermutex.lock();
			players.releaseTicket(p->deinit());
		}
	}


	while(!handedplayers.empty())
	{
		a=handedplayers.pop();

		if(players[a].sck>hc)hc=players[a].sck;
		FD_SET((unsigned int)players[a].sck, &readfds);

		toload.push(&players[a]);
	}



	while(!toload.empty())
	{
		tplayer *p=toload.pop();
		{
			ul mmm=p->playermutex.lock();
			if(loadplayer(p))
			{
				p->toprocess=true;
				loaded.push(p);
			}else
			{
				closesocket(p->sck);
				players.releaseTicket(p->getticket());
			}
		}

	}

}
}catch(const error &e)
{
	logger.log("[%s] Error: %s\n", this->name.c_str(), e.what());
}catch(const std::exception &e)
{
	logger.log("[%s] Exception: %s\n", this->name.c_str(), e.what());
}
#ifndef _DEBUG
catch(...)
{
	logger.elog("[%s::crmain]exception...\n", this->name.c_str());
}
#endif
}

}

int cluster::loadmobspawns()
{
    int count = 0;

    io::CSVReader<10> reader("resources/mobspawns.csv");
    reader.read_header(io::ignore_extra_column,
            "type", "mapid", "x", "y", "z", "rx", "ry", "normal", "agro", "respawnrate");

    int type, mapId, rx, ry, normal, agro, respawnRate;
    float x, y, z;

    while(reader.read_row(type, mapId, x, y, z, rx, ry, normal, agro, respawnRate))
    {
        if( ((type != 667) && (type != 668)) || (this->isPK()))
        {
            new mobspawn(this, type, normal, agro, x, y, z, rx, ry, mapId, respawnRate);
        }
        count++;
    }

    return count;
}

int cluster::loaditemspawns()
{
    int count = 0;

    io::CSVReader<9> reader("resources/itemspawns.csv");
    reader.read_header(io::ignore_extra_column,
            "type", "number", "x", "y", "z", "rx", "ry", "mapid", "respawnrate");

    int type, number, rx, ry, mapId, respawnRate;
    float x, y, z;

    while (reader.read_row(type, number, x, y, z, rx, ry, mapId, respawnRate)) {
        new itemspawn(this, type, number, x, y, z, rx, ry, mapId, respawnRate);
        count++;
    }
}

int cluster::loadnpces()
{
	sqlquery s4(connections.at(0), "npcdata");
	s4.selectw("enabled = 1", "*, length(shop1), length(shop2), length(shop3), length(shop4)");
	int a=0,b;
	int a1, a2,a3,a4;
	int n_type=s4.getTableColumnIndex("type");
	int n_name=s4.getTableColumnIndex("name");
	int n_x=s4.getTableColumnIndex("x");
	int n_y=s4.getTableColumnIndex("y");
	int n_z=s4.getTableColumnIndex("z");
	int n_mapid=s4.getTableColumnIndex("mapid");
	int n_id=s4.getTableColumnIndex("id");
	int n_direction=s4.getTableColumnIndex("direction");
	int n_shop1=s4.getTableColumnIndex("shop1");
	int n_shop2=s4.getTableColumnIndex("shop2");
	int n_shop3=s4.getTableColumnIndex("shop3");
	int n_shop4=s4.getTableColumnIndex("shop4");
	int n_chat=s4.getTableColumnIndex("chat");
	int n_greet=s4.getTableColumnIndex("greetingtext");
	int n_len1=s4.getColumnIndex("length(shop1)");
	int n_len2=s4.getColumnIndex("length(shop2)");
	int n_len3=s4.getColumnIndex("length(shop3)");
	int n_len4=s4.getColumnIndex("length(shop4)");
	while(s4.next())
	{
		a1=toInt(s4[n_len1]);	//17
		a2=toInt(s4[n_len2]);
		a3=toInt(s4[n_len3]);
		a4=toInt(s4[n_len4]);
		b=toInt(s4[n_type]);
		if((b!=0)||(this->isPK()))new character_npc(this, b, s4[n_name], toFloat(s4[n_x]), toFloat(s4[n_y]), toFloat(s4[n_z]), toInt(s4[n_mapid]), toInt(s4[n_direction]), s4.getptr(n_shop1), s4.getptr(n_shop2), s4.getptr(n_shop3), s4.getptr(n_shop4), a1, a2, a3, a4, toInt(s4[n_id]), s4[n_chat], s4[n_greet]);
		a++;
	}
	s4.freeup();
	return a;
}


void cluster::cmain()
{
	std::list<int>::iterator i;
	int a,b;
	bool schsh=false;
	long long lastclupdate=0, schs=0, siegestatupdate=0;

	connections.resize(2);
	for(a=0;a<(int)connections.size();a++)
	{
		connections[a]=new mcon();
		connections[a]->init(mysqlhost, mysqluser, mysqlpasswd, mysqldb);
	}

	dbaccounts.init(connections[0], "accounts");
	dbcharacters.init(connections[0], "characters");
	dbitemlist.init(connections[0], "itemlist");
	dbmails.init(connections[0], "mails");
	dbskilllist2.init(connections[0], "skilllist2");
	dbguilds.init(connections[0], "guilds");
	dbclusters.init(connections[0], "clusters");

	b=128*4/cellsize;
	grid.addmap(1,1, 32*b+1);
	grid.addmap(2,2, 5*b+1);
//	for(a=21;a<=25;a++)grid.addmap(a,a, 1*b);
	for(a=100;a<=103;a++)grid.addmap(a,a, 5*b+1);
	grid.addmap(120,120, 5*b+1);
//	for(a=151;a<=167;a++)grid.addmap(a,a, 1*b);
	for(a=200;a<=/*208*/204;a++)grid.addmap(a,a, 5*b+1);

	grid.addmap(210,210, 1*b+1);
	grid.addmap(220,220, 1*b+1);
	grid.addmap(230,230, 1*b+1);

//	for(a=241;a<=244;a++)grid.addmap(a,a, 1*b);

	new character_obj(42, this, 1, 5079.33f, 55.85f, 3987.0f, 2723, 100, new objteleportdmine(201, 743, 85, 783, 10395));	//mine entrance card
	new character_obj(42, this, 201, 741.28f, 85.0f, 781.43f, 905, 100, new objteleport(1, 5075, 55.15f, 3986));

	new character_obj(44, this, 201, 1447.28f, 85.0f, 709.62f, 905, 100, new objteleport(201, 1455, 85, 711.5f));
	new character_obj(44, this, 201, 1455, 85.0f, 713.5f, 905, 100, new objteleport(201, 1446, 85, 711.42f));

	new character_obj(44, this, 201, 1750.28f, 85.0f, 793.87f, 0, 100, new objteleport(201, 1747.59f, 85, 802.62f));
	new character_obj(44, this, 201, 1745.28f, 85.0f, 800.00f, 1800, 100, new objteleport(201, 1748.20f, 85, 792));


//	new character_obj(706, this, 2, (128+69.518339f)*4, (194.759125f/4), (128+80.2330788f)*4, 882, 505, new objteleportkebaras(10422));
//	new character_obj(706, this, 2, (128+51.744476f)*4, (190.993256f/4), (128+70.145020f)*4, 2769, 651, new objteleportkebaras(10422));
//	new character_obj(706, this, 2, (128+67.281067f)*4, (175.178467f/4), (128+110.124786f)*4, 2975, 745, new objteleportkebaras(10422));


	new character_obj(33, this, 201, 1073.2f, 85.0f, 696.82f, 0, 100);
	new character_obj(31, this, 201, 1084.50f, 83.0f, 679.80f, 900, 100, new objopendoor(10423 ));	//roach zone card
	{
		objopendoor *p=new objopendoor(10348, true);
		new character_obj(31, this, 200, 1180.31f, 78.0f, 625.71f, 0, 100, p);
		new character_obj(30, this, 200, 1172.31f, 80.25f, 622.50f, 900, 50, new objdooropener(p));
	}

	new flarisdroom(this);

	mobspawner_oaf blueroach_larva(717, 3);
	mobspawner_oaf pinkroach_larva(716, 3);
	mobspawner_q_oaf pinkroach_queen(705, 1, 10405);	//wake roach water
	mobspawner_q_oaf blueroach_queen(718, 1, 10405);	//wake roach water

	new gobjspawner(this, 38, 201, 1217, 85, 789, 5, 300, 16, 12, blueroach_larva);
	new gobjspawner(this, 38, 201, 1259, 85, 799, 3, 60, 6, 7, blueroach_larva);
	new gobjspawner(this, 38, 201, 1243, 85, 813, 3, 300, 10, 10, blueroach_larva);
	new gobjspawner(this, 38, 201, 1251, 85, 779, 5, 60, 13, 11, blueroach_larva);

	new gobjspawner(this, 36, 201, 1391, 85, 668, 3, 300, 11, 11, pinkroach_larva);
	new gobjspawner(this, 36, 201, 1380, 85, 701, 3, 300, 12, 13, pinkroach_larva);
	new gobjspawner(this, 36, 201, 1435, 85, 692, 3, 300,  8, 11, pinkroach_larva);
	new gobjspawner(this, 36, 201, 1429, 85, 665, 5, 300, 19, 12, pinkroach_larva);

	new gobjspawner(this, 35, 201, 1430, 85, 662, 1,  60, 22, 13, pinkroach_queen);
	new gobjspawner(this, 35, 201, 1381, 85, 695, 1,  60, 12, 11, pinkroach_queen);
	new gobjspawner(this, 35, 201, 1393, 85, 666, 1,  60, 12, 15, pinkroach_queen);

	new gobjspawner(this, 37, 201, 1211, 85, 782, 1,  60, 13,  9, blueroach_queen);
	new gobjspawner(this, 37, 201, 1250, 85, 777, 1,  60, 10, 11, blueroach_queen);
	new gobjspawner(this, 37, 201, 1243, 85, 813, 1,  60, 10,  7, blueroach_queen);


	new f_telc(this, 1, vector3d<>(7622.39f, 154.75f, 4282.13f), 5, vector3d<>(707, 80, 693), 200);
	new f_telc(this, 200, vector3d<>(681, 80, 681), 5, vector3d<>(7614, 156, 4267), 1);

	new f_telc(this, 202, vector3d<>(1321, 98, 1198), 5, vector3d<>(6968.384766f, 100.0f, 3328.863037f), 1);
	new f_telc(this, 202, vector3d<>(1310, 97, 1354), 5, vector3d<>(6968.384766f, 100.0f, 3328.863037f), 1);

	logger.log("Loading mobspawn data\n");
	a=loadmobspawns();
	logger.log("Loading itemspawn data\n");
	a=loaditemspawns();
	logger.log("Loading npc data\n");
	a=loadnpces();
/*
	tesztplayer=&players[1];
	players[1].init(300, reciever(), std::string("eh"), 1, this);
	players[1].dbid=79;
	b=0;
	for(a=0;a<3;a++)players[1].banks[a].init(&players[1], a);
	players[1].load(b);
	players[1].gridcell.add();

	players[2].init(300, reciever(), std::string("eh"), 1, this);
	players[2].dbid=80;
	for(a=0;a<3;a++)players[2].banks[a].init(&players[2], a);
	players[2].load(b);
	players[2].gridcell.add();

	players[3].init(300, reciever(), std::string("eh"), 1, this);
	players[3].dbid=81;
	for(a=0;a<3;a++)players[3].banks[a].init(&players[3], a);
	players[3].load(b);
	players[3].gridcell.add();

	std::list<tplayer*> tesztplayers;
	tesztplayers.push_back(&players[1]);
	tesztplayers.push_back(&players[2]);
	tesztplayers.push_back(&players[3]);
*/
	/*9*/

	logger.log("%s started\npk=%d\n", name.c_str(), (int)pkval);
	do
	{
	try
	{
		while(!endprg)
		{
			{
				pmutex::unlocker mm=this->clustermutex.lock();
				ido=GetTickCount();

				if(sieget!=0)
				{
					if(sieget<=ido)
					{
						sieget=0;
						psiege();
					}
				}
				if((siegestate==running)||(siegestate==teleportdown))
				{
					if(siegestatupdate<ido)
					{
						siegestatupdate=ido+30*1000;
						showsiegestats();
					}
				}

				{
					tplayer *p;
					while(!loaded.empty())
					{
						p=loaded.pop();
						dsplayers.push_front(p);
						p->dslistpos=dsplayers.begin();
						toprocess.push(p);
						startrecv.push(p);
					}


					while(!toprocess.empty())
					{
						p=toprocess.pop();
						{
							ul m=p->playermutex.lock();
							p->process();
						}
					}
				}

/*
				if(!tesztplayers.empty())
				{
					for(std::list<tplayer*>::iterator i=tesztplayers.begin();i!=tesztplayers.end();++i)
						genrndpacket(*i);
				}
*/
				if(lastclupdate<ido)
				{
					lastclupdate=ido+60000;
					a=0;
					if(nmaxplayers*3/4<=dsplayers.size())a=1;
					if(nmaxplayers<=dsplayers.size())a=2;
					dbclusters.addupdate("state", toString(a));
					dbclusters.addupdate("up", toString((long long)time(0)));
					dbclusters.update("sid = " + toString(servernumber) + " and cid = " + toString(clusternumber+1));
				}

				schsh=(schs<ido);
				if(schsh)
				{
					schs=ido+desiredlatency;
					while(!cmain_toerase.empty())
					{
						tplayer *p=cmain_toerase.pop();
						dsplayers.erase(p->dslistpos);
						{
							ul m=p->playermutex.lock();
							p->removefromcluster();
						}
						doselect_toerase.push(p);
					}

					while(!objtimers.empty())
					{
						if(objtimers.top().gettime()>ido)break;
						objtimercont a=objtimers.top();
						objtimers.pop();
						a.call();
					}

					{
						removeitem();
						domulticasts();
						grid.domulticast();
						checkparties();
						for(std::list<tplayer*>::iterator i=dsplayers.begin();i!=dsplayers.end();++i)
						{
							(*i)->domulticast();
							(*i)->getasyncbuffer();
							(*i)->send();
							if(((*i)->last_active())<ido-10*60*1000)(*i)->seterrorstatenl();
						}
					}

					playeract.run(ido);
					movers.run(ido);
					mobagro.run(ido);
					mobattack.run(ido);
					mobact.rund(ido);	//rund deletes the object if it returns true
					petpick.run(ido);
					buffed.run(ido);
					dot.run(ido);
					npcchat.run(ido);
					objspawners.run(ido);
					actionslots.rund(ido);
				}
				processmarklogged();
			}

			if(nplayers!=0)
			{
//				long long tt2=GetTickCount();
				if(!schsh)
				{
					long long t=GetTickCount()-ido;
					if(t<=desiredlatency23)
					{
						do{
							Sleep(2);
							if(!toprocess.empty())break;
							t=GetTickCount()-ido;
						}while(t<desiredlatency);
					}
//					logger.log("Waited %d\n", (int)(GetTickCount()-tt2));
				}
			}else Sleep(100);
		}
	}catch(const error &e)
	{
		logger.log("[%s] Error: %s\n", this->name.c_str(), e.what());
	}
	catch(const std::exception &e)
	{
		logger.log("[%s] Exception: %s\n", this->name.c_str(), e.what());
	}
#ifndef _DEBUG
	catch(...)
	{
		logger.elog("[%s]exception...\n", this->name.c_str());
	}
#endif

	}while(!endprg);

    pthread_join(*crthread,NULL);

	{
//		std::list<tplayer*>::iterator i;
//		for(i=dsplayers.begin();i!=dsplayers.end();++i)
		for(std::vector2<tplayer>::iterator i=players.players.begin();i!=players.players.end();++i)
		if(i->valid())
		{
			ul mmm=(i)->playermutex.lock();
			(i)->save();
		}
		for(std::vector2<tplayer>::iterator i=players.players.begin();i!=players.players.end();++i)
		if(i->valid())
		{
			ul mmm=(i)->playermutex.lock();
			(i)->removefromcluster();
		}
	}
	lsendprg=true;
    pthread_join(*lsthread,NULL);


	logger.log("%s ended\n", name.c_str());
	dbaccounts.freeup();
	dbcharacters.freeup();
	dbitemlist.freeup();
	dbmails.freeup();
	dbskilllist2.freeup();
	dbmonsterlist.freeup();
	dbguilds.freeup();
	dbclusters.freeup();

	std::string cname=this->name;
	delete this;
	logger.log("%s closed\n", cname.c_str());
}

cluster::cluster(int n, const char *nev, bool pk1)
:players(nmaxplayers),nplayers(0),endprg(false),clusternumber(n)
,bbs(0),movers(250, moverfunkc),actionslots(600, assfunkc)
,mobact(30000, mobactfunkc),mobattack(1000, mobattackfunkc)
,mobagro(1000, mobagrofunkc),asbs(0),asooo(0)
,petpick(500, petpickfunkc),buffed(1000, chkbuffsfunkc),dot(900, dotfunkc)
,objspawners(30000, objspawnerfunkc),cwcage(false)
,npcchat(30000, npcchatfunkc),playeract(1000, playeractfunkc)
,tesztplayer(0),siegestate(tsnone),sieget(0),siegetimertype(0)
,lsendprg(false),send_siege_combat(false)
{
//	size_t stacksize;
	pkval=pk1;
	mobs.resize(20000, 0);
	npcs.resize(nmaxplayers+1000, 0);
	items.resize(100000, 0);
	objs.resize(nmaxplayers+1000, 0);

	bbs=new buffer;
	mobid=100000000;//+nmaxmobs;
	objid=400000000;
	npcid=700000000;
	itemid=1000000000;
	tp.n=n;
	tp.c= std::shared_ptr<cluster>(this);

	this->name=nev;

	cthread=new pthread_t;
	pthread_attr_init(&ptca);
//	size_t stacksize;
//	pthread_attr_getstacksize (&ptca, &stacksize);
//	stacksize+=10*1024*1024;
//	pthread_attr_setstacksize (&ptca, stacksize);
	pthread_attr_setdetachstate(&ptca, PTHREAD_CREATE_JOINABLE);

    pthread_create(cthread, &ptca,clusterthread,(void*)&tp);
	pthread_attr_destroy(&ptca);

	crthread=new pthread_t;
	pthread_attr_init(&ptca);
	pthread_attr_setdetachstate(&ptca, PTHREAD_CREATE_JOINABLE);

    pthread_create(crthread, &ptca,recieverthread,(void*)&tp);
	pthread_attr_destroy(&ptca);

	lsthread=new pthread_t;
	pthread_attr_init(&ptca);
	pthread_attr_setdetachstate(&ptca, PTHREAD_CREATE_JOINABLE);

    pthread_create(lsthread, &ptca,loadsavethread,(void*)&tp);
	pthread_attr_destroy(&ptca);

	ido=GetTickCount();
}

cluster::~cluster()
{
	endprg=true;
//	logger.log("Closeing %s\n", name.c_str());
	delete bbs;
	bbs=0;
}

void cluster::crmain()
{
	std::vector2<std::shared_ptr<mcon>> c;
	c.resize(2);

	c.resize(2);
	for(int a=0;a<(int)c.size();a++)
	{
		c[a]= std::shared_ptr<mcon>(new mcon());
		c[a]->init(mysqlhost, mysqluser, mysqlpasswd, mysqldb);
	}

	dbaccounts4load.init(c[0].get(), "accounts");
	dbcharacters4load.init(c[1].get(), "characters");
	doselect();
	dbcharacters4load.freeup();
	dbaccounts4load.freeup();
}

void cluster::lsmain()
{
	charsavedata s1;
	saveconn=new mcon();
	saveconn->init(mysqlhost, mysqluser, mysqlpasswd, mysqldb);

	while(!lsendprg)
	{
	try
	{
	while(!lsendprg)
	{
		while(!charsave_queue.empty())
		{
			s1=charsave_queue.pop();
			try
			{
			logger.log("[save]");
			s1.s1->update("id="+toString(s1.dbid));
			s1.s2->update("id="+toString(s1.accid));
			if(s1.p!=0)logged_and_saved.push(s1.p);
			logger.log("[ok]");

			}catch(error &e)
			{
				logger.log("Query error at saveing character %d Query: %s\n", s1.dbid, e.what());
			}catch(std::exception &e)
			{
				logger.log("Exception at saveing character %d Exception: %s\n", s1.dbid, e.what());
			}catch(...)
			{
				logger.log("Query error at saveing character %d.\n", s1.dbid);
			}
			delete s1.s1;
			delete s1.s2;
		}
		Sleep(1);
	}
	}catch(...)
	{
	}
	}
}

void* loadsavethread(void *t)
{
	cluster *c;
	int cancelstate;
	c=((threadparm*)t)->c.get();
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,&cancelstate);
	//mysql_thread_init();
	c->lsmain();
	//mysql_thread_end();
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,&cancelstate);
	pthread_exit((void*)0);
	return 0;
}

void* recieverthread(void *t)
{
	cluster *c;
	int cancelstate;
	c=((threadparm*)t)->c.get();
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,&cancelstate);
	//mysql_thread_init();
	c->crmain();
	//mysql_thread_end();
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,&cancelstate);
	pthread_exit((void*)0);
	return 0;
}

void* clusterthread(void *t)
{
	cluster *c;
	int cancelstate;
	c=((threadparm*)t)->c.get();
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,&cancelstate);
	//mysql_thread_init();
	c->cmain();
	//mysql_thread_end();
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,&cancelstate);
	pthread_exit((void*)0);
	return 0;
}

void cluster::handower(int sck1, reciever &sr1, std::string cip1)
{
	pmutex::unlocker m=this->clustermutex.lock();
	//insert player here
#ifdef _DEBUG
#ifdef _LOGALOT
	logger.log("Handower ");
#endif
#endif
	int ticket;
	if(players.hasFree())
	{
		ticket=players.getTicket();
		players[ticket].init(sck1, sr1, cip1, ticket, this);
//		ul mm=handedplayersmutex.lock();
		handedplayers.push(ticket);
#ifdef _DEBUG
#ifdef _LOGALOT
		logger.log("success \n");
#endif
#endif
	}else
	{
		closesocket(sck1);
//#ifdef _DEBUG
//#ifdef _LOGALOT
		logger.log("fail. Cluster is full\n");
//#endif
//#endif
	}
}
int cluster::getitemid(character_item *m)
{
	int r;
	if(itemidlist.empty())
	{
		itemidlist.push_back(itemid);
		itemid++;
		if(itemid>=2140000000)itemid=1000000000;
	}
	r=itemidlist.front();
	itemidlist.pop_front();
	if((int)items.size()<r-1000000000)items.resize(r-1000000000+1000, 0);
	items[r-1000000000]=m;
	return r;
}
character_item* cluster::getitem(int id)
{
	character_item *r=0;
	id-=1000000000;
	if((id>=0)&&(id<(int)items.size()))r=items[id];
	return r;
}

void cluster::releaseitemid(int id)
{
	items[id-1000000000]=0;
	itemidlist.push_back(id);
}


int cluster::getmobid(character_mob* m)
{
	int r;
	if(mobidlist.empty())
	{
		mobidlist.push_back(mobid);
		mobid++;
		if(mobid>=399999999)mobid=100000000;//+nmaxmobs;
	}
	r=mobidlist.front();
	mobidlist.pop_front();
	if((int)mobs.size()<r-100000000)mobs.resize(r-100000000+1000, 0);
	mobs[r-100000000]=m;
	return r;
}


void cluster::releasemobid(int id)
{
	mobs[id-100000000]=0;
	mobidlist.push_back(id);
}

tplayer* cluster::getplayer(int id)
{
	tplayer* r=0;
	if((id>=0)&&(id<nmaxplayers))
	{
		r=&players[id];
		if(r->getticketnl()==-1)r=0;
	}

	return r;
}

character_mob* cluster::getmob(int id)
{
	character_mob* r=0;
	id-=100000000;
	if((id>=0)&&(id<(int)mobs.size()))r=mobs[id];
	return r;
}

int cluster::getobjid(character_obj *m)
{
	if(objidlist.empty())
	{
		objidlist.push_back(objid);
		objid++;
		if(objid>=699999999)objid=400000000;
	}
	int r=objidlist.front();
	if((int)objs.size()<r-400000000)objs.resize(r-400000000+1000, 0);
	objs[r-400000000]=m;
	objidlist.pop_front();
	return r;
}
void cluster::releaseobjid(int id)
{
	objs[id-400000000]=0;
	objidlist.push_back(id);
}

character_obj* cluster::getobj(int id)
{
	character_obj *r=0;
	id-=400000000;
	if((id>=0)&&(id<(int)objs.size()))r=objs[id];
	return r;
}

int cluster::getnpcid(character_npc *m)
{
	int r;
	if(npcidlist.empty())
	{
		npcidlist.push_back(npcid);
		npcid++;
		if(npcid>=999999999)npcid=700000000;
	}

	r=npcidlist.front();
	if((int)npcs.size()<r-700000000)npcs.resize(r-700000000+1000, 0);
	npcs[r-700000000]=m;
	npcidlist.pop_front();
	return r;
}
void cluster::releasenpcid(int id)
{
	npcs[id-700000000]=0;
	npcidlist.push_back(id);
}

character_npc* cluster::getnpc(int id)
{
	character_npc* r=0;
	id-=700000000;
	if((id>=0)&&(id<(int)npcs.size()))r=npcs[id];
	return r;
}

void cluster::additem(itemremover *p)
{
	itemremove.push_back(p);
}

void cluster::removeitem()
{
	if(!itemremove.empty())
	{
		while(itemremove.front()->i==0)
		{
			delete itemremove.front();
			itemremove.pop_front();
			if(itemremove.empty())return;
		}
		while(itemremove.front()->i->getspawntime()<=ido)
		{
			delete itemremove.front();
			itemremove.pop_front();
			if(itemremove.empty())return;
			while(itemremove.front()->i==0)
			{
				delete itemremove.front();
				itemremove.pop_front();
				if(itemremove.empty())return;
			}
		}
	}
}



void cluster::doplayeract()
{
	std::list<tplayer*>::iterator i;
	for(i=dsplayers.begin();i!=dsplayers.end();++i)
		if((*i)->validnl())
		{
			ul mmm=(*i)->playermutex.lock();
			(*i)->timer();
		}
}


char* cluster::print(const char *s, ...)
{
	va_list lista;
	if(s!=0)
	{
		va_start(lista, s);
		vsprintf(ppuffer, s, lista);
		va_end(lista);
	}else ppuffer[0]=0;
	return &ppuffer[0];
}

bool cluster::npcinrange(int a)
{
	return ((a>=700000000)&&(a<=999999999));
}
bool cluster::objinrange(int a)
{
	return ((a>=400000000)&&(a<=699999999));
}
bool cluster::iteminrange(int a)
{
	return (a>=1000000000);
}
bool cluster::mobinrange(int a)
{
	return ((a>=100000000)&&(a<=399999999));
}
bool cluster::playerinrange(int a)
{
	return ((a>=0)&&(a<nmaxplayers));
}

void cluster::marklogged(const std::string username)
{
	markloggedlist.push(username);
#ifdef _DEBUG
#ifdef _LOGALOT
	logger.log("Marked for cluster %d %s\n", clusternumber, name.c_str());
#endif
#endif
}

void cluster::processmarklogged()
{
	std::list<tplayer*>::iterator i;
	pmutex::unlocker mm;
	std::string name;
	while(!markloggedlist.empty())
	{
#ifdef _DEBUG
#ifdef _LOGALOT
		logger.log("processmarks\n");
#endif
#endif
		name=markloggedlist.pop();
		for(i=dsplayers.begin();i!=dsplayers.end();++i)
		if((*i)->valid())
		{
			ul m=(*i)->playermutex.lock();
			if(name==(*i)->username)
			{
#ifdef _DEBUG
#ifdef _LOGALOT
				logger.log("Found marked\n");
#endif
#endif
				(*i)->seterrorstatenl();
				break;
			}
		}
	}
}

void cluster::multicast(buffer &bs)
{
	bbs->add(bs);
}

void cluster::multicastooo(buffer &bs)
{
	ooolist.push_back(bs);
}

clustercont::clustercont(mcon *con, int n, int servernumber)
:nclusters(n),bs(0),ooo(0)
{
	sqlquery s1(con, "clusters");
	clusters.resize(nclusters);
	logger.log("%d Clusters\n", nclusters);
	s1.selectw(std::string("sid=")+toString(servernumber)+std::string(" AND cid>0"));
	int a=0;
	while(s1.next())
	{
		clusters[a]=new cluster(a, s1["name"].c_str(), toInt(s1["pk"])>0);
		a++;
	}
	s1.freeup();
}

clustercont::~clustercont()
{
	pmutex::unlocker m=clustercontmutex.lock();
	delete bs;
	bs=0;
	delete ooo;
	ooo=0;
}

void clustercont::multicast(buffer &bs1)
{
	pmutex::unlocker m=clustercontmutex.lock();
	for(int a=0;a<nclusters;a++)
	{
		{
			pmutex::unlocker m2=clusters[a]->asyncbuffermutex.lock();
			if(clusters[a]->asbs==0)clusters[a]->asbs=new buffer;
			clusters[a]->asbs->copy(bs1);
		}
	}
}

void clustercont::multicastooo(buffer &bs1)
{
	pmutex::unlocker m=clustercontmutex.lock();
	for(int a=0;a<nclusters;a++)
	{
		{
			pmutex::unlocker m2=clusters[a]->asyncbuffermutex.lock();
			if(clusters[a]->asooo==0)clusters[a]->asooo=new buffer;
			clusters[a]->asooo->copy(bs1);
		}
	}
}

void cluster::domulticasts()
{
	buffer *bs, *ooo;
	{
		pmutex::unlocker m=asyncbuffermutex.lock();
		bs=asbs;
		ooo=asooo;
		asbs=0;
		asooo=0;
	}
	if(bs!=0)
	{
		multicast(*bs);
		delete bs;
	}
	if(ooo!=0)
	{
		multicastooo(*ooo);
		delete ooo;
	}

	std::list<tplayer*>::iterator i;
	std::list<buffer>::iterator bi;

//	if((!ooolist.empty())||(bbs->getcommandnumber()>0))for(std::vector2<tplayer>::iterator i=players.players.begin();i!=players.players.end();++i)
//	if(i->isUsed())
	if((!ooolist.empty())||(bbs->getcommandnumber()>0))
	for(i=dsplayers.begin();i!=dsplayers.end();++i)
	{
		ul mmm=(*i)->playermutex.lock();
		if((*i)->validnl())
		{
			if(bbs->getcommandnumber()>0)(*i)->add(*bbs);
			if(!ooolist.empty())
				for(bi=ooolist.begin();bi!=ooolist.end();++bi)
				{
					//(*i)->addbuffer(*bi);
					(*i)->ooo->add(*bi);
					(*i)->raiseooo();
				}
		}
	}

	if(bbs->getcommandnumber()>0)
	{
		delete bbs;
		bbs=new buffer;
	}
	ooolist.clear();
}

void cluster::checkparties()
{
	while(!linkattacks.empty())
	{
		if(linkattacks.back()->testlink(ido))
		{
			linkattacks.back()->alinkr.clear();
			linkattacks.pop_back();
		}else break;
	}
	while(!pscircle.empty())
	{
		if(pscircle.back()->testasc(ido))
		{
			pscircle.back()->ascr.clear();
			pscircle.pop_back();
		}else break;
	}
}

void cluster::notice2(const std::string &t)
{
	buffer bs;
	bs << 0x00ff00ea;
	bs.sndpstr(t);
	clusters->multicastooo(bs);
}

typedef void (*PROCFUNKC)(tplayer*);
extern PROCFUNKC t_0000[65536];
extern PROCFUNKC t_00ff[65536];
extern PROCFUNKC t_f000[65536];
extern PROCFUNKC t_ffff[65536];

	std::vector<int> vv(1000);

void init11()
{
	for(int a=0;a<65536;a++)if(t_0000[a]!=0)vv.push_back(a);
	for(int a=0;a<65536;a++)if(t_00ff[a]!=0)vv.push_back(0x00ff0000|a);
	for(int a=0;a<65536;a++)if(t_f000[a]!=0)vv.push_back(0xf0000000|a);
	for(int a=0;a<65536;a++)if(t_ffff[a]!=0)vv.push_back(0xffff0000|a);
}

void cluster::genrndpacket(tplayer *p)
{

	if(p!=0)
	{
/*
		reciever *s=p->sr;

		p->sr=new reciever();
		int a=rnd(50)+15;

		p->sr->header[0]=0x5e;
		*(int*)(&p->sr->header[5])=a;
		p->sr->torecieve=a+4;

		p->sr->puffer.resize(p->sr->torecieve+5);
		p->sr->rpuffer=p->sr->torecieve;

		*(int*)(&p->sr->puffer[0])=-1;
		*(int*)(&p->sr->puffer[4])=-1;
		*(int*)(&p->sr->puffer[8])=vv.at(rnd(vv.size()));
		for(int b=12;b<a+4;b++)p->sr->puffer[b]=rnd(256);


		p->process();
		delete p->sr;
		p->sr=s;
*/
		if(rnd(100)<50)
		{
			p->gridcell.half_remove();
			logger.log("halfremover\n");
		}
		p->changemap(p->getmapid(), rnd(8000), rnd(200), rnd(8000));
	}
}


void cluster::setsiegestate(tsiegestate a)
{
	switch(a)
	{
	case tsnone:
//		if((siegestate==apply)||(siegestate==ending))
		{
			siegestate=a;
		}//else logger.log("Wrong siegestate %d\n", (int)a);
		break;
	case apply:
//		if((siegestate==tsnone)||(siegestate==ending))
		{
			siegestate=a;
		}//else logger.log("Wrong siegestate %d\n", (int)a);
		break;
	case prepare:
		if(siegestate==apply)
		{
			siegestate=a;
			siege_prepare();
		}else logger.log("Wrong siegestate %d\n", (int)a);
		break;
	case teleportin:
		if(siegestate==prepare)
		{
			siegestate=a;
			siege_teleportin();
		}else logger.log("Wrong siegestate %d\n", (int)a);
		break;
	case teleportdown:
		if(siegestate==teleportin)
		{
			siegestate=a;
			siege_teleportdown();
		}else logger.log("Wrong siegestate %d\n", (int)a);
		break;
	case running:
		if(siegestate==teleportdown)
		{
			siegestate=a;
			siege_running();
		}else logger.log("Wrong siegestate %d\n", (int)a);
		break;
	case ending:
		if(siegestate==running)
		{
			siegestate=a;
		}
		else logger.log("Wrong siegestate %d\n", (int)a);
		break;
	}
}

void cluster::cancelsiege(int n)
{
	if(n>=0)logger.log("Not enough guilds (%d), siege cancelled\n", n);
	siegetimertype=0;
	siegeguilds.clear();
	siegeplayers.clear();

	setsiegestate(apply);
}

void cluster::siege_running()
{
	send_siege_combat=true;
	siegetimertype=0x68;
	sieget=ido+2*60*1000;	//30mins

	gw_attackable();
}

void cluster::gw_attackable()
{
	buffer bs;
	int a;
	for(std::list<tguild::tguildmember*>::iterator i=siegeplayers.begin();i!=siegeplayers.end();++i)
	{
		tplayer *p=(*i)->p;
		if((p!=0)&&(p->cl==this))
		{
			a=2;
			if((siegestate==running)&&((*i)->teleported_down))a=1;
			p->siege_attackable=(a==1);
			bs.cmd(p->getId(), 0xb8) << (char)16 << p->getId() << a;
		}

	}

	for(std::list<tguild::tguildmember*>::iterator i=siegeplayers.begin();i!=siegeplayers.end();++i)
	{
		tplayer *p=(*i)->p;
		if((p!=0)&&(p->cl==this))
		{
			p->bs->add(bs);
		}
	}
}

void cluster::siege_ending()
{
	send_siege_combat=false;
	siegetimertype=0x68;
	sieget=ido+20*1000;	//20 sec
/*
	for(std::list<tguild*>::iterator i=siegeguilds.begin();i!=siegeguilds.end();++i)
	{
		(*i)->siege_end(false);
	}
*/
}

void cluster::siege_teleportdown()
{
	siegetimertype=0x68;
	sieget=ido+2*60*1000;	//10mins
	for(std::list<tguild*>::iterator i=siegeguilds.begin();i!=siegeguilds.end();++i)
	{
		(*i)->siege_teleportdown();
	}
	gw_attackable();
	int a = getsieget()-ido;
	for(std::list<tplayer*>::iterator i=ongwmap.begin();i!=ongwmap.end();++i)
	{
		tplayer *p=*i;
		p->bs->cmd(p->getId(), 0xb8) << (char)49 << a << getsiegetimertype();
	}

}

void cluster::siege_teleportin()
{
	for(std::list<tguild*>::iterator i=siegeguilds.begin();i!=siegeguilds.end();++i)
	{
		if(!(*i)->siege_createline_finish(this))
		{
			i=siegeguilds.erase(i);
			if(i==siegeguilds.end())break;
		}
	}
	if(siegeguilds.size()<2)
	{
		cancelsiege(siegeguilds.size());
		return;
	}
	siegetimertype=0x68;
	sieget=ido+2*60*1000;	//5mins
	for(std::list<tguild*>::iterator i=siegeguilds.begin();i!=siegeguilds.end();++i)
	{
		(*i)->siege_teleportin();
	}

}

void cluster::siege_prepare()
{
	int nguilds=0;
	std::vector2<int> guildids;
	{
		ul m=dbguildsiege_mutex.lock();
		sqlquery &s1=dbguildsiege;
		s1.select("count(*)");
		if(s1.next())
		{
			nguilds=toInt(s1[0]);
		}
		s1.freeup();
		if(nguilds<2)
		{
			cancelsiege(nguilds);
			return;
		}

		s1.selectw("guildid>=0 order by money desc");
		while(s1.next())
		{
			guildids.push_back(toInt(s1["guildid"]));
		}
		s1.freeup();
	}
	if(guildids.size()<2)
	{
		cancelsiege(guildids.size());
		return;
	}
	siegeguilds.clear();
	siegeplayers.clear();
	{
		int b=0;
		siegetimertype=0x66;
		sieget=ido+2*60*1000;	//1 hour
		for(size_t a=0;(a<guildids.size())&&(b<8);++a,++b)
		{
			tguild *g=tguild::getguild(guildids[a]);
			siegeguilds.push_back(g);
			g->siege_createline_start();
			tplayer *p=g->getmaster();
			if(p!=0)p->gwprepare();
		}
	}
}



void cluster::psiege()
{
	sieget=0;
	switch(siegestate)
	{
	case tsnone:
		break;
	case apply:
		break;
	case prepare:
		setsiegestate(teleportin);
		break;
	case teleportin:
		setsiegestate(teleportdown);
		break;
	case teleportdown:
		setsiegestate(running);
		break;
	case running:
		setsiegestate(ending);
		break;
	case ending:
		setsiegestate(tsnone);
		break;
	}
}

void cluster::sendsiegedata(tplayer *p)
{
	if(siegeguilds.size()>1)
	{
		p->bs->cmd(-1, 0xb8) << (char)17;
		*p->bs << (int)siegeguilds.size();
		for(std::list<tguild*>::iterator i=siegeguilds.begin();i!=siegeguilds.end();++i)
		{
//			*p->bs << (int)(*i)->getguildid();
			(*i)->sendsiegepacket(p);
		}
	}
}

void cluster::showsiegestats()
{
	std::map<int, tguild::tguildmember*> pr;
	std::list<tguild::tguildmember*> pr2;
	std::map<int, tguild*> gr;
	std::list<tguild*> gr2;

	for(std::list<tguild::tguildmember*>::iterator i=siegeplayers.begin();i!=siegeplayers.end();++i)
	{
		pr.insert(std::pair<int, tguild::tguildmember*>((*i)->point, *i));
	}
	for(std::list<tguild*>::iterator i=siegeguilds.begin();i!=siegeguilds.end();++i)
	{
		gr.insert(std::pair<int, tguild*>((*i)->siegepoint, *i));
	}

	int a=0;
	for(std::map<int, tguild::tguildmember*>::reverse_iterator i=pr.rbegin();(i!=pr.rend())&&(a<4);++i,++a)
	{
		pr2.push_back(i->second);
	}
	int b=0;
	for(std::map<int, tguild*>::reverse_iterator i=gr.rbegin();(i!=gr.rend())&&(b<4);++i,++b)
	{
		gr2.push_back(i->second);
	}

	buffer bs;
	bs.cmd(-1, 0xb8) << (char)33 << gr2.size();
	for(std::list<tguild*>::iterator i=gr2.begin();i!=gr2.end();++i)
	{
		bs << 1;
		bs.sndpstr((*i)->name);
		bs << (*i)->siegepoint;
	}
	a = getsieget()-ido;
//	bs.cmd(-1, 0xb8) << (char)34 << a;


	for(std::list<tplayer*>::iterator i=ongwmap.begin();i!=ongwmap.end();++i)
	{
		(*i)->add(bs);
		tplayer *p=*i;
		if(p->bs!=0)p->bs->cmd(p->getId(), 0xb8) << (char)49 << a << getsiegetimertype();
	}
}

float cluster::getclienttime()
{
	const int d=60*60*24;
	const float h=60.0f*60.0f;
	int t1=ido;
	t1/=1000;
	t1*=120;
	t1%=d;
	if(t1<0)t1+=d;
	float t=t1;
	t1/=h;
	return t1;
}
