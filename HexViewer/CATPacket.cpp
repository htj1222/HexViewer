#include "StdAfx.h"
#include "CATPacket.h"


CCATPacket::CCATPacket(void)
{
	Init();
}

CCATPacket::~CCATPacket(void)
{
}

void CCATPacket::Init()
{
	is_exist_data_ = false;

	pointer_field_=0;
	table_id_=0;					//8bit
	section_syntax_indicator_=0;	//1bit
	//'0' 1 bslbf
	//reserved 2 bslbf
	section_length_=0;			//12bit	
	//reserved 18 bslbf
	version_number_=0;			//5bit
	current_next_indicator_=0;	//1bit
	section_number_=0;			//8bit
	last_section_number_=0;		//8bit
	
	crc_32_=0;					//32bit
}

void CCATPacket::SetPos(int pos_input)
{
	pos_=pos_input;
}

void CCATPacket::PlusDataPosition(int plus)
{
	pos_ += plus;
}

void CCATPacket::PrintCATInfo()
{
	if(is_exist_data_){
	cout << "== CAT packet fields == "<< endl;
	cout << "table_id : " << hex <<(int)table_id_ << dec << endl;
	cout << "section_syntax_indicator : " << section_syntax_indicator_ << endl;
	cout << "section_length : " << section_length_ << endl;
	cout << "version_number : " << (int)version_number_ << endl;
	cout << "current_next_indicator : " << current_next_indicator_ << endl;
	cout << "section_number : " << (int)section_number_ << endl;
	cout << "last_section_number : " << (int)last_section_number_ << endl<<endl;
		
	int size = (section_length_-9)/4;
	{
		for(int i=0; i<size; i++)
		{
		}
	}
	cout << "CRC_32 : " <<hex<< (crc_32_) << dec<< endl<<endl;
	is_exist_data_ = false;
	}
}


void CCATPacket::SetHeaderInfo(unsigned char* data)
{
	is_exist_data_ = true;

	pointer_field_ = data[pos_];	//8bit
	PlusDataPosition(1);

	table_id_ = data[pos_];	//8bit
	PlusDataPosition(1);

	section_syntax_indicator_ = (data[pos_] & 0x80) >> 7;	//1bit
	//'0'
	//2 reserved
	section_length_  = (data[pos_] & 0x03  ) << 8; //4bit¿¡¼­ 2bit´Â 00
	section_length_ += (data[pos_+1]		 );		 //8bit
	PlusDataPosition(2);//+16bit

	//16bit reserved
	PlusDataPosition(2);//+16bit

	//2 reserved
	version_number_			= (data[pos_] & 0x3E  ) >> 1;//5bit
	current_next_indicator_	= (data[pos_] & 0x01  );		//1bit
	PlusDataPosition(1);//+8bit

	section_number_ = (data[pos_]);
	PlusDataPosition(1);//+8bit

	last_section_number_ = (data[pos_]);
	PlusDataPosition(1);//+8bit

	int size = (section_length_-9)/4;

	for(int i=0; i<size; i++)
	{
		//descriptor();
	}

	crc_32_  = (data[pos_  ])<<24;	//8bit
	crc_32_ += (data[pos_+1])<<16;	//8bit
	crc_32_ += (data[pos_+2])<<8;		//8bit
	crc_32_ += (data[pos_+3]);		//8bit
	PlusDataPosition(4);//+32bit

	//PrintInfo();
}