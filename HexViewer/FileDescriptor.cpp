#include "StdAfx.h"

FileDescriptor::FileDescriptor(void)
{
	//file_name_ = "TEST150_ASSET11.mpg";
	file_name_ = "notake.mpg";
	FileOpen();
}

FileDescriptor::~FileDescriptor(void)
{
}

void FileDescriptor::CloseFile()
{
	fclose(p_file_);
}


void FileDescriptor::FileOpen(){
	if ((p_file_ = fopen(file_name_, "rb")) == NULL)	{
		perror("Cannot open file");
		exit(1);
	}
}

//������ ����� ������
__int64 FileDescriptor::GetFileSize() {
	struct _stati64 statbuf;

	if ( _stati64(file_name_, &statbuf) ) return -1; // ���� ���� ���: ���� ������ -1 �� ��ȯ

	return statbuf.st_size;                        // ���� ũ�� ��ȯ
}

//����� ���ϰ� ��Ŷ������ ������
__int64 FileDescriptor::GetPacketCount() {
	long long fileSize;
	
	fileSize = GetFileSize();

	return (__int64) ceil(fileSize / (double)(PACKET_SIZE));
}

//��Ŷ������ ����
void FileDescriptor::GetPacketData(long long page)
{
	//��������ŭ �̵���Ų��.
	fseek(p_file_, page * PACKET_SIZE, 0);

	//��Ŷ�����ŭ �����͸� �޾Ƽ� �ִ´�.
	fread(packet_data_, 1, PACKET_SIZE , p_file_);
}

void FileDescriptor::TSPacketDataAnalysis()
{
	//��Ŷ�м��� ���� ���� ������.
	ts_packet_.GetHeaderInfo(packet_data_);
}

//���
void FileDescriptor::PrintInfo() 
{
	ts_packet_.PrintHeaderInfo();
}

void FileDescriptor::Reset() 
{
	ts_packet_.Reset();
}

//cc�� ���� �ʱ�ȭ �Լ�
void FileDescriptor::SetPidValueInit()
{
	GetPacketData(1);				//������ ����
	TSPacketDataAnalysis();			//��Ŷ �м� ����
	ts_packet_.SetPidValueInit(packet_data_);
}

void FileDescriptor::CheckContinuityCounter()
{
	ts_packet_.CheckContinuityCounter(packet_data_);
}

void FileDescriptor::PrintErrorCount()
{
	cout << "error(" <<  ts_packet_.cc_error_counter << ")" << endl;
}

void FileDescriptor::PrintHex() {
	int c;
	unsigned hexIndex = 0, printablesIndex = 0, readBytes = 0;
	char currentLine[CONSOLE_COLS];	
	
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
			puts(currentLine);

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