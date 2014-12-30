#include "StdAfx.h"
#include "PMTPacket.h"



PMTPacket::PMTPacket(void)
{
	Init();
}


PMTPacket::~PMTPacket(void)
{
}

void PMTPacket::Reset()
{
	if(is_exist_data_){
	free(stream_info_);
	Init();
	}
}

void PMTPacket::Init()
{
	is_exist_data_ = false;

	pointer_field_ = 0;			//8bit
	table_id_ = 0;					//8bit
	section_syntax_indicator_ = 0;	//1bit
	//'0' 1 bslbf
	//reserved 2 bslbf
	section_length_ = 0;			//12bit

	program_number_ = 0;			//16bit
	
	//reserved 2 bslbf
	version_number_ = 0;			//5bit	
	current_next_indicator_ = 0;	//1bit
	section_number_ = 0;			//8bit
	last_section_number_ = 0;		//8bit


	//reserved 3 bslbf
	pcr_pid_ = 0;//13bit
	//reserved 4 bslbf
	program_info_length_ = 0;		//12bit

	stream_info_ = NULL;
	stream_info_size_ = 0;

	crc_32_ = 0;					//32bit
}

void PMTPacket::SetPos(int pos_input)
{
	pos_=pos_input;
}

void PMTPacket::PlusDataPosition(int plus)
{
	pos_ += plus;
}

bool PMTPacket::isAudioStreamType(uint8 stream_type)
{
	switch(stream_type)
	{
	case 0x03://ISO/IEC 11172-3 Audio
	case 0x04://ISO/IEC 13818-3 Audio
	case 0x0F://ISO/IEC 13818-7 Audio with ADTS transport syntax(AAC_AUDIO)
	case 0x11://ISO/IEC 14496-3 Audio with the LATM transport syntax as defined in ISO/IEC 14496-3(MPEG4_AUDIO)
	case 0x1C://ISO/IEC 14496-3 Audio, without using any additional transport syntax, such as DST, ALS and SLS
	case 0x81://(AC3_AUDIO)
	case 0x82://(DTS_AUDIO)
		return true;
	default:return false;
	}
}

bool PMTPacket::isVideoStreamType(uint8 stream_type)
{
	switch(stream_type)
	{
	case 0x01://ISO/IEC 11172-2 Video
	case 0x02://ITU-T Rec. H.262 | ISO/IEC 13818-2 Video or ISO/IEC 11172-2 constrained parameter video stream
	//case 0x10://ISO/IEC 14496-2 Visual
	case 0x1B://AVC video stream as defined in ITU-T Rec. H.264 | ISO/IEC 14496-10 Video(H264_VIDEO)
	case 0x1E://Auxiliary video stream as defined in ISO/IEC 23002-3
	case 0x42://(AVS_VIDEO)
		return true;
	default:return false;
	}
}

void PMTPacket::PrintPMTInfo()
{
	if(is_exist_data_){
	cout << "== PMT packet fields == "<< endl;
	cout << "table_id : "					<< hex <<(int)table_id_ << dec		<< endl;
	cout << "section_syntax_indicator : "	<< section_syntax_indicator_			<< endl;
	cout << "section_length : "				<< (int)section_length_				<< endl;
	cout << "program_number : "				<< (int)program_number_				<< endl;
	cout << "version_number : "				<< (int)version_number_				<< endl;
	cout << "current_next_indicator : "		<< current_next_indicator_			<< endl;
	cout << "section_number : "				<< (int)section_number_				<< endl;
	cout << "last_section_number : "		<< (int)last_section_number_			<< endl<<endl;
	
	cout << "PCR_PID : "					<< (int)pcr_pid_						<< endl;
	cout << "program_info_length : "		<< (int)program_info_length_			<< endl << endl;
		
	for(int i=0; i<stream_info_size_; i++)
	{
		cout << "stream_type : "				<< (int)stream_info_[i].stream_type_;
		if(isAudioStreamType(stream_info_[i].stream_type_)){
			cout << "(is Audio Stream)"			<<endl;
		}
		else if(isVideoStreamType(stream_info_[i].stream_type_)){
			cout << "(is Video Stream)"			<<endl;
		}else{
			cout << "(?)"			<<endl;
		}
		cout << "elementary_PID : "				<< (int)stream_info_[i].elementary_pid_		 <<endl;
		cout << "ES_info_length : "				<< (int)stream_info_[i].es_info_length_		 <<endl<<endl;				
	}
	cout << "CRC_32 : " <<hex<< (crc_32_) << dec<< endl<<endl;
	is_exist_data_ = false;
	}
}

