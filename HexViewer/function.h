#include "TSPacket.h"
#pragma once


class function
{
private :
	
public:
	TSPacket packetInfo;
	function(void);
	~function(void);
	__int64 getPacketCount(FILE *pSrc, char* fileName);
	void printHex(FILE *pSrc, long long page);
	void GetPacketData(FILE *pSrc, int* packetData);
	void printInfo();
};
