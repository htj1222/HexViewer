#include "StdAfx.h"
#include "function.h"
#include "TSPacket.h"
#include <stdio.h>
#include <math.h>		// ceil()
#include <string.h>		// memset()
#include <ctype.h>		// isprint()
#include <sys/stat.h>	// _stati64()

#define CONSOLE_COLS  80//�ܼ�â ����
#define PACKET_SIZE  188//packet������
#define HEX_LINES 12	//����Ҷ��μ�
#define SPACER    15	//�߰��� ����


function::function(void)
{
}

function::~function(void)
{
}

//������ ����� ������
__int64 getFileSize(char *filename) {
	struct _stati64 statbuf;

	if ( _stati64(filename, &statbuf) ) return -1; // ���� ���� ���: ���� ������ -1 �� ��ȯ

	return statbuf.st_size;                        // ���� ũ�� ��ȯ
}

//����� ���ϰ� ��Ŷ������ ������
__int64 function::getPacketCount(FILE *pSrc, char* fileName ) {
	long long fileSize;
	
	fseek(pSrc, 0, SEEK_END);
	fileSize = ftell(pSrc);	
	rewind(pSrc);

	fileSize = getFileSize(fileName);

	return (__int64) ceil(fileSize / (double)(PACKET_SIZE));
}

//��Ŷ������ ����
void function::GetPacketData(FILE *pSrc, int* packetData)
{
	for(int i=0; i<PACKET_SIZE; i++)
	{
		packetData[i] = fgetc(pSrc);
	}
}

//���
void function::printHex(FILE *pSrc, long long page) {
	int c;
	unsigned hexIndex = 0, printablesIndex = 0, readBytes = 0;
	char currentLine[CONSOLE_COLS];	
	int packetData[PACKET_SIZE];

	//��������ŭ �̵���Ų��.
	fseek(pSrc, page * PACKET_SIZE, 0);

	//��Ŷ�����ŭ �����͸� �޾Ƽ� �ִ´�.
	GetPacketData(pSrc, packetData);

	//��Ŷ�м��� ���� ���� ������.
	packetInfo.HeaderInfo(packetData);

	//����� ������ �ʱ�ȭ
	memset(currentLine, ' ', CONSOLE_COLS); //�������� �迭�ʱ�ȭ
	currentLine[CONSOLE_COLS - 1] = '\0';	//NULL�߰�

	//��Ŷ�� ���
	for (int i=0; i<PACKET_SIZE; i++) {			
		c = packetData[i];

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
}