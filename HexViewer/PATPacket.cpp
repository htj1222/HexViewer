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
	free(pid_info_);
	Init();
	}
}
void PATPacket::Init()
{
	is_exist_data_ = false;
	
	pointer_field_ = 0;			//8bit
	table_id_ = 0;				//8bit
	section_syntax_indicator_ = 0;	//1bit
	//'0' 1 bslbf
	//reserved 2 bslbf
	section_length_ = 0;			//12bit
	transport_stream_id_ = 0;		//16bit
	//reserved 2 bslbf
	version_number_ = 0;			//5bit
	current_next_indicator_ = 0;	//1bit
	section_number_ = 0;			//8bit
	last_section_number_ = 0;		//8bit
	
	pid_info_ = NULL;

	crc_32_ = 0;					//32bit
}

void PATPacket::SetPos(int pos_input)
{
	pos_=pos_input;
}

void PATPacket::PlusDataPosition(int plus)
{
	pos_ += plus;
}

void PATPacket::PrintPATInfo()
{
	if(is_exist_data_){
	cout << "== PAT packet fields == "<< endl;
	cout << "table_id : " << hex <<(int)table_id_ << dec << endl;
	cout << "section_syntax_indicator : " << section_syntax_indicator_ << endl;
	cout << "section_length : " << section_length_ << endl;
	cout << "transport_stream_id : " << transport_stream_id_ << endl;
	cout << "version_number : " << (int)version_number_ << endl;
	cout << "current_next_indicator : " << current_next_indicator_ << endl;
	cout << "section_number : " << (int)section_number_ << endl;
	cout << "last_section_number : " << (int)last_section_number_ << endl<<endl;
		
	int size = (section_length_-9)/4;
	if(pid_info_ != NULL){
		for(int i=0; i<size; i++)
		{
			cout << "program_number : " << pid_info_[i].program_number_ << endl;

			if(pid_info_[i].program_number_==0)
			{
				cout << "network_PID : " << pid_info_[i].network_pid_ << endl;
			}else{
				cout << "program_map_PID : " << pid_info_[i].program_map_pid_ << endl;
			}
		}
	}
	cout << "CRC_32 : " <<hex<< (crc_32_) << dec<< endl<<endl;
	is_exist_data_ = false;
	
	}
}

void PATPacket::SetHeaderInfo(unsigned char* data)
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

	transport_stream_id_  = (data[pos_  ] ) << 8; //8bit
	transport_stream_id_ += (data[pos_+1] );		//8bit
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
	pid_info_ = (PIDinfo *) malloc(sizeof(PIDinfo) * size);

	for(int i=0; i<size; i++)
	{
		pid_info_[i].program_number_  = (data[pos_]) << 8;
		pid_info_[i].program_number_ += (data[pos_+1]);
		PlusDataPosition(2);//+16bit
		
		//3reserved
		if(pid_info_[i].program_number_==0)
		{
			pid_info_[i].network_pid_  = (data[pos_] & 0x1F) << 8;
			pid_info_[i].network_pid_ += (data[pos_+1]);
			PlusDataPosition(2);
		}else{
			pid_info_[i].program_map_pid_  = (data[pos_] & 0x1F) << 8;
			pid_info_[i].program_map_pid_ += (data[pos_+1]);
			PlusDataPosition(2);
		}
	}

	crc_32_  = (data[pos_  ])<<24;	//8bit
	crc_32_ += (data[pos_+1])<<16;	//8bit
	crc_32_ += (data[pos_+2])<<8;	//8bit
	crc_32_ += (data[pos_+3]);		//8bit
	PlusDataPosition(4);//+32bit
}