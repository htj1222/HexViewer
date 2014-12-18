#include "StdAfx.h"
#include "TSPacket.h"
#include "PESPacket.h"
#include <iostream>
using namespace std;

TSPacket::TSPacket(void)
{
	Init();
}

TSPacket::~TSPacket(void)
{
	//free(privateDataByte);
}

void TSPacket::HeaderInfo(int* data)
{
	//초기화
	pos=0;

	syncbyte = data[pos];									//8bit
	transportErrorIndicator	  = (data[pos+1]>>7		  ) == 1 ? true : false;	//1bit
	payloadUnitStartIndicator = (data[pos+1]>>6 & 0x01) == 1 ? true : false;	//1bit
	transportPriorityIndicator= (data[pos+1]>>5 & 0x01) == 1 ? true : false;	//1bit

	unsigned short tempPID = data[pos+1]<<3 & 0xF8;		//5bit
	tempPID = tempPID << 5;								
	tempPID += data[pos+2];							//5+8bit 13bit
	PID = tempPID;									//13bit

	transportScramblingControl	= data[pos+3]>>6;		//2bit
	adaptationFieldControl		= data[pos+3]>>4 & 0x03;//2bit
	continuityCounter			= data[pos+3]	 & 0x0F;//4bit

	PlusDataPosition(4);//32bit

	PrintHeaderInfo();

	if(adaptationFieldControl == 2 || adaptationFieldControl == 3){
		AdaptationField(data, adaptationFieldControl);
		PrintAdaptationInfo();
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
			//PESPacket Start
			PESPacket pesPacket;
			pesPacket.SetPos(pos);
			pesPacket.HeaderInfo(data);
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

void TSPacket::AdaptationField(int* data, unsigned char adaptationFieldControl)
{
	adaptationFieldLength	= data[pos];
	PlusDataPosition(1);//8bit

	//길이가 0이상일 경우
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

		//PCR플레그가 1이면 PCR값을 구한다
		//+48bit (6)
		if(PCRFlag)
		{
			uint64 tempPCR = 0;
			tempPCR += data[pos  ] << 25;	//8bit씩 저장//6
			tempPCR += data[pos+1] << 17;	//8bit씩 저장//7
			tempPCR += data[pos+2] << 9;	//8bit씩 저장//8
			tempPCR += data[pos+3] << 1;	//8bit씩 저장//9
			tempPCR += data[pos+4] >> 7;	//마지막 1bit//10
			programClockReferenceBase = tempPCR;	//33bit

			reserved;	//미사용하는 값			//6bit

			tempPCR = 0;
			tempPCR += (data[pos+4] & 0x01) << 8;	//상위 1bit저장//10
			tempPCR += data[pos+5];				//8bit저장//11
			programClockReferenceExtension = tempPCR;//9bit

			//PCR값 정의 PCR(i) = PCR_base(i) x 300 + PCR_ext(i)에 따라 값 계산후 저장
			resultPCR = programClockReferenceBase * 300 + programClockReferenceExtension;

			PlusDataPosition(6);//+48bit
		}

		//OPCR플레그가 1이면 OPCR을 구한다.
		//+48bit (6)
		if(OPCRFlag)
		{
			uint64 tempOPCR = 0;
			tempOPCR += data[pos  ] << 25;	//8bit씩 저장
			tempOPCR += data[pos+1] << 17;	//8bit씩 저장
			tempOPCR += data[pos+2] << 9;	//8bit씩 저장
			tempOPCR += data[pos+3] << 1;	//8bit씩 저장
			tempOPCR += data[pos+4] >> 7;	//마지막 1bit
			programClockReferenceBase = tempOPCR;	//33bit

			reserved;	//미사용하는 값			//6bit

			tempOPCR = 0;
			tempOPCR += (data[pos+4] & 0x01) << 8;	//상위 1bit저장
			tempOPCR += data[pos+5];				//8bit저장
			programClockReferenceExtension = tempOPCR;//9bit

			//PCR값 정의 PCR(i) = PCR_base(i) x 300 + PCR_ext(i)에 따라 값 계산후 저장
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
			
			//사이즈 지정하여 생성
			privateDataByte = (uint8 *) malloc(sizeof(uint8) * transportPrivateDataLength); 
			for(int i=0; i<transportPrivateDataLength; i++)
			{
				privateDataByte[i] = data[pos];
				//8bit씩 증가
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
}

void TSPacket::PrintHeaderInfo()
{
	cout<<"\n == Transport packet fields == "<< endl;
	cout<<"syncbyte: "					<<	hex << (int)syncbyte			<<endl;
	cout<<"transportErrorIndicator: "	<<	transportErrorIndicator			<<endl;
	cout<<"payloadUnitStartIndicator: "	<<	payloadUnitStartIndicator		<<endl;
	cout<<"transportPriorityIndicator: "<<	transportPriorityIndicator		<<endl;
	cout<<"PID: "						<<	dec << PID						<<endl;
	cout<<"transportScramblingControl: "<<	(int)transportScramblingControl	<<endl;
	cout<<"adaptationFieldControl: "	<<	(int)adaptationFieldControl		<<endl;
	cout<<"continuityCounter: "			<<	(int)continuityCounter			<<endl<<endl;	
}

void TSPacket::PrintAdaptationInfo()
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
}