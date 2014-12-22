#pragma once
class CCATPacket
{
private:
	int pos;
	
	uint8 pointer_field;			//8bit
	uint8 table_id;					//8bit
	bool section_syntax_indicator;	//1bit
	//'0' 1 bslbf
	//reserved 2 bslbf
	uint16 section_length;			//12bit
	//reserved 18 bslbf
	uint8 version_number;			//5bit
	bool current_next_indicator;	//1bit
	uint8 section_number;			//8bit
	uint8 last_section_number;		//8bit
	
	uint32 CRC_32;					//32bit


public:
	void Init();	
	void SetPos(int pos);
	void PlusDataPosition(int plus);
	void HeaderInfo(int* data);
	void PrintInfo();


	CCATPacket(void);
	~CCATPacket(void);
};

