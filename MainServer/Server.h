#pragma once

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <vector>
#include <process.h>

#include <cstdint>
#include <cassert>

#include "sqlite3.h"
#include "HttpHelper.h"

constexpr int32_t MAX_CONNECTION_COUNT = 100;
constexpr int32_t MAX_SOCKET_BUFFER_SIZE = 8192 + 1; // \0
constexpr int32_t PORT_NUMBER = 80;

class Server
{
public:
	int32_t Run();
	void Terminate();

	static Server* GetServer();

private:
	Server();
	~Server();

    static uint32_t processClient(void* clientSocket);

	void openSocket();
	void closeSocket(SOCKET socket);
	void printSocketError();

private:
	static Server* mServer;
	static uint32_t mConnectionCount;
	static SOCKET mSocket;
    static std::vector<HANDLE> mThreadHandles;

	sockaddr_in mServerAddr;
	WSADATA* mWsaData;
};

