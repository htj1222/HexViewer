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
	pos_=pos_input;
}

void PESPacket::Init()
{
	is_exist_data_ = false;

	packet_start_code_prefix_ = 0;	//24bit
	stream_id_ = 0;					//8bit
	pes_packet_length_ = 0;			//16bit

	pes_scrambling_control_ = 0;//2bit
	pes_priority_ = 0;//1bit
	data_alignment_indicator_ = 0;//1bit
	copyright_ = 0;//1bit
	original_or_copy_ = 0;//1bit
	pts_dts_flags_ = 0;//2bit
	escr_flag_ = 0;//1bit
	es_rate_flag_ = 0;//1bit
	dsm_trick_mode_flag_ = 0;//1bit
	additional_copy_info_flag_ = 0;//1bit
	pes_crc_flag_ = 0;//1bit
	pes_extension_flag_ = 0;//1bit
	pes_header_data_length_ = 0;//8bit
	
	pts_ = 0;//33bit
	dts_ = 0;//33bit

	escr_base_ = 0;//33bit
	escr_extension_ = 0;//9bit

	es_rate_ = 0;//22bit

	trick_mode_control_ = 0;//3bit
	field_id_ = 0;//2bit
	intra_slice_refresh_ = 0;//1bit
	frequency_truncation_ = 0;//2bit
	rep_cntrl_ = 0;//5bit
	
	additional_copy_info_ = 0;//7bit
	previous_pes_packet_crc_ = 0;//16bit

	pes_private_data_flag_ = 0;//1bit
	pack_header_field_flag_ = 0;//1bit
	program_packet_sequence_counter_flag_ = 0;//1bit
	p_std_buffer_flag_ = 0;//1bit
	//reserved 3 bslbf
	pes_extension_flag_2_ = 0;//1bit

	pes_private_data_[2];//128bit

	pack_field_length_ = 0;//8bit

	program_packet_sequence_counter_ = 0;//7bit
	//bool marker_bit;//1bit
	mpeg1_mpeg2_identifier_ = 0;//1bit
	original_stuff_length_ = 0;//6bit

	//'01' 2 bslbf
	p_std_buffer_scale_ = 0;//1bit
	p_std_buffer_size_ = 0;//13bit

	pes_extension_field_length_ = 0;//7bit

	packet_info_buffer_ = "";
}

void PESPacket::PlusDataPosition(int plus)
{
	pos_ += plus;
}

