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
	network::OpenSocketOverlappedIOMode(mHttpsSocket, network::HTTPS_PORT_NUMBER, mSSL, true);
	mIOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, THREAD_COUNT);

	HANDLE eventHandle = WSACreateEvent();

	OVERLAPPED overlapped;
	ZeroMemory(&overlapped, sizeof(OVERLAPPED));

	overlapped.hEvent = eventHandle;
	assert(overlapped.hEvent != WSA_INVALID_EVENT);

	eventHandle = nullptr;

	std::string ipAddressBuffer;
	ipAddressBuffer.reserve(32);
	socket_t clientSocket = NULL;
	SSL* ssl = nullptr;
	sockaddr_in clientSockAddr;
	ZeroMemory(&clientSockAddr, sizeof(sockaddr));

	CreateThread(nullptr, 0, HttpsServer::checkQuitMessage, nullptr, NULL, nullptr);

	DWORD flags = 0;
	int ret = 0;
	int chunkIndex = 0;
	int chunkSize = WSA_MAXIMUM_WAIT_EVENTS;
	int size = 0;

	mClientThreadPool->Init(mIOCPHandle);

	while (!mbIsQuitButtonPressed)
	{
		clientSocket = network::ProcessAccept(mHttpsSocket, clientSockAddr, ipAddressBuffer);
		if (clientSocket == NULL)
		{
			//mLogger->error("Run() : clientSocket was NULL");
			continue;
		}

		SOCKETINFO* socketInfo = new SOCKETINFO();
		ZeroMemory(socketInfo, sizeof(SOCKETINFO));
		socketInfo->sendMemoryBlock = GetMemoryBlock();
		socketInfo->recvMemoryBlock = GetMemoryBlock();

		if (socketInfo->sendMemoryBlock == nullptr || socketInfo->recvMemoryBlock == nullptr)
		{
			delete socketInfo;
			closesocket(mHttpsSocket);
			continue;
		}

		CreateIoCompletionPort(reinterpret_cast<HANDLE>(clientSocket), mIOCPHandle, clientSocket, THREAD_COUNT); // TODO 마지막 스레드 수 상수로 지정하기
		ssl = SSL_new(mSSLCTX);
		SSL_set_accept_state(ssl);
		socketInfo->socket = clientSocket;
		socketInfo->pendingCount = 0;
		socketInfo->isbClosed = false;
		InitializeSRWLock(&socketInfo->srwLock);
		socketInfo->session = createClientSession(clientSocket, nullptr, ssl, ipAddressBuffer);
		socketInfo->recvbytes = 0;
		socketInfo->sendbytes = 0;
		socketInfo->sendPendingBytes = 0;
		socketInfo->recvBuffer.buf = reinterpret_cast<char*>(socketInfo->recvMemoryBlock->ptr);
		socketInfo->recvBuffer.len = BLOCK_SIZE;
		socketInfo->sendBuffer.buf = reinterpret_cast<char*>(socketInfo->sendMemoryBlock->ptr);
		socketInfo->sendBuffer.len = BLOCK_SIZE;

		//mLogger->info("Run() : client connected, ip : {}", socketInfo->session->ip->c_str());

		// SOCKETINFO 구조체의 첫 번째 요소 overlapped를 넘겨줌으로써 나중에 type cast로 나머지 SOCKETINFO 멤버 접속 가능해짐
		ret = WSARecv(clientSocket, &socketInfo->recvBuffer, 1, nullptr, &flags, &socketInfo->overlapped, nullptr);
		httpHelper::InterLockedIncrement(socketInfo);

		if (ret == SOCKET_ERROR)
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)
			{
				printSocketError();
				assert(false);
			}
		}
		continue;
	}

	delete this;
	return 0;
}

MemoryBlock* HttpsServer::GetMemoryBlock()
{
	return mMemoryPool->Allocate();
}

void HttpsServer::PutMemoryBlock(MemoryBlock* memoryBlock)
{
	return mMemoryPool->DeAllocate(memoryBlock);
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
	, mMemoryPool(new MemoryPool())
	, mClientThreadPool(ClientThreadPool::GetInstance(mMemoryPool))
	, mHttpsSocket(NULL)
	, mIOCPHandle(NULL)
	, mLogger(spdlog::rotating_logger_mt("HttpsServer", "logs/log.txt", MAX_LOGGER_SIZE, MAX_LOGGER_FILES))
{
	mServer = this;

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
}

HttpsServer::~HttpsServer()
{
	delete mClientThreadPool;
	delete mMemoryPool;

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

	CloseHandle(mIOCPHandle);
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

ClientSession* HttpsServer::createClientSession(socket_t clientSocket, HANDLE clientEventHandle, SSL* clientSSL, std::string& ip)
{
	ClientSession* clientSession = new ClientSession();
	clientSession->sessionID = mSessionIDSequence++;
	clientSession->processingCount = 0;
	InitializeSRWLock(&clientSession->lock);
	clientSession->clientSocket = clientSocket;
	clientSession->eventHandle = clientEventHandle;
	clientSession->httpObject = new HttpObject();
	clientSession->clientSSLConnection = clientSSL;
	clientSession->clientSSLReadBIO = BIO_new(BIO_s_mem());
	clientSession->clientSSLWriteBIO = BIO_new(BIO_s_mem());
	clientSession->currentOperation = OPERATION::RECEIVE;
	BIO_set_nbio(clientSession->clientSSLReadBIO, 1);
	SSL_set_bio(clientSSL, clientSession->clientSSLReadBIO, clientSession->clientSSLWriteBIO);
	SSL_set_accept_state(clientSession->clientSSLConnection);

	clientSession->sessionTimer = new SessionTimer();
	clientSession->ip = new std::string(ip);
	clientSession->bIsSSLRetryConnection = false;
	clientSession->bIsSSLConnected = false;
	clientSession->bIsDisconnected = false;

	return clientSession;
}
