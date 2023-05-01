#include "HttpsServer.h"

HttpsServer* HttpsServer::mServer = nullptr;

//static functions
HttpsServer* HttpsServer::GetServer()
{
	if (mServer == nullptr)
	{
		mServer = new HttpsServer();
	}

	return mServer;
}

int32_t HttpsServer::Run()
{
	openSocket();

	HANDLE eventHandle = WSACreateEvent();
	WSANETWORKEVENTS netEvents;
	ZeroMemory(&netEvents, sizeof(netEvents));

	WSAEventSelect(mHttpsSocket, eventHandle, FD_ACCEPT);

	socket_t clientSocket = NULL;

	reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, &HttpsServer::checkQuitMessage, nullptr, 0, nullptr));

	while (!mbIsQuitButtonPressed)
	{
		WSAWaitForMultipleEvents(1, &eventHandle, false, WSA_WAIT_TIMEOUT, false);
		WSAEnumNetworkEvents(mHttpsSocket, eventHandle, &netEvents);

		switch (netEvents.lNetworkEvents)
		{
		case FD_ACCEPT:
			clientSocket = processAccept(mHttpsSocket);

			if (clientSocket == NULL)
			{
				std::cout << "Run() : clientSocket was NULL" << std::endl;
				continue;
			}
			else
			{
				mServer->mClients.push_back(new HttpsClient(mServer, mRouter, mServer->mSRWLock, mServer->mSSLCTX, clientSocket));
				clientSocket = NULL;
			}
			
			break;
		default:
			continue;
		}
	}

	WSACloseEvent(eventHandle);

	delete this;
	return 0;
}

uint32_t HttpsServer::checkQuitMessage(void*)
{
	char ch = 0;

	while (ch != 'q')
	{
		ch = _getch();
	}

	mServer->mbIsQuitButtonPressed = true;

	return 0;
}

// common functions
HttpsServer::HttpsServer()
	: mbIsQuitButtonPressed(false)
	, mHttpSocket(NULL)
	, mHttpsSocket(NULL)
	, mSRWLock(new SRWLOCK())
	, mRouter(new HttpRouter())
{
	mServer = this;
	InitializeSRWLock(mSRWLock);

	SSL_load_error_strings();
	SSL_library_init();
	OpenSSL_add_ssl_algorithms();

	mSSLCTX = SSL_CTX_new(TLS_server_method());
	assert(mSSLCTX != nullptr);

	if (SSL_CTX_use_certificate_file(mSSLCTX, SERVER_CERT_FILE, SSL_FILETYPE_PEM) <= 0)
	{
		std::cout << "SSL start up Failed(HttpsServer constructor)" << std::endl;
	}

	if (SSL_CTX_use_PrivateKey_file(mSSLCTX, SERVER_KEY_FILE, SSL_FILETYPE_PEM) <= 0)
	{
		std::cout << "SSL start up Failed(HttpsServer constructor)" << std::endl;
	}

	if (!SSL_CTX_check_private_key(mSSLCTX)) {
		std::cout << "Private key does not match the certificate public key\n" << std::endl;
	}

	mSSL = SSL_new(mSSLCTX);
	SSL_set_mode(mSSL, SSL_MODE_AUTO_RETRY);

	int result = WSAStartup(MAKEWORD(2, 2), nullptr);
	assert(result == 0);

	if (result != 0)
	{
		std::cout << "Socket start up Failed(HttpsServer constructor)" << std::endl;
	}
}

HttpsServer::~HttpsServer()
{
	for (auto x : mClients)
	{
		delete x;
	}

	HttpHelper::DeleteHttpHelper();
	delete mSRWLock;
	delete mRouter;

	SSL_shutdown(mSSL);
	SSL_free(mSSL);
	SSL_CTX_free(mSSLCTX);

	int result = WSACleanup();

	assert(result == 0);
}

void HttpsServer::runHttpServer()
{
	HANDLE eventHandle = WSACreateEvent();
	WSANETWORKEVENTS netEvents;
	ZeroMemory(&netEvents, sizeof(netEvents));

	WSAEventSelect(mHttpSocket, eventHandle, FD_ACCEPT);

	socket_t clientSocket = NULL;

	while (!mbIsQuitButtonPressed)
	{
		WSAWaitForMultipleEvents(1, &eventHandle, false, WSA_WAIT_TIMEOUT, false);
		WSAEnumNetworkEvents(mHttpSocket, eventHandle, &netEvents);

		switch (netEvents.lNetworkEvents)
		{
		case FD_ACCEPT:
			clientSocket = processAccept(mHttpSocket);

			if (clientSocket == NULL)
			{
				std::cout << "http Run() : clientSocket was NULL" << std::endl;
				continue;
			}
			else
			{
				sendRedirectMessage(clientSocket);
				clientSocket = NULL;
			}
			break;
		default:
			continue;
		}
	}

	WSACloseEvent(eventHandle);
}

void HttpsServer::sendRedirectMessage(socket_t clientSocket)
{

}

void HttpsServer::openSocket()
{
	mHttpSocket = socket(AF_INET, SOCK_STREAM, 0); // http
	assert(mHttpSocket != INVALID_SOCKET);

	mHttpsSocket = socket(AF_INET, SOCK_STREAM, 0); // https
	assert(mHttpsSocket != INVALID_SOCKET);

	sockaddr_in httpServerAddr; // http
	ZeroMemory(&httpServerAddr, sizeof(sockaddr_in));
	httpServerAddr.sin_family = AF_INET;
	httpServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	httpServerAddr.sin_port = htons(HTTP_PORT_NUMBER);

	sockaddr_in httpsServerAddr; // https
	ZeroMemory(&httpsServerAddr, sizeof(sockaddr_in));
	httpsServerAddr.sin_family = AF_INET;
	httpsServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	httpsServerAddr.sin_port = htons(HTTPS_PORT_NUMBER);

	const DWORD optValue = 1; // true
	setsockopt(mHttpSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&optValue), sizeof(optValue)); // http
	setsockopt(mHttpsSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&optValue), sizeof(optValue)); // https
	SSL_set_fd(mSSL, static_cast<int>(mHttpsSocket));

	int32_t returnValue = 0;
	returnValue = bind(mHttpSocket, reinterpret_cast<sockaddr*>(&httpServerAddr), sizeof(sockaddr_in)); // http
	assert(returnValue != SOCKET_ERROR);

	returnValue = bind(mHttpsSocket, reinterpret_cast<sockaddr*>(&httpsServerAddr), sizeof(sockaddr_in)); // https
	assert(returnValue != SOCKET_ERROR);

	returnValue = listen(mHttpSocket, MAX_CONNECTION_COUNT); // http
	assert(returnValue != SOCKET_ERROR);

	returnValue = listen(mHttpsSocket, MAX_CONNECTION_COUNT); // https
	assert(returnValue != SOCKET_ERROR);
}

void HttpsServer::printSocketError()
{
	char* msg = nullptr;
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, WSAGetLastError(),
		LANG_SYSTEM_DEFAULT, reinterpret_cast<char*>(&msg), 0, nullptr);

	if (msg != nullptr)
	{
		std::cout << msg << std::endl;
	}
	LocalFree(msg);
}

socket_t HttpsServer::processAccept(socket_t socket)
{
	socket_t clientSocket = 0;

	clientSocket = accept(socket, nullptr, NULL);

	if (clientSocket == INVALID_SOCKET)
	{
		printSocketError();
		return NULL;
	}

	return clientSocket;
}

