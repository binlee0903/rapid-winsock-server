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
	network::OpenSocket(mHttpsSocket, network::HTTPS_PORT_NUMBER, mSSL, true);

	HANDLE eventHandle = WSACreateEvent();
	WSANETWORKEVENTS netEvents;
	ZeroMemory(&netEvents, sizeof(netEvents));

	WSAEventSelect(mHttpsSocket, eventHandle, FD_ACCEPT);

	socket_t clientSocket = NULL;
	sockaddr_in clientSockAddr;
	ZeroMemory(&clientSockAddr, sizeof(sockaddr));

	reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, &HttpsServer::checkQuitMessage, nullptr, 0, nullptr));

	while (!mbIsQuitButtonPressed)
	{
		WSAWaitForMultipleEvents(1, &eventHandle, false, WSA_WAIT_TIMEOUT, false);
		WSAEnumNetworkEvents(mHttpsSocket, eventHandle, &netEvents);

		switch (netEvents.lNetworkEvents)
		{
		case FD_ACCEPT:
			clientSocket = network::ProcessAccept(mHttpsSocket, clientSockAddr);

			if (clientSocket == NULL)
			{
				std::cout << "Run() : clientSocket was NULL" << std::endl;
				continue;
			}
			else
			{
				mServer->mClients.push_back(new HttpsClient(mServer, mServer->mSRWLock, mServer->mSSLCTX, clientSocket, clientSockAddr));
				clientSocket = NULL;
				ZeroMemory(&clientSockAddr, sizeof(sockaddr));
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

uint32_t __stdcall HttpsServer::checkQuitMessage(void*)
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
	, mHttpsSocket(NULL)
	, mSRWLock(new SRWLOCK())
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

	SSL_shutdown(mSSL);
	SSL_free(mSSL);
	SSL_CTX_free(mSSLCTX);

	int result = WSACleanup();

	assert(result == 0);
}

void HttpsServer::sendRedirectMessage(socket_t clientSocket)
{

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
