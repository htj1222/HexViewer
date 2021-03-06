#pragma once
#include "StdAfx.h"


class CCATPacket
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
	//reserved 18 bslbf
	uint8 version_number_;			//5bit
	bool current_next_indicator_;	//1bit
	uint8 section_number_;			//8bit
	uint8 last_section_number_;		//8bit
	
	uint32 crc_32_;					//32bit

	string packet_info_buffer_;

public:
	void Init();	
	void SetPos(int pos);
	void PlusDataPosition(int plus);
	void SetHeaderInfo(unsigned char* data);
	void SetPrintCATInfo();

	string GetPacketInfoBuffer();

	CCATPacket(void);
	~CCATPacket(void);
};

