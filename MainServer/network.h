#pragma once

#include <WinSock2.h>
#include <ws2tcpip.h>

#include <openssl/ssl3.h>
#include <openssl/err.h>

#include <cstdint>
#include <cassert>

namespace network
{
	using socket_t = decltype(socket(0, 0, 0));

	constexpr uint16_t MAX_CONNECTION_COUNT = 1000; // max clients count
	constexpr uint16_t MAX_SOCKET_BUFFER_SIZE = 8192;
	constexpr uint16_t HTTP_PORT_NUMBER = 80;
	constexpr uint16_t HTTPS_PORT_NUMBER = 443;
	constexpr uint16_t TIME_OUT = 3000;

	void OpenSocket(socket_t targetSocket, uint16_t portNumber, SSL* ssl, bool isbSecureSocket);

	socket_t ProcessAccept(socket_t socket, sockaddr_in& sockaddrIn);
}
