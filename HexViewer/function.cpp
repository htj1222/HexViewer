#include "StdAfx.h"
#include "function.h"
#include "TSPacket.h"
#include <stdio.h>
#include <math.h>		// ceil()
#include <string.h>		// memset()
#include <ctype.h>		// isprint()
#include <sys/stat.h>	// _stati64()

#define CONSOLE_COLS  80//콘솔창 넓이
#define PACKET_SIZE  188//packet사이즈
#define HEX_LINES 12	//출력할라인수
#define SPACER    15	//중간에 띄우기


function::function(void)
{
}

function::~function(void)
{
}

//파일의 사이즈를 가져옴
__int64 getFileSize(char *filename) {
	struct _stati64 statbuf;

	if ( _stati64(filename, &statbuf) ) return -1; // 파일 정보 얻기: 에러 있으면 -1 을 반환

	return statbuf.st_size;                        // 파일 크기 반환
}

//사이즈를 구하고 패킷단위로 나눈다
__int64 function::getPacketCount(FILE *pSrc, char* fileName ) {
	long long fileSize;
	
	fseek(pSrc, 0, SEEK_END);
	fileSize = ftell(pSrc);	
	rewind(pSrc);

	fileSize = getFileSize(fileName);

	return (__int64) ceil(fileSize / (double)(PACKET_SIZE));
}

//패킷데이터 저장
void function::GetPacketData(FILE *pSrc, int* packetData)
{
	for(int i=0; i<PACKET_SIZE; i++)
	{
		packetData[i] = fgetc(pSrc);
	}
}

//출력
void function::printHex(FILE *pSrc, long long page) {
	int c;
	unsigned hexIndex = 0, printablesIndex = 0, readBytes = 0;
	char currentLine[CONSOLE_COLS];	
	int packetData[PACKET_SIZE];

	//페이지만큼 이동시킨다.
	fseek(pSrc, page * PACKET_SIZE, 0);

	//패킷사이즈만큼 데이터를 받아서 넣는다.
	GetPacketData(pSrc, packetData);

	//패킷분석을 위해 값을 보낸다.
	packetInfo.HeaderInfo(packetData);

	//출력할 라인을 초기화
	memset(currentLine, ' ', CONSOLE_COLS); //공백으로 배열초기화
	currentLine[CONSOLE_COLS - 1] = '\0';	//NULL추가

	//패킷을 출력
	for (int i=0; i<PACKET_SIZE; i++) {			
		c = packetData[i];

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
			puts(currentLine);

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
}