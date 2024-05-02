#include "network.h"

network::socket_t network::ProcessAccept(network::socket_t socket, sockaddr_in sockaddrIn, std::string& address)
{
	socket_t clientSocket = 0;
	int* size = new int(sizeof(sockaddr_in));

	clientSocket = accept(socket, reinterpret_cast<sockaddr*>(&sockaddrIn), size);

	if (clientSocket == INVALID_SOCKET)
	{
		return NULL;
	}

	const DWORD optValue = 1;
	setsockopt(clientSocket, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<const char*>(&optValue), sizeof(DWORD));

	std::stringstream is;

	char buffer[32];
	inet_ntop(AF_INET, &sockaddrIn.sin_addr, buffer, 32);

	is << buffer;
	is << "/";
	is << sockaddrIn.sin_port;

	address = is.str();
	delete size;

	return clientSocket;
}

void network::OpenSocket(network::socket_t& targetSocket, uint16_t portNumber, SSL* ssl, bool isbSecureSocket)
{
	targetSocket = socket(AF_INET, SOCK_STREAM, 0);
	assert(targetSocket != INVALID_SOCKET);

	sockaddr_in serverAddr;
	ZeroMemory(&serverAddr, sizeof(sockaddr_in));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(portNumber);

	const DWORD optValue = 1; // true
	setsockopt(targetSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&optValue), sizeof(optValue));

	if (isbSecureSocket == true)
	{
		SSL_set_fd(ssl, static_cast<int>(targetSocket));
	}

	int32_t returnValue = 0;
	returnValue = bind(targetSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(sockaddr_in));
	assert(returnValue != SOCKET_ERROR);

	returnValue = listen(targetSocket, MAX_CLIENT_CONNECTION_COUNT);
	assert(returnValue != SOCKET_ERROR);
}

void network::ProcessRedirect(socket_t serverSocket)
{
	socket_t clientSocket = 0;
	sockaddr_in sockaddrIn;
	int* size = new int(sizeof(sockaddr_in));

	clientSocket = accept(serverSocket, reinterpret_cast<sockaddr*>(&sockaddrIn), size);

	if (clientSocket == INVALID_SOCKET)
	{
		return;
	}

	std::stringstream is;

	is << "HTTP/1.1 302 redirect\r\n";

#ifdef _DEBUG
	is << "Location: https://localhost/\r\n";
#else
	is << "Location: https://www.binlee-blog.com/\r\n";
#endif
	is << "Content-Type: text/html\r\n";
	is << "Content-Length: 0\r\n\r\n";

	std::string buffer = is.str();

	send(clientSocket, buffer.c_str(), buffer.size(), NULL);
	closesocket(clientSocket);

	delete size;
}
