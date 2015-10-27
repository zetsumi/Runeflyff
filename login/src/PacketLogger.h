#pragma once

#include <cstdio>
#include <cstring>
#include <string>

class PacketLogger
{
public:
	enum LogMode {
		BINARY,
		TEXT
	};

	PacketLogger(const std::string& filename, LogMode mode);
	virtual ~PacketLogger(void);

	void logPacket(char *packet, int len = 0);

protected:
	void writeText(char *str, int len);
	void writeBinary(char* str, int len);

private:
	FILE* fh;
	LogMode mode;
};
