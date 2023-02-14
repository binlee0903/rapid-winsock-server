#include "HttpsServer.h"

HttpsServer* HttpsServer::mServer = nullptr;
//HTMLPageRouter* HttpsServer::mRouter = nullptr;

HttpsServer::HttpsServer()
	: mWsaData(new WSADATA())
	, mSRWLock(new SRWLOCK())
	, mTextFileContainer(new HttpFileContainer())
	, mIsQuit(false)
{
	mServer = this;
	/*mRouter = new HTMLPageRouter();*/
	mThreadHandles.reserve(100);
	mConnectionCount = 0;
	mSocket = 0;
	InitializeSRWLock(mSRWLock);

	SSL_load_error_strings();
	SSL_library_init();
	OpenSSL_add_ssl_algorithms();

	mSSLCTX = SSL_CTX_new(TLS_server_method());
	assert(mSSLCTX != nullptr);

	if (SSL_CTX_use_certificate_file(mSSLCTX, SERVER_CERT_FILE, SSL_FILETYPE_PEM) <= 0)
	{
		std::wcout << L"SSL start up Failed(HttpsServer constructor)" << std::endl;
	}

	if (SSL_CTX_use_PrivateKey_file(mSSLCTX, SERVER_KEY_FILE, SSL_FILETYPE_PEM) <= 0)
	{
		std::wcout << L"SSL start up Failed(HttpsServer constructor)" << std::endl;
	}

	if (!SSL_CTX_check_private_key(mSSLCTX)) {
		std::wcout << L"Private key does not match the certificate public key\n" << std::endl;
	}
	
	mSSL = SSL_new(mSSLCTX);
	SSL_set_mode(mSSL, SSL_MODE_AUTO_RETRY);

	ZeroMemory(&mServerAddr, sizeof(sockaddr_in));
	mServerAddr.sin_family = AF_INET;
	mServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	mServerAddr.sin_port = htons(PORT_NUMBER);

	int result = WSAStartup(MAKEWORD(2, 2), mWsaData);
	assert(result == 0);

	if (result != 0)
	{
		std::wcout << L"Socket start up Failed(HttpsServer constructor)" << std::endl;
	}
}

HttpsServer::~HttpsServer()
{
	for (auto x : mThreadHandles)
	{
		CloseHandle(x);
	}

	for (auto x : mThreadHandles)
	{
		CloseHandle(x);
	}

	HttpHelper::DeleteHttpHelper();
	/*delete mRouter;*/
	delete mWsaData;
	delete mSRWLock;

	SSL_shutdown(mSSL);
	SSL_free(mSSL);
	SSL_CTX_free(mSSLCTX);

	int result = WSACleanup();

	assert(result == 0);
	if (result != 0)
	{
		printSocketError();
	}
}

int32_t HttpsServer::Run()
{
	openSocket();

	int32_t returnValue = 0;
	returnValue = bind(mSocket, (sockaddr*)&mServerAddr, sizeof(sockaddr_in));

	assert(returnValue != SOCKET_ERROR);
	if (returnValue == SOCKET_ERROR)
	{
		printSocketError();
		return -1;
	}

	returnValue = listen(mSocket, MAX_CONNECTION_COUNT);
	if (returnValue == SOCKET_ERROR)
	{
		printSocketError();
		return -1;
	}

	HANDLE eventHandle = WSACreateEvent();
	WSANETWORKEVENTS netEvents;
	ZeroMemory(&netEvents, sizeof(netEvents));

	WSAEventSelect(mSocket, eventHandle, FD_ACCEPT);

	SOCKET clientSocket = NULL;
	sockaddr_in clientSockAddr;
	int32_t addrLen = sizeof(sockaddr_in);

	reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, &HttpsServer::CheckQuitMessage, nullptr, 0, nullptr));

	while (!mIsQuit)
	{
		WSAWaitForMultipleEvents(1, &eventHandle, false, WSA_WAIT_TIMEOUT, false);
		WSAEnumNetworkEvents(mSocket, eventHandle, &netEvents);

		switch (netEvents.lNetworkEvents)
		{
		case FD_ACCEPT:
			clientSocket = processAccept();
			mConnectionCount++;
			mThreadHandles.push_back(reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0,
				&HttpsServer::processClient,
				reinterpret_cast<void*>(clientSocket), 0, nullptr)));
			clientSocket = NULL;
			ZeroMemory(&clientSockAddr, sizeof(clientSockAddr));
			break;
		default:
			continue;
		}
	}

	WSACloseEvent(eventHandle);

	delete this;
	return 0;
}

