#ifndef __error_h__
#define __error_h__

#include <string>
#include "stringf.h"
#include <stdexcept>

class error : public std::exception
{
	std::string ab;
public:
	error(const std::string &a1, const std::string &b1)
	{
		ab=a1+b1;
	}
	error(const error& e)
	{
		ab=e.ab;
	}
	error& operator=(const error& e)
	{
		if(this!=&e)ab=e.ab;
	}

	virtual const char* what() const throw()
	{
		return ab.c_str();
	}

	virtual ~error() throw() {
	}
};

#endif
