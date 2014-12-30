#pragma once
#include "StdAfx.h"

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned int   uint32;
typedef unsigned long  uint64;


class StreamInfo
{
public:	
	uint8 stream_type_;				//8bit
	//reserved 3 bslbf
	uint16 elementary_pid_;			//13bit
	//reserved 4 bslbf
	uint16 es_info_length_;			//12bit	
};

class PMTPacket
{

public:
	bool is_exist_data_;		//데이터 존재 유무 저장

	int pos_;

	uint8 pointer_field_;			//8bit
	uint8 table_id_;					//8bit
	bool section_syntax_indicator_;	//1bit
	//'0' 1 bslbf
	//reserved 2 bslbf
	uint16 section_length_;			//12bit

	uint16 program_number_;			//16bit
	
	//reserved 2 bslbf
	uint8 version_number_;			//5bit	
	bool current_next_indicator_;	//1bit
	uint8 section_number_;			//8bit
	uint8 last_section_number_;		//8bit


	//reserved 3 bslbf
	uint16 pcr_pid_;//13bit
	//reserved 4 bslbf
	uint16 program_info_length_;		//12bit

	StreamInfo* stream_info_;
	int stream_info_size_;

	uint32 crc_32_;					//32bit


	void Init();	
	void SetPos(int pos);
	void PlusDataPosition(int plus);
	void SetHeaderInfo(unsigned char* data);
	void PrintPMTInfo();
	void Reset();
	bool isAudioStreamType(uint8 stream_type);
	bool isVideoStreamType(uint8 stream_type);
	PMTPacket(void);
	~PMTPacket(void);
};