void HttpsServer::openSocket()
{
	SOCKET tempSocket = NULL;
	tempSocket = socket(AF_INET, SOCK_STREAM, 0);

	assert(tempSocket != 0);
	assert(mSocket == 0);

	if (tempSocket == INVALID_SOCKET)
	{
		printSocketError();
	}
	else if (mSocket == 0)
	{
		const DWORD optValue = 1;
		setsockopt(tempSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&optValue), sizeof(optValue));
		mSocket = tempSocket;
		SSL_set_fd(mSSL, mSocket);
	}
	else
	{
		std::wcout << L"HttpsServer.cpp openSocket() attempted open socket twice" << std::endl;
	}
}

void HttpsServer::closeSocket(SOCKET socket)
{
	if (closesocket(socket) != 0)
	{
		printSocketError();
	}
}

HttpsServer* HttpsServer::GetServer()
{
	if (mServer == nullptr)
	{
		mServer = new HttpsServer();
	}

	return mServer;
}

HttpFileContainer* HttpsServer::GetHttpFileContainer()
{
	return mTextFileContainer;
}

HTMLPageRouter* HttpsServer::GetHTMLPageRouter()
{
	/*return mRouter;*/
	return nullptr;
}

SSL* HttpsServer::GetSSL() const
{
	return mSSL;
}

SSL_CTX* HttpsServer::GetSSLCTX() const
{
	return mSSLCTX;
}

std::unordered_set<std::string>* HttpsServer::GetBlackLists()
{
	return &mBlackLists;
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
	delete this;
}

SOCKET HttpsServer::processAccept()
{
	SOCKET clientSocket = 0;
	sockaddr_in clientSockAddr;
	int32_t addrLen = sizeof(sockaddr_in);

	clientSocket = accept(mSocket, reinterpret_cast<sockaddr*>(&clientSockAddr), &addrLen);
	if (clientSocket == INVALID_SOCKET)
	{
		printSocketError();
		return NULL;
	}

	mClientSockets.push_back(clientSocket);
	return clientSocket;
}

uint32_t HttpsServer::CheckQuitMessage(void*)
{
	char ch = 0;

	while (ch != 'q')
	{
		ch = _getch();
	}

	mServer->mIsQuit = true;

	return 0;
}

uint32_t HttpsServer::processClient(void* clientSocketArg)
{
	SOCKET clientSocket = reinterpret_cast<SOCKET>(clientSocketArg);
	IClient* client = new HttpsClient();
	if (client->InitializeClient(mServer, mServer->mSRWLock, clientSocket) != 0)
	{
		delete client;
		_endthreadex(0);
		return 0;
	}

	mServer->mClients.push_back(client);
	mServer->mConnectionCount++;

	HANDLE clientEventHandle = client->GetEventHandle();
	WSANETWORKEVENTS netEvents;
	bool bGotRequest = false;

	while (true)
	{
		WSAWaitForMultipleEvents(1, &clientEventHandle, false, INFINITE, false);
		WSAEnumNetworkEvents(clientSocket, clientEventHandle, &netEvents);

		client->IncreaseRequestCount();

		if (netEvents.lNetworkEvents & FD_READ)
		{
			int retCode = client->ProcessRead();

			if (retCode == -1)
			{
				break;
			}
			else if (retCode == 0)
			{
				client->ProcessWrite();

				if (client->IsKeepAlive() == false)
				{
					client->ProcessClose();
					break;
				}
			}
		}

		if (client->GetRequestCount() > 100)
		{
			mServer->mBlackLists.insert(client->GetClientIP());
			client->ProcessClose();
		}

		if (netEvents.lNetworkEvents & FD_CLOSE)
		{
			client->ProcessClose();
			break;
		}
	}

    return 0;
}
