#ifndef __pmutex_h__
#define __pmutex_h__

#include <pthread.h>
class pmutex
{
	pthread_mutex_t mutex;
	pmutex(const pmutex&);
	pmutex& operator=(const pmutex&);
	class unlockerInitializer
	{
		friend class pmutex;
		pmutex *owner;
	public: // private: JDS 2/21/15 - making this public allows to build under Visual Studio 2010+
		unlockerInitializer(const unlockerInitializer &a):owner(a.owner){};
		unlockerInitializer& operator=(const unlockerInitializer &a)
		{
			owner=a.owner;
			return *this;
		}
		unlockerInitializer(pmutex *o):owner(o){}
	};
public:
	pmutex()
	{
		pthread_mutex_init(&mutex, 0);
	}
	~pmutex()
	{
		pthread_mutex_lock(&mutex);
		pthread_mutex_unlock(&mutex);
		pthread_mutex_destroy(&mutex);
	}
	class unlocker
	{
		friend class pmutex;
		pmutex *owner;
		void operator &();
		void operator[](int);
		unlocker& operator=(const unlocker&);
	public:
		unlocker():owner(0){}
		unlocker(const unlockerInitializer& a):owner(a.owner){}
		~unlocker()
		{
			if(owner!=0)owner->unlock();
		}
	};
	unlockerInitializer relockwhen(pmutex &a)
	{
		pthread_mutex_unlock(&mutex);
		{
			unlocker m=a.lock();
			pthread_mutex_lock(&mutex);
		}
		return unlockerInitializer(&a);
	}
	unlockerInitializer lock()
	{
		pthread_mutex_lock(&mutex);
		return unlockerInitializer(this);
	}
	static unlockerInitializer dontlock()
	{
		return unlockerInitializer(0);
	}
	bool trylock(unlocker &a)
	{
		bool r=(pthread_mutex_trylock(&mutex)==0);
		if(r)a.owner=this;
		return r;
	}
/*
	void e_lock()
	{
		pthread_mutex_lock(&mutex);
	}

	void e_unlock()
	{
		pthread_mutex_unlock(&mutex);
	}

	void e_lock(unlocker &u)
	{
		pthread_mutex_lock(&mutex);
		u.owner=this;
	}

	void e_unlock(unlocker &u)
	{
		u.owner=0;
		pthread_mutex_unlock(&mutex);
	}
*/
private:
	friend class unlocker;
	void unlock()
	{
		pthread_mutex_unlock(&mutex);
	}
};
/*
class qmutex:public pmutex
{
	void *locker;
public:
	qmutex():locker(0){}
	unlockerInitializer lock(void *l)
	{
		bool r=(pthread_mutex_trylock(&mutex)==0);
		pmutex *r2=this;
		if(r)
		{
			locker=l;
		}else
		{
			if(l!=locker)
			{
				pthread_mutex_lock(&mutex);
				locker=l;
			}else
			{
				r2=0;
			}
		}
		return unlockerInitializer(r2);
	}
};
*/
typedef pmutex::unlocker ul;
typedef pmutex pm;

template <class T>
class mtype
{
	T val;
	pmutex mutex;
	mtype(const mtype &a);
	mtype<T>& operator=(const mtype &a);
public:
	mtype(){};
	mtype(const T& a)
	{
		ul m=mutex.lock();
		val=a;
	};
	void operator=(const T& a)
	{
		ul m=mutex.lock();
		val=a;
	}
	T operator()()
	{
		ul m=mutex.lock();
		return val;
	}
	ul lock(T *&a)
	{
		ul m=mutex.lock();
		a=&val;
		return m;
	}
};
/*
class refcount
{
	int ref;
	refcount(const refcount &a);
	refcount& operator=(const refcount &a);
public:
	refcount():ref(0){}
	virtual void onref0()=0;
	void inc()
	{
		ref++;
	}
	void dec()
	{
		ref--;
		if(ref<=0)onref0();
	}
};

class mrefcount
{
	int ref;
	pmutex mutex;
	mrefcount(const mrefcount &a);
	mrefcount& operator=(const mrefcount &a);
public:
	mrefcount():ref(0){}
	virtual void onref0()=0;
	void inc()
	{
		ul m=mutex.lock();
		ref++;
	}
	void dec()
	{
		ul m=mutex.lock();
		ref--;
		if(ref==0)onref0();
	}
};

template <class T>
class refpointer
{
	T *p;
public:
	refpointer():p(0){}
	refpointer(const refpointer<T> &a)
	{
		p=a.p;
		if(p!=0)p->inc();
	}
	refpointer(T *a)
	{
		p=a;
		if(p!=0)p->inc();
	}
	refpointer<T>& operator=(const refpointer<T> &a)
	{
		if(&a!=this)
		{
			if(p!=0)p->dec();
			p=a.p;
			if(p!=0)p->inc();
		}
		return *this;
	}
	refpointer<T>& operator=(T *a)
	{
		if(a!=p)
		{
			if(p!=0)p->dec();
			p=a;
			if(p!=0)p->inc();
		}
		return *this;
	}
	~refpointer()
	{
		if(p!=0)p->dec();
	}

	T& operator*()
	{
		return *p;
	}
	T* operator->()
	{
		return p;
	}
	T* operator()()
	{
		return p;
	}
	bool operator==(const refpointer<T> &a) const
	{
		return p==a.p;
	}
	bool operator!=(const refpointer<T> &a) const
	{
		return p!=a.p;
	}
	bool operator==(T *a) const
	{
		return p==a;
	}
	bool operator!=(T *a) const
	{
		return p!=a;
	}
};
*/
#endif
