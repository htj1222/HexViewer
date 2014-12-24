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

	pointer_field=0;
	table_id=0;					//8bit
	section_syntax_indicator=0;	//1bit
	//'0' 1 bslbf
	//reserved 2 bslbf
	section_length=0;			//12bit	
	//reserved 18 bslbf
	version_number=0;			//5bit
	current_next_indicator=0;	//1bit
	section_number=0;			//8bit
	last_section_number=0;		//8bit
	
	CRC_32=0;					//32bit
}

void CCATPacket::SetPos(int pos_input)
{
	pos=pos_input;
}

void CCATPacket::PlusDataPosition(int plus)
{
	pos += plus;
}

void CCATPacket::PrintCATInfo()
{
	if(is_exist_data_){
	cout << "== CAT packet fields == "<< endl;
	cout << "table_id : " << hex <<(int)table_id << dec << endl;
	cout << "section_syntax_indicator : " << section_syntax_indicator << endl;
	cout << "section_length : " << section_length << endl;
	cout << "version_number : " << (int)version_number << endl;
	cout << "current_next_indicator : " << current_next_indicator << endl;
	cout << "section_number : " << (int)section_number << endl;
	cout << "last_section_number : " << (int)last_section_number << endl<<endl;
		
	int size = (section_length-9)/4;
	{
		for(int i=0; i<size; i++)
		{
		}
	}
	cout << "CRC_32 : " <<hex<< (CRC_32) << dec<< endl<<endl;
	is_exist_data_ = false;
	}
}


void CCATPacket::HeaderInfo(unsigned char* data)
{
	is_exist_data_ = true;

	pointer_field = data[pos];	//8bit
	PlusDataPosition(1);

	table_id = data[pos];	//8bit
	PlusDataPosition(1);

	section_syntax_indicator = (data[pos] & 0x80) >> 7;	//1bit
	//'0'
	//2 reserved
	section_length  = (data[pos] & 0x03  ) << 8; //4bit¿¡¼­ 2bit´Â 00
	section_length += (data[pos+1]		 );		 //8bit
	PlusDataPosition(2);//+16bit

	//16bit reserved
	PlusDataPosition(2);//+16bit

	//2 reserved
	version_number			= (data[pos] & 0x3E  ) >> 1;//5bit
	current_next_indicator	= (data[pos] & 0x01  );		//1bit
	PlusDataPosition(1);//+8bit

	section_number = (data[pos]);
	PlusDataPosition(1);//+8bit

	last_section_number = (data[pos]);
	PlusDataPosition(1);//+8bit

	int size = (section_length-9)/4;

	for(int i=0; i<size; i++)
	{
		//descriptor();
	}

	CRC_32  = (data[pos  ])<<24;	//8bit
	CRC_32 += (data[pos+1])<<16;	//8bit
	CRC_32 += (data[pos+2])<<8;		//8bit
	CRC_32 += (data[pos+3]);		//8bit
	PlusDataPosition(4);//+32bit

	//PrintInfo();
}