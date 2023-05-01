#include "HttpsClient.h"

HttpsClient::HttpsClient(IServer* server, HttpRouter* router, SRWLOCK* srwLock, SSL_CTX* sslCTX, socket_t clientSocket)
	: mSSL(SSL_new(sslCTX))
	, mSRWLock(srwLock)
	, mServer(server)
	, mHttpObject(new HttpObject())
	, mSocket(clientSocket)
	, mEventHandle(WSACreateEvent())
	, mbIsKeepAlive(false)
	, mbIsSSLConnected(false)
	, mRequestCount(0)
	, mClientAddr()
	, mRouter(router)
{
	WSAEventSelect(mSocket, mEventHandle, FD_READ | FD_CLOSE);

	_beginthreadex(nullptr, 0, &HttpsClient::Run, nullptr, NULL, nullptr);
}

HttpsClient::~HttpsClient()
{
	SSL_shutdown(mSSL);
	SSL_free(mSSL);
	delete mHttpObject;
	WSACloseEvent(mEventHandle);
	closesocket(mSocket);
}

void HttpsClient::IncreaseRequestCount()
{
	mRequestCount++;
}

uint32_t HttpsClient::Run(void* clientArg)
{
	HttpsClient* client = reinterpret_cast<HttpsClient*>(clientArg);

	WSANETWORKEVENTS netEvents;

	while (true)
	{
		WSAWaitForMultipleEvents(1, &client->mEventHandle, false, INFINITE, false);
		WSAEnumNetworkEvents(client->mSocket, client->mEventHandle, &netEvents);
		client->mRequestCount++;

		if (netEvents.lNetworkEvents & FD_READ)
		{
			int retCode = client->ProcessRequest();

			if (retCode == HTTPS_CLIENT_NO_AVAILABLE_DATA)
			{
				client->ProcessClose();
				break;
			}
		}

		if (netEvents.lNetworkEvents & FD_CLOSE)
		{
			client->ProcessClose();
			break;
		}
	}
}

bool HttpsClient::IsKeepAlive()
{
	return mbIsKeepAlive;
}

void HttpsClient::printSocketError()
{
	char* msg = nullptr;
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, WSAGetLastError(),
		LANG_SYSTEM_DEFAULT, reinterpret_cast<char*>(&msg), 0, nullptr);

	if (msg != nullptr)
	{
		AcquireSRWLockExclusive(mSRWLock);
		std::cout << msg << std::endl;
		ReleaseSRWLockExclusive(mSRWLock);
	}
	LocalFree(msg);
}

int8_t HttpsClient::ProcessRequest()
{
	if (mbIsSSLConnected == false)
	{
		if (processSSLHandshake() != 0)
		{
			return HTTPS_CLIENT_ERROR;
		}
	}

	std::string buffer;
	buffer.reserve(BUFFER_SIZE * 2);

	uint32_t receivedDataLength = receiveData(&buffer);

	if (receivedDataLength == 0)
	{
		return HTTPS_CLIENT_NO_AVAILABLE_DATA;
	}

	mHttpHelper->ParseHttpHeader(mHttpObject, buffer);

	if (mClientAddr.empty() == false)
	{
		AcquireSRWLockExclusive(mSRWLock);
		std::cout << "Client IP : " << mClientAddr << " Method : " << mHttpObject->GetHttpMethod() << std::endl;
		ReleaseSRWLockExclusive(mSRWLock);
	}

	return writeHttpResponse();
}

int8_t HttpsClient::writeHttpResponse()
{
	std::vector<int8_t>* header = new std::vector<int8_t>();
	std::vector<int8_t>* responseBody;

	header->reserve(BUFFER_SIZE);

	mRouter->CreateHeader(mHttpObject, header);
	mRouter->WriteServiceFileToVector(mHttpObject, responseBody);

	int32_t sslErrorCode = 0;
	size_t wroteSize = 0;

	while (sslErrorCode <= 0)
	{
		sslErrorCode = SSL_write_ex(mSSL, &header[wroteSize], header->size() - wroteSize, &wroteSize);
		sslErrorCode = SSL_get_error(mSSL, sslErrorCode);

		if (sslErrorCode == SSL_ERROR_WANT_WRITE)
		{
			continue;
		}
		else if (sslErrorCode != SSL_ERROR_NONE)
		{
			delete header;

			AcquireSRWLockExclusive(mSRWLock);
			std::cout << "ssl write failed, error Code : " << sslErrorCode << std::endl;
			ReleaseSRWLockExclusive(mSRWLock);
			ProcessClose();
			return HTTPS_CLIENT_ERROR;
		}
	}

	wroteSize = 0;

	while (sslErrorCode <= 0)
	{
		sslErrorCode = SSL_write_ex(mSSL, &responseBody[wroteSize], responseBody->size() - wroteSize, &wroteSize);
		sslErrorCode = SSL_get_error(mSSL, sslErrorCode);

		if (sslErrorCode == SSL_ERROR_WANT_WRITE)
		{
			continue;
		}
		else if (sslErrorCode != SSL_ERROR_NONE)
		{
			delete header;

			AcquireSRWLockExclusive(mSRWLock);
			std::cout << "ssl write failed, error Code : " << sslErrorCode << std::endl;
			ReleaseSRWLockExclusive(mSRWLock);
			ProcessClose();
			return HTTPS_CLIENT_ERROR;
		}
	}

	delete header;
	return HTTPS_CLIENT_OK;
}

void HttpsClient::ProcessClose()
{
	AcquireSRWLockExclusive(mSRWLock);
	std::cout << "Client Disconnected : " << mClientAddr << std::endl;
	ReleaseSRWLockExclusive(mSRWLock);

	delete this;
}

uint64_t HttpsClient::receiveData(std::string* content)
{
	uint8_t buffer[BUFFER_SIZE];

	u_long avaliableDataSize = 0;
	int32_t sslErrorCode = 0;
	uint64_t receivedDataLength = 0;
	uint64_t recvLenSum = 0;

	if (ioctlsocket(mSocket, FIONREAD, &avaliableDataSize) != 0)
	{
		printSocketError();
		return 0;
	}

	do
	{
		ZeroMemory(buffer, sizeof(buffer));
		sslErrorCode = SSL_read_ex(mSSL, buffer, BUFFER_SIZE, &receivedDataLength);
		sslErrorCode = SSL_get_error(mSSL, sslErrorCode);

		if (receivedDataLength != 0)
		{
			for (uint32_t i = 0; i < receivedDataLength; ++i)
			{
				content->push_back(buffer[i]);
			}

			recvLenSum += receivedDataLength;
		}

		avaliableDataSize = SSL_pending(mSSL);
	} while (avaliableDataSize != 0);

	return recvLenSum;
}

int HttpsClient::processSSLHandshake()
{
	SSL_set_fd(mSSL, static_cast<int>(mSocket));

	int retCode = 0;
	int errorCode = 0;

	while (retCode <= 0)
	{
		retCode = SSL_accept(mSSL);
		errorCode = SSL_get_error(mSSL, retCode);

		if (errorCode == SSL_ERROR_WANT_READ)
		{
			continue;
		}
		else if (errorCode != SSL_ERROR_NONE)
		{
			AcquireSRWLockExclusive(mSRWLock);
			std::cout << "ssl accept failed, error Code : " << errorCode << std::endl;
			ReleaseSRWLockExclusive(mSRWLock);
			ProcessClose();
			return HTTPS_CLIENT_ERROR;
		}
	}

	mbIsSSLConnected = true;
	return HTTPS_CLIENT_OK;
}