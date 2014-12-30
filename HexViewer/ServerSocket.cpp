#include "StdAfx.h"
#include "ServerSocket.h"

ServerSocket::ServerSocket(void)
{
	//char SocketIP[14] = "192.168.0.61";
	//strcpy(SocketIP,"172.16.39.140");
	strcpy(SocketIP,"127.0.0.1");
	SocketPort = 6000;
	
	clientNumber=0;

	if(WSAStartup(MAKEWORD(2,2),&wsaData) != 0){
		ErrorHandling("WSAStartup() error! \n");
	}

	// ����ȭ�� ���� ������ ���� �Լ�
	hMutex=CreateMutex(NULL, FALSE, NULL);        // ���� (X), ������(O), �̸� (X)
	if(hMutex==NULL){
		ErrorHandling("CreateMutex() error");
	}

	//���ϻ���
	serverSocket = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
	if( serverSocket == INVALID_SOCKET ){
		ErrorHandling("socket() error! \n");
	}

	cout << "================Server================" << endl << "Ip : " << SocketIP << " Port : " << SocketPort << endl;

}

void ServerSocket::StartServer()
{
	Binding();
	Listen();
	Accept();
}
void ServerSocket::ErrorHandling(char* message)
{
	printf("%s \n",message);
	exit(1);
}

void ServerSocket::Binding()
{
	//���ϼ���
	memset(&serverAddress,0,sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr =  inet_addr(SocketIP);
	serverAddress.sin_port = htons(SocketPort);

	//���ε�
	if(bind(serverSocket,(SOCKADDR*)&serverAddress,sizeof(SOCKADDR_IN)) == SOCKET_ERROR){
		ErrorHandling("bind() error! \n");
	}
}


void ServerSocket::Listen()
{
	//����
	if(listen(serverSocket,5) == -1 ){
		ErrorHandling("listen() error! \n");
	}
}

void ServerSocket::Accept()
{
	//����ڸ� ��� �ޱ����� �ݺ��� ����
	while(1)
	{
		//Ŭ���̾�Ʈ �����۾�
		clientAddressLength = sizeof(clientAddress);
		clientSocketTemp = accept(serverSocket,(SOCKADDR*)&clientAddress, &clientAddressLength);

		if(clientSocketTemp == INVALID_SOCKET){
			ErrorHandling("accept() error! \n");
		}

		// Mutex�� ����
		WaitForSingleObject(hMutex, INFINITE);

		cout << "Ŭ���̾�Ʈ ���� �Ϸ�(" <<  inet_ntoa(clientAddress.sin_addr) << ") " << endl;

		//����� Ŭ���̾�Ʈ �Է�
		clientSocketArray[clientNumber] = clientSocketTemp;

		//������ ����
		serverThread[clientNumber] = (HANDLE)_beginthreadex(NULL, 0, ThreadFunction, (void*)clientSocketArray[clientNumber], 0, (unsigned*)&dwServerThreadID[clientNumber]);
		if(serverThread[clientNumber] == 0)	{
			ErrorHandling("_beginthreadex() error");
		}

		// ����� non-signaled ���¿��� sinagled ���·� ��ȯ
		ReleaseMutex(hMutex);

		clientNumber++;
	}
}

//�����带 �θ��� ����
unsigned int __stdcall ServerSocket::ThreadFunction(void* arg)
{
	SOCKET clientSocket = (SOCKET)arg;
	static_cast<ServerSocket*>(arg)->ReadAndWrite(clientSocket);
	return 0;
}

//�����尡 �θ��� �Լ� �б�� ���⸦ ���
void ServerSocket::ReadAndWrite(SOCKET clientSocket)
{
	char buf[SendBufSize];
	char Sendbuf[SendBufSize] = "Test Server -sdj=s ssdd : 123123888";

	strcat(Sendbuf, "\r\n");

	memset(buf, ' ', SendBufSize); //�������� �迭�ʱ�ȭ	
	buf[SendBufSize - 1] = '\0';	//NULL�߰�

	recv(clientSocket, buf, SendBufSize, 0);
	cout << "recv : " << buf <<endl;

	send(clientSocket, Sendbuf, SendBufSize,0);
	cout << "send : " << Sendbuf <<endl;

	memset(Sendbuf, ' ', SendBufSize); //�������� �迭�ʱ�ȭ
	buf[SendBufSize - 1] = '\0';	//NULL�߰�

	recv(clientSocket, buf, SendBufSize, 0);
	cout << "recv : " << buf <<endl;
	/*
	while(1){
		send(clientSocket, buf, SendBufSize,0);
	}*/
}


ServerSocket::~ServerSocket(void)
{
	closesocket(serverSocket);
}
