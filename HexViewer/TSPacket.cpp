#include "StdAfx.h"


TSPacket::TSPacket(void)
{
	Init();
}

TSPacket::~TSPacket(void)
{
}

//cc체크, pid구분을 위한 초기화
void TSPacket::SetPidValueInit(unsigned char* data)
{
	cc_error_counter_ = 0;

    list<ContinuityCounterValue>::iterator list_iter;	//STL의 list
	list_iter=cc_list_.begin();							//시작을 가리키도록 한다.
	
	list<ContinuityCounterValue>::iterator find_pid;//동일한 pid검사용
	
	//pid값이 같지 않을경우 추가
	for(int i=0; i<pmt_packet_.stream_info_size_; i++)
	{
		FindSamePID compare_temp;
		compare_temp.ComparePID = (int)pmt_packet_.stream_info_[i].elementary_pid_;//pid값 저장
		find_pid = find_if( cc_list_.begin(), cc_list_.end(), compare_temp );	//find_if함수를 사용하여 검색

		if( find_pid != cc_list_.end() )	{
			cout << "error!!";//같은 값이 있을경우
		} else{
			//같은 값이 없을경우 추가
			ContinuityCounterValue temp;
			temp.pid_ = compare_temp.ComparePID;
			temp.last_continuity_counter_ = 0;

			if(pmt_packet_.isAudioStreamType(pmt_packet_.stream_info_[i].stream_type_)){
				temp.is_video_ = false;
			} else{
				temp.is_video_ = true;
			}
			
			cc_list_.push_back(temp);			//추가
		}
	}
}
  
void TSPacket::CheckContinuityCounter(unsigned char* data)
{
	ContinuityCounterValue temp;
	temp.pid_ = ( ( data[1] & 0x1F	)<<8 ) | (data[2]);	
	temp.last_continuity_counter_	= data[3]	 & 0x0F;//4bit
	adaptation_field_control_		= data[3]>>4 & 0x03;//2bit

	if(temp.pid_ != 8191){ //8191 = 1FFF(NULL값)
		list<ContinuityCounterValue>::iterator find_pid;//검색용 iterator
		FindSamePID compare_temp;
		compare_temp.ComparePID = temp.pid_;
		find_pid = find_if( cc_list_.begin(), cc_list_.end(), compare_temp );//검색

		if( find_pid != cc_list_.end() )  //같은 pid인 경우
		{	  
			if( (( (*find_pid).last_continuity_counter_)) == temp.last_continuity_counter_) //현재 cc와 이전의 cc비교
			{
				if(++(*find_pid).last_continuity_counter_ == 16)	{//15이면 0으로 초기화
					(*find_pid).last_continuity_counter_ = 0;
				}
				//cout << "same cc:"<<(int)(*FindPID).last_continuity_counter_<<". PID : "<< compare_temp.ComparePID <<endl;
			}else{
				if(adaptation_field_control_ == 0 || adaptation_field_control_ == 2){ //Adaptation field control이 0, 2인 경우 증가X					
					//cout << "same (ada : "<<(int)adaptationFieldControl<<") PID : "<< compare_temp.ComparePID <<endl;
				}else{
					cc_error_counter_++;  //cc 에러!!
					//cout << "not same"<<endl;
				}
			}
		}
	}else{
		//cout<<"NULL" << endl;
	}
}


