#pragma once

#pragma comment(lib,"ws2_32.lib")

using namespace std;
const int SendBufSize=1024;

class ServerSocket
{
private :
	
	//���ϼ�������
	int clientNumber;
	char SocketIP[14];
	int SocketPort;
	bool boolServerFlag;

	WSADATA wsaData;

	SOCKADDR_IN serverAddress;
	SOCKADDR_IN clientAddress;

	SOCKET serverSocket;
	SOCKET clientSocketTemp;
	SOCKET clientSocketArray[10]; //10����� ���� �� �ִ� ����

	int clientAddressLength;

	//������
	HANDLE serverThread[10];
	DWORD dwServerThreadID[10];

	// ������
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