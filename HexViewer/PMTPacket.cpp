#include "StdAfx.h"
#include "PMTPacket.h"



PMTPacket::PMTPacket(void)
{
	Init();
}


PMTPacket::~PMTPacket(void)
{
}

void PMTPacket::Init()
{
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

void PMTPacket::PrintInfo()
{
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
	
	int size = ( (section_length-13) - program_info_length ) / 5;	
	for(int i=0; i<size; i++)
	{
		cout << "stream_type : "				<< (int)streamInfo[i].stream_type			 <<endl;
		cout << "elementary_PID : "				<< (int)streamInfo[i].elementary_PID		 <<endl;
		cout << "ES_info_length : "				<< (int)streamInfo[i].ES_info_length		 <<endl<<endl;
	}
	cout << "CRC_32 : " <<hex<< (CRC_32) << dec<< endl<<endl;
}

void PMTPacket::HeaderInfo(int* data)
{
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

	int size = ( (section_length-13) - program_info_length ) / 5;
	streamInfo = (StreamInfo *) malloc(sizeof(StreamInfo) * size);

	for(int i=0; i<size; i++)
	{
		streamInfo[i].stream_type = data[pos]; //8bit
		PlusDataPosition(1);//+8bit

		//3bit reserved
		streamInfo[i].elementary_PID  = (data[pos]	& 0x1F) <<8; //5bit
		streamInfo[i].elementary_PID += data[pos+1];			 //8bit
		PlusDataPosition(2);//+16bit

		//3bit reserved
		streamInfo[i].ES_info_length  = (data[pos]	& 0x0F) <<8; //4bit
		streamInfo[i].ES_info_length += data[pos+1];			 //8bit
		PlusDataPosition(2);//+16bit
		for(int j=0; j<streamInfo[i].ES_info_length; j++)
		{
			//descriptor();
		}
		PlusDataPosition(streamInfo[i].ES_info_length);//+@bit		
	}

	CRC_32  = (data[pos  ])<<24;	//8bit
	CRC_32 += (data[pos+1])<<16;	//8bit
	CRC_32 += (data[pos+2])<<8;		//8bit
	CRC_32 += (data[pos+3]);		//8bit
	PlusDataPosition(4);//+32bit

	PrintInfo();
}