#include "StdAfx.h"
#include "PESPacket.h"
#include <iostream>
using namespace std;

PESPacket::PESPacket(void)
{
	Init();
}

PESPacket::~PESPacket(void)
{
}

void PESPacket::SetPos(int pos_input)
{
	pos=pos_input;
}

void PESPacket::Init()
{
	packet_start_code_prefix=0;	//24bit
	stream_id=0;					//8bit
	PES_packet_length=0;			//16bit

	PES_scrambling_control=0;//2bit
	PES_priority=0;//1bit
	data_alignment_indicator=0;//1bit
	copyright=0;//1bit
	original_or_copy=0;//1bit
	PTS_DTS_flags=0;//2bit
	ESCR_flag=0;//1bit
	ES_rate_flag=0;//1bit
	DSM_trick_mode_flag=0;//1bit
	additional_copy_info_flag=0;//1bit
	PES_CRC_flag=0;//1bit
	PES_extension_flag=0;//1bit
	PES_header_data_length=0;//8bit
	
	PTS=0;//33bit
	DTS=0;//33bit
	ESCR_base=0;//33bit
	ESCR_extension=0;//9bit

	ES_rate=0;//22bit
	
	trick_mode_control=0;//3bit
	field_id=0;//2bit
	intra_slice_refresh=0;//1bit
	frequency_truncation=0;//2bit

	rep_cntrl=0;//5bit

	additional_copy_info=0;//7bit

	previous_PES_packet_CRC=0;//16bit

	PES_private_data_flag=0;//1bit
	pack_header_field_flag=0;//1bit
	program_packet_sequence_counter_flag=0;//1bit
	P_STD_buffer_flag=0;//1bit
	//reserved 3 bslbf
	PES_extension_flag_2=0;//1bit

	PES_private_data[2]=0;//128bit

	pack_field_length=0;//8bit

	program_packet_sequence_counter=0;//7bit
	//bool marker_bit=0;//1bit
	MPEG1_MPEG2_identifier=0;//1bit
	original_stuff_length=0;//6bit

	//'01' 2 bslbf
	P_STD_buffer_scale=0;//1bit
	P_STD_buffer_size=0;//13bit

	PES_extension_field_length=0;//7bit
}

void PESPacket::PlusDataPosition(int plus)
{
	pos += plus;
}

