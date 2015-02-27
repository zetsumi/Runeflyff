
#include "stdafx.h"
#include "main.h"
#include "logger.h"
#include "vector2.h"
#include <string>
#include "item.h"
#include "adat.h"
#include "mysql.h"
#include "buff.h"
#include "quest.h"


using namespace std;

#define __GenDropsByItemLevel__

vector3d<> arenapos(8491.26f, /*91.59f*/0, 3473);
float arenarange=25*25;

float dropmult=10.0f;
double moneymultiplier=100.0;
long double expMultiplier=50;
long double skillExpMultiplier=100;
int partypointsperlevel=10;
int partyexpszorzo=10;
unsigned long long explose=1.0;
const bool forcedchangeonblink=true;

int itemstaytime=1000*60;
int itemlocktime=1000*30;

const float md=1.0f;

struct tmdrop
{
	int dropid;
	int mobid;
	tmdrop(int a,int b):dropid(a), mobid(b){}
};

std::vector2<tmdrop> mdrops;

int ultimatestats1[9]={35,83,26,-1,-1,1,4,2,3};

int uphitbonus1[11]={0,0,0,5,10,10,20,20,30,30,45};
int upblockbonus1[11]={0,0,0,3,3,3,6,6,10,10,15};
int uphpbonus1[11]={0,0,0,0,0,5,5,5,10,10,20};
int upmagicbonus1[11]={0,0,0,3,4,5,6,7,8,9,10};
int upallstatbonus1[11]={0,0,0,0,0,1,1,1,2,2,3};


int ultimatebonus1[9][5][5]=
{
{
	{228,455,683,910,1365},
	{455,910,1365,1820,2730},
	{683,1365,2048,2730,4095},
	{910,1820,2730,3640,5460},
	{1138,2275,3413,4550,6825}
},{
	{13,25,38,50,75},
	{25,50,75,100,150},
	{38,75,113,150,225},
	{50,100,150,200,300},
	{63,125,188,250,375}
},{
	{10,20,30,40,60},
	{20,40,60,80,120},
	{30,60,90,120,180},
	{40,80,120,160,240},
	{50,100,150,200,300}
},{
	{1,2,3,4,6},
	{2,3,5,6,9},
	{2,4,6,8,12},
	{3,5,8,10,15},
	{3,6,9,12,18}
},{
	{2,4,6,8,12},
	{3,6,9,12,18},
	{4,8,12,16,24},
	{5,10,15,20,30},
	{6,12,18,24,36}
},{
	{3,5,8,10,15},
	{5,10,15,20,30},
	{8,15,23,30,45},
	{10,20,30,40,60},
	{13,25,38,50,75}
},{
	{3,5,8,10,15},
	{5,10,15,20,30},
	{8,15,23,30,45},
	{10,20,30,40,60},
	{13,25,38,50,75}
},{
	{3,5,8,10,15},
	{5,10,15,20,30},
	{8,15,23,30,45},
	{10,20,30,40,60},
	{13,25,38,50,75}
},{
	{3,5,8,10,15},
	{5,10,15,20,30},
	{8,15,23,30,45},
	{10,20,30,40,60},
	{13,25,38,50,75}
}
};


