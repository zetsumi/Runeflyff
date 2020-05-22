#ifndef __playershop_h__
#define __playershop_h__

#include "platform_threading.h"
#include "vector2.h"
#include <map>
#include <string>

class tplayer;
class buffer;
class playershop
{
	std::mutex shopmutex;
	std::string name;
	std::vector2<unsigned char> itemek;
	std::vector2<int> nitemek;
	std::vector2<int> price;
	bool opened;
	tplayer *owner;
	std::map<int, tplayer*> players;
	void shopmulticast(buffer &bs, tplayer *player);
	void shopmulticast2(buffer &bs, tplayer *player);
	int nplayers;
public:
	bool getopened()
	{
		return opened;
	}
	const std::string& getname()
	{
		return name;
	}
	void additemtoshop(int a, int b, int c, int d);
	bool removeitem(int a);
	bool buyfromshop(int b, tplayer *player, int c, int d	);
	void talk(const std::string &p, tplayer *player);
	void login(tplayer *p);
	void logout(tplayer *p);
	playershop(tplayer *o);
	void open(const std::string& n);
	void close(tplayer *player);
};

#endif
