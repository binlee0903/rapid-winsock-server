#pragma once

#include <WinSock2.h>
#include <iostream>
#include <vector>

class Server
{
public:
	Server();
	~Server();

	void OpenSocket();
	void CloseSocket(SOCKET socket);

private:
	void printSocketError();

private:
	WSADATA* mWsaData;
	std::vector<SOCKET> mSockets;
};

