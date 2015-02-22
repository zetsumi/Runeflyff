#ifndef __trade_h__
#define __trade_h__

#include "vector2.h"
#include "pmutex.h"

class tplayer;
class tradeclass
{
	pmutex mutex;
    bool completetrade();
    tplayer *player1, *player2;
    int money1, money2, end, enderid, player1id, player2id;
    bool ok1, ok2, ok3, ok4;
	std::vector2<int> inv1;
	std::vector2<int> inv2;
	std::vector2<int> num1;
	std::vector2<int> num2;
    ~tradeclass();
public:
	void del(tplayer *player);
	void setender(int a, int id)
	{
		ul mm=mutex.lock();
		end=a;
		enderid=id;
	}
    tradeclass(tplayer *p1, tplayer *p2, int, int);
    void setok(tplayer *player);
    bool setok2(tplayer *player);
    void setitem(tplayer *player, int a, int b, int c);
    void setmoney(tplayer *player, int a);
};

#endif
