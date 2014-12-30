#include "StdAfx.h"


TSPacket::TSPacket(void)
{
	Init();
}

TSPacket::~TSPacket(void)
{
}

//ccüũ, pid������ ���� �ʱ�ȭ
void TSPacket::SetPidValueInit(unsigned char* data)
{
	cc_error_counter_ = 0;

    list<ContinuityCounterValue>::iterator list_iter;	//STL�� list
	list_iter=cc_list_.begin();							//������ ����Ű���� �Ѵ�.
	
	list<ContinuityCounterValue>::iterator find_pid;//������ pid�˻��
	
	//pid���� ���� ������� �߰�
	for(int i=0; i<pmt_packet_.stream_info_size_; i++)
	{
		FindSamePID compare_temp;
		compare_temp.ComparePID = (int)pmt_packet_.stream_info_[i].elementary_pid_;//pid�� ����
		find_pid = find_if( cc_list_.begin(), cc_list_.end(), compare_temp );	//find_if�Լ��� ����Ͽ� �˻�

		if( find_pid != cc_list_.end() )	{
			cout << "error!!";//���� ���� �������
		} else{
			//���� ���� ������� �߰�
			ContinuityCounterValue temp;
			temp.pid_ = compare_temp.ComparePID;
			temp.last_continuity_counter_ = 0;

			if(pmt_packet_.isAudioStreamType(pmt_packet_.stream_info_[i].stream_type_)){
				temp.is_video_ = false;
			} else{
				temp.is_video_ = true;
			}
			
			cc_list_.push_back(temp);			//�߰�
		}
	}
}
  
