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

int HttpsClient::InitializeClient(IServer* server, SRWLOCK* srwLock, SOCKET clientSocket)
{
	mServer = server;
	mHttpHelper = HttpHelper::GetHttpHelper();
	mSSLCTX = mServer->GetSSLCTX();
	/*mRouter = mServer->GetHTMLPageRouter();*/
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

	int32_t clientAddrLen = sizeof(mClientAddr);
	getpeername(mSocket, reinterpret_cast<sockaddr*>(&mClientSockAddr), &clientAddrLen);
	InetNtop(AF_INET, &mClientSockAddr.sin_addr, mClientAddr, sizeof(mClientAddr) / 2);

	mbIsKeepAlive = false;
	mbIsSSLConnected = false;
	mbIsReceivedInitialHttpHeader = false;

	return 0;
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
	mbIsReceivedInitialHttpHeader = true;

	// TODO : Check Http args

	return 0;
}

int HttpsClient::ProcessWrite()
{
	std::string response;
	response.reserve(BUFFER_SIZE);
	mHttpHelper->CreateHttpResponse(mHttpObject, response);

	int32_t returnValue = 0;
	size_t writeSize = 0;
	int32_t errorCode = 0;

	while (returnValue <= 0)
	{
		returnValue = SSL_write_ex(mSSL, &response.c_str()[writeSize], response.length() - writeSize, &writeSize);
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
	char buffer[BUFFER_SIZE];

	u_long readLen = 0;
	int recvLen = 0;
	int recvLenSum = 0;

	do
	{
		int retValue = ioctlsocket(mSocket, FIONREAD, &readLen);
		if (retValue != 0)
		{
			printSocketError();
			break;
		}

		if (readLen == 0)
		{
			break;
		}

		if (readLen > BUFFER_SIZE)
		{
			readLen = BUFFER_SIZE;
		}


		//recvLen = recv(clientSocket, buffer, readLen, 0);
		recvLen = SSL_read(mSSL, buffer, readLen);

		if (recvLen != 0)
		{
			for (uint16_t i = 0; i < readLen; ++i)
			{
				if (buffer[i] == '\r' && buffer[i + 1] == '\n')
				{
					if (buffer[i + 2] == '\r' && buffer[i + 3] == '\n')
					{
						content->push_back(L'\0');
						break;
					}
					content->push_back(buffer[i]);
				}
				else
				{
					content->push_back(buffer[i]);
				}
			}

			recvLenSum += recvLen;
		}
	} while (recvLen != 0);

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