void TSPacket::GetHeaderInfo(unsigned char* data)
{
	//초기화
	pos_=0;

	sync_byte_ = data[pos_];														//8bit
	transport_error_indicator_		= (data[pos_+1]>>7		 ) == 1 ? true : false;	//1bit
	payload_unit_start_indicator_	= (data[pos_+1]>>6 & 0x01) == 1 ? true : false;	//1bit
	transport_priority_indicator_	= (data[pos_+1]>>5 & 0x01) == 1 ? true : false;	//1bit

	pid_  = ( ( data[pos_+1] & 0x1F	)<<8 ) | (data[pos_+2]);	

	transport_scrambling_control_	= data[pos_+3]>>6;		//2bit
	adaptation_field_control_		= data[pos_+3]>>4 & 0x03;//2bit
	continuity_counter_				= data[pos_+3]	 & 0x0F;//4bit

	PlusDataPosition(4);//32bit
	
	if(adaptation_field_control_ == 2 || adaptation_field_control_ == 3){
		GetAdaptationField(data, adaptation_field_control_);
	}

	if(adaptation_field_control_ == '01' || adaptation_field_control_ == '11') {
		/*for (i = 0; i < N; i++){
			data_byte 8 bslbf
		}*/
	}

	if(payload_unit_start_indicator_)
	{
		if(data[pos_] == 0 && data[pos_+1] == 0 && data[pos_+2] == 1)
		{			
			pes_packet_.SetPos(pos_);
			pes_packet_.SetHeaderInfo(data);
		}else if(data[pos_] == 0 && data[pos_+1] == 0)
		{			
			pat_packet_.SetPos(pos_);
			pat_packet_.SetHeaderInfo(data);
		}else if(data[pos_] == 0 && data[pos_+1] == 1)
		{
			cat_packet_.SetPos(pos_);
			cat_packet_.SetHeaderInfo(data);
		}else if(data[pos_] == 0 && data[pos_+1] == 2)
		{
			pmt_packet_.SetPos(pos_);
			pmt_packet_.SetHeaderInfo(data);
		}
	}
}

void TSPacket::PlusDataPosition(int plus)
{
	pos_ += plus;
}

int  TSPacket::getDataPosition()
{
	return pos_;
}

