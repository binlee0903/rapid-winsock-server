#pragma once

#include <codecvt>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <vector>
#include <map>
#include <process.h>
#include <locale>
#include <synchapi.h>

#include <cstdint>
#include <cassert>

#include "sqlite3.h"
#include "HttpHelper.h"

constexpr uint16_t BUFFER_SIZE = 512;
constexpr uint16_t MAX_CONNECTION_COUNT = 100;
constexpr uint16_t MAX_SOCKET_BUFFER_SIZE = 8192 + 1; // \0
constexpr uint16_t PORT_NUMBER = 80;
constexpr uint16_t TIME_OUT = 3000;

class Server final
{
public:
	int32_t Run();

	static Server* GetServer();

private:
	Server();
	~Server();

    static uint32_t processClient(void* clientSocket);
	static uint32_t receiveClientData(SOCKET clientSocket, std::wstring& content);

	void openSocket();
	void closeSocket(SOCKET socket);
	void printSocketError();
private:
	static std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> mConverter;

	static Server* mServer;
	static std::multimap<std::wstring, std::wstring> mClientData;
	uint32_t mConnectionCount;
	SOCKET mSocket;
    std::vector<HANDLE> mThreadHandles;

	sockaddr_in mServerAddr;
	WSADATA* mWsaData;
	SRWLOCK* mSRWLock;
};

