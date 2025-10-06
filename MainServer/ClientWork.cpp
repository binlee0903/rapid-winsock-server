#include "stdafx.h"
#include "ClientWork.h"

SRWLOCK ClientWork::mSRWLock = { 0 };
HttpResponseGenerator* ClientWork::mHttpResponseGenerator = HttpResponseGenerator::GetRouter();

ClientWork::ClientWork(ClientSession* clientSession, ClientSessionType sessionType)
	: mHttpObject(new HttpObject())
	, mClientSession(clientSession)
	, mClientSessionType(sessionType)
	, misbProcessing(false)
{
}

ClientWork::~ClientWork()
{
	delete mHttpObject;
}

void ClientWork::FinishWork() const
{
	delete this;
}

ClientWork::ERROR_CODE ClientWork::Run(void* clientArg)
{
	STATUS status = STATUS::HTTPS_CLIENT_OK;
	misbProcessing = true;

	if (mClientSessionType == ClientSessionType::SESSION_READ)
	{
		AcquireSRWLockExclusive(&mClientSession->lock);
		mClientSession->processingCount++;
		ReleaseSRWLockExclusive(&mClientSession->lock);
	}

	switch (mClientSessionType)
	{
	case ClientSessionType::SESSION_READ:
		status = ProcessRequest();
		AcquireSRWLockExclusive(&mClientSession->lock);
		mClientSession->processingCount--;
		ReleaseSRWLockExclusive(&mClientSession->lock);
		misbProcessing = false;

		switch (status)
		{
		case ClientWork::HTTPS_CLIENT_OK:
			break;
		case ClientWork::HTTPS_CLIENT_ERROR:
			return ERROR_SSL;
		case ClientWork::HTTPS_CLIENT_NO_AVAILABLE_DATA:
			return ERROR_NONE;
		case ClientWork::HTTPS_CLIENT_INVALID_HTTP_HEADER:
			break;

		case ClientWork::HTTPS_CLIENT_ZERO_RETURN:
			return ERROR_ZERO_RETURN;

		default:
			break;
		}

		break;

	case ClientSessionType::SESSION_CLOSE:
		if (IsProcessing() == true)
		{
			return ERROR_CLOSE_BEFORE_WORK_DONE;
		}

		closeConnection();
		break;
	}

	return ERROR_NONE;
}

bool ClientWork::IsProcessing() const
{
	bool ret = false;

	AcquireSRWLockExclusive(&mClientSession->lock);
	ret = mClientSession->processingCount > 0;
	ReleaseSRWLockExclusive(&mClientSession->lock);

	return ret;
}

bool ClientWork::IsThisWorkProcessing() const
{
	return misbProcessing;
}

ClientSession* ClientWork::GetClientSession() const
{
	return this->mClientSession;
}

ClientWork::STATUS ClientWork::ProcessRequest()
{
	std::string buffer;
	buffer.reserve(BUFFER_SIZE * 2);

	uint64_t receivedDataLength = receiveData(&buffer);

	if (receivedDataLength == 0)
	{
		return HTTPS_CLIENT_NO_AVAILABLE_DATA;
	}

	if (receivedDataLength == HTTPS_CLIENT_ZERO_RETURN)
	{
		return HTTPS_CLIENT_ZERO_RETURN;
	}

	if (httpHelper::PrepareResponse(mHttpObject, buffer) == false)
	{
		return HTTPS_CLIENT_INVALID_HTTP_HEADER;
	}

	return writeHttpResponse();
}

ClientSessionType ClientWork::GetType() const
{
	return this->mClientSessionType;
}

bool ClientWork::IsNull()
{
	return mHttpObject == nullptr;
}

