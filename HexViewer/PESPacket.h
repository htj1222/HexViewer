#pragma once
#include "StdAfx.h"


typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned int   uint32;
typedef unsigned long  uint64;


class PESPacket
{
private:
	bool is_exist_data_;		//데이터 존재 유무 저장

	uint32 packet_start_code_prefix_;	//24bit
	uint8  stream_id_;					//8bit
	uint16 pes_packet_length_;			//16bit

	uint8 pes_scrambling_control_;//2bit
	bool pes_priority_;//1bit
	bool data_alignment_indicator_;//1bit
	bool copyright_;//1bit
	bool original_or_copy_;//1bit
	uint8 pts_dts_flags_;//2bit
	bool escr_flag_;//1bit
	bool es_rate_flag_;//1bit
	bool dsm_trick_mode_flag_;//1bit
	bool additional_copy_info_flag_;//1bit
	bool pes_crc_flag_;//1bit
	bool pes_extension_flag_;//1bit
	uint8 pes_header_data_length_;//8bit
	
	uint64 pts_;//33bit
	uint64 dts_;//33bit

	uint64 escr_base_;//33bit
	uint16 escr_extension_;//9bit

	uint32 es_rate_;//22bit

	uint8 trick_mode_control_;//3bit
	uint8 field_id_;//2bit
	bool intra_slice_refresh_;//1bit
	uint8 frequency_truncation_;//2bit
	uint8 rep_cntrl_;//5bit
	
	uint8 additional_copy_info_;//7bit

	uint16 previous_pes_packet_crc_;//16bit

	bool pes_private_data_flag_;//1bit
	bool pack_header_field_flag_;//1bit
	bool program_packet_sequence_counter_flag_;//1bit
	bool p_std_buffer_flag_;//1bit
	//reserved 3 bslbf
	bool pes_extension_flag_2_;//1bit

	uint64 pes_private_data_[2];//128bit

	uint8 pack_field_length_;//8bit

	uint8 program_packet_sequence_counter_;//7bit
	//bool marker_bit;//1bit
	bool mpeg1_mpeg2_identifier_;//1bit
	uint8 original_stuff_length_;//6bit

	//'01' 2 bslbf
	bool p_std_buffer_scale_;//1bit
	uint16 p_std_buffer_size_;//13bit

	uint8 pes_extension_field_length_;//7bit

	int pos_;
public:
	enum TrickModeControl
	{
		fast_forward	=0,
		slow_motion		=1,
		freeze_frame	=2,
		fast_reverse	=3,
		slow_reverse	=4
	};

	enum StreamId
	{
		program_stream_map	=0xBC,
		private_stream_1	=0xBD,
		padding_stream		=0xBE,
		private_stream_2	=0xBF,
		//audio stream		110x xxxx
		//video stream		1110 xxxx
		ECM_stream			=0xF0,
		EMM_stream			=0xF1,
		DSMCC_stream		=0xF2,
		ISO_IEC_13522_stream=0xF3,
		typeA_stream		=0xF4,
		typeB_stream		=0xF5,
		typeC_stream		=0xF6,
		typeD_stream		=0xF7,
		typeE_stream		=0xF8,
		ancillary_stream	=0xF9,
		SL_packetized_stream=0xFA,
		FlexMux_stream		=0xFB,
		metadata_stream		=0xFC,
		extended_stream_id	=0xFD,
		reserved_data_stream=0xFE,
		program_stream_directory=0xFF
	};

	void SetPos(int pos);
	void Init();
	void SetHeaderInfo(unsigned char* data);
	void PlusDataPosition(int plus);

	void PrintPESInfo();

	PESPacket(void);
	~PESPacket(void);
};