void  PESPacket::SetHeaderInfo(unsigned char* data)
{
	is_exist_data_ = true;

	packet_start_code_prefix_  = data[pos_ ]	<<16;	//24bit
	packet_start_code_prefix_ += data[pos_+1]	<<8;	//24bit
	packet_start_code_prefix_ += data[pos_+2]	;		//24bit
	PlusDataPosition(3);//+24bit

	stream_id_ = data[pos_];			//8bit
	PlusDataPosition(1);//+8bit

	pes_packet_length_  = data[pos_ ]<<8;			//16bit
	pes_packet_length_ += data[pos_+1]	;			//16bit
	PlusDataPosition(2);//+16bit

	uint8 id = stream_id_;

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
		pes_scrambling_control_		= (data[pos_] & 0x30	) >>4;//2bit
		pes_priority_				= (data[pos_] & 0x08	) >>3;//1bit
		data_alignment_indicator_	= (data[pos_] & 0x04	) >>2;//1bit
		copyright_					= (data[pos_] & 0x02	) >>1;//1bit
		original_or_copy_			= (data[pos_] & 0x01	)	 ;//1bit
		PlusDataPosition(1);//+8bit

		pts_dts_flags_				= (data[pos_] & 0xC0	) >>6;//2bit
		escr_flag_					= (data[pos_] & 0x20	) >>5;//1bit
		es_rate_flag_				= (data[pos_] & 0x10	) >>4;//1bit
		dsm_trick_mode_flag_		= (data[pos_] & 0x08	) >>3;//1bit
		additional_copy_info_flag_	= (data[pos_] & 0x04	) >>2;//1bit
		pes_crc_flag_				= (data[pos_] & 0x02	) >>1;//1bit
		pes_extension_flag_			= (data[pos_] & 0x01	)    ;//1bit
		PlusDataPosition(1);//+8bit

		pes_header_data_length_		= (data[pos_]);//8bit
		PlusDataPosition(1);//+8bit

		if(pts_dts_flags_ == 2)//'10'
		{
			//0010 //4bit
			pts_	 = (data[pos_ ]	&	0x0E)	<< 29;
			pts_	+= (data[pos_+1]			)	<< 22;
			pts_	+= (data[pos_+2]	&	0xFE)	<< 14;
			pts_	+= (data[pos_+3]			)	<< 7;
			pts_	+= (data[pos_+4]	&	0xFE)	>> 1;
			PlusDataPosition(5);//+40bit
		}

		if(pts_dts_flags_ == 3)//'11'
		{
			//'0011'
			pts_	 = (data[pos_ ]	&	0x0E)	<< 29;
			pts_	+= (data[pos_+1]			)	<< 22;
			pts_	+= (data[pos_+2]	&	0xFE)	<< 14;
			pts_	+= (data[pos_+3]			)	<< 7;
			pts_	+= (data[pos_+4]	&	0xFE)	>> 1;
			PlusDataPosition(5);//+40bit

			//'0001'
			dts_	 = (data[pos_ ]	&	0x0E)	<< 29;
			dts_	+= (data[pos_+1]			)	<< 22;
			dts_	+= (data[pos_+2]	&	0xFE)	<< 14;
			dts_	+= (data[pos_+3]			)	<< 7;
			dts_	+= (data[pos_+4]	&	0xFE)	>> 1;
			PlusDataPosition(5);//+40bit
		}

		if(escr_flag_)//+48bit
		{
			//2bit
			escr_base_	 = (data[pos_ ]	&	0x38)	<< 30;
			escr_base_	+= (data[pos_ ]	&	0x03)	<< 28;
			escr_base_	+= (data[pos_+1]			)	<< 20;
			escr_base_	+= (data[pos_+2]	&	0xF8)	<< 15;
			escr_base_	+= (data[pos_+2]	&	0x03)	<< 13;
			escr_base_	+= (data[pos_+3]			)	<< 5;
			escr_base_	+= (data[pos_+4]	&	0xF8)	;
			//33bit

			escr_extension_ = (data[pos_+4]	&	0x03) <<7;
			escr_extension_ = (data[pos_+5]	&	0xFE) >>1;
			//9bit

			PlusDataPosition(6);//+48bit
		}

		if(es_rate_flag_)//+24bit
		{
			//1bit marker
			es_rate_  = (data[pos_]	& 0x7F) << 16;	//22bit
			es_rate_ += (data[pos_+1] )		<< 8;	//22bit
			es_rate_ += (data[pos_+2]	& 0xFE) >>1;	//22bit
			PlusDataPosition(3);//+24bit
		}

		if(dsm_trick_mode_flag_)
		{
			trick_mode_control_ = (data[pos_]	& 0xE0) >>5;//3bit
			if(trick_mode_control_ == fast_forward){
				field_id_			 = (data[pos_]	& 0x18) >>3;//2bit
				intra_slice_refresh_  = (data[pos_]	& 0x04) >>2;//1bit
				frequency_truncation_ = (data[pos_]	& 0x03);	//2bit
			}
			if(trick_mode_control_ == slow_motion ){
				rep_cntrl_ = (data[pos_]	& 0x1F);//5bit
			}
			if(trick_mode_control_ == freeze_frame){
				field_id_			 = (data[pos_]	& 0x18) >>3;//2bit
			}
			if(trick_mode_control_ == fast_reverse){
				field_id_			  = (data[pos_]	& 0x18) >>3;//2bit
				intra_slice_refresh_  = (data[pos_]	& 0x04) >>2;//1bit
				frequency_truncation_ = (data[pos_]	& 0x03);	//2bit
			}
			if(trick_mode_control_ == slow_reverse){
				rep_cntrl_ = (data[pos_]	& 0x1F);//5bit
			}
			PlusDataPosition(1);//+8bit
		}

		if(additional_copy_info_flag_)
		{
			//1bit marker
			additional_copy_info_ = (data[pos_]	& 0x7F);//7bit
			PlusDataPosition(1);//+8bit
		}

		if(pes_crc_flag_)
		{
			previous_pes_packet_crc_ = (data[pos_]) << 8;
			previous_pes_packet_crc_ = (data[pos_+1]) ;
			PlusDataPosition(2);//+16bit
		}

		if(pes_extension_flag_)
		{
			pes_private_data_flag_				= (data[pos_] & 0x80	) >>7;//1bit
			pack_header_field_flag_				= (data[pos_] & 0x40	) >>6;//1bit
			program_packet_sequence_counter_flag_= (data[pos_] & 0x20	) >>5;//1bit
			p_std_buffer_flag_					= (data[pos_] & 0x10	) >>4;//1bit
			//reserved 3 bslbf
			pes_extension_flag_2_				= (data[pos_] & 0x01	)	 ;//1bit
			PlusDataPosition(1);//+8bit

			if(pes_private_data_flag_)
			{
				pes_private_data_[0]	 = (data[pos_ ]) << 56;
				pes_private_data_[0]	+= (data[pos_+1]) << 48;
				pes_private_data_[0]	+= (data[pos_+2]) << 40;
				pes_private_data_[0]	+= (data[pos_+3]) << 32;
				pes_private_data_[0]	+= (data[pos_+4]) << 24;
				pes_private_data_[0]	+= (data[pos_+5]) << 16;
				pes_private_data_[0]	+= (data[pos_+6]) << 8;
				pes_private_data_[0]	+= (data[pos_+7]);

				pes_private_data_[1]	 = (data[pos_+8]) << 56;
				pes_private_data_[1]	+= (data[pos_+9]) << 48;
				pes_private_data_[1]	+= (data[pos_+10]) << 40;
				pes_private_data_[1]	+= (data[pos_+11]) << 32;
				pes_private_data_[1]	+= (data[pos_+12]) << 24;
				pes_private_data_[1]	+= (data[pos_+13]) << 16;
				pes_private_data_[1]	+= (data[pos_+14]) << 8;
				pes_private_data_[1]	+= (data[pos_+15]);
				PlusDataPosition(16);//+128bit
			}
			if(pack_header_field_flag_)
			{
				pack_field_length_ = (data[pos_]);
				/////////FIXME
				//pack_header();
				PlusDataPosition(1);//+8bit
			}
			if(program_packet_sequence_counter_flag_)
			{
				//1bit marker
				program_packet_sequence_counter_ = (data[pos_]	& 0x7F);//7bit
				//1bit marker
				mpeg1_mpeg2_identifier_ = (data[pos_+1]	& 0x40) >> 6;//1bit
				original_stuff_length_  = (data[pos_+1]	& 0x3F);	 //6bit
				PlusDataPosition(2);//+16bit
			}
			if(p_std_buffer_flag_)
			{
				//'01'
				p_std_buffer_scale_  = (data[pos_]	& 0x20) >> 5;//1bit
				p_std_buffer_size_   = (data[pos_]	& 0x1F) << 8;//5bit
				p_std_buffer_size_  += (data[pos_+1]	& 0x1F) << 8;//8bit
				PlusDataPosition(2);//+16bit
			}
			if(pes_extension_flag_2_)
			{
				//1bit marker
				pes_extension_field_length_ = (data[pos_]	& 0x7F);//7bit
				for(int i=0; i<pes_extension_field_length_; i++)
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
			for(int i=0; i<pes_packet_length_; i++)
			{
				//PES_packet_data_byte 8bit
			}
	}
	else if(id == padding_stream)
	{
		for(int i=0; i<pes_packet_length_; i++)
		{
			//padding_byte 8bit
		}
	}	
}

string PESPacket::GetPacketInfoBuffer()
{
	return packet_info_buffer_;
}

void PESPacket::SetPrintPESInfo(){
	if(is_exist_data_){
		//packet_info_buffer_ += "packet_start_code_prefix : "	+ packet_start_code_prefix		+"\n";
		packet_info_buffer_ +="\n == PES header fields == \n";
		packet_info_buffer_ += "stream_id : "					+ to_string((long long)(int)stream_id_					)+"\n";
		packet_info_buffer_ += "PES_packet_length : "			+ to_string((long long)(int)pes_packet_length_			)+"\n";

		uint8 id = stream_id_;
		if(	   id != program_stream_map
			&& id != padding_stream
			&& id != private_stream_2
			&& id != ECM_stream
			&& id != EMM_stream
			&& id != program_stream_directory
			&& id != DSMCC_stream
			&& id != typeE_stream)
		{
			packet_info_buffer_ += "PES_scrambling_control : "		+ to_string((long long)(int)pes_scrambling_control_		)+"\n";
			packet_info_buffer_ += "PES_priority : "				+ to_string((long long)pes_priority_ 					)+"\n";
			packet_info_buffer_ += "data_alignment_indicator : "	+ to_string((long long)data_alignment_indicator_		)+"\n";
			packet_info_buffer_ += "copyright : "					+ to_string((long long)copyright_						)+"\n";
			packet_info_buffer_ += "original_or_copy : "			+ to_string((long long)original_or_copy_				)+"\n";
			if(pts_dts_flags_ == 2)//'10'
			{
				packet_info_buffer_ += "PTS_flag : true\n";
			}

			if(pts_dts_flags_ == 3)//'11'
			{
				packet_info_buffer_ += "PTS_flag : true				\n";
				packet_info_buffer_ += "DTS_flag : true				\n";			
			}
			packet_info_buffer_ += "ESCR_flag : "					+ to_string((long long)escr_flag_					)+"\n";
			packet_info_buffer_ += "ES_rate_flag : "				+ to_string((long long)es_rate_flag_				)+"\n";
			packet_info_buffer_ += "DSM_trick_mode_flag : "			+ to_string((long long)dsm_trick_mode_flag_			)+"\n";
			packet_info_buffer_ += "additional_copy_info_flag : "	+ to_string((long long)additional_copy_info_flag_	)+"\n";
			packet_info_buffer_ += "PES_CRC_flag : "				+ to_string((long long)pes_crc_flag_				)+"\n";
			packet_info_buffer_ += "PES_extension_flag : "			+ to_string((long long)pes_extension_flag_			)+"\n";
			packet_info_buffer_ += "PES_header_data_length : "		+ to_string((long long)(int)pes_header_data_length_	)+"\n";

			if(pts_dts_flags_ == 2)//'10'
			{
				packet_info_buffer_ += "PTS : "			+ 	to_string((long long)pts_		)+"\n";
			}

			if(pts_dts_flags_ == 3)//'11'
			{
				packet_info_buffer_ += "PTS : "			+ 	to_string((long long)pts_		)+"\n";
				packet_info_buffer_ += "DTS : "			+ 	to_string((long long)dts_		)+"\n";
			}

			if(escr_flag_)//+48bit
			{
				packet_info_buffer_ += "ESCR_base : "				+ 	to_string((long long)escr_base_				)+"\n";
				packet_info_buffer_ += "ESCR_extension : "			+ 	to_string((long long)escr_extension_		)+"\n";
			}

			if(es_rate_flag_)//+24bit
			{
				packet_info_buffer_ += "ES_rate : "			+ 	to_string((long long)es_rate_		)+"\n";			
			}

			if(dsm_trick_mode_flag_)
			{
				packet_info_buffer_ += "trick_mode_control : "			+ 	to_string((long long)trick_mode_control_		)+"\n";			

				if(trick_mode_control_ == fast_forward){
					packet_info_buffer_ += "field_id : "						+ 	to_string((long long)field_id_				)+"\n";
					packet_info_buffer_ += "intra_slice_refresh : "				+ 	to_string((long long)intra_slice_refresh_	)+"\n";
					packet_info_buffer_ += "frequency_truncation : "			+ 	to_string((long long)frequency_truncation_	)+"\n";
				}
				if(trick_mode_control_ == slow_motion ){
					packet_info_buffer_ += "rep_cntrl : "						+ 	to_string((long long)rep_cntrl_				)+"\n";
				}
				if(trick_mode_control_ == freeze_frame){
					packet_info_buffer_ += "field_id : "						+ 	to_string((long long)field_id_				)+"\n";
				}
				if(trick_mode_control_ == fast_reverse){
					packet_info_buffer_ += "field_id : "						+ 	to_string((long long)field_id_				)+"\n";
					packet_info_buffer_ += "intra_slice_refresh : "				+ 	to_string((long long)intra_slice_refresh_	)+"\n";
					packet_info_buffer_ += "frequency_truncation : "			+ 	to_string((long long)frequency_truncation_	)+"\n";
				}
				if(trick_mode_control_ == slow_reverse){
					packet_info_buffer_ += "rep_cntrl : "						+ 	to_string((long long)rep_cntrl_				)+"\n";
				}
			}

			if(additional_copy_info_flag_)
			{
				packet_info_buffer_ += "additional_copy_info : "		+ 	to_string((long long)additional_copy_info_	)+"\n";
			}

			if(pes_crc_flag_)
			{
				packet_info_buffer_ += "previous_PES_packet_CRC : "		+ 	to_string((long long)previous_pes_packet_crc_	)+"\n";			
			}

			if(pes_extension_flag_)
			{
				packet_info_buffer_ += "PES_private_data_flag : "				+ 	to_string((long long)pes_private_data_flag_	)+"\n";			
				packet_info_buffer_ += "pack_header_field_flag : "				+ 	to_string((long long)pack_header_field_flag_	)+"\n";			
				packet_info_buffer_ += "program_packet_sequence_counter_flag : "+ 	to_string((long long)program_packet_sequence_counter_flag_	)+"\n";			
				packet_info_buffer_ += "P_STD_buffer_flag : "					+ 	to_string((long long)p_std_buffer_flag_	)+"\n";			
				packet_info_buffer_ += "PES_extension_flag_2 : "				+ 	to_string((long long)pes_extension_flag_2_	)+"\n";			
				packet_info_buffer_ += "PES_private_data_flag : "				+ 	to_string((long long)pes_private_data_flag_	)+"\n";			

				if(pes_private_data_flag_)
				{
					//FIXME
					packet_info_buffer_ += "PES_private_data : "		+ 	to_string((long long)pes_private_data_[0]	)+ to_string((long long)pes_private_data_[1]) +"\n";
				}
				if(pack_header_field_flag_)
				{
					packet_info_buffer_ += "pack_field_length : "		+ 	to_string((long long)pack_field_length_	 )+"\n";				 
				}
				if(program_packet_sequence_counter_flag_)
				{
					packet_info_buffer_ += "program_packet_sequence_counter : "		+ 	to_string((long long)program_packet_sequence_counter_	 )+"\n";
					packet_info_buffer_ += "MPEG1_MPEG2_identifier : "				+ 	to_string((long long)mpeg1_mpeg2_identifier_			 )+"\n";
					packet_info_buffer_ += "original_stuff_length : "				+ 	to_string((long long)original_stuff_length_				)+"\n";
				}
				if(p_std_buffer_flag_)
				{
					packet_info_buffer_ += "P_STD_buffer_scale : "		+ 	to_string((long long)p_std_buffer_scale_	)+"\n";
					packet_info_buffer_ += "P_STD_buffer_size : "		+ 	to_string((long long)p_std_buffer_size_		)+"\n";				
				}
				if(pes_extension_flag_2_)
				{
					packet_info_buffer_ += "PES_extension_field_length : "		+  to_string((long long)pes_extension_field_length_	 )+"\n";								
				}
			}		
		}
		packet_info_buffer_ += "\n";
		is_exist_data_ = false;
	}else{
		packet_info_buffer_ = "";
	}
}

void PESPacket::PrintPESInfo()
{
	char bufffer[1024] = "";
	strcpy(bufffer,packet_info_buffer_.c_str());
	cout << bufffer;
}