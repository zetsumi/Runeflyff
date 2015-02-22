#ifndef __stringf_h__
#define __stringf_h__

#include <string>

std::string toHex(unsigned int a);
std::string toHex(unsigned long long a);
std::string toString(unsigned long long a);
std::string toString(long long a);
std::string operator+(const char *s, const std::string& s2);
std::string toString(int a);
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
