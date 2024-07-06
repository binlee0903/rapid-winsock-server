#pragma once

#include <WinSock2.h>
#include <cstdint>
#include <string>

#include <openssl/ssl.h>

#include "SessionTimer.h"
using socket_t = decltype(socket(0, 0, 0));

struct ClientSession
{
	uint32_t sessionID;
	int32_t processingCount;
	socket_t clientSocket;
	HANDLE eventHandle;
	SSL* clientSSLConnection;
	SessionTimer* sessionTimer;
	std::string* ip;
	bool bIsSSLConnected;
	bool bIsSSLRetryConnection;
};