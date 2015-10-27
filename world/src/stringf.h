#ifndef __stringf_h__
#define __stringf_h__

#include <string>
#include <ctype.h>

std::string toHex(uint32_t a);
std::string toHex(uint64_t a);
std::string toString(uint32_t a);
std::string toString(uint64_t a);
std::string toString(int64_t a);
std::string operator+(const char *s, const std::string& s2);
std::string toString(int32_t a);
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
