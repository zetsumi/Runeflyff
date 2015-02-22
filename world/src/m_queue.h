#ifndef __m_queue_h__
#define __m_queue_h__

#include <queue>
#include "pmutex.h"

template <class T>
class m_queue:private std::queue<T>
{
	mutable pmutex mutex;
public:
	bool empty() const
	{
		ul m=mutex.lock();
		return std::queue<T>::empty();
	}
	T pop()
	{
		ul m=mutex.lock();
		T r=std::queue<T>::front();
		std::queue<T>::pop();
		return r;
	}
	void push(const T& a)
	{
		ul m=mutex.lock();
		std::queue<T>::push(a);
	}
	size_t size() const
	{
		ul m=mutex.lock();
		return std::queue<T>::size();
	}
};


#endif
