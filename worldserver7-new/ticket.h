#ifndef __ticket_h__
#define __ticket_h__


#include "vector2.h"
#include <list>
#include "pmutex.h"

template <class T>
class tplayercont
{
	pmutex mutex;
private:
	friend class tplayer;
	friend class cluster;
	std::vector2<T> players;
	std::list<int> tickets;

public:
	bool hasFree()
	{
		ul m=mutex.lock();
		return !tickets.empty();
	}
	int getTicket()
	{
		ul m=mutex.lock();
		int a=tickets.front();
		tickets.pop_front();
		return a;
	}
	void releaseTicket(int a)
	{
		ul m=mutex.lock();
		tickets.push_front(a);
	}
	tplayercont(int n)
	{
		int a;
		for(a=1;a<n;a++)tickets.push_front(a);
		players.resize(n);
	}
	T& operator[](int a)
	{
		return players.at(a);
	}
};

#endif
