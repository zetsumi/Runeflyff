#ifndef __scheduler_h__
#define __scheduler_h__

extern const int desiredlatency;
#include <list>
#include "vector2.h"

class error;

template <typename T>
class schedulerl
{
	long long nexttime, interval;
	std::vector2<std::list<T> > cont;
	int act;
	int n;
	bool (*f)(T);
	int iterating;
	bool deleted;
	typename std::list<T>::iterator itt;
public:
	typedef typename std::list<T>::iterator listIteratorT;
	//typedef typename std::list<T>::const_iterator listIteratorT_const;
	class sremover
	{
		friend class schedulerl<T>;
		schedulerl<T> *owner;
		listIteratorT remover;
		int n;
		int c;
		void set(int a, listIteratorT i, schedulerl<T> *o)
		{
//#ifdef _DEBUG
			if(owner!=0)throw error("wasn't removed", "scheduler sremover set");
//#endif
			remover=i;
			c=a;
			owner=o;
		}
	sremover(const sremover&);
	sremover& operator=(const sremover&);
	public:
		bool isset()
		{
			return owner!=0;
		}
		void clear()
		{
			owner=0;
		}
		sremover()
		{
			c=-1;
			owner=0;
		}
		void remove()
		{
			if(owner!=0)
			{
				owner->remove(*this);
				owner=0;
			}
		}
		~sremover()
		{
			if(owner!=0)owner->remove(*this);
			owner=0;
		}
	};
	schedulerl(long long i, bool (*g)(T))
	:interval(i),act(0),iterating(-1),nexttime(0),f(g)
	{
		n=(int)(interval/(const long long)desiredlatency);
		interval/=n;
		cont.resize(n);
	}
	void run(long long ido)
	{
//		if(ido>=nexttime)
		{
			nexttime=ido+interval;
			iterating=act;
			for(itt=cont[act].begin();itt!=cont[act].end();++itt)
			{
				deleted=false;
				if(f(*itt))
				{
					if(!deleted)itt=cont[act].erase(itt);
				}
				if(itt==cont[act].end())break;
			}
			iterating=-1;
			act++;
			if(act>=n)act=0;
		}
	}
	void rund(long long ido)
	{
		if(ido>=nexttime)
		{
			nexttime=ido+interval;
			iterating=act;
			for(itt=cont[act].begin();itt!=cont[act].end();++itt)
			{
				deleted=false;
				T v=*itt;
				if(f(*itt))
				{
					if(!deleted)
					{
						delete *itt;
						itt=cont[act].erase(itt);
					}else delete v;
				}
				if(itt==cont[act].end())break;
			}
			iterating=-1;
			act++;
			if(act>=n)act=0;
		}
	}

	void insert(int id, T p, sremover &i)
	{
		int a=id%n;
		if(i.owner!=this)
		{
			cont[a].push_front(p);
			i.set(a, cont[a].begin(), this);
		}
	}
	void remove(sremover &i)
	{
		int a=i.c;
		if(i.owner!=this)return;
//#ifdef _DEBUG
//		if(i.owner!=this)throw "remover error";
//#endif
		if(a!=iterating)cont[a].erase(i.remover);
		else
		{
			if(itt==i.remover)
			{
				itt=cont[a].erase(itt);
				deleted=true;
			}else
			{
				cont[a].erase(i.remover);
			}
		}
		i.clear();
	}
};

#endif
