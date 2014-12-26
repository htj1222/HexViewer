
#pragma once
#include "StdAfx.h"
#include "TSPacket.h"


#define CONSOLE_COLS  80//�ܼ�â ����
#define PACKET_SIZE  188//packet������
#define HEX_LINES 12	//����Ҷ��μ�
#define SPACER    15	//�߰��� ����

class FileDescriptor
{
private:
	FILE *p_file_;			//���� ������
	char* file_name_;			//���� ��

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
