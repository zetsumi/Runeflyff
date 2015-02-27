#ifndef __vector2_h__
#define __vector2_h__

#include <vector>
#include <config.h>

namespace std
{
template <typename T>
class vector2 : private std::vector<T>
{
public:
	vector2(){}
	vector2(size_t a):std::vector<T>::vector(a){}

    typedef templateTypedef std::vector<T>::iterator iterator;
	typedef templateTypedef std::vector<T>::const_iterator const_iterator;
	typedef templateTypedef std::vector<T>::reverse_iterator reverse_iterator;
	typedef templateTypedef std::vector<T>::const_reverse_iterator const_reverse_iterator;
	void resize(size_t a)
	{
		std::vector<T>::resize(a);
	}
	void resize(size_t a, const T& b)
	{
		std::vector<T>::resize(a,b);
	}
	void reserve(size_t a)
	{
		std::vector<T>::reserve(a);
	}
	T& operator[](size_t a)
	{
		return std::vector<T>::at(a);	///**/
	}
	const T& operator[](size_t a) const
	{
		return std::vector<T>::at(a);	///**/
	}
	T& at(size_t a)
	{
		return std::vector<T>::at(a);
	}
	const T& at(size_t a) const
	{
		return std::vector<T>::at(a);
	}
	size_t size() const
	{
		return std::vector<T>::size();
	}
    templateTypedef vector<T>::iterator begin()
	{
		return std::vector<T>::begin();
	}
    templateTypedef vector<T>::const_iterator begin() const
	{
		return std::vector<T>::begin();
	}
    templateTypedef vector<T>::iterator end()
	{
		return std::vector<T>::end();
	}
    templateTypedef vector<T>::const_iterator end() const
	{
		return std::vector<T>::end();
	}
    templateTypedef vector<T>::reverse_iterator rbegin()
	{
		return std::vector<T>::rbegin();
	}
    templateTypedef vector<T>::const_reverse_iterator rbegin() const
	{
		return std::vector<T>::rbegin();
	}
    templateTypedef vector<T>::reverse_iterator rend()
	{
		return std::vector<T>::rend();
	}
    templateTypedef vector<T>::const_reverse_iterator rend() const
	{
		return std::vector<T>::rend();
	}
	void push_back(const T& a)
	{
		std::vector<T>::push_back(a);
	}
	void pop_back()
	{
		std::vector<T>::pop_back();
	}
	bool empty() const
	{
		return std::vector<T>::empty();
	}
	void clear()
	{
		std::vector<T>::clear();
	}
};
}

#endif
