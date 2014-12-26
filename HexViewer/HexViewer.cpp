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
	
	FileDescriptor descriptor;
	
	char key;
	int counter;
	
	
	float gap;
    time_t startTime=0, endTime=0;
	/*
    printf("������ �����մϴ�...\n") ;
    //���� ����
    startTime=clock();
	*/
	packetCount = descriptor.GetPacketCount();	//�� ��Ŷ�� ���� ����
	descriptor.SetPidValueInit();				//pid���а� cc�� ���� �ʱ�ȭ
	descriptor.Reset();							//�� ����

	while(1) {
		system("cls");
		
		descriptor.GetPacketData(currentPacket);//������ ����
		descriptor.TSPacketDataAnalysis();		//��Ŷ �м� ����
		descriptor.PrintInfo();					//�������
		descriptor.PrintHex();					//hex�� ���
		descriptor.Reset();						//�� ����

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
			  descriptor.GetPacketData(++counter);//������ ����
			  descriptor.CheckContinuityCounter();
		  }
		  descriptor.PrintErrorCount();

		  endTime=clock();
		  printf("������ �������ϴ�...\n") ;
		  // �ð� ���
		  gap=(float)(endTime-startTime)/(CLOCKS_PER_SEC); //���
		  // ���� �ð� ���
		  printf("���� �ð� : %f ��\n", gap);
		  //break;
	  case 'q':
		  descriptor.Reset();
		  descriptor.CloseFile();		  
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