void  PESPacket::HeaderInfo(int* data)
{
	packet_start_code_prefix  = data[pos  ]	<<16;	//24bit
	packet_start_code_prefix += data[pos+1]	<<8;	//24bit
	packet_start_code_prefix += data[pos+2]	;		//24bit
	PlusDataPosition(3);//+24bit

	stream_id = data[pos];			//8bit
	PlusDataPosition(1);//+8bit

	PES_packet_length  = data[pos  ]<<8;			//16bit
	PES_packet_length += data[pos+1]	;			//16bit
	PlusDataPosition(2);//+16bit

	uint8 id = stream_id;

	if(	   id != program_stream_map
		&& id != padding_stream
		&& id != private_stream_2
		&& id != ECM_stream
		&& id != EMM_stream
		&& id != program_stream_directory
		&& id != DSMCC_stream
		&& id != typeE_stream)
	{
		//'10'
		PES_scrambling_control		= (data[pos] & 0x30	) >>4;//2bit
		PES_priority				= (data[pos] & 0x08	) >>3;//1bit
		data_alignment_indicator	= (data[pos] & 0x04	) >>2;//1bit
		copyright					= (data[pos] & 0x02	) >>1;//1bit
		original_or_copy			= (data[pos] & 0x01	)	 ;//1bit
		PlusDataPosition(1);//+8bit

		PTS_DTS_flags				= (data[pos] & 0xC0	) >>6;//2bit
		ESCR_flag					= (data[pos] & 0x20	) >>5;//1bit
		ES_rate_flag				= (data[pos] & 0x10	) >>4;//1bit
		DSM_trick_mode_flag			= (data[pos] & 0x08	) >>3;//1bit
		additional_copy_info_flag	= (data[pos] & 0x04	) >>2;//1bit
		PES_CRC_flag				= (data[pos] & 0x02	) >>1;//1bit
		PES_extension_flag			= (data[pos] & 0x01	)    ;//1bit
		PlusDataPosition(1);//+8bit

		PES_header_data_length		= (data[pos]);//8bit
		PlusDataPosition(1);//+8bit

		if(PTS_DTS_flags == 2)//'10'
		{
			//0010 //4bit
			PTS	 = (data[pos  ]	&	0x0E)	<< 29;
			PTS	+= (data[pos+1]			)	<< 22;
			PTS	+= (data[pos+2]	&	0xFE)	<< 14;
			PTS	+= (data[pos+3]			)	<< 7;
			PTS	+= (data[pos+4]	&	0xFE)	>> 1;
			PlusDataPosition(5);//+40bit
		}

		if(PTS_DTS_flags == 3)//'11'
		{
			//'0011'
			PTS	 = (data[pos  ]	&	0x0E)	<< 29;
			PTS	+= (data[pos+1]			)	<< 22;
			PTS	+= (data[pos+2]	&	0xFE)	<< 14;
			PTS	+= (data[pos+3]			)	<< 7;
			PTS	+= (data[pos+4]	&	0xFE)	>> 1;
			PlusDataPosition(5);//+40bit

			//'0001'
			DTS	 = (data[pos  ]	&	0x0E)	<< 29;
			DTS	+= (data[pos+1]			)	<< 22;
			DTS	+= (data[pos+2]	&	0xFE)	<< 14;
			DTS	+= (data[pos+3]			)	<< 7;
			DTS	+= (data[pos+4]	&	0xFE)	>> 1;
			PlusDataPosition(5);//+40bit
		}

		if(ESCR_flag)//+48bit
		{
			//2bit
			ESCR_base	 = (data[pos  ]	&	0x38)	<< 30;
			ESCR_base	+= (data[pos  ]	&	0x03)	<< 28;
			ESCR_base	+= (data[pos+1]			)	<< 20;
			ESCR_base	+= (data[pos+2]	&	0xF8)	<< 15;
			ESCR_base	+= (data[pos+2]	&	0x03)	<< 13;
			ESCR_base	+= (data[pos+3]			)	<< 5;
			ESCR_base	+= (data[pos+4]	&	0xF8)	;
			//33bit

			ESCR_extension = (data[pos+4]	&	0x03) <<7;
			ESCR_extension = (data[pos+5]	&	0xFE) >>1;
			//9bit

			PlusDataPosition(6);//+48bit
		}

		if(ES_rate_flag)//+24bit
		{
			//1bit marker
			ES_rate  = (data[pos]	& 0x7F) << 16;	//22bit
			ES_rate += (data[pos+1] )		<< 8;	//22bit
			ES_rate += (data[pos+2]	& 0xFE) >>1;	//22bit
			PlusDataPosition(3);//+24bit
		}

		if(DSM_trick_mode_flag)
		{
			trick_mode_control = (data[pos]	& 0xE0) >>5;//3bit
			if(trick_mode_control == fast_forward){
				field_id			 = (data[pos]	& 0x18) >>3;//2bit
				intra_slice_refresh  = (data[pos]	& 0x04) >>2;//1bit
				frequency_truncation = (data[pos]	& 0x03);	//2bit
			}
			if(trick_mode_control == slow_motion ){
				rep_cntrl = (data[pos]	& 0x1F);//5bit
			}
			if(trick_mode_control == freeze_frame){
				field_id			 = (data[pos]	& 0x18) >>3;//2bit
			}
			if(trick_mode_control == fast_reverse){
				field_id			 = (data[pos]	& 0x18) >>3;//2bit
				intra_slice_refresh  = (data[pos]	& 0x04) >>2;//1bit
				frequency_truncation = (data[pos]	& 0x03);	//2bit
			}
			if(trick_mode_control == slow_reverse){
				rep_cntrl = (data[pos]	& 0x1F);//5bit
			}
			PlusDataPosition(1);//+8bit
		}

		if(additional_copy_info_flag)
		{
			//1bit marker
			additional_copy_info = (data[pos]	& 0x7F);//7bit
			PlusDataPosition(1);//+8bit
		}

		if(PES_CRC_flag)
		{
			previous_PES_packet_CRC = (data[pos]) << 8;
			previous_PES_packet_CRC = (data[pos+1]) ;
			PlusDataPosition(2);//+16bit
		}

		if(PES_extension_flag)
		{
			PES_private_data_flag				= (data[pos] & 0x80	) >>7;//1bit
			pack_header_field_flag				= (data[pos] & 0x40	) >>6;//1bit
			program_packet_sequence_counter_flag= (data[pos] & 0x20	) >>5;//1bit
			P_STD_buffer_flag					= (data[pos] & 0x10	) >>4;//1bit
			//reserved 3 bslbf
			PES_extension_flag_2				= (data[pos] & 0x01	)	 ;//1bit
			PlusDataPosition(1);//+8bit

			if(PES_private_data_flag)
			{
				PES_private_data[0]	 = (data[pos  ]) << 56;
				PES_private_data[0]	+= (data[pos+1]) << 48;
				PES_private_data[0]	+= (data[pos+2]) << 40;
				PES_private_data[0]	+= (data[pos+3]) << 32;
				PES_private_data[0]	+= (data[pos+4]) << 24;
				PES_private_data[0]	+= (data[pos+5]) << 16;
				PES_private_data[0]	+= (data[pos+6]) << 8;
				PES_private_data[0]	+= (data[pos+7]);

				PES_private_data[1]	 = (data[pos+8]) << 56;
				PES_private_data[1]	+= (data[pos+9]) << 48;
				PES_private_data[1]	+= (data[pos+10]) << 40;
				PES_private_data[1]	+= (data[pos+11]) << 32;
				PES_private_data[1]	+= (data[pos+12]) << 24;
				PES_private_data[1]	+= (data[pos+13]) << 16;
				PES_private_data[1]	+= (data[pos+14]) << 8;
				PES_private_data[1]	+= (data[pos+15]);
				PlusDataPosition(16);//+128bit
			}
			if(pack_header_field_flag)
			{
				pack_field_length = (data[pos]);
				/////////FIXME
				//pack_header();
				PlusDataPosition(1);//+8bit
			}
			if(program_packet_sequence_counter_flag)
			{
				//1bit marker
				program_packet_sequence_counter = (data[pos]	& 0x7F);//7bit
				//1bit marker
				MPEG1_MPEG2_identifier = (data[pos+1]	& 0x40) >> 6;//1bit
				original_stuff_length  = (data[pos+1]	& 0x3F);	 //6bit
				PlusDataPosition(2);//+16bit
			}
			if(P_STD_buffer_flag)
			{
				//'01'
				P_STD_buffer_scale  = (data[pos]	& 0x20) >> 5;//1bit
				P_STD_buffer_size   = (data[pos]	& 0x1F) << 8;//5bit
				P_STD_buffer_size  += (data[pos+1]	& 0x1F) << 8;//8bit
				PlusDataPosition(2);//+16bit
			}
			if(PES_extension_flag_2)
			{
				//1bit marker
				PES_extension_field_length = (data[pos]	& 0x7F);//7bit
				for(int i=0; i<PES_extension_field_length; i++)
				{
					//reserved
				}
			}
		}
		/*		for (i = 0; i < N1; i++) {
		stuffing_byte 8 bslbf
		}
		for (i = 0; i < N2; i++) {
		PES_packet_data_byte 8 bslbf
		}
		*/
	}
	else if(
		   id != program_stream_map
		|| id != private_stream_2
		|| id != ECM_stream
		|| id != EMM_stream
		|| id != program_stream_directory
		|| id != DSMCC_stream
		|| id != typeE_stream){
			for(int i=0; i<PES_packet_length; i++)
			{
				//PES_packet_data_byte 8bit
			}
	}
	else if(id == padding_stream)
	{
		for(int i=0; i<PES_packet_length; i++)
		{
			//padding_byte 8bit
		}
	}
	PrintPESInfo();
}