void TSPacket::GetAdaptationField(unsigned char* data, unsigned char adaptationFieldControl)
{
	is_exist_data_ = true;

	adaptation_field_length_	= data[pos_];
	PlusDataPosition(1);//8bit

	//길이가 0이상일 경우
	if(adaptation_field_length_ > 0){
		discontinuity_indicator_				= ((data[pos_] & 0x80) >> 7) == 1 ? true : false;
		random_access_indicator_				= ((data[pos_] & 0x40) >> 6) == 1 ? true : false;
		elementary_stream_priority_indicator_	= ((data[pos_] & 0x20) >> 5) == 1 ? true : false;
		pcr_flag_								= ((data[pos_] & 0x10) >> 4) == 1 ? true : false;
		opcr_flag_								= ((data[pos_] & 0x08) >> 3) == 1 ? true : false;
		splicing_point_flag_					= ((data[pos_] & 0x04) >> 2) == 1 ? true : false;
		transport_private_data_flag_			= ((data[pos_] & 0x02) >> 1) == 1 ? true : false;
		adaptation_field_extension_flag_		= ((data[pos_] & 0x01)	   ) == 1 ? true : false;

		PlusDataPosition(1);//8bit

		//PCR플레그가 1이면 PCR값을 구한다
		//+48bit (6)
		if(pcr_flag_)
		{
			uint64 temp_pcr = 0;
			temp_pcr += data[pos_  ] << 25;	//8bit씩 저장//6
			temp_pcr += data[pos_+1] << 17;	//8bit씩 저장//7
			temp_pcr += data[pos_+2] << 9;	//8bit씩 저장//8
			temp_pcr += data[pos_+3] << 1;	//8bit씩 저장//9
			temp_pcr += data[pos_+4] >> 7;	//마지막 1bit//10
			program_clock_reference_base_ = temp_pcr;	//33bit

			reserved_;	//미사용하는 값			//6bit

			temp_pcr = 0;
			temp_pcr += (data[pos_+4] & 0x01) << 8;	//상위 1bit저장//10
			temp_pcr += data[pos_+5];				//8bit저장//11
			program_clock_reference_extension_ = temp_pcr;//9bit

			//PCR값 정의 PCR(i) = PCR_base(i) x 300 + PCR_ext(i)에 따라 값 계산후 저장
			result_pcr_ = program_clock_reference_base_ * 300 + program_clock_reference_extension_;

			PlusDataPosition(6);//+48bit
		}

		//OPCR플레그가 1이면 OPCR을 구한다.
		//+48bit (6)
		if(opcr_flag_)
		{
			uint64 tempOPCR = 0;
			tempOPCR += data[pos_  ] << 25;	//8bit씩 저장
			tempOPCR += data[pos_+1] << 17;	//8bit씩 저장
			tempOPCR += data[pos_+2] << 9;	//8bit씩 저장
			tempOPCR += data[pos_+3] << 1;	//8bit씩 저장
			tempOPCR += data[pos_+4] >> 7;	//마지막 1bit
			program_clock_reference_base_ = tempOPCR;	//33bit

			reserved_;	//미사용하는 값			//6bit

			tempOPCR = 0;
			tempOPCR += (data[pos_+4] & 0x01) << 8;	//상위 1bit저장
			tempOPCR += data[pos_+5];				//8bit저장
			program_clock_reference_extension_ = tempOPCR;//9bit

			//PCR값 정의 PCR(i) = PCR_base(i) x 300 + PCR_ext(i)에 따라 값 계산후 저장
			result_opcr_ = program_clock_reference_base_ * 300 + program_clock_reference_extension_;

			PlusDataPosition(6);//+48bit
		}

		//8bit (1)
		if(splicing_point_flag_)
		{
			splice_countdown_ = data[pos_];
			PlusDataPosition(1);
		}
		
		//8bit+8*nbit
		if(transport_private_data_flag_)
		{
			//8bit
			transport_private_data_length_ = data[pos_];
			PlusDataPosition(1);
			
			//사이즈 지정하여 생성
			private_data_byte_ = (uint8 *) malloc(sizeof(uint8) * transport_private_data_length_); 
			for(int i=0; i<transport_private_data_length_; i++)
			{
				private_data_byte_[i] = data[pos_];
				//8bit씩 증가
				PlusDataPosition(1);
			}			
		}

		if(adaptation_field_extension_flag_)
		{
			adaptation_field_extension_length_ = data[pos_];	//8bit
			PlusDataPosition(1);//+8bit

			ltw_flag_				= ((data[pos_] & 0x80) >> 7) == 1 ? true : false;	//1bit
			piecewise_rate_flag_	= ((data[pos_] & 0x40) >> 6) == 1 ? true : false;	//1bit
			seamless_splice_flag_	= ((data[pos_] & 0x20) >> 5) == 1 ? true : false;	//1bit
			PlusDataPosition(1);//5bit reserved + 3bit

			if(ltw_flag_)
			{
				ltw_valid_flag_	= ((data[pos_] & 0x80) >> 7) == 1 ? true : false;	//1bit
				ltw_offset_		= data[pos_  ] & 0x7F << 8;							//15bit
				ltw_offset_		+= data[pos_+1];										//15bit
				PlusDataPosition(2);//16bit
			}

			if(piecewise_rate_flag_)
			{
				//2bit reserved
				piecewise_rate_  = (data[pos_]	& 0x3F) << 16;	//22bit
				piecewise_rate_ += (data[pos_+1] ) << 8;		//22bit
				piecewise_rate_ += (data[pos_+2] );				//22bit
				PlusDataPosition(3);//24bit
			}

			if(seamless_splice_flag_)
			{
				splice_type_ = data[pos_] >> 4;			//4bit
				dts_next_au_  = (data[pos_]  & 0x0E) << 29;	//3bit
				dts_next_au_ += (data[pos_+1]	   ) << 22;	//8bit
				dts_next_au_ += (data[pos_+2]& 0x0E) << 14;	//8bit
				dts_next_au_ += (data[pos_+3]	   ) << 7;	//8bit
				dts_next_au_ += (data[pos_+4]& 0x0E) >> 1;	//8bit
				PlusDataPosition(5);//40bit
			}
		}
	}
}
void TSPacket::Init()
{
	sync_byte_=0;						//8bit
	transport_error_indicator_=0;		//1bit
	payload_unit_start_indicator_=0;	//1bit
	transport_priority_indicator_=0;	//1bit
	pid_=0;							//13bit
	transport_scrambling_control_=0;	//2bit
	adaptation_field_control_=0;		//2bit
	continuity_counter_=0;			//4bit

	adaptation_field_length_=0;			//8bit
	discontinuity_indicator_=0;			//1bit
	random_access_indicator_=0;			//1bit
	elementary_stream_priority_indicator_=0;//1bit
	pcr_flag_=0;							//1bit
	opcr_flag_=0;							//1bit
	splicing_point_flag_=0;				//1bit
	transport_private_data_flag_=0;			//1bit
	adaptation_field_extension_flag_=0;		//1bit

	//pcr
	program_clock_reference_base_=0;		//33bit
	reserved_=0;							//6bit
	program_clock_reference_extension_=0;	//9bit
	result_pcr_=0;

	//opcr
	original_program_clock_reference_base_=0;		//33bit
	original_program_clock_reference_extension_=0;	//9bit
	result_opcr_=0;

	//spliceCountdown
	splice_countdown_=0;//8bit

	//privateData
	transport_private_data_length_=0;	//8bit
	private_data_byte_=NULL;			//8*n bit

	//extension
	adaptation_field_extension_length_=0;	//8bit
	ltw_flag_=0;							//1bit
	piecewise_rate_flag_=0;				//1bit
	seamless_splice_flag_=0;				//1bit
	//+5bit reserved

	//ltw
	ltw_valid_flag_=0; //1bit
	ltw_offset_=0;	//15bit

	//piecewise
	piecewise_rate_=0;//22bit

	//seamless
	splice_type_=0;	//4bit
	dts_next_au_=0;	//36bit

	is_exist_data_= false;
	packet_info_buffer_ = "";
}

