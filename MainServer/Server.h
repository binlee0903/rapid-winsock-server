#pragma once

#include <WinSock2.h>
#include <iostream>
#include <vector>

class Server
{
public:
	~Server();

	void Run();

	static Server* GetServer();

private:
	Server();

	void openSocket();
	void closeSocket(SOCKET socket);
	void printSocketError();

private:
	static Server* mServer;

	WSADATA* mWsaData;
	std::vector<SOCKET> mSockets;
};

