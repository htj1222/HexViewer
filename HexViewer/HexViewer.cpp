// HexViewer.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h> // system()
#include <string.h> // memset()
#include <ctype.h>  // tolower()
#include <conio.h>  // getch()
#include <time.h> //clock(), time_t����
#include "HTTPConnect.h"
#include "ServerSocket.h"


int _tmain(int argc, const char* argv[])
{
	PacketAnalyzer packet_analyzer("data\\notake.mpg");
	
	char key;
	int counter;
	//�ð������� ����
	float gap;
    time_t startTime=0, endTime=0;

	/*
    printf("������ �����մϴ�...\n") ;
    //���� ����
    startTime=clock();
	*/

	ServerSocket server;
	server.StartServer();
	
	while(1) {
		system("cls");

		packet_analyzer.GetPacketData();
		packet_analyzer.TSPacketDataAnalysis();		//��Ŷ �м� ����
		packet_analyzer.SetPrintInfo();				//��°� ����
		packet_analyzer.PrintInfo();				//�������		
		packet_analyzer.Reset();					//�� ����

		printf("\nPacket: %I64u/%I64u    1 (����) 2 (����) F(ã��) Q (����)? : ", packet_analyzer.GetCurrentPacket()+1, packet_analyzer.GetTotalPacket());
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
	//���� ��
    endTime=clock();
    printf("������ �������ϴ�...\n") ;

    // �ð� ���
    gap=(float)(endTime-startTime)/(CLOCKS_PER_SEC); //���

    // ���� �ð� ���
    printf("���� �ð� : %f ��\n", gap);*/

	return 0;
}
