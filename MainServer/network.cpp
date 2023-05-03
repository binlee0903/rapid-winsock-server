#include "network.h"

network::socket_t network::ProcessAccept(network::socket_t socket, sockaddr_in& sockaddrIn)
{
	socket_t clientSocket = 0;

	clientSocket = accept(socket, reinterpret_cast<sockaddr*>(&sockaddrIn), NULL);

	if (clientSocket == INVALID_SOCKET)
	{
		return NULL;
	}

	return clientSocket;
}

void network::OpenSocket(network::socket_t targetSocket, uint16_t portNumber, SSL* ssl, bool isbSecureSocket)
{
	targetSocket = socket(AF_INET, SOCK_STREAM, 0); // http
	assert(targetSocket != INVALID_SOCKET);

	targetSocket = socket(AF_INET, SOCK_STREAM, 0); // https
	assert(targetSocket != INVALID_SOCKET);

	sockaddr_in httpServerAddr; // http
	ZeroMemory(&httpServerAddr, sizeof(sockaddr_in));
	httpServerAddr.sin_family = AF_INET;
	httpServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	httpServerAddr.sin_port = htons(portNumber);

	const DWORD optValue = 1; // true
	setsockopt(targetSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&optValue), sizeof(optValue));

	if (isbSecureSocket == true)
	{
		SSL_set_fd(ssl, static_cast<int>(targetSocket));
	}

	int32_t returnValue = 0;
	returnValue = bind(targetSocket, reinterpret_cast<sockaddr*>(&httpServerAddr), sizeof(sockaddr_in)); // http
	assert(returnValue != SOCKET_ERROR);

	returnValue = listen(targetSocket, MAX_CONNECTION_COUNT); // http
	assert(returnValue != SOCKET_ERROR);
}
