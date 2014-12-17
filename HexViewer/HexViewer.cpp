// HexViewer.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
#include "stdafx.h"
#include <stdio.h>
#include <iostream>
#include <stdlib.h> // system()
#include <string.h> // memset()
#include <ctype.h>  // tolower()
#include <conio.h>  // getch()

//char* fileName = "UHD_fin2.mpg";
char* fileName = "notake.mpg";
using namespace std;

int _tmain(int argc, const char* argv[])
{
	__int64 currentPacket = 0, packetCount = 0;
	FILE *pFile;
	function _function;
	char key;

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
		

		_function.printHex(pFile, currentPacket);
		_function.packetInfo.printInfo();

		printf("\nPacket %I64u/%I64u 1 (이전) 2 (다음) Q (종료)? : ", currentPacket + 1, packetCount);
		cin >> key;

		switch (tolower(key)) {
	  case '1':
		  if (currentPacket > 0) --currentPacket;
		  break;
	  case '2':
		  if (currentPacket+1 < packetCount) ++currentPacket;
		  break;
	  case 'q':
		  fclose(pFile);
		  return 0;
		}
	}

	return 0;
}
