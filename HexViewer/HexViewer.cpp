// HexViewer.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h> // system()
#include <string.h> // memset()
#include <ctype.h>  // tolower()
#include <conio.h>  // getch()
#include <time.h> //clock(), time_t변수
#include "HTTPConnect.h"
#include "ServerSocket.h"


int _tmain(int argc, const char* argv[])
{
	PacketAnalyzer packet_analyzer("data\\notake.mpg");
	
	char key;
	int counter;
	//시간측정용 변수
	float gap;
    time_t startTime=0, endTime=0;

	/*
    printf("측정을 시작합니다...\n") ;
    //측정 시작
    startTime=clock();
	*/

	ServerSocket server;
	server.StartServer();
	
	while(1) {
		system("cls");

		packet_analyzer.GetPacketData();
		packet_analyzer.TSPacketDataAnalysis();		//패킷 분석 시작
		packet_analyzer.SetPrintInfo();				//출력값 저장
		packet_analyzer.PrintInfo();				//정보출력		
		packet_analyzer.Reset();					//값 리셋

		printf("\nPacket: %I64u/%I64u    1 (이전) 2 (다음) F(찾기) Q (종료)? : ", packet_analyzer.GetCurrentPacket()+1, packet_analyzer.GetTotalPacket());
		cin >> key;
		
		switch (tolower(key)) {

		case '1':
			packet_analyzer.GetPreviousPacketData();
		  break;

		case '2':
			packet_analyzer.GetNextPacketData();			
			break;

		case 'f':
			__int64 temp;
			cin >> (__int64)temp;
			if(temp >0 && (temp) <= packet_analyzer.GetTotalPacket())
				packet_analyzer.FindPakcetData(temp-1);
			break;

		case 'c':
			packet_analyzer.CheckContinuityCounter();
			packet_analyzer.PrintErrorCount();
			//break;

		case 'q':
			packet_analyzer.Reset();
			packet_analyzer.CloseFile();		  
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
