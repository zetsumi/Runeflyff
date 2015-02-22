#ifndef __stringf_h__
#define __stringf_h__

#include <string>

std::string toHex(unsigned int a);
std::string toHex(unsigned long long a);
std::string toString(unsigned long long a);
std::string toString(unsigned int a);
std::string toString(long long a);
std::string operator+(const char *s, const std::string& s2);
std::string toString(int a);
std::string toString(float a);
int toInt(const std::string &str);
float toFloat(const std::string &str);
int toInt(const char *p);
float toFloat(const char *p);
unsigned long long toUnsignedLongLong(const std::string &str);
std::string& tostring(std::string &str);
std::string toString(const char *p);
//int strlen(const char *p);
std::string catclaws(const std::string &p);

#endif
