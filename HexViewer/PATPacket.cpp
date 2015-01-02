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

	packet_info_buffer_ = "";
}

void PATPacket::SetPos(int pos_input)
{
	pos_=pos_input;
}

void PATPacket::PlusDataPosition(int plus)
{
	pos_ += plus;
}

string PATPacket::GetPacketInfoBuffer()
{
	return packet_info_buffer_;
}

void PATPacket::SetPrintPATInfo()
{
	if(is_exist_data_){
		packet_info_buffer_ += "== PAT packet fields == \n";
		packet_info_buffer_ += "table_id : " +					to_string((long long)(int)table_id_				)+"\n";
		packet_info_buffer_ += "section_syntax_indicator : " +	to_string((long long)section_syntax_indicator_	)+"\n";
		packet_info_buffer_ += "section_length : " +			to_string((long long)section_length_			)+"\n";
		packet_info_buffer_ += "transport_stream_id : " +		to_string((long long)transport_stream_id_		)+"\n";
		packet_info_buffer_ += "version_number : " +			to_string((long long)(int)version_number_		)+"\n";
		packet_info_buffer_ += "current_next_indicator : " +	to_string((long long)current_next_indicator_	)+"\n";
		packet_info_buffer_ += "section_number : " +			to_string((long long)(int)section_number_		)+"\n";
		packet_info_buffer_ += "last_section_number : " +		to_string((long long)(int)last_section_number_	)+"\n\n";

		int size = (section_length_-9)/4;
		if(pid_info_ != NULL){
			for(int i=0; i<size; i++)
			{
				packet_info_buffer_ += "program_number : " + to_string((long long)pid_info_[i].program_number_ )+"\n";

				if(pid_info_[i].program_number_==0)
				{
					packet_info_buffer_ += "network_PID : " + to_string((long long)pid_info_[i].network_pid_ )+"\n";
				}else{
					packet_info_buffer_ += "program_map_PID : " + to_string((long long)pid_info_[i].program_map_pid_ )+"\n";
				}
			}
		}
		packet_info_buffer_ += "CRC_32 : " +to_string((long long)(crc_32_) ) + "\n\n";
		is_exist_data_ = false;
	}else{
		packet_info_buffer_ = "";
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