ClientWork::STATUS ClientWork::writeHttpResponse()
{
	int8_t* responseBody; // allocate heap memory internally. this needs to be replaced by memorypool 
	size_t responseSize = mHttpResponseGenerator->Generate(mHttpObject, &responseBody);

	int32_t sslErrorCode = 0;
	size_t chunkCount = (responseSize / BASIC_SSL_CHUNK_SIZE) + 1;
	size_t wroteSize = 0;
	size_t wroteSizeToSSL = 0;
	u_long availSize = 0;

	char buffer[512];
	ZeroMemory(buffer, sizeof(buffer));

	while (wroteSize < responseSize)
	{
		ERR_clear_error();

		if (responseSize - wroteSize > BASIC_SSL_CHUNK_SIZE)
		{
			sslErrorCode = SSL_write_ex(mClientSession->clientSSLConnection, &responseBody[wroteSize], BASIC_SSL_CHUNK_SIZE, &wroteSizeToSSL);
		}
		else
		{
			sslErrorCode = SSL_write_ex(mClientSession->clientSSLConnection, &responseBody[wroteSize], responseSize - BASIC_SSL_CHUNK_SIZE * (chunkCount - 1), &wroteSizeToSSL);
		}

		if (sslErrorCode <= 0)
		{
			sslErrorCode = SSL_get_error(mClientSession->clientSSLConnection, sslErrorCode);

			if (sslErrorCode == SSL_ERROR_ZERO_RETURN)
			{
				delete[] responseBody;
				shutdown(mClientSession->clientSocket, SD_SEND);
				SSL_shutdown(mClientSession->clientSSLConnection);
				return HTTPS_CLIENT_ZERO_RETURN;
			}

			if (sslErrorCode != SSL_ERROR_NONE)
			{
				delete[] responseBody;

				AcquireSRWLockExclusive(&mSRWLock);
				std::cout << "ssl write failed, error Code : " << sslErrorCode << std::endl;
				ERR_error_string_n(sslErrorCode, buffer, 512);
				std::cout << buffer << std::endl;
				ReleaseSRWLockExclusive(&mSRWLock);
				return HTTPS_CLIENT_ERROR;
			}
		}
		else
		{
			wroteSize += wroteSizeToSSL;
		}
	}

	delete[] responseBody;
	return HTTPS_CLIENT_OK;
}

void ClientWork::closeConnection()
{
	assert(mClientSession->processingCount == 0);
	mClientSession->bIsDisconnected = true;
	delete mClientSession->ip;
	delete mClientSession->sessionTimer;
	shutdown(mClientSession->clientSocket, SD_RECEIVE);
	WSACloseEvent(mClientSession->eventHandle);
	SSL_free(mClientSession->clientSSLConnection);
	closesocket(mClientSession->clientSocket);
	delete mClientSession;
}

uint64_t ClientWork::receiveData(std::string* content)
{
	uint8_t buffer[BUFFER_SIZE];

	u_long avaliableDataSize = 0;
	int32_t sslErrorCode = 0;
	size_t receivedDataLength = 0;
	uint64_t recvLenSum = 0;

	if (ioctlsocket(mClientSession->clientSocket, FIONREAD, &avaliableDataSize) != 0)
	{
		return 0;
	}

	do
	{
		ERR_clear_error();
		ZeroMemory(buffer, sizeof(buffer));
		sslErrorCode = SSL_read_ex(mClientSession->clientSSLConnection, buffer, BUFFER_SIZE, &receivedDataLength);

		if (sslErrorCode <= 0)
		{
			sslErrorCode = SSL_get_error(mClientSession->clientSSLConnection, sslErrorCode);

			if (sslErrorCode == SSL_ERROR_ZERO_RETURN)
			{
				shutdown(mClientSession->clientSocket, SD_SEND);
				SSL_shutdown(mClientSession->clientSSLConnection);
				return HTTPS_CLIENT_ZERO_RETURN;
			}

			if (sslErrorCode == SSL_ERROR_WANT_READ)
			{
				return HTTPS_CLIENT_NO_AVAILABLE_DATA;
			}

			if (sslErrorCode != SSL_ERROR_NONE)
			{
				return HTTPS_CLIENT_ERROR;
			}
		}
		else
		{
			if (receivedDataLength != 0)
			{
				for (uint32_t i = 0; i < receivedDataLength; ++i)
				{
					content->push_back(buffer[i]);
				}

				recvLenSum += receivedDataLength;
			}
		}

		avaliableDataSize = SSL_pending(mClientSession->clientSSLConnection);
	} while (avaliableDataSize != 0);

	return recvLenSum;
}