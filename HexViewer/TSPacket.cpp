#include "StdAfx.h"
#include "TSPacket.h"
#include <iostream>
using namespace std;

TSPacket::TSPacket(void)
{
}


TSPacket::~TSPacket(void)
{
	//free(privateDataByte);
}


void TSPacket::HeaderInfo(int* data)
{
	syncbyte = data[0];									//8bit
	transportErrorIndicator	  = (data[1]>>7		  ) == 1 ? true : false;	//1bit
	payloadUnitStartIndicator = (data[1]>>6 & 0x01) == 1 ? true : false;	//1bit
	transportPriorityIndicator= (data[1]>>5 & 0x01) == 1 ? true : false;	//1bit

	unsigned short tempPID = data[1]<<3 & 0xF8;		//5bit
	tempPID = tempPID << 5;								
	tempPID += data[2];								//5+8bit 13bit
	PID = tempPID;									//13bit

	transportScramblingControl	= data[3]>>6;		//2bit
	adaptationFieldControl		= data[3]>>4 & 0x03;//2bit
	continuityCounter			= data[3]	 & 0x0F;//4bit

	if(adaptationFieldControl == 2 || adaptationFieldControl == 3){
		AdaptationField(data, adaptationFieldControl);
	}
	if(adaptationFieldControl == '01' || adaptationFieldControl == '11') {
		/*for (i = 0; i < N; i++){
			data_byte 8 bslbf
		}*/
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
	//초기화
	pos=0;
	PlusDataPosition(4);

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
				ltwOffset = data[pos] & 0x7FFF;									//15bit
				PlusDataPosition(2);//16bit
			}

			if(piecewiseRateFlag)
			{
				//2bit reserved
				piecewiseRate = data[pos] & 0x7FFFFF;	//22bit
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

void TSPacket::printInfo() {
	cout<<" == Adaptation fields == "<< endl;
	cout<<"Adaptation_field_length: "<<	adaptationFieldLength				<<endl;
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
}