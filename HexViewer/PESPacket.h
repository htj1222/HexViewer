#pragma once
class PESPacket
{
private:
	uint32 packet_start_code_prefix;	//24bit
	uint8  stream_id;					//8bit
	uint16 PES_packet_length;			//16bit

	uint8 PES_scrambling_control;//2bit
	bool PES_priority;//1bit
	bool data_alignment_indicator;//1bit
	bool copyright;//1bit
	bool original_or_copy;//1bit
	uint8 PTS_DTS_flags;//2bit
	bool ESCR_flag;//1bit
	bool ES_rate_flag;//1bit
	bool DSM_trick_mode_flag;//1bit
	bool additional_copy_info_flag;//1bit
	bool PES_CRC_flag;//1bit
	bool PES_extension_flag;//1bit
	uint8 PES_header_data_length;//8bit
	
	uint64 PTS;//33bit
	uint64 DTS;//33bit

	uint64 ESCR_base;//33bit
	uint16 ESCR_extension;//9bit

	uint32 ES_rate;//22bit

	uint8 trick_mode_control;//3bit
	uint8 field_id;//2bit
	bool intra_slice_refresh;//1bit
	uint8 frequency_truncation;//2bit
	uint8 rep_cntrl;//5bit
	
	uint8 additional_copy_info;//7bit

	uint16 previous_PES_packet_CRC;//16bit

	bool PES_private_data_flag;//1bit
	bool pack_header_field_flag;//1bit
	bool program_packet_sequence_counter_flag;//1bit
	bool P_STD_buffer_flag;//1bit
	//reserved 3 bslbf
	bool PES_extension_flag_2;//1bit

	uint64 PES_private_data[2];//128bit

	uint8 pack_field_length;//8bit

	uint8 program_packet_sequence_counter;//7bit
	//bool marker_bit;//1bit
	bool MPEG1_MPEG2_identifier;//1bit
	uint8 original_stuff_length;//6bit

	//'01' 2 bslbf
	bool P_STD_buffer_scale;//1bit
	uint16 P_STD_buffer_size;//13bit

	uint8 PES_extension_field_length;//7bit

	int pos;
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
	void HeaderInfo(int* data);
	void PlusDataPosition(int plus);

	void PrintPESInfo();

	PESPacket(void);
	~PESPacket(void);
};

