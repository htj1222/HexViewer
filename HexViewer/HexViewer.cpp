// HexViewer.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h> // system()
#include <string.h> // memset()
#include <ctype.h>  // tolower()
#include <conio.h>  // getch()
#include "HTTPConnect.h"
#include "ServerSocket.h"
#include <time.h> //clock(), time_t변수

int _tmain(int argc, const char* argv[])
{
	__int64 currentPacket = 0, packetCount = 0;
	
	FileDescriptor descriptor;
	packetCount = descriptor.GetPacketCount();
	char key;		
	
	/*
	float gap;
    time_t startTime=0, endTime=0;

    printf("측정을 시작합니다...\n") ;
    //측정 시작
    startTime=clock();*/
	while(1) {
		system("cls");
		
		descriptor.GetPacketData(currentPacket);
		descriptor.PrintInfo();
		descriptor.PrintHex();
		descriptor.Reset();

		printf("\nPacket: %I64u/%I64u    1 (이전) 2 (다음) F(찾기) Q (종료)? : ", currentPacket + 1, packetCount);
		cin >> key;
		
		switch (tolower(key)) {
	  case '1':
		  if (currentPacket > 0) --currentPacket;
		  break;
	  case '2':
		  if (currentPacket+1 < packetCount) ++currentPacket;		  
		  break;
	  case 'f':
		  __int64 temp;
		  cin >> (__int64)temp;
		  if(temp >0 && temp <= packetCount)
			  currentPacket = temp-1;		  
		  break;
	  case 'q':
		  descriptor.Reset();
		  descriptor.CloseFile();		  
		  return 0;
		}

	}

	/*
	//측정 끝
    endTime=clock();
    printf("측정이 끝났습니다...\n") ;

    // 시간 계산
    gap=(float)(endTime-startTime)/(CLOCKS_PER_SEC); //계산

    // 측정 시간 출력
    printf("측정 시간 : %f 초\n", gap);*/

	return 0;
}
