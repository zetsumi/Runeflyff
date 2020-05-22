#ifndef __ticket_h__
#define __ticket_h__


#include "vector2.h"
#include <list>
#include "platform_threading.h"

template <class T>
class tplayercont
{
	std::mutex mutex;
private:
	friend class tplayer;
	friend class cluster;
	std::vector2<T> players;
	std::list<int> tickets;

public:
	bool hasFree()
	{
	    std::lock_guard<std::mutex> guard(this->mutex);
		return !tickets.empty();
	}
	int getTicket()
	{
        std::lock_guard<std::mutex> guard(this->mutex);
		int a=tickets.front();
		tickets.pop_front();
		return a;
	}
	void releaseTicket(int a)
	{
        std::lock_guard<std::mutex> guard(this->mutex);
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