void TSPacket::Reset()
{
	if(transport_private_data_flag_)
	{
		free(private_data_byte_);
	}
	Init();	
	pes_packet_.Init(); //pes
	pat_packet_.Reset(); //pat
	cat_packet_.Init(); //cat
	pmt_packet_.Reset(); //pmt
}

void TSPacket::SetPrintHeaderInfo()
{
	packet_info_buffer_ +="  \n == Transport packet fields == \n";
	if(pid_ != 0){
		if(pid_ == 8191){packet_info_buffer_ +="    ======= NULL Packet =======\n";}else{
			list<ContinuityCounterValue>::iterator FindPID;

			FindSamePID compare_temp;
			compare_temp.ComparePID = pid_;		
			FindPID = find_if( cc_list_.begin(), cc_list_.end(), compare_temp );

			if( FindPID != cc_list_.end() )	{
				if( (( (*FindPID).is_video_)) == true){
					packet_info_buffer_ += "  ======= Video Packet =======\n";
				}else{
					packet_info_buffer_ += "  ======= Audio Packet =======\n";
				}
			}
		}
	}else{
		packet_info_buffer_ += "\n";
	}
	packet_info_buffer_ +="  syncbyte: 0x"				+	to_string((long long)(int)sync_byte_)+"\n";	
	packet_info_buffer_ +="  transportErrorIndicator: "	+	to_string((long long)transport_error_indicator_)			+"\n";	
	packet_info_buffer_ +="  payloadUnitStartIndicator: "	+	to_string((long long)payload_unit_start_indicator_)		+"\n";
	packet_info_buffer_ +="  transportPriorityIndicator: "+	to_string((long long)transport_priority_indicator_)		+"\n";
	packet_info_buffer_ +="  PID: "						+	to_string((long long)pid_)								+"\n";
	packet_info_buffer_ +="  transportScramblingControl: "+	to_string((long long)(int)transport_scrambling_control_)	+"\n";
	packet_info_buffer_ +="  adaptationFieldControl: "	+	to_string((long long)(int)adaptation_field_control_)		+"\n";
	packet_info_buffer_ +="  continuityCounter: "			+	to_string((long long)(int)continuity_counter_)			+"\n\n";	
		
	SetPrintAdaptationInfo();

	pes_packet_.SetPrintPESInfo();
	packet_info_buffer_ += pes_packet_.GetPacketInfoBuffer();

	pat_packet_.SetPrintPATInfo();
	packet_info_buffer_ += pat_packet_.GetPacketInfoBuffer();

	cat_packet_.SetPrintCATInfo();
	packet_info_buffer_ += cat_packet_.GetPacketInfoBuffer();
	
	pmt_packet_.SetPrintPMTInfo();
	packet_info_buffer_ += pmt_packet_.GetPacketInfoBuffer();	
}

