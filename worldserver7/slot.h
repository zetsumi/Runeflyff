#ifndef __slot_h__
#define __slot_h__

#include <string>

class inventory;
class slot
{
	int type2;
	int type;
	int id;
	std::string text;
public:
	slot():id(0),type(0),type2(0){}
	void clear()
	{
		id=0;
		type=0;
		type2=0;
		text="";
	}
	void set(int id, int type, int type2)
	{
		this->id=id;
		this->type=type;
		this->type2=type2;
		text="";
	}

	void set(const std::string &p)
	{
		type=8;
		type2=0;
		id=0;
		text=p;
	}

	int getType(){return type;};
	int getType2(){return type2;};
	int getId(){return id;};

	const std::string& getText(){return text;};
	char* save(int &h, inventory &inv);
	int load(char *p, inventory &inv, int job);

};

#endif