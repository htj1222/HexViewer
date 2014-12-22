#pragma once

class StreamInfo
{
public:	
	uint8 stream_type;				//8bit
	//reserved 3 bslbf
	uint16 elementary_PID;			//13bit
	//reserved 4 bslbf
	uint16 ES_info_length;			//12bit	
};

class PMTPacket
{
private:
	int pos;

	uint8 pointer_field;			//8bit
	uint8 table_id;					//8bit
	bool section_syntax_indicator;	//1bit
	//'0' 1 bslbf
	//reserved 2 bslbf
	uint16 section_length;			//12bit

	uint16 program_number;			//16bit
	
	//reserved 2 bslbf
	uint8 version_number;			//5bit	
	bool current_next_indicator;	//1bit
	uint8 section_number;			//8bit
	uint8 last_section_number;		//8bit


	//reserved 3 bslbf
	uint16 PCR_PID;//13bit
	//reserved 4 bslbf
	uint16 program_info_length;		//12bit

	StreamInfo* streamInfo;
	
	uint32 CRC_32;					//32bit
public:
	void Init();	
	void SetPos(int pos);
	void PlusDataPosition(int plus);
	void HeaderInfo(int* data);
	void PrintInfo();

	PMTPacket(void);
	~PMTPacket(void);
};