int skillmaxlevel1[32][45]={\
	{10,10,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*vagrant*/\
	{10,10,10,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*merc*/\
	{10,10,10,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*acrobat*/\
	{10,10,10,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*assist*/\
	{10,10,10,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*mage*/\
	{10,10,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*puppeteer*/\
	{10,10,10,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,-1,-1,-1,-1,-1,10,10,10,10,10,10,10,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*knigth*/\
	{10,10,10,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,-1,-1,-1,-1,-1,10,10,10,10,10,10,10,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*blade*/\
	{10,10,10,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,-1,-1,-1,10,10,10,10,10,10,10,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*jester*/\
	{10,10,10,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,-1,-1,-1,10,10,10,10,10,10,10,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*ranger*/\
	{10,10,10,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,-1,-1,-1,10,10,10,10,10,10, 1, 3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*rm*/\
	{10,10,10,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,-1,-1,-1,10,10,10,10,10,10,10,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*bp*/\
	{10,10,10,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,-1,-1,-1,10,10,10,10,10,10,10,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*psy*/\
	{10,10,10,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,-1,-1,-1,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,-1,-1,-1},	/*ele*/\
	{10,10,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*gatekeeper*/\
	{10,10,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*doppler*/\
	{10,10,10,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,-1,-1,-1,-1,-1,10,10,10,10,10,10,10,10, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*knigth master*/\
	{10,10,10,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,-1,-1,-1,-1,-1,10,10,10,10,10,10,10,10, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*blade master*/\
	{10,10,10,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,-1,-1,-1,10,10,10,10,10,10,10,10, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*jester master*/\
	{10,10,10,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,-1,-1,-1,10,10,10,10,10,10,10,10, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*ranger master*/\
	{10,10,10,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,-1,-1,-1,10,10,10,10,10,10, 1, 3, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*rm master*/\
	{10,10,10,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,-1,-1,-1,10,10,10,10,10,10,10,10, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*bp master*/\
	{10,10,10,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,-1,-1,-1,10,10,10,10,10,10,10,10, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*psy master*/\
	{10,10,10,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,-1,-1,-1,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10, 5,-1,-1},	/*ele master*/\
	{10,10,10,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,-1,-1,-1,-1,-1,10,10,10,10,10,10,10,10, 5, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*knigth hero*/\
	{10,10,10,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,-1,-1,-1,-1,-1,10,10,10,10,10,10,10,10, 5, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*blade hero*/\
	{10,10,10,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,-1,-1,-1,10,10,10,10,10,10,10,10, 5, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*jester hero*/\
	{10,10,10,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,-1,-1,-1,10,10,10,10,10,10,10,10, 5, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*ranger hero*/\
	{10,10,10,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,-1,-1,-1,10,10,10,10,10,10, 1, 3, 5, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*rm hero*/\
	{10,10,10,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,-1,-1,-1,10,10,10,10,10,10,10,10, 5, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*bp hero*/\
	{10,10,10,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,-1,-1,-1,10,10,10,10,10,10,10,10, 5, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*psy hero*/\
	{10,10,10,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,-1,-1,-1,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10, 5, 5,-1},	/*ele hero*/\
};


int skills1[32][45]={\
	{1,2,3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*vagrant*/\
	{1,2,3,4,5,6,7,8,9,10,11,12,13,14,108,109,111,112,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*merc*/\
	{1,2,3,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*acrobat*/\
	{1,2,3,44,52,46,114,49,104,45,20,51,53,113,105,115,48,116,117,50,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*assist*/\
	{1,2,3,30,31,32,33,34,35,36,37,64,65,69,70,107,118,119,120,121,122,123,124,125,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*mage*/\
	{1,2,3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*puppeteer*/\
 	{1,2,3,4,5,6,7,8,9,10,11,12,13,14,108,109,111,112,-1,-1,-1,-1,-1,128,129,130,131,132,133,134,135,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*knigth*/\
	{1,2,3,4,5,6,7,8,9,10,11,12,13,14,108,109,111,112,-1,-1,-1,-1,-1,136,137,138,139,140,141,142,143,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*blade*/\
	{1,2,3,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,-1,-1,-1,207,208,209,210,211,212,213,214,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*jester*/\
	{1,2,3,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,-1,-1,-1,215,216,217,218,219,220,221,222,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*ranger*/\
	{1,2,3,44,52,46,114,49,104,45,20,51,53,113,105,115,48,116,117,50,-1,-1,-1,150,151,148,147,146,144,145,149,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*rm*/\
	{1,2,3,44,52,46,114,49,104,45,20,51,53,113,105,115,48,116,117,50,-1,-1,-1,156,152,154,157,158,153,155,159,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*bp*/\
	{1,2,3,30,31,32,33,34,35,36,37,64,65,69,70,107,118,119,120,121,-1,-1,-1,160,161,162,163,164,165,166,167,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*psy*/\
	{1,2,3,30,31,32,33,34,35,36,37,64,65,69,70,107,118,119,120,121,-1,-1,-1,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,-1,-1,-1},	/*ele*/\
	{1,2,3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*gatekeeper*/\
	{1,2,3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*doppler*/\
	{1,2,3,4,5,6,7,8,9,10,11,12,13,14,108,109,111,112,-1,-1,-1,-1,-1,128,129,130,131,132,133,134,135,310,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*knigth master*/\
	{1,2,3,4,5,6,7,8,9,10,11,12,13,14,108,109,111,112,-1,-1,-1,-1,-1,136,137,138,139,140,141,142,143,309,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*blade master*/\
	{1,2,3,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,-1,-1,-1,207,208,209,210,211,212,213,214,311,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*jester master*/\
	{1,2,3,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,-1,-1,-1,215,216,217,218,219,220,221,222,312,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*ranger master*/\
	{1,2,3,44,52,46,114,49,104,45,20,51,53,113,105,115,48,116,117,50,-1,-1,-1,150,151,148,147,146,144,145,149,316,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*rm master*/\
	{1,2,3,44,52,46,114,49,104,45,20,51,53,113,105,115,48,116,117,50,-1,-1,-1,156,152,154,157,158,153,155,159,315,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*bp master*/\
	{1,2,3,30,31,32,33,34,35,36,37,64,65,69,70,107,118,119,120,121,-1,-1,-1,160,161,162,163,164,165,166,167,314,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*psy master*/\
	{1,2,3,30,31,32,33,34,35,36,37,64,65,69,70,107,118,119,120,121,-1,-1,-1,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,313,-1,-1},	/*ele master*/\
	{1,2,3,4,5,6,7,8,9,10,11,12,13,14,108,109,111,112,-1,-1,-1,-1,-1,128,129,130,131,132,133,134,135,310,238,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*knigth hero*/\
	{1,2,3,4,5,6,7,8,9,10,11,12,13,14,108,109,111,112,-1,-1,-1,-1,-1,136,137,138,139,140,141,142,143,309,237,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*blade hero*/\
	{1,2,3,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,-1,-1,-1,207,208,209,210,211,212,213,214,311,239,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*jester hero*/\
	{1,2,3,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,-1,-1,-1,215,216,217,218,219,220,221,222,312,240,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*ranger hero*/\
	{1,2,3,44,52,46,114,49,104,45,20,51,53,113,105,115,48,116,117,50,-1,-1,-1,150,151,148,147,146,144,145,149,316,244,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*rm hero*/\
	{1,2,3,44,52,46,114,49,104,45,20,51,53,113,105,115,48,116,117,50,-1,-1,-1,156,152,154,157,158,153,155,159,315,243,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*bp hero*/\
	{1,2,3,30,31,32,33,34,35,36,37,64,65,69,70,107,118,119,120,121,-1,-1,-1,160,161,162,163,164,165,166,167,314,242,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/*psy hero*/\
	{1,2,3,30,31,32,33,34,35,36,37,64,65,69,70,107,118,119,120,121,-1,-1,-1,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,313,241,-1},	/*ele hero*/\
};

#define st(str,sta,dex,intl) (str+sta*0x100+dex*0x10000+intl*0x1000000)

unsigned int itemstats1[49]={st(0,0,0,0),
st(1,0,0,0), st(0,0,1,0), st(0,0,0,1), st(0,1,0,0), st(1,1,0,0), st(1,0,1,0), st(0,1,1,0), st(0,1,0,1),
st(2,0,0,0), st(0,0,2,0), st(0,0,0,2), st(0,2,0,0), st(2,1,0,0), st(1,2,0,0), st(2,0,1,0), st(1,0,2,0), st(0,1,2,0), st(0,2,1,0), st(0,1,0,2), st(0,2,0,1),
st(3,0,0,0), st(0,0,3,0), st(0,0,0,3), st(0,3,0,0), st(2,2,0,0), st(1,3,0,0), st(3,2,0,0), st(2,0,2,0), st(3,0,1,0), st(1,0,3,0), st(0,2,2,0), st(0,1,3,0), st(0,3,1,0), st(0,2,0,2), st(0,1,0,3), st(0,3,0,1),
st(4,0,0,0), st(0,0,4,0), st(0,0,0,4), st(0,4,0,0), st(3,2,0,0), st(2,3,0,0), st(3,0,2,0), st(2,0,3,0), st(0,2,3,0), st(0,3,2,0), st(0,2,0,3), st(0,3,0,2)
};

#undef st

int ringbonus1[21]={1,2,3,4,5,6,7,8,9,10,11,13,15,17,19,21,24,27,31,35,40};
int defbonus1[21]={44,49,54,59,64,70,76,82,89,96,104,113,123,135,150,168,189,213,240,270,310};
int atkbonus1[21]={10,15,20,25,30,35,41,48,56,66,77,90,105,125,150,170,200,235,275,320,375};
int hpbonus1[21]={82,118,154,190,226,262,298,334,370,418,466,514,562,610,658,706,766,826,886,946,1018};
int mpbonus1[21]={57,65,73,81,89,97,105,113,121,129,137,146,155,164,173,182,191,200,209,218,227};
int fpbonus1[21]={57,65,73,81,89,97,105,113,121,129,137,146,155,164,173,182,191,200,209,218,227};



int elementl[6]={0,1,2,3,5,4};
int delementl[6]={0,2,3,4,5,1};
int selementl[6]={0,5,1,2,3,4};
int stlk[6]={0, DST_MASTRY_FIRE, DST_MASTRY_WATER, DST_MASTRY_ELECTRICITY, DST_MASTRY_EARTH, DST_MASTRY_WIND};
int dstlk[6]={0, DST_RESIST_FIRE, DST_RESIST_WATER, DST_RESIST_ELECTRICITY, DST_RESIST_EARTH, DST_RESIST_WIND};

int stlknl[6]={0, DST_MASTRY_FIRE, DST_MASTRY_WATER, DST_MASTRY_ELECTRICITY, DST_MASTRY_WIND, DST_MASTRY_EARTH};
int dstlknl[6]={0, DST_RESIST_FIRE, DST_RESIST_WATER, DST_RESIST_ELECTRICITY, DST_RESIST_WIND, DST_RESIST_EARTH};


int petstats1[7]={1,4,2,3,83,26,35};

int petbonus1[7][9]=
{
	{1,2,4,7,11,16,22,29,37},	//str
	{1,2,4,7,11,16,22,29,37},	//sta
	{1,2,4,7,11,16,22,29,37},	//dex
	{1,2,4,7,11,16,22,29,37},	//int
	{5,10,20,35,55,80,110,145,185},	//atk
	{4,8,16,28,44,64,88,116,148},	//def
	{91,182,364,637,1001,1456,2002,2639,3367}		//maxhp
};

int enctable1[11] = {0, 2, 4, 6, 8, 10, 13, 16, 19, 21, 24};
long double hpj1[16]={
		0.8999999761581420900,  // 0.9
		1.5000000000000000000,  // 1.5
		1.3999999761581421000,  // 1.4
		1.3999999761581421000,  // 1.4
		1.3999999761581421000,  // 1.4
		1.6000000238418579000,  // 1.6
		2.0000000000000000000,  // 2.0
		1.6000000238418579000,  // 1.6
		1.6000000238418579000,  // 1.6
		1.6000000238418579000,  // 1.6
		1.6000000238418579000,  // 1.6
		1.7999999523162842000,  // 1.8
		1.5000000000000000000,  // 1.5
		1.5000000000000000000,  // 1.5
		0.6999999880790710400,  // 0.7
		0.6999999880790710400}; // 0.7

long double mpj1[16]={
		0.3000000119209289600,  // 0.3
		0.5000000000000000000,  // 0.5
		0.5000000000000000000,  // 0.5
		1.2999999523162842000,  // 1.3
		1.7000000476837158000,  // 1.7
		0.5000000000000000000,  // 0.5
		0.6000000238418579100,  // 0.6
		0.6000000238418579100,  // 0.6
		0.5000000000000000000,  // 0.5
		0.5000000000000000000,  // 0.5
		1.7999999523162842000,  // 1.8
		1.0000000000000000000,  // 1.0
		2.0000000000000000000,  // 2.0
		2.0000000000000000000,  // 2.0
		1.0000000000000000000,  // 1.0
		0.5000000000000000000}; // 0.5

long double fpj1[16]={
		0.3000000119209289600,  // 0.3
		0.6999999880790710400,  // 0.7
		0.5000000000000000000,  // 0.5
		0.6000000238418579100,  // 0.6
		0.3000000119209289600,  // 0.3
		0.5000000000000000000,  // 0.5
		0.8999999761581420900,  // 0.9
		0.8000000119209289600,  // 0.8
		0.6999999880790710400,  // 0.7
		0.6000000238418579100,  // 0.6
		0.4000000059604644800,  // 0.4
		0.6999999880790710400,  // 0.7
		0.4000000059604644800,  // 0.4
		0.4000000059604644800,  // 0.4
		0.5000000000000000000,  // 0.5
		0.5000000000000000000}; // 0.5

long double stadef1[16]={
1.188235294118,
1.541176470588,
1.576470588235,
1.388235294118,
1.388235294118,
1.388235294118,
1.976470588235,
1.682352941176,
1.776470588235,
1.682352941176,
1.388235294118,
1.882352941176,
1.482352941176,
1.482352941176,
1.482352941176,
1.482352941176};

long double leveldef1[16]={
0.714444444444,
0.714444444444,
0.714444444444,
0.713137254902,
0.713137254902,
0.713137254902,
0.714640522876,
0.714444444444,
0.714444444444,
0.714444444444,
0.713137254902,
0.714444444444,
0.714640522876,
0.714640522876,
0.714640522876,
0.714640522876};

long double defbase1[16]={20,26,24,22,22,22,30,26,27,26,22,29,24,24,24,24};


int petids1[8][3]=
{
	{731,731,731},
	{732,733,884},
	{734,735,885},
	{736,737,886},
	{742,743,889},
	{738,739,887},
	{744,745,890},
	{740,741,888}
};

int skillexp21[20]={0, 40, 100, 138, 194, 266, 346, 430, 856, 1016, 2064, 2272, 3210, 3470, 4690, 5010, 6576, 6990, 9602, 12000};
int skillacc1[20]={0, 40, 140, 278, 472, 738, 1084, 1514, 2370, 3386, 5450, 7722, 10932, 14402, 19092, 24102, 30678, 37668, 47270, 59270};
int job11[32]={0,1,2,3,4,5,1,1,2,2,3,3,4,4,5,5,1,1,2,2,3,3,4,4,1,1,2,2,3,3,4,4};
int job21[32]={0,0,0,0,0,0,6,7,8,9,10,11,12,13,14,15,6,7,8,9,10,11,12,13,6,7,8,9,10,11,12,13};
int bufflist1[]={7,8,9,10,20,46,48,49,50,52,53,108,109,111, 113, 114,115,116, 128,129,130,143,145,146,147,148,150,156,161,165,169, 172,175,178,181,191,192,193, 195,199,207,208,209,210, 221, 309, 310, 311,312,313,314,315,237,239,240,241,242};

//int upgradeChances1[10]={100, 100, 100, 100, 100,100,100,100,100,100};
//int upgradeChancesUlt1[10]={100, 100, 100, 100, 100,100,100,100,100,100};
//int upgradeChancesJewel1[20]={100, 100, 100, 100, 100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100};

int upgradeChances1[10]={95, 90, 80, 70, 60,50,40,30,20,10};
int upgradeChancesUlt1[10]={95, 90, 80, 70, 60,50,40,30,20,10};
int upgradeChancesJewel1[20]={95, 95, 90, 85, 80,75,70,65,60,55,50,45,40,35,30,25,20,15,10,10};

int guildexp1[][2]={
{0,	0 },
{24,	5200 },
{205,	14625 },
{485,	34666 },
{1353,	58035 	},
{2338,	100285 	},
{4547,	139343 	},
{6788,	208000 	},
{11045,	263250 	},
{15151,	361110 	},
{22183,	432575 	},
{28800,	561600 	},
{39340,	649113 	},
{49135,	810727 	},
{63920,	914063 	},
{71608,	1024000 },
{84365,	1064483 },
{91041,	1148727 },
{109698,	1238429 },
{115152,	1300000 },
{134545,	1443000 },
{156813,	1601730 },
{182351,	1777920 },
{211610,	1973491 },
{245099,	2190574 },
{283396,	2431537 },
{327152,	2699005 },
{377106,	2995896 },
{434090,	3325443 },
{499049,	3691241 },
{573046,	4097276 },
{657283,	4547976 },
{753119,	5048252 },
{862086,	5603559 },
{985913,	6219950 },
{1126550,	6904144 },
{1286198,	7663599 },
{1467338,	8506594 },
{1672765,	9442319 },
{1905631,	10480973},
{2169488,	11633879},
{2468335,	12913606},
{2806677,	14334102},
{3189588,	15910852},
{3622778,	17661045},
{4112677,	19603760},
{4666517,	21760172},
{5292439,	24153791},
{5999599,	26810707},
{7075777,	29759885}
};
std::vector2<int> nguildmembers;

unsigned long long expdata1[150]={
14,
20,
36,
90,
152,
250,
352,
480,
591,
743,
973,
1290,
1632,
1928,
2340,
3480,
4125,
4995,
5880,
7840,
6875,
8243,
10380,
13052,
16450,
20700,
26143,
31950,
38640,
57035,
65000,
69125,
72000,
87239,
105863,
128694,
182307,
221450,
269042,
390368,
438550,
458137,
468943,
560177,
669320,
799963,
1115396,
1331100,
1590273,
2306878,
2594255,
2711490,
2777349,
3318059,
3963400,
4735913,
6600425,
7886110,
9421875,
13547310,
15099446,
15644776,
15885934,
18817757,
22280630,
26392968,
36465972,
43184958,
51141217,
73556918,
81991117,
84966758,
86252845,
102171368,
120995493,
143307208,
198000645,
234477760,
277716683,
381795797,
406848219,
403044458,
391191019,
442876559,
501408635,
567694433,
749813704,
849001357,
961154774,
1309582668,
1382799035,
1357505030,
1305632790,
1464862605,
1628695740,
1810772333,
2348583653,
2611145432,
2903009208,
3919352097,
4063358600,
3916810682,
4314535354,
4752892146,
5235785988,
5767741845,
6353744416,
6999284849,
7710412189,
8493790068,
9356759139,
10307405867,
11354638303,
12508269555,
13779109742,
15179067292,
16721260528,
18420140598,
20291626883,
22353256174,
24624347002,
27126180657,
29882200612,
32918232194,
36262724585,
39947017402,
44005634371,
48476606823,
53401830076,
58827456011,
64804325542,
71388445017,
78641511031,
86631488552,
95433247789,
105129265764,
115810399166,
127576735721,
140538532070,
154817246928,
170546679216,
187874221825,
206962242762,
227989606627,
251153350660,
276670531087,
304780257046,
335745931162,
369857717768,
369857717768
};

int pxpdata1[]={1,1,
2,
2,
3,
4,
4,
5,
5,
6,
6,
7,
8,
9,
10,
12,
13,
15,
17,
19,
22,
25,
29,
33,
38,
43,
50,
57,
64,
73,
83,
95,
108,
123,
140,
159,
181,
206,
235,
263,
294,
329,
368,
412,
462,
517,
579,
648,
726,
812,
910,
1018,
1140,
1277,
1430,
1602,
1794,
2009,
2250,
2497,
2772,
3076,
3415,
3790,
4207,
4670,
5183,
5753,
6386,
7088,
7868,
8733,
9694,
10760,
11943,
13256,
14714,
16333,
18129,
20123,
22336,
24793,
27520,
30548,
33908,
37637,
41777,
46372,
51473,
56620,
62282,
68510,
75361,
82897,
90358,
98490,
107354,
117015,
127546,
139025,
150147,
162159,
175132,
189142,
204273,
220615,
238264,
257326,
277912,
300145,
324156,
350089,
378096,
408343,
441011,
476292,
514395,
555547,
599990,
647990,
699829,
755815,
816280,
881583,
952109,
1028278,
1110541,
1199384,
1295334,
1398961,
1510878,
1631748,
1762288,
1903271,
2055533,
2219976,
2397574,
2589380,
2796530,
3020252,
3261873,
3522822,
3804648,
4109020,
4437742,
4792761,
5176182,
5590276,
6037498,
6520498};

int ultimateweap1[50][2]={{22026,	22368},
{22027,	22369},
{22028,	22370},
{22029,	22371},
{22030,	22373},
{22031,	22374},
{22032,	22375},
{22033,	22376},
{22034,	20378},
{22035,	22379},
{22036,	22380},
{22037,	22381},
{22038,	22383},
{22039,	22384},
{22040,	22385},
{22041,	22386},
{22042,	22388},
{22043,	22389},
{22044,	22390},
{22045,	22391},
{22046,	22393},
{22047,	22394},
{22048,	22395},
{22049,	22396},
{22050,	22398},
{22051,	22399},
{22052,	22400},
{22053,	22401},
{22054,	22403},
{22055,	22404},
{22056,	22405},
{22057,	22406},
{22058,	22408},
{22059,	22409},
{22060,	22410},
{22061,	22411},
{22062,	22413},
{22063,	22414},
{22064,	22415},
{22065,	22416},
{22354,	22372},
{22355,	22377},
{22356,	22382},
{22357,	22387},
{22358,	22392},
{22359,	22397},
{22360,	22412},
{22361,	22417},
{22362,	22407},
{22363,	22402}};

//-----

std::vector2<int> upgradeChances;
std::vector2<int> upgradeChancesUlt;
std::vector2<int> upgradeChancesJewel;
std::vector2<std::vector2<int> > ultimateweap;

std::vector2<vector3d<> > towns;
std::vector2<vector3d<> > lodelights;
std::vector2<vector3d<> > gwtele;
std::vector2<int> bufflist;
std::vector2<int> job1;
std::vector2<int> job2;
std::vector2<int> skillexp2;
std::vector2<int> skillacc;
std::vector2<int> petstats;
std::vector2<std::vector2<int> > petbonus;
std::vector2<int> enctable;
std::vector2<long double> hpj;
std::vector2<long double> mpj;
std::vector2<long double> fpj;
std::vector2<long double> stadef;
std::vector2<long double> leveldef;
std::vector2<long double> defbase;
std::vector2<std::vector2<int> > petids;
std::vector2<int> skillsupliment;

std::vector2<unsigned int> itemstats;
std::vector2<int> ringbonus;
std::vector2<int> defbonus;
std::vector2<int> atkbonus;
std::vector2<int> hpbonus;
std::vector2<int> mpbonus;
std::vector2<int> fpbonus;


std::vector2<tguildexp> guildexp;

std::vector2<std::vector2<int> > skillmaxlevel;
std::vector2<std::vector2<int> > skills;
std::vector2<itemdata> itemlist;

std::vector2<int> ultimatestats;

std::vector2<int> uphitbonus;
std::vector2<int> upblockbonus;
std::vector2<int> uphpbonus;
std::vector2<int> upmagicbonus;
std::vector2<int> upallstatbonus;
std::vector2<unsigned long long > explist;
std::vector2<unsigned long long > pxplist;

std::vector2<std::vector2<std::vector2<int> > > ultimatebonus;
std::vector2<std::vector2<tskilldata> > skilllist;
std::vector2<tskilldata > cskilllist;
std::vector2<mobdata2> monsterlist;


int nbufflist=sizeof(bufflist1)/sizeof(int);

void setskillelement(int skill, int e, float d=0.5)
{
	for(int c=0;c<20;c++)
	{
		skilllist[skill][c].element=e;
		skilllist[skill][c].elementval=(int)(c*d);
	}
}

void datainit()
{
	int a,b,c;
	upgradeChances.resize(10);
	upgradeChancesUlt.resize(10);
	upgradeChancesJewel.resize(20);
	ultimateweap.resize(50);
	towns.reserve(3);
	lodelights.reserve(5);
	gwtele.reserve(4);
	job1.resize(32);
	job2.resize(32);
	skillexp2.resize(20);
	skillacc.resize(20);
	petstats.resize(7);
	petbonus.resize(7);
	enctable.resize(11);
	hpj.resize(16);
	mpj.resize(16);
	fpj.resize(16);
	stadef.resize(16);
	leveldef.resize(16);
	defbase.resize(16);
	petids.resize(8);
	itemstats.resize(49);

	ringbonus.resize(21);
	defbonus.resize(21);
	atkbonus.resize(21);
	hpbonus.resize(21);
	mpbonus.resize(21);
	fpbonus.resize(21);

	skillmaxlevel.resize(32);
	skills.resize(32);
	ultimatestats.resize(9);

	uphitbonus.resize(11);
	upblockbonus.resize(11);
	uphpbonus.resize(11);
	upmagicbonus.resize(11);
	upallstatbonus.resize(11);

	explist.resize(151);
	pxplist.resize(151);

	nguildmembers.resize(50);
	for(a=0;a<20;a++)nguildmembers[a]=(a&0xfffffffe)+30;
	for(a=20;a<49;a++)nguildmembers[a]=a+29;
	nguildmembers[49]=50;

	skilllist.resize(character_buffable::maxskillid);
	skillsupliment.resize(character_buffable::maxskillid, 0);
	for(a=0;a<400;a++)skilllist[a].resize(20);
	monsterlist.resize(1000);

	for(b=0;b<32;b++)
	{
		for(a=0;a<45;a++)
		{
			skillmaxlevel[b].push_back(skillmaxlevel1[b][a]);
			skills[b].push_back(skills1[b][a]);
		}
	}

	guildexp.reserve(50);
	for(a=0;a<50;a++)
	{
		guildexp.push_back(tguildexp(guildexp1[a][0], guildexp1[a][1]));
	}

	for(a=0;a<9;a++)ultimatestats[a]=ultimatestats1[a];
	for(a=0;a<11;a++)
	{
		uphitbonus[a]=uphitbonus1[a];
		upblockbonus[a]=upblockbonus1[a];
		uphpbonus[a]=uphpbonus1[a];
		upmagicbonus[a]=upmagicbonus1[a];
		upallstatbonus[a]=upallstatbonus1[a];
	}
	ultimatebonus.resize(9);
	for(c=0;c<9;c++)
	{
		for(b=0;b<5;b++)
		{
			ultimatebonus[c].resize(5);
			for(a=0;a<5;a++)
			{
				ultimatebonus[c][b].push_back(ultimatebonus1[c][b][a]);
			}
		}
	}

	itemdata id1;
	sqlquery s1(connections[0], string("itemlist"));
	s1.select("max(id)");
	if(s1.next())
	{
		b=toInt(s1[0]);
		itemlist.resize(b+1);
	}
	s1.freeup();

	s1.selectw("enabled!=0");
	int c_id=s1.getColumnIndex("id");
	int c_name=s1.getColumnIndex("name");
	int c_itemLV=s1.getColumnIndex("limitlevel1");
	int c_itemjob=s1.getColumnIndex("itemjob");
	int c_abilitymin=s1.getColumnIndex("abilitymin");
	int c_abilitymax=s1.getColumnIndex("abilitymax");
	int c_packmax=s1.getColumnIndex("packmax");
	int c_parts=s1.getColumnIndex("parts");
	int c_partsub=s1.getColumnIndex("partsub");
	int c_cost=s1.getColumnIndex("cost");
	int c_handed=s1.getColumnIndex("handed");
	int c_weapontype=s1.getColumnIndex("weapontype");
	int c_attackspeed=s1.getColumnIndex("attackspeed");
	int c_itemsex=s1.getColumnIndex("itemsex");
	int c_destparam1=s1.getColumnIndex("destparam1");
	int c_destparam2=s1.getColumnIndex("destparam2");
	int c_destparam3=s1.getColumnIndex("destparam3");
	int c_adjparamval1=s1.getColumnIndex("adjparamval1");
	int c_adjparamval2=s1.getColumnIndex("adjparamval2");
	int c_adjparamval3=s1.getColumnIndex("adjparamval3");
	int c_flightspeed=s1.getColumnIndex("flightspeed");
	int c_duration=s1.getColumnIndex("endurance");
	int c_sfx=s1.getColumnIndex("sfxobj3");
	int c_gendrop=s1.getColumnIndex("gendrop");
	logger.log("Item data\n");
	a=0;
	while(s1.next())
	{
		a++;
		id1.id=toInt(s1[c_id]);
		id1.name=s1[c_name];
		id1.name=(char)34 + id1.name + (char)34;
		id1.level=toInt(s1[c_itemLV]);
		if(id1.level>150)id1.level=1;
		id1.job=toInt(s1[c_itemjob]);
		id1.min=toInt(s1[c_abilitymin]);
		id1.max=toInt(s1[c_abilitymax]);
		id1.num=toInt(s1[c_packmax]);
		id1.eqslot=toInt(s1[c_parts]);
		id1.eqslot2=toInt(s1[c_partsub]);
		id1.cost=toInt(s1[c_cost]);
		id1.handed=toInt(s1[c_handed]);
		id1.weapontype=toInt(s1[c_weapontype]);
		id1.aspd=toFloat(s1[c_attackspeed]);
		id1.gender=toInt(s1[c_itemsex]);
		id1.duration=toInt(s1[c_duration]);
		id1.gendrop=toInt(s1[c_gendrop]);

		id1.destparam1=toInt(s1[c_destparam1]);
		id1.destparam2=toInt(s1[c_destparam2]);
		id1.destparam3=toInt(s1[c_destparam3]);
		id1.adjparamval1=toInt(s1[c_adjparamval1]);
		id1.adjparamval2=toInt(s1[c_adjparamval2]);
		id1.adjparamval3=toInt(s1[c_adjparamval3]);
		id1.flightspeed=toFloat(s1[c_flightspeed]);
		id1.sfx=toInt(s1[c_sfx]);
		id1.valid=true;
		itemlist[id1.id]=id1;
	}
	s1.freeup();
	logger.log("Skill data\n");

	itemlist[3204].destparam1=52;
	itemlist[3204].adjparamval1=2;
	itemlist[3207].destparam1=52;
	itemlist[3207].adjparamval1=4;
	itemlist[3208].destparam1=52;
	itemlist[3208].adjparamval1=7;

	itemlist[3209].destparam1=53;
	itemlist[3209].adjparamval1=2;
	itemlist[3212].destparam1=53;
	itemlist[3212].adjparamval1=4;
	itemlist[3213].destparam1=53;
	itemlist[3213].adjparamval1=7;

	itemlist[3214].destparam1=66;
	itemlist[3214].adjparamval1=2;
	itemlist[3217].destparam1=66;
	itemlist[3217].adjparamval1=4;
	itemlist[3218].destparam1=66;
	itemlist[3218].adjparamval1=7;

	itemlist[3219].destparam1=72;
	itemlist[3219].adjparamval1=2;
	itemlist[3222].destparam1=72;
	itemlist[3222].adjparamval1=4;
	itemlist[3223].destparam1=72;
	itemlist[3223].adjparamval1=7;

	itemlist[3224].destparam1=54;
	itemlist[3224].adjparamval1=3;
	itemlist[3227].destparam1=54;
	itemlist[3227].adjparamval1=5;
	itemlist[3228].destparam1=54;
	itemlist[3228].adjparamval1=10;

	itemlist[3229].destparam1=52;
	itemlist[3229].adjparamval1=1;

	itemlist[3240].destparam1=52;
	itemlist[3240].adjparamval1=3;

	itemlist[3241].destparam1=53;
	itemlist[3241].adjparamval1=1;

	itemlist[3242].destparam1=53;
	itemlist[3242].adjparamval1=3;

	itemlist[3237].destparam1=66;
	itemlist[3237].adjparamval1=1;

	itemlist[3238].destparam1=72;
	itemlist[3238].adjparamval1=1;

	itemlist[3243].destparam1=54;
	itemlist[3243].adjparamval1=1;

	itemlist[3244].destparam1=54;
	itemlist[3244].adjparamval1=3;

	itemlist[3245].destparam1=76;
	itemlist[3245].adjparamval1=100;

	itemlist[3246].destparam1=76;
	itemlist[3246].adjparamval1=400;






	itemlist[3251].destparam1=300;
	itemlist[3251].adjparamval1=20;

	itemlist[3252].destparam1=300;
	itemlist[3252].adjparamval1=40;

	itemlist[3253].destparam1=14;
	itemlist[3253].adjparamval1=2;

	itemlist[3254].destparam1=14;
	itemlist[3254].adjparamval1=5;




	itemlist[3521].destparam1=11;
	itemlist[3521].adjparamval1=200;
	itemlist[3522].destparam1=11;
	itemlist[3522].adjparamval1=500;


	itemlist[3523].destparam1=1;
	itemlist[3523].adjparamval1=1;
	itemlist[3524].destparam1=1;
	itemlist[3524].adjparamval1=2;
	itemlist[3525].destparam1=4;
	itemlist[3525].adjparamval1=1;
	itemlist[3526].destparam1=4;
	itemlist[3526].adjparamval1=2;
	itemlist[3527].destparam1=2;
	itemlist[3527].adjparamval1=1;
	itemlist[3528].destparam1=2;
	itemlist[3528].adjparamval1=2;
	itemlist[3529].destparam1=3;
	itemlist[3529].adjparamval1=1;
	itemlist[3530].destparam1=3;
	itemlist[3530].adjparamval1=2;
	itemlist[3523].destparam1=1;
	itemlist[3523].adjparamval1=1;
	itemlist[3524].destparam1=1;
	itemlist[3524].adjparamval1=2;
	itemlist[3531].destparam1=82;
	itemlist[3531].adjparamval1=1;
	itemlist[3532].destparam1=82;
	itemlist[3532].adjparamval1=2;

	sqlquery &s2=dbskilllist2;
	sqlquery s9(connections[0], "skilllist");
	s2.select();
	a=0;
	while(s2.next())
	{
		b=toInt(s2[1]);
		c=toInt(s2[2]);
		--c;
		skilllist[b][c].id=b;
		skilllist[b][c].level=c+1;

		s9.selectw("id="+toString(b), "exetarget, continuouspain, skillready, itemtype, referstat1, referstat2, spelltype, expertmax, refertarget1, refertarget2, refervalue1, refervalue2, itemjob, handed, weapontype");
		if(s9.next())
		{
			skilllist[b][c].skilltype=toInt(s9[0]);
			skilllist[b][c].dot=toInt(s9[1]);
			skilllist[b][c].cooldown=toInt(s9[2]);
			skilllist[b][c].element=0;//toInt(s9[3]);
			skilllist[b][c].element2=0;
			skilllist[b][c].elementval=c*25/toInt(s9[7]);
			skilllist[b][c].elementval2=c*25/toInt(s9[7]);
			skilllist[b][c].rstat1=toInt(s9[4]);
			skilllist[b][c].rstat2=toInt(s9[5]);
			if(skilllist[b][c].rstat1<0)skilllist[b][c].rstat1=0;
			if(skilllist[b][c].rstat2<0)skilllist[b][c].rstat2=0;
			skilllist[b][c].rval1=toInt(s9[10]);
			skilllist[b][c].rval2=toInt(s9[11]);
			skilllist[b][c].rtarget1=toInt(s9[8]);
			skilllist[b][c].rtarget2=toInt(s9[9]);
			skilllist[b][c].playerjob=toInt(s9[12]);
			skilllist[b][c].handed=toInt(s9[13]);
			skilllist[b][c].weapontype=toInt(s9[14]);

			{
				int a=toInt(s9[6]);
				skilllist[b][c].magic=(a>0);

				switch(a)
				{
				case 4:
					skilllist[b][c].element=3;
					break;
				case 5:
					skilllist[b][c].element=1;
					break;
				case 6:
					skilllist[b][c].element=4;	//wind
					break;
				case 7:
					skilllist[b][c].element=2;
					break;
				case 8:
					skilllist[b][c].element=5;	//earth
					break;
				case 11:
					skilllist[b][c].element=1;
					skilllist[b][c].element2=5;	//earth
					break;
				case 12:
					skilllist[b][c].element=4;	//wind
					skilllist[b][c].element2=3;	//
					break;
				case 13:
					skilllist[b][c].element=5;	//earth
					skilllist[b][c].element2=4;	//wind
					break;
				case 14:
					skilllist[b][c].element=5;	//earth
					skilllist[b][c].element2=2;
					break;
				}
			}

		}
		s9.freeup();

		skilllist[b][c].min=toInt(s2[3]);
		skilllist[b][c].max=toInt(s2[4]);
//		skilllist[b][c].val5=toInt(s2[5]);
//		skilllist[b][c].val6=toInt(s2[6]);
//		skilllist[b][c].val7=toInt(s2[7]);
//		skilllist[b][c].val8=toInt(s2[8]);
		skilllist[b][c].prob=toInt(s2[9]);
//		skilllist[b][c].val10=toInt(s2[10]);
//		skilllist[b][c].val11=toInt(s2[11]);
		skilllist[b][c].stat1=toInt(s2[12]);
		skilllist[b][c].stat2=toInt(s2[13]);
		skilllist[b][c].sval1=toInt(s2[14]);
		skilllist[b][c].sval2=toInt(s2[15]);
//		skilllist[b][c].val16=toInt(s2[16]);
//		skilllist[b][c].val17=toInt(s2[17]);
//		skilllist[b][c].val18=toInt(s2[18]);
//		skilllist[b][c].val19=toInt(s2[19]);
//		skilllist[b][c].val20=toInt(s2[20]);
//		skilllist[b][c].val21=toInt(s2[21]);
//		skilllist[b][c].val22=toInt(s2[22]);
//		skilllist[b][c].val23=toInt(s2[23]);
		skilllist[b][c].mpreq=toInt(s2[24]);
		skilllist[b][c].fpreq=toInt(s2[25]);
//		skilllist[b][c].val26=toInt(s2[26]);
		skilllist[b][c].val27=toInt(s2[27]);	//motion time?
		skilllist[b][c].range=toInt(s2[28]);
//		skilllist[b][c].val29=toInt(s2[29]);
//		skilllist[b][c].val30=toInt(s2[30]);
		skilllist[b][c].duration=toInt(s2[31]);
		if(skilllist[b][c].duration<0)skilllist[b][c].duration=0;
//		skilllist[b][c].val32=toInt(s2[32]);
//		skilllist[b][c].val33=toInt(s2[33]);
		skilllist[b][c].pxp=toInt(s2[34]);
		if(skilllist[b][c].pxp<1)skilllist[b][c].pxp=1;
		skilllist[b][c].astime=toInt(s2[35]);
		a++;
	}
	s2.freeup();

	b=139;for(a=0;a<skilllist[b].size();a++)skilllist[b][a].targetless_aoe=true;
	b=151;for(a=0;a<skilllist[b].size();a++)skilllist[b][a].targetless_aoe=true;
	b=158;for(a=0;a<skilllist[b].size();a++)skilllist[b][a].targetless_aoe=true;
	b=168;for(a=0;a<skilllist[b].size();a++)skilllist[b][a].targetless_aoe=true;
	b=173;for(a=0;a<skilllist[b].size();a++)skilllist[b][a].targetless_aoe=true;

	skillsupliment[156]=bpposter;
	skillsupliment[157]=bpposter;
	skillsupliment[158]=bpposter;
	skillsupliment[159]=bpposter;

	skillsupliment[148]=rmposter;
	skillsupliment[149]=rmposter;
	skillsupliment[150]=rmposter;
	skillsupliment[151]=rmposter;

	skillsupliment[194]=arrows;
	skillsupliment[198]=arrows;
	skillsupliment[202]=arrows;
	skillsupliment[196]=arrows;
	skillsupliment[200]=arrows;

	skillsupliment[215]=arrows;
	skillsupliment[216]=arrows;
	skillsupliment[217]=arrows;
	skillsupliment[218]=arrows;
	skillsupliment[219]=arrows;
	skillsupliment[222]=arrows;
	skillsupliment[220]=arrows;


	cskilllist.resize(character_buffable::camm);

	cskilllist[8].stat1=35;
	cskilllist[8].sval1=300;
	cskilllist[9].stat1=52;
	cskilllist[9].sval1=50;
	cskilllist[15].stat1=66;
	cskilllist[15].sval1=20;
	cskilllist[21].stat1=11;
	cskilllist[21].sval1=50;
	cskilllist[21].stat2=68;
	cskilllist[21].sval2=150;

	logger.log("Monster data\n");
	sqlquery &s3=dbmonsterlist;
	s3.select();
	int m_id=s3.getColumnIndex("id");
	int m_str=s3.getColumnIndex("str");
	int m_sta=s3.getColumnIndex("sta");
	int m_dex=s3.getColumnIndex("dex");
	int m_intl=s3.getColumnIndex("intl");
	int m_Level=s3.getColumnIndex("Level");
	int m_AtkMin=s3.getColumnIndex("AtkMin");
	int m_AtkMax=s3.getColumnIndex("AtkMax");
	int m_Size=s3.getColumnIndex("Size");
	int m_ElementalType=s3.getColumnIndex("ElementalType");
	int m_AddHp=s3.getColumnIndex("AddHp");
	int m_HR=s3.getColumnIndex("HR");
	int m_ER=s3.getColumnIndex("ER");
	int m_ExpValue=s3.getColumnIndex("ExpValue");
	int m_FxpValue=s3.getColumnIndex("FxpValue");
	int m_Flying=s3.getColumnIndex("Flying");
	int m_giant=s3.getColumnIndex("giant");
	int m_NeturealArmor=s3.getColumnIndex("NeturealArmor");
	int m_speed=s3.getColumnIndex("Speed");
	int m_atkd=s3.getColumnIndex("ReAttackDelay");
	int m_name=s3.getColumnIndex("name");
	int m_cash=s3.getColumnIndex("Cash");
	int m_relectric=s3.getColumnIndex("ResistElectricity");
	int m_rfire=s3.getColumnIndex("ResistFire");
	int m_rwind=s3.getColumnIndex("ResistWind");
	int m_rwater=s3.getColumnIndex("ResistWater");
	int m_rearth=s3.getColumnIndex("ResistEarth");
	a=0;
	while(s3.next())
	{
		b=toInt(s3[m_id]);
		while((int)monsterlist.size()<=b)monsterlist.push_back(mobdata2());
		monsterlist[b].id=b;
		monsterlist[b].name=s3[m_name];
		monsterlist[b].str=toInt(s3[m_str]);
		monsterlist[b].sta=toInt(s3[m_sta]);
		monsterlist[b].dex=toInt(s3[m_dex]);
		monsterlist[b].intl=toInt(s3[m_intl]);
		monsterlist[b].level=toInt(s3[m_Level]);
		monsterlist[b].atkmin=toInt(s3[m_AtkMin]);
		monsterlist[b].atkmax=toInt(s3[m_AtkMax]);
		monsterlist[b].size=toInt(s3[m_Size]);
		if(monsterlist[b].size<=0)monsterlist[b].size=100;
		monsterlist[b].element=toInt(s3[m_ElementalType]);
		monsterlist[b].hp=toInt(s3[m_AddHp]);
		monsterlist[b].hit=toInt(s3[m_HR]);
		monsterlist[b].flee=toInt(s3[m_ER]);
		monsterlist[b].exp=toInt(s3[m_ExpValue]);
		monsterlist[b].fxp=toInt(s3[m_FxpValue]);
		monsterlist[b].flying=toInt(s3[m_Flying]);
		monsterlist[b].giant=(toInt(s3[m_giant])!=0);
		monsterlist[b].def=toInt(s3[m_NeturealArmor]);
		monsterlist[b].speed=toFloat(s3[m_speed]);
		monsterlist[b].attackdelay=toInt(s3[m_atkd]);
		monsterlist[b].cash=toInt(s3[m_cash]);
		monsterlist[b].resistele[0]=toFloat(s3[m_rfire]);
		monsterlist[b].resistele[1]=toFloat(s3[m_rwater]);
		monsterlist[b].resistele[2]=toFloat(s3[m_relectric]);
		monsterlist[b].resistele[3]=toFloat(s3[m_rwind]);
		monsterlist[b].resistele[4]=toFloat(s3[m_rearth]);
		monsterlist[b].valid=true;
		a++;
	}
	s3.freeup();


	for(std::map<int, tquest*>::iterator questIter=tquest::quests.begin();questIter!=tquest::quests.end();++questIter)
	{
		for(std::map<int, tquest::tquestitems>::iterator questItemsIter=questIter->second->questitems.begin();questItemsIter!=questIter->second->questitems.end();++questItemsIter)
		{
			monsterlist.at(questItemsIter->second.monsterid)
                .quests.push_back( mobdata2::questdata(
                    questItemsIter->second.dropid,
                    questItemsIter->second.number,
                    questItemsIter->second.dropchance,
                    questIter->second) );
		}
		a=0;
		for(std::vector2<std::pair<int, int> >::iterator j=questIter->second->killmobs.begin();j!=questIter->second->killmobs.end();++j)
		{
			monsterlist.at(j->first).quest_killmobs.insert(std::pair<int, std::pair<int, int> >(questIter->second->questid, std::pair<int, int>(j->second, a)));
			a++;
		}
	}

	{
		sqlquery s5(connections[0], "drops");
		s5.select();

		int md_mobid=s5.getColumnIndex("mobid");
		int md_minlvl=s5.getColumnIndex("minlvl");
		int md_maxlvl=s5.getColumnIndex("maxlvl");
		int md_itemid=s5.getColumnIndex("itemid");
		int md_dropchance=s5.getColumnIndex("dropchance");
		int md_min=s5.getColumnIndex("nmin");
		int md_max=s5.getColumnIndex("nmax");

		while(s5.next())
		{
			a=toInt(s5[md_mobid]);
			if(a==-1)
			{
				a=toInt(s5[md_minlvl]);
				b=toInt(s5[md_maxlvl]);
				c=toInt(s5[md_itemid]);
				float d=toFloat(s5[md_dropchance]);
				int e=toInt(s5[md_min]);
				int f=toInt(s5[md_max]);

				for(std::vector2<mobdata2>::iterator i=monsterlist.begin();i!=monsterlist.end();++i)
				{
					if((i->level>=a)&&(i->level<=b))i->drops.push_back(mobdata2::tdrop(c, d, e, f));
				}
			}else
			{
				monsterlist[a].drops.push_back(mobdata2::tdrop(toInt(s5[md_itemid]), toInt(s5[md_dropchance]), toInt(s5[md_min]), toInt(s5[md_max])));
			}
		}
		s5.freeup();
	}
//	logger.log("%d %d\n", itemlist.size(), monsterlist.size());
	c=0;

#ifdef __GenDropsByItemLevel__
{
	logger.log("Drops\n");
	std::vector2<std::list<mobdata2*> > levelmobs;
	levelmobs.resize(explist.size()+1);
	for(a=0;a<(int)monsterlist.size();a++)
	{
		if(monsterlist[a].id>0)
		{
			if((monsterlist[a].level>0)&&(monsterlist[a].level<(int)explist.size()+1))
			{
				levelmobs[monsterlist[a].level].push_back(&monsterlist[a]);
			}
		}
	}

	for(c=0;c<(int)itemlist.size();c++)
	{
		if(itemlist[c].gendrop!=0)
		{
			float dch=0.1f;	//drop chance for regular
			if(c>=22000)dch=0.0001f;	//greens
			int d,e;
			float b;
			if(itemlist[c].gendrop==1)
			{
				d=itemlist[c].level-5;
				e=itemlist[c].level+5;
			}else if(itemlist[c].gendrop==2)
			{
				d=1;
				e=150;
			}
			if(d<1)d=1;
			if(e>(int)explist.size()+1)e=(int)explist.size()+1;
			for(;d<e;d++)
			{
				if(d<levelmobs.size())
				{
					for(std::list<mobdata2*>::iterator j=levelmobs[d].begin();j!=levelmobs[d].end();++j)
					{
						float b=dch;
						if((*j)->giant)b*=5.0f;	//giants
						(*j)->drops.push_back(mobdata2::tdrop(c, b, 1, 1));
					}
				}else break;
			}
		}
	}
}
#endif

#define dmd(a,b) mdrops.push_back(tmdrop(a, b));monsterlist[b].mdrop=a;monsterlist[b+1].mdrop=a;monsterlist[b+2].mdrop=a;monsterlist[b+3].mdrop=a;

	dmd(2950, 20);
	dmd(2951, 96);
	dmd(2952, 24);
	dmd(2953, 28);
	dmd(2954, 88);
	dmd(2955, 32);
	dmd(2956, 36);
	dmd(2957, 60);
	dmd(2958, 40);
	dmd(2959, 44);
	dmd(2960, 48);
	dmd(2969, 120);
	dmd(2962, 68);
	dmd(2961, 64);
	dmd(2975, 552);
	dmd(2976, 400);
	dmd(2963, 92);
	dmd(2977, 528);
	dmd(2978, 544);
	dmd(2964, 103);
	dmd(2968, 84);
	dmd(2972, 56);
	dmd(2965, 72);
	dmd(2966, 80);
	dmd(2979, 540);
	dmd(2973, 107);
	dmd(2980, 508);
	dmd(2981, 548);
	dmd(2982, 504);
	dmd(2996, 622);
	dmd(2971, 112);
	dmd(2983, 512);
	dmd(2997, 646);
	dmd(2984, 404);
	dmd(2985, 516);
	dmd(2998, 618);
	dmd(2986, 568);
	dmd(2987, 500);
	dmd(2999, 602);
	dmd(2988, 536);
	dmd(3000, 638);
	dmd(3001, 630);
	dmd(3002, 598);
	dmd(3003, 594);
	dmd(3004, 626);
	dmd(3005, 606);
	dmd(3006, 642);
	dmd(3007, 614);
	dmd(3008, 650);
	dmd(3009, 610);	//glaph
	dmd(3010, 634);

	dmd(2992,581);
	dmd(2993, 585);
	dmd(2994, 589);

	dmd(2967, 52);
	dmd(2991, 572);
	dmd(2990, 560);
	dmd(2989, 564);


	dmd(5995, 577);


	for(int a=0;a<(int)mdrops.size();a++)
	{
		itemlist[mdrops[a].dropid].mobid=mdrops[a].mobid;
//		monsterlist[mdrops[a].mobid].mdrop=mdrops[a].dropid;
	}
/*
	monsterlist[21].mdrop=2950;mdrops.push_back(tmdrop(2950, 21));
	monsterlist[24].mdrop=2951;
	mdrops.push_back(tmdrop(2951, 24));
	monsterlist[28].mdrop=2952;
	mdrops.push_back(tmdrop(2952, 28));
	monsterlist[32].mdrop=2953;
	mdrops.push_back(tmdrop(2953, 32));
	monsterlist[36].mdrop=2954;
	mdrops.push_back(tmdrop(2954, 36));
*/

#undef dmd
	a=0;
/*
	FILE *f1;
	fopen_s(&f1, "exp.dat", "r+b");
	if(f1!=0){
		fseek(f1, 0, SEEK_END);
		b=ftell(f1);
		fseek(f1, 0, SEEK_SET);
		if(b%8!=0)logger.log("exp.dat migth be corrupted...\n");
		b/=8;
		for(c=0;c<b;c++)
		{
			if((int)explist.size()<c)explist.push_back(0);
			fread(&exp1, 8, 1, f1);
			explist[c]=exp1;
			a++;
		}
		fclose(f1);
	}else logger.elog("Can't open exp.dat!\n");
	logger.log("max level=%d\n", a);
	nexplist=a;
*/
	pxplist.resize(150);
	explist.resize(150);
	for(a=0;a<150;a++)
	{
		pxplist[a]=pxpdata1[a];
		explist[a]=expdata1[a];
	}




	for(a=0;a<49;a++)itemstats[a]=itemstats1[a];
	for(a=0;a<21;a++)
	{
		ringbonus[a]=ringbonus1[a];
		defbonus[a]=defbonus1[a];
		atkbonus[a]=atkbonus1[a];
		hpbonus[a]=hpbonus1[a];
		mpbonus[a]=mpbonus1[a];
		fpbonus[a]=fpbonus1[a];
	}
	for(a=0;a<7;a++)petstats[a]=petstats1[a];
	for(b=0;b<7;b++)
	{
		petbonus[b].resize(9);
		for(a=0;a<9;a++)
		{
			petbonus[b][a]=petbonus1[b][a];
		}
	}
	for(a=0;a<11;a++)enctable[a]=enctable1[a];

	for(a=0;a<16;a++)
	{
		hpj[a]=hpj1[a];
		mpj[a]=mpj1[a];
		fpj[a]=fpj1[a];
		stadef[a]=stadef1[a];
		leveldef[a]=leveldef1[a];
		defbase[a]=defbase1[a];
	}
	for(b=0;b<8;b++)
	{
		petids[b].resize(3);
		for(a=0;a<3;a++)
		{
			petids[b][a]=petids1[b][a];
		}
	}
	for(a=0;a<20;a++)
	{
		skillexp2[a]=skillexp21[a];
		skillacc[a]=skillacc1[a];
		upgradeChancesJewel[a]=upgradeChancesJewel1[a];
	}
	for(a=0;a<32;a++)
	{
		job1[a]=job11[a];
		job2[a]=job21[a];
	}
	nbufflist=(signed)(sizeof(bufflist1)/sizeof(int));
	bufflist.resize(nbufflist);
	for(a=0;a<nbufflist;a++)
	{
		bufflist[a]=bufflist1[a];
	}

	gwtele.push_back(vector3d<>(1265.94f, 83.0f, 1254.35f));
	gwtele.push_back(vector3d<>(1263.26f, 83.0f, 1305.46f));
	gwtele.push_back(vector3d<>(1310.11f, 83.0f, 1303.18f));
	gwtele.push_back(vector3d<>(1314.63f, 83.0f, 1253.43f));

	towns.push_back(vector3d<>(6968.384766f, 100.0f, 3328.863037f));	//flaris
	towns.push_back(vector3d<>(8470.578125f, 100.0f, 3635.788086f));	//sm
	towns.push_back(vector3d<>(3808.401123f,  59.0f, 4455.150879f));

	lodelights.push_back(vector3d<>(6968.384766f, 100.0f, 3328.863037f));	//flaris town
	lodelights.push_back(vector3d<>(7342, 159, 3870));	//flaris
	lodelights.push_back(vector3d<>(8441, 87, 2678));	//sm
	lodelights.push_back(vector3d<>(5514, 75, 3891));	//d1
	lodelights.push_back(vector3d<>(3218, 11, 3410));	//d3
	for(a=0;a<10;a++)
	{
		upgradeChances[a]=upgradeChances1[a];
		upgradeChancesUlt[a]=upgradeChancesUlt1[a];
	}
	for(b=0;b<50;b++)
	{
		ultimateweap[b].resize(2);
		for(a=0;a<2;a++)
		{
			ultimateweap[b][a]=ultimateweap1[b][a];
		}
	}
}


void datafree()
{
}

