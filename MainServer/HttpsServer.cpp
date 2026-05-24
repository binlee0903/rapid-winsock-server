#include "stdafx.h"
#include "HttpsServer.h"


bool HttpsServer::mbIsQuitButtonPressed = false;
MemoryPool* HttpsServer::mMemoryPool = new MemoryPool();
HANDLE HttpsServer::mIOCPHandle = NULL;
socket_t HttpsServer::mHttpsSocket = NULL;
LPFN_ACCEPTEX HttpsServer::mAcceptEx = nullptr;
LPFN_GETACCEPTEXSOCKADDRS HttpsServer::mGetAcceptExSockAddrs = nullptr;
LockFreeQueue<SOCKETINFO*>* HttpsServer::mQueue = new LockFreeQueue<SOCKETINFO*>(MAX_WORK_QUEUE_SIZE);
std::shared_ptr<spdlog::logger> HttpsServer::mLogger(spdlog::rotating_logger_mt("HttpsServer", "logs/log.txt", MAX_LOGGER_SIZE, MAX_LOGGER_FILES));

HttpsServer::HttpsServer()
	: mSessionIDSequence(0)
{
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

	for (size_t i = 0; i < THREAD_COUNT; i++)
	{
		mThreads[i] = NULL;
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
	for (uint32_t i = 0; i < THREAD_COUNT; i++)
	{
		WaitForSingleObject(mThreads[i], INFINITE);
	}

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

int32_t HttpsServer::Start()
{
	// IOCP setting part
	mIOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, THREAD_COUNT);
	assert(mIOCPHandle != NULL);

	network::OpenSocketOverlappedIOMode(mHttpsSocket, network::HTTPS_PORT_NUMBER, mSSL, &mAcceptEx,
		&mGetAcceptExSockAddrs, mIOCPHandle, true);

	HANDLE eventHandle = WSACreateEvent();
	OVERLAPPED overlapped;
	ZeroMemory(&overlapped, sizeof(OVERLAPPED));

	overlapped.hEvent = eventHandle;
	assert(overlapped.hEvent != WSA_INVALID_EVENT);
	// end of iocp setting part

	// temporary variables for client's connection
	DWORD receivedByteCount = 0;
	DWORD flags = 0;
	int32_t ret = 0;
	std::string ipAddressBuffer;
	ipAddressBuffer.reserve(32);
	socket_t clientSocket = NULL;
	sockaddr_in clientSockAddr;
	ZeroMemory(&clientSockAddr, sizeof(sockaddr));
	ClientSession* clientSession;
	SOCKETINFO* socketInfo;
	ClientWork::STATUS status;

	// for 'q' command
	CreateThread(nullptr, 0, HttpsServer::checkQuitMessage, nullptr, NULL, nullptr);

	for (uint32_t i = 0; i < THREAD_COUNT; i++)
	{
		mThreads[i] = CreateThread(nullptr, NULL, &HttpsServer::processNetworkIO, nullptr, NULL, nullptr);
	}

	while (!mbIsQuitButtonPressed)
	{
		if (mQueue->pop(socketInfo) == true)
		{
			// logic
			status = ClientWork::ProcessRequest(socketInfo);


			// error handling

		}
		else
		{
			// etc logic
		}
	}

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
		scanf_s("%c\n", &ch, 1);
	}

	mbIsQuitButtonPressed = true;

	return 0;
}

// common functions
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

SOCKETINFO* HttpsServer::createClientSocket()
{
	socket_t clientSocket = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	SOCKETINFO* socketInfo = new SOCKETINFO();
	ZeroMemory(socketInfo, sizeof(SOCKETINFO));

	socketInfo->isbClosed = false;
	socketInfo->operation = OPERATION::ACCEPT;
	socketInfo->socket = clientSocket;
	socketInfo->sendMemoryBlock = GetMemoryBlock();
	socketInfo->recvMemoryBlock = GetMemoryBlock();

	if (socketInfo->sendMemoryBlock == nullptr && socketInfo->recvMemoryBlock == nullptr)
	{
		// handle full client situation
		delete socketInfo;
		closesocket(mHttpsSocket);

		return nullptr;
	}

	return socketInfo;
}

void HttpsServer::postAccept(SOCKETINFO** socketInfo, int8_t* buffer)
{
	*socketInfo = createClientSocket();

	if (*socketInfo == nullptr)
	{
		return;
	}

	int32_t result = mAcceptEx(
		mHttpsSocket,
		(*socketInfo)->socket,
		buffer,
		0,
		sizeof(sockaddr_in) + 16,
		sizeof(sockaddr_in) + 16,
		0,
		&(*socketInfo)->overlapped
	);

	if (result == FALSE) {
		int error = WSAGetLastError();
		assert(error == WSA_IO_PENDING);
		// WSA_IO_PENDING은 비동기 작업이 성공적으로 시작되었다는 의미
	}
}