void PMTPacket::SetHeaderInfo(unsigned char* data)
{
	is_exist_data_ = true;

	pointer_field_ = data[pos_];	//8bit
	PlusDataPosition(1);

	table_id_ = data[pos_];	//8bit
	PlusDataPosition(1);

	section_syntax_indicator_ = (data[pos_] & 0x80) >> 7;	//1bit
	//'0'
	//2 reserved
	section_length_  = (data[pos_] & 0x03  ) << 8; //4bit에서 2bit는 00
	section_length_ += (data[pos_+1]		 );		 //8bit
	PlusDataPosition(2);//+16bit

	program_number_  = (data[pos_  ] ) << 8; //8bit
	program_number_ += (data[pos_+1] );		//8bit
	PlusDataPosition(2);//+16bit

	//2 reserved
	version_number_			= (data[pos_] & 0x3E  ) >> 1;//5bit
	current_next_indicator_	= (data[pos_] & 0x01  );		//1bit
	PlusDataPosition(1);//+8bit

	section_number_ = (data[pos_]);
	PlusDataPosition(1);//+8bit

	last_section_number_ = (data[pos_]);
	PlusDataPosition(1);//+8bit

	//3bit reserved
	pcr_pid_  = (data[pos_  ] & 0x1F)<<8;	//5bit
	pcr_pid_ += (data[pos_+1]		  );	//8bit
	PlusDataPosition(2);//+16bit

	//4bit reserved
	program_info_length_  = (data[pos_  ] & 0x03)<<8;	//4bit인데 2bit는 고정00
	program_info_length_ += (data[pos_+1]		  );	//8bit
	PlusDataPosition(2);//+16bit

	for(int i=0; i<program_info_length_; i++)
	{
		//descriptor();
	}
	PlusDataPosition(program_info_length_);//+@bit

	int size = ( (section_length_-13) - program_info_length_ );
	stream_info_ = (StreamInfo *) malloc(sizeof(StreamInfo) * size);

	int i=0, pointer=1;
	while(pointer!=(size+1))
	{
		stream_info_[i].stream_type_ = data[pos_]; //8bit
		PlusDataPosition(1);//+8bit
		pointer+=1;

		//3bit reserved
		stream_info_[i].elementary_pid_  = (data[pos_]	& 0x1F) <<8; //5bit
		stream_info_[i].elementary_pid_ += data[pos_+1];			 //8bit
		PlusDataPosition(2);//+16bit
		pointer+=2;

		//3bit reserved
		stream_info_[i].es_info_length_  = (data[pos_]	& 0x03) <<8; //4bit 2bit는 고정00
		stream_info_[i].es_info_length_ += data[pos_+1];			 //8bit
		PlusDataPosition(2);//+16bit
		pointer+=2;

		for(int j=0; j<stream_info_[i].es_info_length_; j++)
		{
			//descriptor();
		}
		PlusDataPosition(stream_info_[i].es_info_length_);//+@bit
		pointer+=stream_info_[i].es_info_length_;
		i++;
	}
	stream_info_size_ = i;

	crc_32_  = (data[pos_  ])<<24;	//8bit
	crc_32_ += (data[pos_+1])<<16;	//8bit
	crc_32_ += (data[pos_+2])<<8;		//8bit
	crc_32_ += (data[pos_+3]);		//8bit
	PlusDataPosition(4);//+32bit
}