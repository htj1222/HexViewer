// HexViewer.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h> // system()
#include <string.h> // memset()
#include <ctype.h>  // tolower()
#include <conio.h>  // getch()
#include "HTTPConnect.h"
#include "ServerSocket.h"
#include <time.h> //clock(), time_t����

int _tmain(int argc, const char* argv[])
{
	__int64 currentPacket = 0, packetCount = 0;
	
	PacketAnalyzer packet_analyzer;
	
	char key;
	int counter;
	
	
	float gap;
    time_t startTime=0, endTime=0;
	/*
    printf("������ �����մϴ�...\n") ;
    //���� ����
    startTime=clock();
	*/
	packetCount = packet_analyzer.GetPacketCount();	//�� ��Ŷ�� ���� ����
	packet_analyzer.SetPidValueInit();				//pid���а� cc�� ���� �ʱ�ȭ
	packet_analyzer.Reset();							//�� ����

	ServerSocket server;
	//server.StartServer();
	
	while(1) {
		system("cls");
		
		packet_analyzer.GetPacketData(currentPacket);//������ ����
		packet_analyzer.TSPacketDataAnalysis();		//��Ŷ �м� ����
		packet_analyzer.PrintInfo();					//�������
		packet_analyzer.PrintHex();					//hex�� ���
		packet_analyzer.Reset();						//�� ����

		printf("\nPacket: %I64u/%I64u    1 (����) 2 (����) F(ã��) Q (����)? : ", currentPacket + 1, packetCount);
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
	  case 'c':

		  printf("������ �����մϴ�...\n") ;
		  //���� ����
		  startTime=clock();

		  counter=1;		  
		  while(counter != (packetCount-1)){
			  //cout<<" Packet :"<<counter+2<<" / ";
			  packet_analyzer.GetPacketData(++counter);//������ ����
			  packet_analyzer.CheckContinuityCounter();
		  }
		  packet_analyzer.PrintErrorCount();

		  endTime=clock();
		  printf("������ �������ϴ�...\n") ;
		  // �ð� ���
		  gap=(float)(endTime-startTime)/(CLOCKS_PER_SEC); //���
		  // ���� �ð� ���
		  printf("���� �ð� : %f ��\n", gap);
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
