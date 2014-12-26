#include "StdAfx.h"
#include "PATPacket.h"


PATPacket::PATPacket(void)
{
	Init();
}


PATPacket::~PATPacket(void)
{
}

void PATPacket::Reset()
{
	if(is_exist_data_){
	free(pidInfo);
	Init();
	}
}
void PATPacket::Init()
{
	is_exist_data_ = false;

	pointer_field=0;
	table_id=0;					//8bit
	section_syntax_indicator=0;	//1bit
	//'0' 1 bslbf
	//reserved 2 bslbf
	section_length=0;			//12bit
	transport_stream_id=0;		//16bit
	//reserved 2 bslbf
	version_number=0;			//5bit
	current_next_indicator=0;	//1bit
	section_number=0;			//8bit
	last_section_number=0;		//8bit
	
	pidInfo=NULL;
	
	CRC_32=0;					//32bit
}

void PATPacket::SetPos(int pos_input)
{
	pos=pos_input;
}

void PATPacket::PlusDataPosition(int plus)
{
	pos += plus;
}

void PATPacket::PrintPATInfo()
{
	if(is_exist_data_){
	cout << "== PAT packet fields == "<< endl;
	cout << "table_id : " << hex <<(int)table_id << dec << endl;
	cout << "section_syntax_indicator : " << section_syntax_indicator << endl;
	cout << "section_length : " << section_length << endl;
	cout << "transport_stream_id : " << transport_stream_id << endl;
	cout << "version_number : " << (int)version_number << endl;
	cout << "current_next_indicator : " << current_next_indicator << endl;
	cout << "section_number : " << (int)section_number << endl;
	cout << "last_section_number : " << (int)last_section_number << endl<<endl;
		
	int size = (section_length-9)/4;
	if(pidInfo != NULL){
		for(int i=0; i<size; i++)
		{
			cout << "program_number : " << pidInfo[i].program_number << endl;

			if(pidInfo[i].program_number==0)
			{
				cout << "network_PID : " << pidInfo[i].network_PID << endl;
			}else{
				cout << "program_map_PID : " << pidInfo[i].program_map_PID << endl;
			}
		}
	}
	cout << "CRC_32 : " <<hex<< (CRC_32) << dec<< endl<<endl;
	is_exist_data_ = false;
	
	}
}

void PATPacket::SetHeaderInfo(unsigned char* data)
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

	transport_stream_id  = (data[pos  ] ) << 8; //8bit
	transport_stream_id += (data[pos+1] );		//8bit
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
	pidInfo = (PIDinfo *) malloc(sizeof(PIDinfo) * size);

	for(int i=0; i<size; i++)
	{
		pidInfo[i].program_number  = (data[pos]) << 8;
		pidInfo[i].program_number += (data[pos+1]);
		PlusDataPosition(2);//+16bit
		
		//3reserved
		if(pidInfo[i].program_number==0)
		{
			pidInfo[i].network_PID  = (data[pos] & 0x1F) << 8;
			pidInfo[i].network_PID += (data[pos+1]);
			PlusDataPosition(2);
		}else{
			pidInfo[i].program_map_PID  = (data[pos] & 0x1F) << 8;
			pidInfo[i].program_map_PID += (data[pos+1]);
			PlusDataPosition(2);
		}
	}

	CRC_32  = (data[pos  ])<<24;	//8bit
	CRC_32 += (data[pos+1])<<16;	//8bit
	CRC_32 += (data[pos+2])<<8;		//8bit
	CRC_32 += (data[pos+3]);		//8bit
	PlusDataPosition(4);//+32bit
}