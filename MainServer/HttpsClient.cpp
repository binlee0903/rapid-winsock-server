#include "HttpsClient.h"

HttpsClient::~HttpsClient()
{
	SSL_shutdown(mSSL);
	SSL_free(mSSL);
	delete mHttpObject;
	WSACloseEvent(mEventHandle);
	closesocket(mSocket);
}

HANDLE HttpsClient::GetEventHandle() const
{
	return mEventHandle;
}

uint16_t HttpsClient::GetRequestCount() const
{
	return mRequestCount;
}

void HttpsClient::IncreaseRequestCount()
{
	mRequestCount++;
}

std::string& HttpsClient::GetClientIP()
{
	return mClientAddr;
}

int HttpsClient::InitializeClient(IServer* server, SRWLOCK* srwLock, SOCKET clientSocket)
{
	mServer = server;
	mRequestCount = 0;
	mHttpHelper = HttpHelper::GetHttpHelper(mServer->GetHttpFileContainer());
	mSSLCTX = mServer->GetSSLCTX();
	mSRWLock = srwLock;
	mHttpObject = new HttpObject();
	mEventHandle = WSACreateEvent();
	mSocket = clientSocket;
	if (mEventHandle == WSA_INVALID_EVENT)
	{
		closesocket(mSocket);
		return -1;
	}

	WSAEventSelect(mSocket, mEventHandle, FD_READ | FD_CLOSE);

	auto blackList = mServer->GetBlackLists();
	auto search = blackList->find(mClientAddr);

	if (search != blackList->end())
	{
		closesocket(mSocket);
		return -1;
	}

	mbIsKeepAlive = false;
	mbIsSSLConnected = false;

	return 0;
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
		std::cout << msg << std::endl;
	}
	LocalFree(msg);
}

int HttpsClient::ProcessRead()
{
	if (mbIsSSLConnected == false)
	{
		if (processSSLHandshake() != 0)
		{
			return -1;
		}
	}

	std::string content;
	content.reserve(BUFFER_SIZE * 2);

	uint32_t receivedDataLength = receiveData(&content);

	if (receivedDataLength == 0)
	{
		return 1;
	}

	mHttpHelper->ParseHttpHeader(mHttpObject, content);

	auto* header = mHttpObject->GetHttpHeader();
	mClientAddr = header->Get("X-Forwarded-For");

	if (mClientAddr.empty() == false)
	{
		AcquireSRWLockExclusive(mSRWLock);
		std::cout << "Client IP : " << mClientAddr << " Method : " << mHttpObject->GetHttpMethod() << std::endl;
		ReleaseSRWLockExclusive(mSRWLock);
	}

	std::string keepAlive = mHttpObject->GetHttpHeader()->Get("Connection");

	if (keepAlive == "keep-alive")
	{
		mbIsKeepAlive = true;
	}
	else
	{
		mbIsKeepAlive = false;
	}

	// TODO : Check Http args

	return 0;
}

int HttpsClient::ProcessWrite()
{
	std::vector<int8_t> response;
	response.reserve(BUFFER_SIZE);
	mHttpHelper->CreateHttpResponse(mHttpObject, response);

	int32_t returnValue = 0;
	size_t writeSize = 0;
	int32_t errorCode = 0;

	while (returnValue <= 0)
	{
		returnValue = SSL_write_ex(mSSL, &response[writeSize], response.size() - writeSize, &writeSize);
		errorCode = SSL_get_error(mSSL, returnValue);

		if (errorCode == SSL_ERROR_WANT_WRITE)
		{
			continue;
		}
		else if (errorCode != SSL_ERROR_NONE)
		{
			AcquireSRWLockExclusive(mSRWLock);
			std::wcout << L"ssl write failed, error Code : " << errorCode << std::endl;
			ReleaseSRWLockExclusive(mSRWLock);
			ProcessClose();
		}
	}

	return 0;
}

int HttpsClient::ProcessClose()
{
	AcquireSRWLockExclusive(mSRWLock);
	std::cout << "Client Disconnected : " << mClientAddr << std::endl;
	ReleaseSRWLockExclusive(mSRWLock);

	delete this;
	return 0;
}

int HttpsClient::ProcessOOB()
{
	delete this;
	return 0;
}

uint32_t HttpsClient::receiveData(std::string* content)
{
	uint8_t buffer[BUFFER_SIZE];

	u_long avaliableDataSize = 0;
	u_long readDataSize = 0;
	int32_t returnValue = 0;
	int32_t errorCode = 0;
	size_t recvLen = 0;
	int recvLenSum = 0;

	int retValue = ioctlsocket(mSocket, FIONREAD, &avaliableDataSize);
	if (retValue != 0)
	{
		printSocketError();
		return 0;
	}

	if (avaliableDataSize == 0 || avaliableDataSize == 24)
	{
		return 0;
	}

	do
	{
		ZeroMemory(buffer, sizeof(buffer));
		returnValue = SSL_read_ex(mSSL, buffer, BUFFER_SIZE, &recvLen);
		errorCode = SSL_get_error(mSSL, returnValue);

		if (recvLen != 0)
		{
			for (uint32_t i = 0; i < recvLen; ++i)
			{
				content->push_back(buffer[i]);
			}

			recvLenSum += recvLen;
		}

		avaliableDataSize = SSL_pending(mSSL);
	} while (avaliableDataSize != 0);

	return recvLenSum;
}

int HttpsClient::processSSLHandshake()
{
	mSSL = SSL_new(mSSLCTX);
	SSL_set_fd(mSSL, mSocket);

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
			std::wcout << L"ssl accept failed, error Code : " << errorCode << std::endl;
			ReleaseSRWLockExclusive(mSRWLock);
			ProcessClose();
			return -1;
		}
	}

	mbIsSSLConnected = true;
	return 0;
}