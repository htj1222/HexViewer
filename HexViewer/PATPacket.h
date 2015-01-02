#pragma once
#include "StdAfx.h"

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned int   uint32;
typedef unsigned long  uint64;


class PIDinfo
{
public:
	uint16 program_number_;			//16bit
	uint16 network_pid_;				//13bit
	uint16 program_map_pid_;			//13bit
};


class PATPacket
{
private:
	bool is_exist_data_;		//데이터 존재 유무 저장

	int pos_;

	uint8 pointer_field_;			//8bit
	uint8 table_id_;				//8bit
	bool section_syntax_indicator_;	//1bit
	//'0' 1 bslbf
	//reserved 2 bslbf
	uint16 section_length_;			//12bit
	uint16 transport_stream_id_;		//16bit
	//reserved 2 bslbf
	uint8 version_number_;			//5bit
	bool current_next_indicator_;	//1bit
	uint8 section_number_;			//8bit
	uint8 last_section_number_;		//8bit
	
	PIDinfo* pid_info_;

	uint32 crc_32_;					//32bit

	string packet_info_buffer_;

public:
	void Init();	
	void SetPos(int pos);
	void PlusDataPosition(int plus);
	void SetHeaderInfo(unsigned char* data);
	void SetPrintPATInfo();
	void Reset();

	string GetPacketInfoBuffer();

	PATPacket(void);
	~PATPacket(void);
};

