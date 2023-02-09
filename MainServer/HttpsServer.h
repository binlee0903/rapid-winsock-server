#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <cstdint>

#include <vector>
#include <process.h>
#include <synchapi.h>

#include <cassert>

#include <openssl/ssl3.h>
#include <openssl/err.h>

#include "HttpHelper.h"
#include "HttpsClient.h"

constexpr uint16_t MAX_CONNECTION_COUNT = 100;
constexpr uint16_t MAX_SOCKET_BUFFER_SIZE = 8192 + 1; // \0
constexpr uint16_t PORT_NUMBER = 443;
constexpr uint16_t TIME_OUT = 3000;

class HttpsServer final : public IServer
{
public:
	virtual int32_t Run();

	static HttpsServer* GetServer();
	virtual SSL* GetSSL() const override;
	virtual SSL_CTX* GetSSLCTX() const override;

private:
	HttpsServer();
	~HttpsServer();

    static uint32_t processClient(void* clientSocket);
	SOCKET processAccept();

	void openSocket();
	void closeSocket(SOCKET socket);
	void printSocketError();
private:
	static HttpsServer* mServer;
	uint32_t mConnectionCount;
	SOCKET mSocket;

	std::vector<IClient*> mClients;

	std::vector<SOCKET> mClientSockets;
    std::vector<HANDLE> mThreadHandles;

	sockaddr_in mServerAddr;
	WSADATA* mWsaData;
	SRWLOCK* mSRWLock;

	SSL* mSSL;
	SSL_CTX* mSSLCTX;
};

