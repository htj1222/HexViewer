#pragma once
#include "StdAfx.h"
#include "TSPacket.h"

#define CONSOLE_COLS  80//�ܼ�â ����
#define PACKET_SIZE  188//packet������
#define HEX_LINES 12	//����Ҷ��μ�
#define SPACER    15	//�߰��� ����

class PacketAnalyzer
{
private:
	FILE *p_file_;				//���� ������
	char* file_name_;			//���� ��

	TSPacket ts_packet_;		//ts��Ŷ
	
	unsigned char packet_data_[PACKET_SIZE+1];//188�� �ִ� ������

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