string TSPacket::GetPacketInfoBuffer(){
	return packet_info_buffer_;
}

void TSPacket::PrintHeaderInfo()
{
	char bufffer[2048] = "";
	strcpy(bufffer,packet_info_buffer_.c_str());
	cout << bufffer;
}

void TSPacket::SetPrintAdaptationInfo()
{	
	if(is_exist_data_)
	{
	packet_info_buffer_ +="   == Adaptation fields == \n";
	packet_info_buffer_ +="  Adaptation_field_length: "	+	to_string((long long)(int)adaptation_field_length_				)+"\n";
	packet_info_buffer_ +="  discontinuity_indicator: "	+	to_string((long long)discontinuity_indicator_					)+"\n";
	packet_info_buffer_ +="  random_access_indicator: "	+	to_string((long long)random_access_indicator_					)+"\n";
	packet_info_buffer_ +="  ES_priority_indicator: "	+	to_string((long long)elementary_stream_priority_indicator_		)+"\n";
	packet_info_buffer_ +="  PCR_flag: "				+	to_string((long long)pcr_flag_									)+"\n";
	packet_info_buffer_ +="  OPCR_flag: "				+	to_string((long long)opcr_flag_									)+"\n";
	packet_info_buffer_ +="  splicing_point_flag: "		+	to_string((long long)splicing_point_flag_						)+"\n";
	packet_info_buffer_ +="  transport_private_data_flag: "		+	to_string((long long)transport_private_data_flag_	)+"\n";
	packet_info_buffer_ +="  adaptation_field_extension_flag: "	+	to_string((long long)adaptation_field_extension_flag_)+"\n";

	if(pcr_flag_){
		packet_info_buffer_ +="  PCR : "	+	to_string((long long)result_pcr_	)+"\n";
	}

	if(opcr_flag_){
		packet_info_buffer_ +="  OPCR : "	+	to_string((long long)result_opcr_	)+"\n";
	}

	if(splicing_point_flag_)
	{
		packet_info_buffer_ +="  spliceCountdown : "	+	to_string((long long)splice_countdown_	)+"\n";
	}

	if(transport_private_data_flag_)
	{
		packet_info_buffer_ +="  transportPrivateDataLength : "	+	to_string((long long)transport_private_data_length_	)+"\n";
		for(int i=0; i<transport_private_data_length_; i++)
		{
			packet_info_buffer_ +="  privateDataByte : "	+	to_string((long long)private_data_byte_[i]	)+"\n";
		}	
	}

	if(adaptation_field_extension_flag_)
	{
		packet_info_buffer_ +="  adaptationFieldExtensionLength : "	+	to_string((long long)adaptation_field_extension_length_	)+"\n";
		packet_info_buffer_ +="  ltwFlag : "				+	to_string((long long)ltw_flag_				)+"\n";
		packet_info_buffer_ +="  piecewiseRateFlag : "	+	to_string((long long)piecewise_rate_flag_	)+"\n";
		packet_info_buffer_ +="  seamlessSpliceFlag : "	+	to_string((long long)seamless_splice_flag_	)+"\n";

		if(ltw_flag_)
		{
			packet_info_buffer_ +="  ltwValidFlag : "	+	to_string((long long)ltw_valid_flag_	)+"\n";
			packet_info_buffer_ +="  ltwOffset : "	+	to_string((long long)ltw_offset_		)+"\n";
		}

		if(piecewise_rate_flag_)
		{
			packet_info_buffer_ +="  piecewiseRate : "	+	to_string((long long)piecewise_rate_	)+"\n";
		}

		if(seamless_splice_flag_)
		{
			packet_info_buffer_ +="  spliceType : "	+	to_string((long long)splice_type_	)+"\n";
			packet_info_buffer_ +="  DTSNextAU : "	+	to_string((long long)dts_next_au_	)+"\n";
		}
	}
	packet_info_buffer_ +="  \n";
	is_exist_data_ = false;
	}
}