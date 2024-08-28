#include "stdafx.h"
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
	HANDLE clientEventHandle;
	ClientSession* tempClientSession;
	WSANETWORKEVENTS netEvents;
	ZeroMemory(&netEvents, sizeof(netEvents));

	WSAEventSelect(mHttpsSocket, eventHandle, FD_ACCEPT);

	std::string dummy = "dummy";
	tempClientSession = createClientSession(mHttpsSocket, eventHandle, mSSL, dummy);
	mClientSessions.push_back(tempClientSession);
	mClientEventHandles.push_back(eventHandle);
	tempClientSession = nullptr;
	eventHandle = nullptr;

	std::string ipAddressBuffer;
	ipAddressBuffer.reserve(32);
	socket_t clientSocket = NULL;
	SSL* ssl = nullptr;
	sockaddr_in clientSockAddr;
	ZeroMemory(&clientSockAddr, sizeof(sockaddr));

	CreateThread(nullptr, 0, HttpsServer::checkQuitMessage, nullptr, NULL, nullptr);

	int index = 0;
	int ret = 0;

	while (!mbIsQuitButtonPressed)
	{
		index = WSAWaitForMultipleEvents(mClientEventHandles.size(), mClientEventHandles.data(), false, 1000, false);

		if (index == WSA_WAIT_FAILED)
		{
			printSocketError();
			continue;
		}

		if (index == WSA_WAIT_TIMEOUT)
		{
			signalForRemainingWorks();
			invalidateSession();
			continue;
		}

		index -= WSA_WAIT_EVENT_0;

		WSAEnumNetworkEvents(mClientSessions[index]->clientSocket, mClientEventHandles[index], &netEvents);

		switch (netEvents.lNetworkEvents)
		{
		case FD_ACCEPT:
			clientSocket = network::ProcessAccept(mHttpsSocket, clientSockAddr, ipAddressBuffer);

			if (clientSocket == NULL)
			{
				mLogger->error("Run() : clientSocket was NULL");
				break;
			}

			clientEventHandle = WSACreateEvent();
			WSAEventSelect(clientSocket, clientEventHandle, FD_READ | FD_CLOSE);

			ssl = SSL_new(mSSLCTX);
			SSL_set_accept_state(ssl);

			tempClientSession = createClientSession(clientSocket, clientEventHandle, ssl, ipAddressBuffer);
			mLogger->info("Run() : client connected, ip : {}", tempClientSession->ip->c_str());

			mClientSessions.push_back(tempClientSession);
			mClientEventHandles.push_back(clientEventHandle);
			tempClientSession = nullptr;
			clientEventHandle = nullptr;
			break;

		case FD_READ:
			mClientSessions[index]->sessionTimer->ResetTimer();

			if (mClientSessions[index]->bIsSSLConnected == false)
			{
				if (mClientSessions[index]->bIsSSLRetryConnection == true)
				{
					mLogger->info("Run() : retry ssl connection, ip : {}", mClientSessions[index]->ip->c_str());
				}
				else
				{
					mLogger->info("Run() : attampt ssl connection, ip : {}", mClientSessions[index]->ip->c_str());
				}

				ret = ProcessSSLHandshake(mClientSessions[index]);

				if (ret == SSL_ERROR_WANT_READ)
				{
					mClientSessions[index]->bIsSSLRetryConnection = true;
					break;
				}

				if (ret != SSL_ERROR_NONE)
				{
					mLogger->info("Run() : failed ssl connection, ip : {}", mClientSessions[index]->ip->c_str());
					SSL_free(mClientSessions[index]->clientSSLConnection);
					CloseHandle(mClientSessions[index]->eventHandle);
					closesocket(mClientSessions[index]->clientSocket);
					delete mClientSessions[index]->sessionTimer;
					delete mClientSessions[index]->ip;
					delete mClientSessions[index];
					mClientSessions[index] = nullptr;
					eraseClient(index);
					break;;
				}

				mClientSessions[index]->bIsSSLConnected = true;
			}
			else
			{
				mClientThreadPool->QueueWork(new ClientWork(mClientSessions[index], ClientSessionType::SESSION_READ));
				mClientThreadPool->Signal(ClientThreadPool::THREAD_EVENT::THREAD_SIGNAL);
			}
			break;

		case FD_CLOSE:
			mLogger->info("Run() : client disconnected, ip : {}", mClientSessions[index]->ip->c_str());
			mClientThreadPool->QueueWork(new ClientWork(mClientSessions[index], ClientSessionType::SESSION_CLOSE));
			mClientThreadPool->Signal(ClientThreadPool::THREAD_EVENT::THREAD_SIGNAL);
			eraseClient(index);
			break;
		}

		ZeroMemory(&netEvents, sizeof(netEvents));
	}

	delete this;
	return 0;
}

