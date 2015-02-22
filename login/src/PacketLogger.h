#pragma once

#include <cstdio>
#include <cstring>

class PacketLogger
{
public:
	enum LogMode {
		BINARY,
		TEXT
	};

	PacketLogger(char* filename, LogMode mode);
	virtual ~PacketLogger(void);

	void logPacket(char *packet, int len = 0);

protected:
	void writeText(char *str, int len);
	void writeBinary(char* str, int len);

private:
	FILE* fh;
	LogMode mode;
};
