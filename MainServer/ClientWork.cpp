#include "ClientWork.h"

SRWLOCK ClientWork::mSRWLock = { 0 };

ClientWork::ClientWork(SSL_CTX* sslCTX, socket_t clientSocket, std::string& clientIP)
	: mHttpHelper(HttpHelper::GetHttpHelper())
	, mClientAddr(clientIP)
	, mSSL(SSL_new(sslCTX))
	, mHttpObject(new HttpObject())
	, mSocket(clientSocket)
	, mEventHandle(WSACreateEvent())
	, mbIsKeepAlive(false)
	, mbIsSSLConnected(false)
{
	SSL_set_accept_state(mSSL);
	WSAEventSelect(mSocket, mEventHandle, FD_READ | FD_CLOSE);
}

ClientWork::~ClientWork()
{
	SSL_shutdown(mSSL);
	SSL_free(mSSL);
	delete mHttpObject;
	WSACloseEvent(mEventHandle);
	closesocket(mSocket);
}

ClientWork::ERROR_CODE ClientWork::Run(void* clientArg)
{
	WSANETWORKEVENTS netEvents;

	int retCode = 0;

	while (true)
	{
		WSAWaitForMultipleEvents(1, &mEventHandle, false, INFINITE, false);
		WSAEnumNetworkEvents(mSocket, mEventHandle, &netEvents);

		if (netEvents.lNetworkEvents & FD_READ)
		{
			retCode = ProcessRequest();

			if (retCode == HTTPS_CLIENT_INVALID_HTTP_HEADER)
			{
				ProcessClose();
				break;
			}
		}

		if (netEvents.lNetworkEvents & FD_CLOSE)
		{
			ProcessClose();
			break;
		}
	}

	return ERROR_NONE;
}

const std::string& ClientWork::GetClientAddr() const
{
	return mClientAddr;
}

bool ClientWork::IsKeepAlive() const
{
	return mbIsKeepAlive;
}

int8_t ClientWork::ProcessRequest()
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

	uint64_t receivedDataLength = receiveData(&buffer);

	if (receivedDataLength == 0)
	{
		return HTTPS_CLIENT_NO_AVAILABLE_DATA;
	}

	if (!mHttpHelper->PrepareResponse(mHttpObject, buffer))
	{
		return HTTPS_CLIENT_INVALID_HTTP_HEADER;
	}

	if (http::IsKeepAlive(mHttpObject) == true)
	{
		mbIsKeepAlive = true;
	}

	if (mClientAddr.empty() == false)
	{
		AcquireSRWLockExclusive(&mSRWLock);
		std::cout << "Client IP : " << mClientAddr << " Dest : " << mHttpObject->GetHttpDest() << std::endl;
		ReleaseSRWLockExclusive(&mSRWLock);
	}

	return writeHttpResponse();
}

int8_t ClientWork::writeHttpResponse()
{
	std::vector<int8_t> response;
	response.reserve(BUFFER_SIZE);

	mHttpHelper->CreateHttpResponse(mHttpObject, response);

	int32_t sslErrorCode = 0;
	size_t responseSize = response.size();
	size_t chunkCount = (responseSize / BASIC_SSL_CHUNK_SIZE) + 1;
	size_t wroteSize = 0;
	size_t wroteSizeToSSL = 0;

	char buffer[512];

	while (wroteSize != responseSize)
	{
		ERR_clear_error();

		if (responseSize - wroteSize >= BASIC_SSL_CHUNK_SIZE)
		{
			sslErrorCode = SSL_write_ex(mSSL, &response[wroteSize], BASIC_SSL_CHUNK_SIZE, &wroteSizeToSSL);
		}
		else
		{
			sslErrorCode = SSL_write_ex(mSSL, &response[wroteSize], responseSize - BASIC_SSL_CHUNK_SIZE * (chunkCount - 1), &wroteSizeToSSL);
		}

		sslErrorCode = SSL_get_error(mSSL, sslErrorCode);

		if (sslErrorCode != SSL_ERROR_NONE)
		{
			AcquireSRWLockExclusive(&mSRWLock);
			std::cout << "ssl write failed, error Code : " << sslErrorCode << std::endl;
			int ret = ERR_get_error();
			ERR_error_string_n(ret, buffer, 512);
			std::cout << buffer << std::endl;
			ReleaseSRWLockExclusive(&mSRWLock);
			return HTTPS_CLIENT_ERROR;
		}

		wroteSize += wroteSizeToSSL;
	}

	return HTTPS_CLIENT_OK;
}

void ClientWork::ProcessClose() const
{
	AcquireSRWLockExclusive(&mSRWLock);
	std::cout << "Client Disconnected : " << mClientAddr << std::endl;
	ReleaseSRWLockExclusive(&mSRWLock);
	delete this;
}

uint64_t ClientWork::receiveData(std::string* content)
{
	uint8_t buffer[BUFFER_SIZE];

	u_long avaliableDataSize = 0;
	int32_t sslErrorCode = 0;
	size_t receivedDataLength = 0;
	uint64_t recvLenSum = 0;

	if (ioctlsocket(mSocket, FIONREAD, &avaliableDataSize) != 0)
	{
		return 0;
	}

	do
	{
		ERR_clear_error();
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

int ClientWork::processSSLHandshake()
{
	SSL_set_fd(mSSL, static_cast<int>(mSocket));

	int retCode = 0;
	int errorCode = 0;
	char buffer[512];
	ZeroMemory(buffer, 512);

	while (retCode <= 0)
	{
		ERR_clear_error();
		retCode = SSL_accept(mSSL);
		errorCode = SSL_get_error(mSSL, retCode);

		if (errorCode == SSL_ERROR_WANT_READ)
		{
			continue;
		}
		else if (errorCode != SSL_ERROR_NONE)
		{
			AcquireSRWLockExclusive(&mSRWLock);
			std::cout << "ssl accept failed, error Code : " << errorCode << std::endl;
			ERR_error_string_n(ERR_get_error(), buffer, 512);
			std::cout << buffer << std::endl;
			ReleaseSRWLockExclusive(&mSRWLock);
			ProcessClose();
			return HTTPS_CLIENT_ERROR;
		}
	}

	mbIsSSLConnected = true;
	return HTTPS_CLIENT_OK;
}