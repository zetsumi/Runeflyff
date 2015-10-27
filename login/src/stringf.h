#ifndef __stringf_h__
#define __stringf_h__

#include <string>
#include <ctype.h>

std::string toHex(uint32_t a);
std::string toHex(uint64_t a);
std::string toString(uint64_t a);
std::string toString(int32_t a);
std::string toString(int64_t a);
std::string operator+(const char *s, const std::string& s2);
std::string toString(float a);
int toInt(std::string str);
float toFloat(std::string str);
int toInt(char *p);
float toFloat(char *p);
std::string& tostring(std::string &str);
std::string toString(char *p);
int strlen(char *p);
std::string idez(const char *p);
std::string idez(const std::string p);


#endif
