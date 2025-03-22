﻿#include "stdafx.h"
#include "ClientWork.h"

SRWLOCK ClientWork::mSRWLock = { 0 };

ClientWork::ClientWork(ClientSession* clientSession, ClientSessionType sessionType)
: mHttpObject(new HttpObject())
, mClientSession(clientSession)
, mClientSessionType(sessionType)
{
	if (sessionType == ClientSessionType::SESSION_READ)
	{
		mClientSession->processingCount += 1;
	}
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

	switch (mClientSessionType)
	{
		case ClientSessionType::SESSION_READ:
			status = ProcessRequest();

			switch (status)
			{
			case ClientWork::HTTPS_CLIENT_OK:
				break;
			case ClientWork::HTTPS_CLIENT_ERROR:
				return ERROR_SSL;
			case ClientWork::HTTPS_CLIENT_NO_AVAILABLE_DATA:
				break;
			case ClientWork::HTTPS_CLIENT_INVALID_HTTP_HEADER:
				break;
			default:
				break;
			}

			mClientSession->processingCount -= 1;
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
	return mClientSession->processingCount > 0;
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

	if (httpHelper::PrepareResponse(mHttpObject, buffer) == false)
	{
		return HTTPS_CLIENT_INVALID_HTTP_HEADER;
	}

	return writeHttpResponse();
}

bool ClientWork::IsNull()
{
	return mHttpObject == nullptr;
}

ClientWork::STATUS ClientWork::writeHttpResponse()
{
	std::vector<int8_t> response;
	response.reserve(BUFFER_SIZE);

	httpHelper::CreateHttpResponse(mHttpObject, response);

	int32_t sslErrorCode = 0;
	size_t responseSize = response.size();
	size_t chunkCount = (responseSize / BASIC_SSL_CHUNK_SIZE) + 1;
	size_t wroteSize = 0;
	size_t wroteSizeToSSL = 0;
	u_long availSize = 0;

	char buffer[512];
	ZeroMemory(buffer, sizeof(buffer));

	while (wroteSize != responseSize)
	{
		ERR_clear_error();

		if (responseSize - wroteSize > BASIC_SSL_CHUNK_SIZE)
		{
			sslErrorCode = SSL_write_ex(mClientSession->clientSSLConnection, &response[wroteSize], BASIC_SSL_CHUNK_SIZE, &wroteSizeToSSL);
		}
		else
		{
			sslErrorCode = SSL_write_ex(mClientSession->clientSSLConnection, &response[wroteSize], responseSize - BASIC_SSL_CHUNK_SIZE * (chunkCount - 1), &wroteSizeToSSL);
		}

		sslErrorCode = SSL_get_error(mClientSession->clientSSLConnection, sslErrorCode);

		if (sslErrorCode == SSL_ERROR_SYSCALL || sslErrorCode == SSL_ERROR_ZERO_RETURN)
		{
			AcquireSRWLockExclusive(&mSRWLock);
			std::cout << "ssl write failed, error Code : " << sslErrorCode << std::endl;
			ERR_error_string_n(sslErrorCode, buffer, 512);
			std::cout << buffer << std::endl;
			ReleaseSRWLockExclusive(&mSRWLock);
			return HTTPS_CLIENT_ERROR;
		}

		if (sslErrorCode == SSL_ERROR_NONE)
		{
			wroteSize += wroteSizeToSSL;
		}
	}

	return HTTPS_CLIENT_OK;
}

void ClientWork::closeConnection()
{
	assert(mClientSession->processingCount == 0);
	mClientSession->bIsDisconnected = true;
	delete mClientSession->ip;
	delete mClientSession->sessionTimer;
	SSL_shutdown(mClientSession->clientSSLConnection);
	WSACloseEvent(mClientSession->eventHandle);
	SSL_free(mClientSession->clientSSLConnection);
	shutdown(mClientSession->clientSocket, SD_BOTH);
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

		if (receivedDataLength != 0)
		{
			for (uint32_t i = 0; i < receivedDataLength; ++i)
			{
				content->push_back(buffer[i]);
			}

			recvLenSum += receivedDataLength;
		}

		avaliableDataSize = SSL_pending(mClientSession->clientSSLConnection);
	} while (avaliableDataSize != 0);

	return recvLenSum;
}