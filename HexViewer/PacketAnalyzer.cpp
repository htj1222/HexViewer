#include "StdAfx.h"

PacketAnalyzer::PacketAnalyzer(char* file_name)
{
	//file_name_ = "TEST150_ASSET11.mpg";
	file_name_ = file_name;
	FileOpen(file_name_);			//���� ����
	SetTotalPacket();				//�� ��Ŷ���� ����
	SetPidValueInit();				//pid���а� cc�� ���� �ʱ�ȭ
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

//������ ����� ������
__int64 PacketAnalyzer::GetFileSize() {
	struct _stati64 statbuf;

	if ( _stati64(file_name_, &statbuf) ) return -1; // ���� ���� ���: ���� ������ -1 �� ��ȯ

	return statbuf.st_size;                        // ���� ũ�� ��ȯ
}

//����� ���ϰ� ��Ŷ������ ������
void PacketAnalyzer::SetTotalPacket() {
	total_packet_ = 0;

	long long fileSize = GetFileSize();
	total_packet_ = (__int64) ceil(fileSize / (double)(PACKET_SIZE));	//�� ��Ŷ�� ���� ����
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
	//��������ŭ �̵���Ų��.
	fseek(p_file_, current_packet_ * PACKET_SIZE, 0);

	//��Ŷ�����ŭ �����͸� �޾Ƽ� �ִ´�.
	fread(packet_data_, PACKET_SIZE, 1, p_file_);
}

//���� ��Ŷ������
void PacketAnalyzer::GetNextPacketData()
{
	if (current_packet_+1 < total_packet_){		
		++current_packet_;
	}
}

//���� ��Ŷ������ 
void PacketAnalyzer::GetPreviousPacketData()
{
	if ((current_packet_) > 0){
		--current_packet_;
	}
}


void PacketAnalyzer::TSPacketDataAnalysis()
{
	//��Ŷ�м��� ���� ���� ������.
	ts_packet_.GetHeaderInfo(packet_data_);
}


void PacketAnalyzer::Reset() 
{
	ts_packet_.Reset();
}

//cc�� ���� �ʱ�ȭ �Լ�
void PacketAnalyzer::SetPidValueInit()
{
	FindPakcetData(1);				//������ ����
	TSPacketDataAnalysis();			//��Ŷ �м� ����
	ts_packet_.SetPidValueInit(packet_data_);
	Reset();						//�� ����

	//�ʱ�ȭ
	current_packet_ = 0;
}

//ccüũ
void PacketAnalyzer::CheckContinuityCounter()
{
	current_packet_ = 0;
	while(current_packet_ != (total_packet_-1)){
		GetNextPacketData();
		ts_packet_.CheckContinuityCounter(packet_data_);
	}
}
//cc�˻��� ���� ���� ���
void PacketAnalyzer::PrintErrorCount()
{
	cout << "error(" <<  ts_packet_.cc_error_counter_ << ")" << endl;
}

//������� ����
void PacketAnalyzer::SetPrintInfo() 
{	
	ts_packet_.SetPrintHeaderInfo();
	SetPrintHexData();

	send_buffer_ = ts_packet_.GetPacketInfoBuffer() + hex_data_buffer_;
}


//���
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

	//����� ������ �ʱ�ȭ
	memset(currentLine, ' ', CONSOLE_COLS); //�������� �迭�ʱ�ȭ
	currentLine[CONSOLE_COLS - 1] = '\0';	//NULL�߰�

	//��Ŷ�� ���
	for (int i=0; i<PACKET_SIZE; i++) {			
		c = packet_data_[i];

		//���̾ƴ϶��
		if (c != EOF) {
			//hex�� ���� ���
			sprintf(currentLine + hexIndex, " %02x", c);
			currentLine[hexIndex + 3] = ' ';			// removes the \0, inserted by sprintf() after the hex byte

			//�����ʿ� ����Ҽ��ִ� ���̶�� ����ϰ� �ƴϸ� .���� ���
			if (isprint(c))
				// 48 = 16 hex * 3 chars
				currentLine[48 + SPACER + printablesIndex++] = c;
			else
				currentLine[48 + SPACER + printablesIndex++] = '.';

			hexIndex += 3;
		}

		//���ٿ� 16�ڸ��̹Ƿ� 16���� üũ, ������ üũ, ���������� üũ
		if (printablesIndex == 16 || c == EOF || i == (PACKET_SIZE-1)) {
			//���
			//puts(currentLine);
			hex_data_buffer_ += currentLine;
			hex_data_buffer_ += "\n";

			//�������̶�� ��
			if (c == EOF)
				break;

			//�� ����
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