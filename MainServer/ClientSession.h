#pragma once

#include <WinSock2.h>
#include <cstdint>
#include <string>

#include <openssl/ssl.h>

using socket_t = decltype(socket(0, 0, 0));

struct ClientSession
{
	uint32_t sessionID;
	int32_t processingCount;
	socket_t clientSocket;
	HANDLE eventHandle;
	HttpObject* httpObject;
	SSL* clientSSLConnection;
	SessionTimer* sessionTimer;
	std::string* ip;
	bool bIsSSLConnected;
	bool bIsSSLRetryConnection;
	bool bIsDisconnected;
};