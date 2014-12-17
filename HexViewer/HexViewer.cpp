// HexViewer.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//


#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h> // system()
#include <string.h> // memset()
#include <ctype.h>  // tolower()
#include <conio.h>  // getch()

//char* fileName = "UHD_fin2.mpg";
char* fileName = "notake.mpg";

int _tmain(int argc, const char* argv[])
{
	__int64 currentPacket = 0, packetCount = 0;
	FILE *pFile;
	function _function;

	/*
	if (argc != 2) {
	printf("Usage: %s <file name>\n", argv[0]);
	exit(1);
	}
	*/

	if ((pFile = fopen(fileName, "rb")) == NULL)
	{
		perror("Cannot open file");
		exit(1);
	}

	packetCount = _function.getPacketCount(pFile, fileName);

	while(1) {
		system("cls");

		printf("File: %s\n", fileName);
		printf("\nPacket %I64u/%I64u < (이전) > (다음) Q (종료)?\n\n", currentPacket + 1, packetCount);

		_function.printHex(pFile, currentPacket);
		_function.packetInfo.printInfo();

		switch (tolower(getch())) {
	  case ',':
		  if (currentPacket > 0) --currentPacket;
		  break;
	  case '.':
		  if (currentPacket+1 < packetCount) ++currentPacket;
		  break;
	  case 'p':
		  
		  break;
	  case 'q':
		  fclose(pFile);
		  return 0;
		}
	}

	return 0;
}
