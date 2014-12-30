#pragma once

#pragma comment(lib,"ws2_32.lib")

using namespace std;
const int SendBufSize=1024;

class ServerSocket
{
private :
	
	//소켓설정관련
	int clientNumber;
	char SocketIP[14];
	int SocketPort;
	bool boolServerFlag;

	WSADATA wsaData;

	SOCKADDR_IN serverAddress;
	SOCKADDR_IN clientAddress;

	SOCKET serverSocket;
	SOCKET clientSocketTemp;
	SOCKET clientSocketArray[10]; //10명까지 들어올 수 있는 소켓

	int clientAddressLength;

	//쓰레드
	HANDLE serverThread[10];
	DWORD dwServerThreadID[10];

	// 뮤텍즈
	HANDLE hMutex;

public:
	ServerSocket(void);
	~ServerSocket(void);

	void StartServer();
	void ErrorHandling(char* message);
	void Binding();
	void Listen();
	void Accept();
	static unsigned int __stdcall ThreadFunction(void* arg);
	void ReadAndWrite(SOCKET clientSocket);
};