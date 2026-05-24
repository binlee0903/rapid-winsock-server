#pragma once

#include <WinSock2.h>
#include <cstdint>
#include <string>

#include <openssl/ssl.h>

using socket_t = decltype(socket(0, 0, 0));

struct ClientSession
{
	uint32_t sessionID;
	HttpObject* httpObject;
	SSL* clientSSLConnection;
	BIO* clientSSLReadBIO;
	BIO* clientSSLWriteBIO;
	SessionTimer* sessionTimer;
	std::string* ip;
	bool bIsSSLConnected;
	bool bIsSSLRetryConnection;
};