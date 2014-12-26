
#pragma once
#include "StdAfx.h"
#include "PESPacket.h"
#include "PATPacket.h"
#include "CATPacket.h"
#include "PMTPacket.h"

#include <list>
#include <iostream>
#include <algorithm> //find()를 위해 필요 

using namespace std;

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned int   uint32;
typedef unsigned long  uint64;

struct ContinuityCounterValue
{	
	uint16 PID_;	
	uint8 last_continuity_counter_;			//4bit
	bool is_video_;
};

struct FindSamePID
{

	bool operator() ( ContinuityCounterValue& value ) const { return value.PID_ == ComparePID; }
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
	int pos;

	//Partial Transport Stream Packet Format
	uint8 syncbyte;						//8bit
	bool transportErrorIndicator;		//1bit
	bool payloadUnitStartIndicator;		//1bit
	bool transportPriorityIndicator;	//1bit
	uint16 PID;							//13bit
	uint8 transportScramblingControl;	//2bit
	uint8 adaptationFieldControl;		//2bit
	uint8 continuityCounter;			//4bit

	//Adaptation Field Format
	uint8 adaptationFieldLength;			//8bit
	bool discontinuityIndicator;			//1bit
	bool randomAccessIndicator;				//1bit
	bool elementaryStreamPriorityIndicator;	//1bit
	bool PCRFlag;							//1bit
	bool OPCRFlag;							//1bit
	bool splicingPointFlag;					//1bit
	bool transportPrivateDataFlag;			//1bit
	bool adaptationFieldExtensionFlag;		//1bit

	//pcr
	uint64 programClockReferenceBase;		//33bit
	uint8  reserved;						//6bit
	uint64 programClockReferenceExtension;	//9bit
	uint64 resultPCR;

	//opcr
	uint64 originalProgramClockReferenceBase;		//33bit
	uint64 originalProgramClockReferenceExtension;	//9bit
	uint64 resultOPCR;

	//spliceCountdown
	uint8 spliceCountdown;//8bit

	//privateData
	uint8 transportPrivateDataLength;//8bit
	uint8* privateDataByte;			//8*n bit

	//extension
	uint8 adaptationFieldExtensionLength;	//8bit
	bool ltwFlag;							//1bit
	bool piecewiseRateFlag;					//1bit
	bool seamlessSpliceFlag;				//1bit
	//+5bit reserved

	//ltw
	bool ltwValidFlag; //1bit
	uint16 ltwOffset; //15bit

	//piecewise
	uint32 piecewiseRate;//22bit

	//seamless
	uint8 spliceType;//4bit
	uint64 DTSNextAU;//36bit
	
	list<ContinuityCounterValue> cc_list_;
	
public:
	int cc_error_counter;

	TSPacket(void);
	~TSPacket(void);

	void Init();
	void SetPidValueInit(unsigned char* data);

	void GetHeaderInfo(unsigned char* data);
	int  getDataPosition();
	void PlusDataPosition(int plus);
	void GetAdaptationField(unsigned char* data, unsigned char adaptationFieldControl);

	void PrintHeaderInfo();
	void PrintAdaptationInfo();
	
	void CheckContinuityCounter(unsigned char* data);

	void Reset();
};
