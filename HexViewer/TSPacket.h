#pragma once
#include "StdAfx.h"
#include "PESPacket.h"
#include "PATPacket.h"
#include "CATPacket.h"
#include "PMTPacket.h"

#include <list>
#include <iostream>
#include <algorithm> //find()를 위해 필요 

struct ContinuityCounterValue
{	
	uint16 pid_;	
	uint8 last_continuity_counter_;			//4bit
	bool is_video_;
};

struct FindSamePID
{
	bool operator() ( ContinuityCounterValue& value ) const { return value.pid_ == ComparePID; }
  	uint16 ComparePID;
};

class TSPacket
{
private:
	PESPacket pes_packet_; //pes
	PATPacket pat_packet_; //pat
	CCATPacket cat_packet_;//cat
	PMTPacket pmt_packet_; //pmt

	bool is_exist_data_;		//데이터 존재 유무 저장

	//position
	int pos_;

	//Partial Transport Stream Packet Format
	uint8 sync_byte_;						//8bit
	bool transport_error_indicator_;		//1bit
	bool payload_unit_start_indicator_;		//1bit
	bool transport_priority_indicator_;		//1bit
	uint16 pid_;							//13bit
	uint8 transport_scrambling_control_;	//2bit
	uint8 adaptation_field_control_;		//2bit
	uint8 continuity_counter_;				//4bit

	//Adaptation Field Format
	uint8 adaptation_field_length_;			//8bit
	bool discontinuity_indicator_;			//1bit
	bool random_access_indicator_;			//1bit
	bool elementary_stream_priority_indicator_;//1bit
	bool pcr_flag_;							//1bit
	bool opcr_flag_;						//1bit
	bool splicing_point_flag_;				//1bit
	bool transport_private_data_flag_;		//1bit
	bool adaptation_field_extension_flag_;	//1bit

	//pcr
	uint64 program_clock_reference_base_;	//33bit
	uint8  reserved_;						//6bit
	uint64 program_clock_reference_extension_;//9bit
	uint64 result_pcr_;

	//opcr
	uint64 original_program_clock_reference_base_;		//33bit
	uint64 original_program_clock_reference_extension_;	//9bit
	uint64 result_opcr_;

	//spliceCountdown
	uint8 splice_countdown_;//8bit

	//privateData
	uint8 transport_private_data_length_;//8bit
	uint8* private_data_byte_;			//8*n bit

	//extension
	uint8 adaptation_field_extension_length_;//8bit
	bool ltw_flag_;							//1bit
	bool piecewise_rate_flag_;				//1bit
	bool seamless_splice_flag_;				//1bit
	//+5bit reserved

	//ltw
	bool ltw_valid_flag_; //1bit
	uint16 ltw_offset_; //15bit

	//piecewise
	uint32 piecewise_rate_;//22bit

	//seamless
	uint8 splice_type_;//4bit
	uint64 dts_next_au_;//36bit
	
	list<ContinuityCounterValue> cc_list_;

	string packet_info_buffer_;
public:
	
	int cc_error_counter_;

	TSPacket(void);
	~TSPacket(void);

	void Init();
	void SetPidValueInit(unsigned char* data);

	void GetHeaderInfo(unsigned char* data);
	int  getDataPosition();
	void PlusDataPosition(int plus);
	void GetAdaptationField(unsigned char* data, unsigned char adaptationFieldControl);

	void SetPrintHeaderInfo();
	void PrintHeaderInfo();
	void SetPrintAdaptationInfo();
	void PrintAdaptationInfo();

	string GetPacketInfoBuffer();
	
	void CheckContinuityCounter(unsigned char* data);

	void Reset();
};
