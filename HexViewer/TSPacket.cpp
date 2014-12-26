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
	cc_error_counter = 0;

    list<ContinuityCounterValue>::iterator list_iter;	//STL�� list
	list_iter=cc_list_.begin();							//������ ����Ű���� �Ѵ�.
	
	list<ContinuityCounterValue>::iterator FindPID;//������ pid�˻��
	
	//pid���� ���� ������� �߰�
	for(int i=0; i<pmt_packet_.streamInfo_size_; i++)
	{
		FindSamePID compare_temp;
		compare_temp.ComparePID = (int)pmt_packet_.streamInfo[i].elementary_PID;//pid�� ����
		FindPID = find_if( cc_list_.begin(), cc_list_.end(), compare_temp );	//find_if�Լ��� ����Ͽ� �˻�

		if( FindPID != cc_list_.end() )	{
			cout << "error!!";//���� ���� �������
		} else{
			//���� ���� ������� �߰�
			ContinuityCounterValue temp;
			temp.PID_ = compare_temp.ComparePID;
			temp.last_continuity_counter_ = 0;

			if(pmt_packet_.isAudioStreamType(pmt_packet_.streamInfo[i].stream_type)){
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
	temp.PID_ = ( ( data[1] & 0x1F	)<<8 ) | (data[2]);	
	temp.last_continuity_counter_	= data[3]	 & 0x0F;//4bit
	adaptationFieldControl			= data[3]>>4 & 0x03;//2bit

	if(temp.PID_ != 8191){ //8191 = 1FFF(NULL��)
		list<ContinuityCounterValue>::iterator FindPID;//�˻��� iterator
		FindSamePID compare_temp;
		compare_temp.ComparePID = temp.PID_;
		FindPID = find_if( cc_list_.begin(), cc_list_.end(), compare_temp );//�˻�

		if( FindPID != cc_list_.end() )  //���� pid�� ���
		{	  
			if( (( (*FindPID).last_continuity_counter_)) == temp.last_continuity_counter_) //���� cc�� ������ cc��
			{
				if(++(*FindPID).last_continuity_counter_ == 16)	{//15�̸� 0���� �ʱ�ȭ
					(*FindPID).last_continuity_counter_ = 0;
				}
				//cout << "same cc:"<<(int)(*FindPID).last_continuity_counter_<<". PID : "<< compare_temp.ComparePID <<endl;
			}else{
				if(adaptationFieldControl == 0 | adaptationFieldControl == 2){ //Adaptation field control�� 0, 2�� ��� ����X					
					//cout << "same (ada : "<<(int)adaptationFieldControl<<") PID : "<< compare_temp.ComparePID <<endl;
				}else{
					cc_error_counter++;  //cc ����!!
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
	pos=0;

	syncbyte = data[pos];														//8bit
	transportErrorIndicator	  = (data[pos+1]>>7		  ) == 1 ? true : false;	//1bit
	payloadUnitStartIndicator = (data[pos+1]>>6 & 0x01) == 1 ? true : false;	//1bit
	transportPriorityIndicator= (data[pos+1]>>5 & 0x01) == 1 ? true : false;	//1bit

	PID  = ( ( data[pos+1] & 0x1F	)<<8 ) | (data[pos+2]);	

	transportScramblingControl	= data[pos+3]>>6;		//2bit
	adaptationFieldControl		= data[pos+3]>>4 & 0x03;//2bit
	continuityCounter			= data[pos+3]	 & 0x0F;//4bit

	PlusDataPosition(4);//32bit
	
	if(adaptationFieldControl == 2 || adaptationFieldControl == 3){
		GetAdaptationField(data, adaptationFieldControl);
	}

	if(adaptationFieldControl == '01' || adaptationFieldControl == '11') {
		/*for (i = 0; i < N; i++){
			data_byte 8 bslbf
		}*/
	}

	if(payloadUnitStartIndicator)
	{
		if(data[pos] == 0 && data[pos+1] == 0 && data[pos+2] == 1)
		{			
			pes_packet_.SetPos(pos);
			pes_packet_.SetHeaderInfo(data);
		}else if(data[pos] == 0 && data[pos+1] == 0)
		{			
			pat_packet_.SetPos(pos);
			pat_packet_.SetHeaderInfo(data);
		}else if(data[pos] == 0 && data[pos+1] == 1)
		{
			cat_packet_.SetPos(pos);
			cat_packet_.SetHeaderInfo(data);
		}else if(data[pos] == 0 && data[pos+1] == 2)
		{
			pmt_packet_.SetPos(pos);
			pmt_packet_.SetHeaderInfo(data);
		}
	}
}

void TSPacket::PlusDataPosition(int plus)
{
	pos += plus;
}

int  TSPacket::getDataPosition()
{
	return pos;
}

void TSPacket::GetAdaptationField(unsigned char* data, unsigned char adaptationFieldControl)
{
	is_exist_data_ = true;

	adaptationFieldLength	= data[pos];
	PlusDataPosition(1);//8bit

	//���̰� 0�̻��� ���
	if(adaptationFieldLength > 0){
		discontinuityIndicator				= ((data[pos] & 0x80) >> 7) == 1 ? true : false;
		randomAccessIndicator				= ((data[pos] & 0x40) >> 6) == 1 ? true : false;
		elementaryStreamPriorityIndicator	= ((data[pos] & 0x20) >> 5) == 1 ? true : false;
		PCRFlag								= ((data[pos] & 0x10) >> 4) == 1 ? true : false;
		OPCRFlag							= ((data[pos] & 0x08) >> 3) == 1 ? true : false;
		splicingPointFlag					= ((data[pos] & 0x04) >> 2) == 1 ? true : false;
		transportPrivateDataFlag			= ((data[pos] & 0x02) >> 1) == 1 ? true : false;
		adaptationFieldExtensionFlag		= ((data[pos] & 0x01)	  ) == 1 ? true : false;

		PlusDataPosition(1);//8bit

		//PCR�÷��װ� 1�̸� PCR���� ���Ѵ�
		//+48bit (6)
		if(PCRFlag)
		{
			uint64 tempPCR = 0;
			tempPCR += data[pos  ] << 25;	//8bit�� ����//6
			tempPCR += data[pos+1] << 17;	//8bit�� ����//7
			tempPCR += data[pos+2] << 9;	//8bit�� ����//8
			tempPCR += data[pos+3] << 1;	//8bit�� ����//9
			tempPCR += data[pos+4] >> 7;	//������ 1bit//10
			programClockReferenceBase = tempPCR;	//33bit

			reserved;	//�̻���ϴ� ��			//6bit

			tempPCR = 0;
			tempPCR += (data[pos+4] & 0x01) << 8;	//���� 1bit����//10
			tempPCR += data[pos+5];				//8bit����//11
			programClockReferenceExtension = tempPCR;//9bit

			//PCR�� ���� PCR(i) = PCR_base(i) x 300 + PCR_ext(i)�� ���� �� ����� ����
			resultPCR = programClockReferenceBase * 300 + programClockReferenceExtension;

			PlusDataPosition(6);//+48bit
		}

		//OPCR�÷��װ� 1�̸� OPCR�� ���Ѵ�.
		//+48bit (6)
		if(OPCRFlag)
		{
			uint64 tempOPCR = 0;
			tempOPCR += data[pos  ] << 25;	//8bit�� ����
			tempOPCR += data[pos+1] << 17;	//8bit�� ����
			tempOPCR += data[pos+2] << 9;	//8bit�� ����
			tempOPCR += data[pos+3] << 1;	//8bit�� ����
			tempOPCR += data[pos+4] >> 7;	//������ 1bit
			programClockReferenceBase = tempOPCR;	//33bit

			reserved;	//�̻���ϴ� ��			//6bit

			tempOPCR = 0;
			tempOPCR += (data[pos+4] & 0x01) << 8;	//���� 1bit����
			tempOPCR += data[pos+5];				//8bit����
			programClockReferenceExtension = tempOPCR;//9bit

			//PCR�� ���� PCR(i) = PCR_base(i) x 300 + PCR_ext(i)�� ���� �� ����� ����
			resultOPCR = programClockReferenceBase * 300 + programClockReferenceExtension;

			PlusDataPosition(6);//+48bit
		}

		//8bit (1)
		if(splicingPointFlag)
		{
			spliceCountdown = data[pos];
			PlusDataPosition(1);
		}
		
		//8bit+8*nbit
		if(transportPrivateDataFlag)
		{
			//8bit
			transportPrivateDataLength = data[pos];
			PlusDataPosition(1);
			
			//������ �����Ͽ� ����
			privateDataByte = (uint8 *) malloc(sizeof(uint8) * transportPrivateDataLength); 
			for(int i=0; i<transportPrivateDataLength; i++)
			{
				privateDataByte[i] = data[pos];
				//8bit�� ����
				PlusDataPosition(1);
			}			
		}

		if(adaptationFieldExtensionFlag)
		{
			adaptationFieldExtensionLength = data[pos];	//8bit
			PlusDataPosition(1);//+8bit

			ltwFlag				= ((data[pos] & 0x80) >> 7) == 1 ? true : false;	//1bit
			piecewiseRateFlag	= ((data[pos] & 0x40) >> 6) == 1 ? true : false;	//1bit
			seamlessSpliceFlag	= ((data[pos] & 0x20) >> 5) == 1 ? true : false;	//1bit
			PlusDataPosition(1);//5bit reserved + 3bit

			if(ltwFlag)
			{
				ltwValidFlag = ((data[pos] & 0x80) >> 7) == 1 ? true : false;	//1bit
				ltwOffset  = data[pos  ] & 0x7F << 8;							//15bit
				ltwOffset += data[pos+1];										//15bit
				PlusDataPosition(2);//16bit
			}

			if(piecewiseRateFlag)
			{
				//2bit reserved
				piecewiseRate  = (data[pos]	& 0x3F) << 16;	//22bit
				piecewiseRate += (data[pos+1] ) << 8;		//22bit
				piecewiseRate += (data[pos+2] );			//22bit
				PlusDataPosition(3);//24bit
			}

			if(seamlessSpliceFlag)
			{
				spliceType = data[pos] >> 4;			//4bit
				DTSNextAU  = (data[pos]  & 0x0E) << 29;	//3bit
				DTSNextAU += (data[pos+1]	   ) << 22;	//8bit
				DTSNextAU += (data[pos+2]& 0x0E) << 14;	//8bit
				DTSNextAU += (data[pos+3]	   ) << 7;	//8bit
				DTSNextAU += (data[pos+4]& 0x0E) >> 1;	//8bit
				PlusDataPosition(5);//40bit
			}
		}
	}
}
void TSPacket::Init()
{
	syncbyte=0;						//8bit
	transportErrorIndicator=0;		//1bit
	payloadUnitStartIndicator=0;	//1bit
	transportPriorityIndicator=0;	//1bit
	PID=0;							//13bit
	transportScramblingControl=0;	//2bit
	adaptationFieldControl=0;		//2bit
	continuityCounter=0;			//4bit

	adaptationFieldLength=0;			//8bit
	discontinuityIndicator=0;			//1bit
	randomAccessIndicator=0;			//1bit
	elementaryStreamPriorityIndicator=0;//1bit
	PCRFlag=0;							//1bit
	OPCRFlag=0;							//1bit
	splicingPointFlag=0;				//1bit
	transportPrivateDataFlag=0;			//1bit
	adaptationFieldExtensionFlag=0;		//1bit

	//pcr
	programClockReferenceBase=0;		//33bit
	reserved=0;							//6bit
	programClockReferenceExtension=0;	//9bit
	resultPCR=0;

	//opcr
	originalProgramClockReferenceBase=0;		//33bit
	originalProgramClockReferenceExtension=0;	//9bit
	resultOPCR=0;

	//spliceCountdown
	spliceCountdown=0;//8bit

	//privateData
	transportPrivateDataLength=0;	//8bit
	privateDataByte=NULL;			//8*n bit

	//extension
	adaptationFieldExtensionLength=0;	//8bit
	ltwFlag=0;							//1bit
	piecewiseRateFlag=0;				//1bit
	seamlessSpliceFlag=0;				//1bit
	//+5bit reserved

	//ltw
	ltwValidFlag=0; //1bit
	ltwOffset=0;	//15bit

	//piecewise
	piecewiseRate=0;//22bit

	//seamless
	spliceType=0;	//4bit
	DTSNextAU=0;	//36bit

	is_exist_data_= false;
}

void TSPacket::Reset()
{
	if(transportPrivateDataFlag)
	{
		free(privateDataByte);
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
	if(PID != 0){
		if(PID == 8191){cout<<"  ======= NULL Packet =======" << endl;}else{
			list<ContinuityCounterValue>::iterator FindPID;

			FindSamePID compare_temp;
			compare_temp.ComparePID = PID;		
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
	cout<<"syncbyte: 0x"				<<	hex << (int)syncbyte			<<endl;
	cout<<"transportErrorIndicator: "	<<	transportErrorIndicator			<<endl;
	cout<<"payloadUnitStartIndicator: "	<<	payloadUnitStartIndicator		<<endl;
	cout<<"transportPriorityIndicator: "<<	transportPriorityIndicator		<<endl;
	cout<<"PID: "						<<	dec << PID						<<endl;
	cout<<"transportScramblingControl: "<<	(int)transportScramblingControl	<<endl;
	cout<<"adaptationFieldControl: "	<<	(int)adaptationFieldControl		<<endl;
	cout<<"continuityCounter: "			<<	(int)continuityCounter			<<endl<<endl;	

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
	cout<<"Adaptation_field_length: "<<	(int)adaptationFieldLength			<<endl;
	cout<<"discontinuity_indicator: "<<	discontinuityIndicator				<<endl;
	cout<<"random_access_indicator: "<<	randomAccessIndicator				<<endl;
	cout<<"ES_priority_indicator: "	<<	elementaryStreamPriorityIndicator	<<endl;
	cout<<"PCR_flag: "				<<	PCRFlag								<<endl;
	cout<<"OPCR_flag: "				<<	OPCRFlag							<<endl;
	cout<<"splicing_point_flag: "	<<	splicingPointFlag					<<endl;
	cout<<"transport_private_data_flag: "		<<	transportPrivateDataFlag		<<endl;
	cout<<"adaptation_field_extension_flag: "	<<	adaptationFieldExtensionFlag	<<endl;

	if(PCRFlag){
		cout<<"PCR : "	<<	resultPCR	<<endl;
	}

	if(OPCRFlag){
		cout<<"OPCR : "	<<	resultOPCR	<<endl;
	}

	if(splicingPointFlag)
	{
		cout<<"spliceCountdown : "	<<	spliceCountdown	<<endl;
	}

	if(transportPrivateDataFlag)
	{
		cout<<"transportPrivateDataLength : "	<<	transportPrivateDataLength	<<endl;
		for(int i=0; i<transportPrivateDataLength; i++)
		{
			cout<<"privateDataByte : "	<<	privateDataByte[i]	<<endl;
		}	
	}

	if(adaptationFieldExtensionFlag)
	{
		cout<<"adaptationFieldExtensionLength : "	<<	adaptationFieldExtensionLength	<<endl;
		cout<<"ltwFlag : "				<<	ltwFlag				<<endl;
		cout<<"piecewiseRateFlag : "	<<	piecewiseRateFlag	<<endl;
		cout<<"seamlessSpliceFlag : "	<<	seamlessSpliceFlag	<<endl;

		if(ltwFlag)
		{
			cout<<"ltwValidFlag : "	<<	ltwValidFlag	<<endl;
			cout<<"ltwOffset : "	<<	ltwOffset		<<endl;
		}

		if(piecewiseRateFlag)
		{
			cout<<"piecewiseRate : "	<<	piecewiseRate	<<endl;
		}

		if(seamlessSpliceFlag)
		{
			cout<<"spliceType : "	<<	spliceType	<<endl;
			cout<<"DTSNextAU : "	<<	DTSNextAU	<<endl;
		}
	}
	cout<<endl;
	is_exist_data_ = false;
	}
}