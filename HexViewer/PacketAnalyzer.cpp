#include "StdAfx.h"

PacketAnalyzer::PacketAnalyzer(char* file_name)
{
	//file_name_ = "TEST150_ASSET11.mpg";
	file_name_ = file_name;
	FileOpen(file_name_);			//파일 오픈
	SetTotalPacket();				//총 패킷수를 설정
	SetPidValueInit();				//pid구분과 cc를 위한 초기화
}

PacketAnalyzer::~PacketAnalyzer(void)
{
}

void PacketAnalyzer::CloseFile()
{
	fclose(p_file_);
}


void PacketAnalyzer::FileOpen(char* file_name_){
	if ((p_file_ = fopen(file_name_, "rb")) == NULL)	{
		perror("Cannot open file");
		exit(1);
	}
}

//파일의 사이즈를 가져옴
__int64 PacketAnalyzer::GetFileSize() {
	struct _stati64 statbuf;

	if ( _stati64(file_name_, &statbuf) ) return -1; // 파일 정보 얻기: 에러 있으면 -1 을 반환

	return statbuf.st_size;                        // 파일 크기 반환
}

//사이즈를 구하고 패킷단위로 나눈다
void PacketAnalyzer::SetTotalPacket() {
	total_packet_ = 0;

	long long fileSize = GetFileSize();
	total_packet_ = (__int64) ceil(fileSize / (double)(PACKET_SIZE));	//총 패킷의 수를 구함
}

__int64 PacketAnalyzer::GetCurrentPacket()	{return current_packet_;}
__int64 PacketAnalyzer::GetTotalPacket()	{return total_packet_;	}
string  PacketAnalyzer::GetSendBuffer()		{return send_buffer_;	}

void PacketAnalyzer::FindPakcetData(__int64 num)
{
	current_packet_ = num;
	GetPacketData();
}

void PacketAnalyzer::GetPacketData()
{
	//페이지만큼 이동시킨다.
	fseek(p_file_, current_packet_ * PACKET_SIZE, 0);

	//패킷사이즈만큼 데이터를 받아서 넣는다.
	fread(packet_data_, PACKET_SIZE, 1, p_file_);
}

//다음 패킷데이터
void PacketAnalyzer::GetNextPacketData()
{
	if (current_packet_+1 < total_packet_){		
		++current_packet_;
	}
}

//이전 패킷데이터 
void PacketAnalyzer::GetPreviousPacketData()
{
	if ((current_packet_) > 0){
		--current_packet_;
	}
}


void PacketAnalyzer::TSPacketDataAnalysis()
{
	//패킷분석을 위해 값을 보낸다.
	ts_packet_.GetHeaderInfo(packet_data_);
}


void PacketAnalyzer::Reset() 
{
	ts_packet_.Reset();
}

//cc를 위한 초기화 함수
void PacketAnalyzer::SetPidValueInit()
{
	FindPakcetData(1);				//데이터 저장
	TSPacketDataAnalysis();			//패킷 분석 시작
	ts_packet_.SetPidValueInit(packet_data_);
	Reset();						//값 리셋

	//초기화
	current_packet_ = 0;
}

//cc체크
void PacketAnalyzer::CheckContinuityCounter()
{
	current_packet_ = 0;
	while(current_packet_ != (total_packet_-1)){
		GetNextPacketData();
		ts_packet_.CheckContinuityCounter(packet_data_);
	}
}
//cc검사후 오류 개수 출력
void PacketAnalyzer::PrintErrorCount()
{
	cout << "error(" <<  ts_packet_.cc_error_counter_ << ")" << endl;
}

//출력정보 저장
void PacketAnalyzer::SetPrintInfo() 
{	
	ts_packet_.SetPrintHeaderInfo();
	SetPrintHexData();

	send_buffer_ = ts_packet_.GetPacketInfoBuffer() + hex_data_buffer_;
}


//출력
void PacketAnalyzer::PrintInfo() 
{	
	ts_packet_.PrintHeaderInfo();
	PrintHex();
}

void PacketAnalyzer::SetPrintHexData() {
	int c;
	unsigned hexIndex = 0, printablesIndex = 0, readBytes = 0;
	char currentLine[CONSOLE_COLS];
	
	hex_data_buffer_ = "";

	//출력할 라인을 초기화
	memset(currentLine, ' ', CONSOLE_COLS); //공백으로 배열초기화
	currentLine[CONSOLE_COLS - 1] = '\0';	//NULL추가

	//패킷을 출력
	for (int i=0; i<PACKET_SIZE; i++) {			
		c = packet_data_[i];

		//끝이아니라면
		if (c != EOF) {
			//hex값 부터 출력
			sprintf(currentLine + hexIndex, " %02x", c);
			currentLine[hexIndex + 3] = ' ';			// removes the \0, inserted by sprintf() after the hex byte

			//오른쪽에 출력할수있는 값이라면 출력하고 아니면 .으로 출력
			if (isprint(c))
				// 48 = 16 hex * 3 chars
				currentLine[48 + SPACER + printablesIndex++] = c;
			else
				currentLine[48 + SPACER + printablesIndex++] = '.';

			hexIndex += 3;
		}

		//한줄에 16자리이므로 16인지 체크, 끝인지 체크, 마지막인지 체크
		if (printablesIndex == 16 || c == EOF || i == (PACKET_SIZE-1)) {
			//출력
			//puts(currentLine);
			hex_data_buffer_ += currentLine;
			hex_data_buffer_ += "\n";

			//마지막이라면 끝
			if (c == EOF)
				break;

			//값 리셋
			hexIndex = 0;
			printablesIndex = 0;
			memset(currentLine, ' ', CONSOLE_COLS);
			currentLine[CONSOLE_COLS - 1] = '\0';
		}
	}

	memset(packet_data_ , ' ', PACKET_SIZE+1);
}

void PacketAnalyzer::PrintHex() {
	char bufffer[1024] = "";
	strcpy(bufffer,hex_data_buffer_.c_str());
	cout << bufffer;
}