void PESPacket::PrintPESInfo()
{
	//cout << "packet_start_code_prefix : "	<< packet_start_code_prefix		<< endl;
	cout<<"\n == PES header fields == "<< endl;
	cout << "stream_id : "					<< hex << (int)stream_id					<< endl;
	cout << "PES_packet_length : "			<< dec << (int)PES_packet_length			<< endl;
		
	uint8 id = stream_id;
	if(	   id != program_stream_map
		&& id != padding_stream
		&& id != private_stream_2
		&& id != ECM_stream
		&& id != EMM_stream
		&& id != program_stream_directory
		&& id != DSMCC_stream
		&& id != typeE_stream)
	{
		cout << "PES_scrambling_control : "		<< (int)PES_scrambling_control		<< endl;
		cout << "PES_priority : "				<< PES_priority 				<< endl;
		cout << "data_alignment_indicator : "	<< data_alignment_indicator		<< endl;
		cout << "copyright : "					<< copyright					<< endl;
		cout << "original_or_copy : "			<< original_or_copy				<< endl;
		if(PTS_DTS_flags == 2)//'10'
		{
			cout << "PTS_flag : "			<< true				<< endl;
		}

		if(PTS_DTS_flags == 3)//'11'
		{
			cout << "PTS_flag : "			<< true				<< endl;
			cout << "DTS_flag : "			<< true				<< endl;			
		}
		cout << "ESCR_flag : "					<< ESCR_flag					<< endl;
		cout << "ES_rate_flag : "				<< ES_rate_flag					<< endl;
		cout << "DSM_trick_mode_flag : "		<< DSM_trick_mode_flag			<< endl;
		cout << "additional_copy_info_flag : "	<< additional_copy_info_flag	<< endl;
		cout << "PES_CRC_flag : "				<< PES_CRC_flag					<< endl;
		cout << "PES_extension_flag : "			<< PES_extension_flag			<< endl;
		cout << "PES_header_data_length : "		<< (int)PES_header_data_length		<< endl;
		
		if(PTS_DTS_flags == 2)//'10'
		{
			cout << "PTS : "			<< 	PTS		<< endl;
		}

		if(PTS_DTS_flags == 3)//'11'
		{
			cout << "PTS : "			<< 	PTS		<< endl;
			cout << "DTS : "			<< 	DTS		<< endl;
		}

		if(ESCR_flag)//+48bit
		{
			cout << "ESCR_base : "				<< 	ESCR_base			<< endl;
			cout << "ESCR_extension : "			<< 	ESCR_extension		<< endl;
		}

		if(ES_rate_flag)//+24bit
		{
			cout << "ES_rate : "			<< 	ES_rate		<< endl;			
		}

		if(DSM_trick_mode_flag)
		{
			cout << "trick_mode_control : "			<< 	trick_mode_control		<< endl;			
			 
			if(trick_mode_control == fast_forward){
				cout << "field_id : "						<< 	field_id				<< endl;
				cout << "intra_slice_refresh : "			<< 	intra_slice_refresh		<< endl;
				cout << "frequency_truncation : "			<< 	frequency_truncation	<< endl;
			}
			if(trick_mode_control == slow_motion ){
				cout << "rep_cntrl : "						<< 	rep_cntrl				<< endl;
			}
			if(trick_mode_control == freeze_frame){
				cout << "field_id : "						<< 	field_id				<< endl;
			}
			if(trick_mode_control == fast_reverse){
				cout << "field_id : "						<< 	field_id				<< endl;
				cout << "intra_slice_refresh : "			<< 	intra_slice_refresh		<< endl;
				cout << "frequency_truncation : "			<< 	frequency_truncation	<< endl;
			}
			if(trick_mode_control == slow_reverse){
				cout << "rep_cntrl : "						<< 	rep_cntrl				<< endl;
			}
		}

		if(additional_copy_info_flag)
		{
			cout << "additional_copy_info : "		<< 	additional_copy_info	<< endl;
		}

		if(PES_CRC_flag)
		{
			cout << "previous_PES_packet_CRC : "		<< 	previous_PES_packet_CRC	<< endl;			
		}

		if(PES_extension_flag)
		{
			cout << "PES_private_data_flag : "				<< 	PES_private_data_flag	<< endl;			
			cout << "pack_header_field_flag : "				<< 	pack_header_field_flag	<< endl;			
			cout << "program_packet_sequence_counter_flag : "<< 	program_packet_sequence_counter_flag	<< endl;			
			cout << "P_STD_buffer_flag : "					<< 	P_STD_buffer_flag	<< endl;			
			cout << "PES_extension_flag_2 : "				<< 	PES_extension_flag_2	<< endl;			
			cout << "PES_private_data_flag : "				<< 	PES_private_data_flag	<< endl;			
			
			if(PES_private_data_flag)
			{
				//FIXME
				cout << "PES_private_data : "		<< 	PES_private_data[0]	<< PES_private_data[1] << endl;
			}
			if(pack_header_field_flag)
			{
				cout << "pack_field_length : "		<< 	pack_field_length	 << endl;				 
			}
			if(program_packet_sequence_counter_flag)
			{
				cout << "program_packet_sequence_counter : "		<< 	program_packet_sequence_counter	 << endl;
				cout << "MPEG1_MPEG2_identifier : "		<< 	MPEG1_MPEG2_identifier	 << endl;
				cout << "original_stuff_length : "		<< 	original_stuff_length	 << endl;
			}
			if(P_STD_buffer_flag)
			{
				cout << "P_STD_buffer_scale : "		<< 	P_STD_buffer_scale	 << endl;
				cout << "P_STD_buffer_size : "		<< 	P_STD_buffer_size	 << endl;				
			}
			if(PES_extension_flag_2)
			{
				cout << "PES_extension_field_length : "		<< 	PES_extension_field_length	 << endl;								
			}
		}		
	}
	cout << endl;
}