void TSPacket::CheckContinuityCounter(unsigned char* data)
{
	ContinuityCounterValue temp;
	temp.pid_ = ( ( data[1] & 0x1F	)<<8 ) | (data[2]);	
	temp.last_continuity_counter_	= data[3]	 & 0x0F;//4bit
	adaptation_field_control_		= data[3]>>4 & 0x03;//2bit

	if(temp.pid_ != 8191){ //8191 = 1FFF(NULL��)
		list<ContinuityCounterValue>::iterator find_pid;//�˻��� iterator
		FindSamePID compare_temp;
		compare_temp.ComparePID = temp.pid_;
		find_pid = find_if( cc_list_.begin(), cc_list_.end(), compare_temp );//�˻�

		if( find_pid != cc_list_.end() )  //���� pid�� ���
		{	  
			if( (( (*find_pid).last_continuity_counter_)) == temp.last_continuity_counter_) //���� cc�� ������ cc��
			{
				if(++(*find_pid).last_continuity_counter_ == 16)	{//15�̸� 0���� �ʱ�ȭ
					(*find_pid).last_continuity_counter_ = 0;
				}
				//cout << "same cc:"<<(int)(*FindPID).last_continuity_counter_<<". PID : "<< compare_temp.ComparePID <<endl;
			}else{
				if(adaptation_field_control_ == 0 || adaptation_field_control_ == 2){ //Adaptation field control�� 0, 2�� ��� ����X					
					//cout << "same (ada : "<<(int)adaptationFieldControl<<") PID : "<< compare_temp.ComparePID <<endl;
				}else{
					cc_error_counter_++;  //cc ����!!
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
	//�ʱ�ȭ
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

	//���̰� 0�̻��� ���
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

		//PCR�÷��װ� 1�̸� PCR���� ���Ѵ�
		//+48bit (6)
		if(pcr_flag_)
		{
			uint64 temp_pcr = 0;
			temp_pcr += data[pos_  ] << 25;	//8bit�� ����//6
			temp_pcr += data[pos_+1] << 17;	//8bit�� ����//7
			temp_pcr += data[pos_+2] << 9;	//8bit�� ����//8
			temp_pcr += data[pos_+3] << 1;	//8bit�� ����//9
			temp_pcr += data[pos_+4] >> 7;	//������ 1bit//10
			program_clock_reference_base_ = temp_pcr;	//33bit

			reserved_;	//�̻���ϴ� ��			//6bit

			temp_pcr = 0;
			temp_pcr += (data[pos_+4] & 0x01) << 8;	//���� 1bit����//10
			temp_pcr += data[pos_+5];				//8bit����//11
			program_clock_reference_extension_ = temp_pcr;//9bit

			//PCR�� ���� PCR(i) = PCR_base(i) x 300 + PCR_ext(i)�� ���� �� ����� ����
			result_pcr_ = program_clock_reference_base_ * 300 + program_clock_reference_extension_;

			PlusDataPosition(6);//+48bit
		}

		//OPCR�÷��װ� 1�̸� OPCR�� ���Ѵ�.
		//+48bit (6)
		if(opcr_flag_)
		{
			uint64 tempOPCR = 0;
			tempOPCR += data[pos_  ] << 25;	//8bit�� ����
			tempOPCR += data[pos_+1] << 17;	//8bit�� ����
			tempOPCR += data[pos_+2] << 9;	//8bit�� ����
			tempOPCR += data[pos_+3] << 1;	//8bit�� ����
			tempOPCR += data[pos_+4] >> 7;	//������ 1bit
			program_clock_reference_base_ = tempOPCR;	//33bit

			reserved_;	//�̻���ϴ� ��			//6bit

			tempOPCR = 0;
			tempOPCR += (data[pos_+4] & 0x01) << 8;	//���� 1bit����
			tempOPCR += data[pos_+5];				//8bit����
			program_clock_reference_extension_ = tempOPCR;//9bit

			//PCR�� ���� PCR(i) = PCR_base(i) x 300 + PCR_ext(i)�� ���� �� ����� ����
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
			
			//������ �����Ͽ� ����
			private_data_byte_ = (uint8 *) malloc(sizeof(uint8) * transport_private_data_length_); 
			for(int i=0; i<transport_private_data_length_; i++)
			{
				private_data_byte_[i] = data[pos_];
				//8bit�� ����
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

void TSPacket::PrintHeaderInfo()
{
	cout<<"\n == Transport packet fields == "<< endl;
	if(pid_ != 0){
		if(pid_ == 8191){cout<<"  ======= NULL Packet =======" << endl;}else{
			list<ContinuityCounterValue>::iterator FindPID;

			FindSamePID compare_temp;
			compare_temp.ComparePID = pid_;		
			FindPID = find_if( cc_list_.begin(), cc_list_.end(), compare_temp );

			if( FindPID != cc_list_.end() )	{
				if( (( (*FindPID).is_video_)) == true){
					cout << "  ======= Video Packet =======" <<endl;
				}else{
					cout << "  ======= Audio Packet =======" <<endl;
				}
			}
		}
	}else{
		cout << "" << endl;
	}
	cout<<"syncbyte: 0x"				<<	hex << (int)sync_byte_				<<endl;
	cout<<"transportErrorIndicator: "	<<	transport_error_indicator_			<<endl;
	cout<<"payloadUnitStartIndicator: "	<<	payload_unit_start_indicator_		<<endl;
	cout<<"transportPriorityIndicator: "<<	transport_priority_indicator_		<<endl;
	cout<<"PID: "						<<	dec << pid_							<<endl;
	cout<<"transportScramblingControl: "<<	(int)transport_scrambling_control_	<<endl;
	cout<<"adaptationFieldControl: "	<<	(int)adaptation_field_control_		<<endl;
	cout<<"continuityCounter: "			<<	(int)continuity_counter_			<<endl<<endl;	

	PrintAdaptationInfo();
	pes_packet_.PrintPESInfo();
	pat_packet_.PrintPATInfo();
	cat_packet_.PrintCATInfo();
	pmt_packet_.PrintPMTInfo();
}

void TSPacket::PrintAdaptationInfo()
{	
	if(is_exist_data_)
	{
	cout<<" == Adaptation fields == "<< endl;
	cout<<"Adaptation_field_length: "<<	(int)adaptation_field_length_				<<endl;
	cout<<"discontinuity_indicator: "<<	discontinuity_indicator_					<<endl;
	cout<<"random_access_indicator: "<<	random_access_indicator_					<<endl;
	cout<<"ES_priority_indicator: "	<<	elementary_stream_priority_indicator_		<<endl;
	cout<<"PCR_flag: "				<<	pcr_flag_									<<endl;
	cout<<"OPCR_flag: "				<<	opcr_flag_									<<endl;
	cout<<"splicing_point_flag: "	<<	splicing_point_flag_						<<endl;
	cout<<"transport_private_data_flag: "		<<	transport_private_data_flag_	<<endl;
	cout<<"adaptation_field_extension_flag: "	<<	adaptation_field_extension_flag_<<endl;

	if(pcr_flag_){
		cout<<"PCR : "	<<	result_pcr_	<<endl;
	}

	if(opcr_flag_){
		cout<<"OPCR : "	<<	result_opcr_	<<endl;
	}

	if(splicing_point_flag_)
	{
		cout<<"spliceCountdown : "	<<	splice_countdown_	<<endl;
	}

	if(transport_private_data_flag_)
	{
		cout<<"transportPrivateDataLength : "	<<	transport_private_data_length_	<<endl;
		for(int i=0; i<transport_private_data_length_; i++)
		{
			cout<<"privateDataByte : "	<<	private_data_byte_[i]	<<endl;
		}	
	}

	if(adaptation_field_extension_flag_)
	{
		cout<<"adaptationFieldExtensionLength : "	<<	adaptation_field_extension_length_	<<endl;
		cout<<"ltwFlag : "				<<	ltw_flag_				<<endl;
		cout<<"piecewiseRateFlag : "	<<	piecewise_rate_flag_	<<endl;
		cout<<"seamlessSpliceFlag : "	<<	seamless_splice_flag_	<<endl;

		if(ltw_flag_)
		{
			cout<<"ltwValidFlag : "	<<	ltw_valid_flag_	<<endl;
			cout<<"ltwOffset : "	<<	ltw_offset_		<<endl;
		}

		if(piecewise_rate_flag_)
		{
			cout<<"piecewiseRate : "	<<	piecewise_rate_	<<endl;
		}

		if(seamless_splice_flag_)
		{
			cout<<"spliceType : "	<<	splice_type_	<<endl;
			cout<<"DTSNextAU : "	<<	dts_next_au_	<<endl;
		}
	}
	cout<<endl;
	is_exist_data_ = false;
	}
}