int HttpsServer::ProcessSSLHandshake(ClientSession* clientSession)
{
	SSL_set_fd(clientSession->clientSSLConnection, static_cast<int>(clientSession->clientSocket));

	int retCode = 0;
	int errorCode = 0;
	uint32_t retryCount = 0;
	char buffer[512];
	ZeroMemory(buffer, 512);

	// infinite
	while (retCode <= 0)
	{
		ERR_clear_error();
		retCode = SSL_accept(clientSession->clientSSLConnection);
		errorCode = SSL_get_error(clientSession->clientSSLConnection, retCode);

		if (errorCode == SSL_ERROR_WANT_READ)
		{
			return SSL_ERROR_WANT_READ;
		}
		else if (errorCode != SSL_ERROR_NONE)
		{
			ERR_error_string_n(ERR_get_error(), buffer, 512);
			mLogger->error("ProcessSSLHandshake() : ssl accept failed, error message : {}", buffer);
			return -1;
		}
	}

	return 0;
}

DWORD __stdcall HttpsServer::checkQuitMessage(LPVOID lpParam)
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
	, mSessionIDSequence(0)
	, mClientThreadPool(ClientThreadPool::GetInstance())
	, mHttpsSocket(NULL)
	, mClientSessions()
	, mLogger(spdlog::rotating_logger_mt("HttpsServer", "./logs/daily.txt", MAX_LOGGER_SIZE, MAX_LOGGER_FILES))
{
	mServer = this;
	mClientSessions.reserve(128);

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

	SSL_CTX_set_mode(mSSLCTX, SSL_MODE_ENABLE_PARTIAL_WRITE);

	mSSL = SSL_new(mSSLCTX);

	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	assert(result == 0);

	if (result != 0)
	{
		std::cout << "Socket start up Failed(HttpsServer constructor)" << std::endl;
	}

	mClientThreadPool->Init();
}

HttpsServer::~HttpsServer()
{
	while (mClientThreadPool->IsThreadsRunning())
	{
		;
	}

	ClientWork::ERROR_CODE errorCode;
	ClientWork* clientCloseWork;

	for (uint32_t i = 0; i < mClientSessions.size(); i++)
	{
		clientCloseWork = new ClientWork(mClientSessions[i], ClientSessionType::SESSION_CLOSE);
		errorCode = clientCloseWork->Run(nullptr);
		assert(errorCode != ClientWork::ERROR_CLOSE_BEFORE_WORK_DONE);
	}

	mClientThreadPool->Signal(ClientThreadPool::THREAD_EVENT::THREAD_CLOSE);
	delete mClientThreadPool;

	HttpHelper::DeleteHttpHelper();

	SSL_CTX_free(mSSLCTX);

	CRYPTO_cleanup_all_ex_data();
	CRYPTO_set_locking_callback(NULL);
	CRYPTO_set_id_callback(NULL);
	OPENSSL_cleanup();
	EVP_cleanup();
	ERR_free_strings();
	sk_SSL_COMP_free(SSL_COMP_get_compression_methods());
	CRYPTO_cleanup_all_ex_data();
	SSL_COMP_free_compression_methods();

	int result = WSACleanup();

	assert(result == 0);
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

void HttpsServer::invalidateSession()
{
	for (uint32_t i = 1; i < mClientSessions.size(); i++)
	{
		mClientSessions[i]->sessionTimer->SetCurrentTime();

		if (mClientSessions[i]->sessionTimer->IsSessionInvalidated())
		{
			mClientThreadPool->QueueWork(new ClientWork(mClientSessions[i], ClientSessionType::SESSION_CLOSE));
			mClientThreadPool->Signal(ClientThreadPool::THREAD_EVENT::THREAD_SIGNAL);
			eraseClient(i);
		}
	}
}

void HttpsServer::signalForRemainingWorks()
{
	if (mClientThreadPool->IsWorkQueueEmpty() != true)
	{
		mClientThreadPool->Signal(ClientThreadPool::THREAD_EVENT::THREAD_SIGNAL);
	}
}

void HttpsServer::eraseClient(uint32_t index)
{
	for (uint32_t i = index; i <= mClientSessions.size() - 1; i++)
	{
		if (i == mClientSessions.size() - 1)
		{
			mClientEventHandles.pop_back();
			mClientSessions.pop_back();
			break;
		}

		mClientSessions[i] = mClientSessions[i + 1];
		mClientEventHandles[i] = mClientEventHandles[i + 1];
	}
}

ClientSession* HttpsServer::createClientSession(socket_t clientSocket, HANDLE clientEventHandle, SSL* clientSSL, std::string& ip)
{
	ClientSession* clientSession = new ClientSession();
	clientSession->sessionID = mSessionIDSequence++;
	clientSession->processingCount = 0;
	clientSession->clientSocket = clientSocket;
	clientSession->eventHandle = clientEventHandle;
	clientSession->clientSSLConnection = clientSSL;
	clientSession->sessionTimer = new SessionTimer();
	clientSession->ip = new std::string(ip);
	clientSession->bIsSSLRetryConnection = false;
	clientSession->bIsSSLConnected = false;

	return clientSession;
}
