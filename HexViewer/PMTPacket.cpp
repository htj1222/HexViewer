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
	free(streamInfo);
	Init();
	}
}

void PMTPacket::Init()
{
	is_exist_data_ = false;

	pointer_field=0;
	table_id=0;					//8bit
	section_syntax_indicator=0;	//1bit
	//'0' 1 bslbf
	//reserved 2 bslbf
	section_length=0;			//12bit
	program_number=0;			//16bit
	//reserved 2 bslbf
	version_number=0;			//5bit
	current_next_indicator=0;	//1bit
	section_number=0;			//8bit
	last_section_number=0;		//8bit
	
	
	//reserved 3 bslbf
	PCR_PID=0;					//13bit
	//reserved 4 bslbf
	program_info_length=0;		//12bit

	streamInfo=NULL;
	streamInfo_size_=0;
	CRC_32=0;					//32bit
}

void PMTPacket::SetPos(int pos_input)
{
	pos=pos_input;
}

void PMTPacket::PlusDataPosition(int plus)
{
	pos += plus;
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
	cout << "table_id : "					<< hex <<(int)table_id << dec		<< endl;
	cout << "section_syntax_indicator : "	<< section_syntax_indicator			<< endl;
	cout << "section_length : "				<< (int)section_length				<< endl;
	cout << "program_number : "				<< (int)program_number				<< endl;
	cout << "version_number : "				<< (int)version_number				<< endl;
	cout << "current_next_indicator : "		<< current_next_indicator			<< endl;
	cout << "section_number : "				<< (int)section_number				<< endl;
	cout << "last_section_number : "		<< (int)last_section_number			<< endl<<endl;
	
	cout << "PCR_PID : "					<< (int)PCR_PID						<< endl;
	cout << "program_info_length : "		<< (int)program_info_length			<< endl << endl;
		
	for(int i=0; i<streamInfo_size_; i++)
	{
		cout << "stream_type : "				<< (int)streamInfo[i].stream_type;
		if(isAudioStreamType(streamInfo[i].stream_type)){
			cout << "(is Audio Stream)"			<<endl;
		}
		else if(isVideoStreamType(streamInfo[i].stream_type)){
			cout << "(is Video Stream)"			<<endl;
		}else{
			cout << "(?)"			<<endl;
		}
		cout << "elementary_PID : "				<< (int)streamInfo[i].elementary_PID		 <<endl;
		cout << "ES_info_length : "				<< (int)streamInfo[i].ES_info_length		 <<endl<<endl;				
	}
	cout << "CRC_32 : " <<hex<< (CRC_32) << dec<< endl<<endl;
	is_exist_data_ = false;
	}
}

void PMTPacket::SetHeaderInfo(unsigned char* data)
{
	is_exist_data_ = true;

	pointer_field = data[pos];	//8bit
	PlusDataPosition(1);

	table_id = data[pos];	//8bit
	PlusDataPosition(1);

	section_syntax_indicator = (data[pos] & 0x80) >> 7;	//1bit
	//'0'
	//2 reserved
	section_length  = (data[pos] & 0x03  ) << 8; //4bit에서 2bit는 00
	section_length += (data[pos+1]		 );		 //8bit
	PlusDataPosition(2);//+16bit

	program_number  = (data[pos  ] ) << 8; //8bit
	program_number += (data[pos+1] );		//8bit
	PlusDataPosition(2);//+16bit

	//2 reserved
	version_number			= (data[pos] & 0x3E  ) >> 1;//5bit
	current_next_indicator	= (data[pos] & 0x01  );		//1bit
	PlusDataPosition(1);//+8bit

	section_number = (data[pos]);
	PlusDataPosition(1);//+8bit

	last_section_number = (data[pos]);
	PlusDataPosition(1);//+8bit

	//3bit reserved
	PCR_PID  = (data[pos  ] & 0x1F)<<8;	//5bit
	PCR_PID += (data[pos+1]		  );	//8bit
	PlusDataPosition(2);//+16bit

	//4bit reserved
	program_info_length  = (data[pos  ] & 0x03)<<8;	//4bit인데 2bit는 고정00
	program_info_length += (data[pos+1]		  );	//8bit
	PlusDataPosition(2);//+16bit

	for(int i=0; i<program_info_length; i++)
	{
		//descriptor();
	}
	PlusDataPosition(program_info_length);//+@bit

	int size = ( (section_length-13) - program_info_length );
	streamInfo = (StreamInfo *) malloc(sizeof(StreamInfo) * size);

	int i=0, pointer=1;
	while(pointer!=(size+1))
	{
		streamInfo[i].stream_type = data[pos]; //8bit
		PlusDataPosition(1);//+8bit
		pointer+=1;

		//3bit reserved
		streamInfo[i].elementary_PID  = (data[pos]	& 0x1F) <<8; //5bit
		streamInfo[i].elementary_PID += data[pos+1];			 //8bit
		PlusDataPosition(2);//+16bit
		pointer+=2;

		//3bit reserved
		streamInfo[i].ES_info_length  = (data[pos]	& 0x03) <<8; //4bit 2bit는 고정00
		streamInfo[i].ES_info_length += data[pos+1];			 //8bit
		PlusDataPosition(2);//+16bit
		pointer+=2;

		for(int j=0; j<streamInfo[i].ES_info_length; j++)
		{
			//descriptor();
		}
		PlusDataPosition(streamInfo[i].ES_info_length);//+@bit
		pointer+=streamInfo[i].ES_info_length;
		i++;
	}
	streamInfo_size_ = i;

	CRC_32  = (data[pos  ])<<24;	//8bit
	CRC_32 += (data[pos+1])<<16;	//8bit
	CRC_32 += (data[pos+2])<<8;		//8bit
	CRC_32 += (data[pos+3]);		//8bit
	PlusDataPosition(4);//+32bit
}