DWORD __stdcall HttpsServer::processNetworkIO(LPVOID lpParam)
{
	int8_t outputBuffer[RECV_DATA_LENGTH + ADDR_BUF_SIZE * 2];
	socket_t clientSocket = NULL;
	DWORD processedByteCount = 0;
	DWORD flags = 0;
	int ret = 0;

	HANDLE iocp;
	SOCKETINFO* socketInfo;
	sockaddr_in* localAddr = new sockaddr_in();
	sockaddr_in* remoteAddr = new sockaddr_in();
	int localAddrLen = sizeof(sockaddr_in);
	int remoteAddrLen = sizeof(sockaddr_in);

	postAccept(&socketInfo, outputBuffer);

	while (true) // need to make goal
	{
		ret = GetQueuedCompletionStatus(mIOCPHandle, &processedByteCount, &clientSocket,
			reinterpret_cast<LPOVERLAPPED*>(&socketInfo), WSA_INFINITE);

		if (ret == 0)
		{
			ret = WSAGetLastError();

			assert(ret == ERROR_IO_PENDING);
		}

		if (socketInfo->operation == OPERATION::RECV && processedByteCount == 0 && socketInfo->pendingCount == 0)
		{
			// FIN
			mMemoryPool->DeAllocate(socketInfo->recvMemoryBlock);
			mMemoryPool->DeAllocate(socketInfo->sendMemoryBlock);
			ClientWork::CloseConnection(socketInfo);
			continue;
		}

		switch (socketInfo->operation)
		{
		case OPERATION::ACCEPT:
			// do accept
			iocp = CreateIoCompletionPort((HANDLE)socketInfo->socket, mIOCPHandle, (u_long)0, 0);

			if (iocp == NULL)
			{
				// Failed to associate with iocp
				delete socketInfo;
				printf("CreateIoCompletionPort associate failed with error: %u\n", GetLastError());
				closesocket(socketInfo->socket);
				return -1;
			}

			mGetAcceptExSockAddrs(
				outputBuffer,
				RECV_DATA_LENGTH,
				ADDR_BUF_SIZE,
				ADDR_BUF_SIZE,
				(LPSOCKADDR*)&localAddr,
				&localAddrLen,
				(LPSOCKADDR*)&remoteAddr,
				&remoteAddrLen
			);

			socketInfo->session->ip = new std::string(inet_ntoa(remoteAddr->sin_addr));
			socketInfo->session->ip->append("/");
			socketInfo->session->ip->append(std::to_string(remoteAddr->sin_port));

			mLogger->info("Run() : client connected, ip : {}", socketInfo->session->ip->c_str());
			socketInfo->session = new ClientSession();

			// call recv
			ret = WSARecv(socketInfo->socket, &socketInfo->recvBuffer, 1, &processedByteCount, &flags, &socketInfo->overlapped, nullptr);

			// wokers should not know about http thing, like datas in IOCP
			if (ret == 0)
			{
				ret = PostQueuedCompletionStatus(mIOCPHandle, processedByteCount, socketInfo->socket, &socketInfo->overlapped);
				if (ret == SOCKET_ERROR)
				{
					ret = WSAGetLastError();
				}
			}

			// later, prepare another accept
			postAccept(&socketInfo, outputBuffer);
			break;

		default:


			mQueue->push(socketInfo);
			break;
		}

		// test this later
		//if (socketInfo->sendPendingBytes != processedByteCount)
		//{
		//	// recv
		//	socketInfo->recvbytes = processedByteCount;

		//	ret = ClientWork::ProcessRequest(socketInfo);
		//	mQueue->push(socketInfo);
		//}
		//else
		//{
		//	// send result
		//}
	}

	delete localAddr;
	delete remoteAddr;

	return 0;
}

ClientSession* HttpsServer::createClientSession(socket_t clientSocket, std::string& ip)
{
	ClientSession* clientSession = new ClientSession();
	clientSession->sessionID = mSessionIDSequence++;
	clientSession->httpObject = new HttpObject();
	clientSession->clientSSLConnection = SSL_new(mSSLCTX);
	clientSession->sessionTimer = new SessionTimer();
	clientSession->ip = new std::string(ip);
	clientSession->bIsSSLRetryConnection = false;
	clientSession->bIsSSLConnected = false;

	return clientSession;
}

void HttpsServer::destroyClientSession(ClientSession* clientSession)
{
	delete clientSession->httpObject;
	delete clientSession->sessionTimer;
	delete clientSession->ip;
	delete clientSession;
}
