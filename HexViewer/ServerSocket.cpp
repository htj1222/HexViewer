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

	// 동기화를 위한 뮤텍즈 생성 함수
	hMutex=CreateMutex(NULL, FALSE, NULL);        // 보안 (X), 무소유(O), 이름 (X)
	if(hMutex==NULL){
		ErrorHandling("CreateMutex() error");
	}

	//소켓생성
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
	//소켓설정
	memset(&serverAddress,0,sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr =  inet_addr(SocketIP);
	serverAddress.sin_port = htons(SocketPort);

	//바인딩
	if(bind(serverSocket,(SOCKADDR*)&serverAddress,sizeof(SOCKADDR_IN)) == SOCKET_ERROR){
		ErrorHandling("bind() error! \n");
	}
}


void ServerSocket::Listen()
{
	//리슨
	if(listen(serverSocket,5) == -1 ){
		ErrorHandling("listen() error! \n");
	}
}

void ServerSocket::Accept()
{
	//사용자를 계속 받기위해 반복문 설정
	while(1)
	{
		//클라이언트 소켓작업
		clientAddressLength = sizeof(clientAddress);
		clientSocketTemp = accept(serverSocket,(SOCKADDR*)&clientAddress, &clientAddressLength);

		if(clientSocketTemp == INVALID_SOCKET){
			ErrorHandling("accept() error! \n");
		}

		// Mutex를 소유
		WaitForSingleObject(hMutex, INFINITE);

		cout << "클라이언트 연결 완료(" <<  inet_ntoa(clientAddress.sin_addr) << ") " << endl;

		//연결된 클라이언트 입력
		clientSocketArray[clientNumber] = clientSocketTemp;

		//쓰레드 시작
		serverThread[clientNumber] = (HANDLE)_beginthreadex(NULL, 0, ThreadFunction, (void*)clientSocketArray[clientNumber], 0, (unsigned*)&dwServerThreadID[clientNumber]);
		if(serverThread[clientNumber] == 0)	{
			ErrorHandling("_beginthreadex() error");
		}

		// 뮤텍즈를 non-signaled 상태에서 sinagled 상태로 변환
		ReleaseMutex(hMutex);

		clientNumber++;
	}
}

//쓰레드를 부르기 위해
unsigned int __stdcall ServerSocket::ThreadFunction(void* arg)
{
	SOCKET clientSocket = (SOCKET)arg;
	static_cast<ServerSocket*>(arg)->ReadAndWrite(clientSocket);
	return 0;
}

//쓰레드가 부르는 함수 읽기와 쓰기를 사용
void ServerSocket::ReadAndWrite(SOCKET clientSocket)
{
	char get_bufffer_[SendBufSize];
	char send_bufffer[SendBufSize] = "";
	char* get_file_name="";
	__int64 get_packet_num;
	
	memset(get_bufffer_, ' ', SendBufSize); //공백으로 배열초기화	
	get_bufffer_[SendBufSize - 1] = '\0';	//NULL추가
		
	memset(send_bufffer, ' ', SendBufSize); //공백으로 배열초기화
		

	recv(clientSocket, get_bufffer_, SendBufSize, 0);
	cout << "recv : " << get_bufffer_ <<endl;
		
	char delimit_string[] = "/";
	char *p_token = NULL;
	p_token = strtok(get_bufffer_, delimit_string);
	get_file_name = p_token;
	p_token = strtok(NULL, delimit_string);
	get_packet_num = atoi(p_token);
	
	cout << "fileName : "	 << get_file_name << endl;
	cout << "get_packet_num : " << get_packet_num << endl;

	PacketAnalyzer packet_analyzer(get_file_name);
	packet_analyzer.FindPakcetData(get_packet_num);

	packet_analyzer.GetPacketData();
	packet_analyzer.TSPacketDataAnalysis();		//패킷 분석 시작
	packet_analyzer.SetPrintInfo();				//출력값 저장
	
	//packet_analyzer.PrintInfo();				//정보출력		
	//packet_analyzer.Reset();					//값 리셋

	//packet_analyzer.SetPrintHexData();
	//packet_analyzer.PrintHex();

	strcpy(send_bufffer,packet_analyzer.GetSendBuffer().c_str());
	//strcat(send_bufffer, "\r\n");
	
	send(clientSocket, send_bufffer, SendBufSize,0);
	cout << "send : " << send_bufffer <<endl;

	/*
	recv(clientSocket, get_bufffer_, SendBufSize, 0);
	cout << "recv : " << get_bufffer_ <<endl;
	
	while(1){
		send(clientSocket, buf, SendBufSize,0);
	}*/
}


ServerSocket::~ServerSocket(void)
{
	closesocket(serverSocket);
}
