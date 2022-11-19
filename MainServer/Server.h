#pragma once

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <vector>

#include <cstdint>
#include <cassert>

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

	void openSocket();
	void closeSocket(SOCKET socket);
	void printSocketError();

private:
	static Server* mServer;
	static uint32_t mConnectionCount;
	static SOCKET mSocket;

	sockaddr_in mServerAddr;

	WSADATA* mWsaData;
	std::vector<HANDLE> mThreadHandles;

};

