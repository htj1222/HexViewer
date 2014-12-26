
#pragma once
#include "StdAfx.h"
#include "TSPacket.h"


#define CONSOLE_COLS  80//콘솔창 넓이
#define PACKET_SIZE  188//packet사이즈
#define HEX_LINES 12	//출력할라인수
#define SPACER    15	//중간에 띄우기

class FileDescriptor
{
private:
	FILE *p_file_;			//파일 포인터
	char* file_name_;			//파일 명

	TSPacket ts_packet_; //ts
	
	unsigned char packet_data_[PACKET_SIZE+1];

public:
	FileDescriptor(void);
	~FileDescriptor(void);

	void FileOpen();

	__int64 GetPacketCount();
	__int64 GetFileSize();
	void GetPacketData(long long page);
	void TSPacketDataAnalysis();

	void CheckContinuityCounter();
	void SetPidValueInit();
	void PrintErrorCount();

	void PrintInfo();
	void PrintHex();
	void CloseFile();
	void Reset();
};
