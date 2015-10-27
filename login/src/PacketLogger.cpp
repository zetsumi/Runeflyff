#include "PacketLogger.h"

PacketLogger::PacketLogger(const std::string& filename, LogMode mode) : mode(mode)
{
	if(mode == BINARY)
		fh = fopen(filename.c_str(), "wb");
	else
		fh = fopen(filename.c_str(), "w");
}

PacketLogger::~PacketLogger(void)
{
	fclose(fh);
}

void PacketLogger::logPacket(char *packet, int len /*= 0*/)
{
	if(mode == BINARY) {
		writeBinary(packet, len);
	}
	else {
		writeText(packet, len);
	}
	fflush(fh);
}

void PacketLogger::writeText(char *str, int len)
{
	if(!len) {
		len = strlen(str);
	}

	fprintf(fh, "Packet START");

	int count = 1;
	for(int x = 0; x < len; x++) {
		if(count == 1 || count % 16 == 0) {
			fprintf(fh, "\n%02x: ", count);
		}
		fprintf(fh, "%02x ", (int)str[x]);
		count++;
	}

	fprintf(fh, "\nPacket END\n\n");
}

void PacketLogger::writeBinary(char* str, int len)
{
	fwrite(str, sizeof(char), len, fh);
}
