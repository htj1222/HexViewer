#pragma once
#include "StdAfx.h"
#include "TSPacket.h"

#define CONSOLE_COLS  80//콘솔창 넓이
#define PACKET_SIZE  188//packet사이즈
#define HEX_LINES 12	//출력할라인수
#define SPACER    15	//중간에 띄우기

class PacketAnalyzer
{
private:
	FILE *p_file_;				//파일 포인터
	char* file_name_;			//파일 명

	TSPacket ts_packet_;		//ts패킷
	
	unsigned char packet_data_[PACKET_SIZE+1];//188씩 넣는 데이터

	__int64 current_packet_;
	__int64 total_packet_;

	string send_buffer_;

public:
	PacketAnalyzer(void);
	~PacketAnalyzer(void);

	void FileOpen(char* file_name_);

	void SetTotalPacket();
	__int64 GetFileSize();

	void GetPacketData();
	void GetNextPacketData();
	void GetPreviousPacketData();
	void FindPakcetData(__int64 num);

	void TSPacketDataAnalysis();

	void CheckContinuityCounter();
	void SetPidValueInit();
	void PrintErrorCount();

	void SetPrintHexData();

	void PrintInfo();
	void PrintHex();
	void CloseFile();
	void Reset();

	__int64 GetCurrentPacket();
	__int64 GetTotalPacket();
	string GetSendBuffer();
};
