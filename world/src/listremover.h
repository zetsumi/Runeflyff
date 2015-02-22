#ifndef __listremover_h__
#define __listremover_h__


template <class T>
class listremover
{
	typename std::list<T>::iterator i;
	typename std::list<T> *owner;
public:
	listremover():owner(0){}
	~listremover()
	{
		if(owner!=0)remove();
	}
	void set(typename std::list<T> *o, T &a)
	{
		if(owner!=0)remove();
		owner=o;
		owner->push_front(a);
		i=owner->begin();
	}
	void remove()
	{
		if(owner!=0)
		{
			typename std::list<T> *o=owner;
			owner=0;
			o->erase(i);
		}
	}
	void clear()
	{
		owner=0;
	}
};

#endif
