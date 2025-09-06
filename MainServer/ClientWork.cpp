#include "stdafx.h"
#include "ClientWork.h"

bool ClientWork::IsProcessing(SOCKETINFO* socketInfo)
{
	return socketInfo->session->processingCount > 0;
}

int ClientWork::ProcessSSLHandshake(SOCKETINFO* socketInfo)
{
	int retCode = 0;
	int errorCode = 0;
	int pending = 0;
	uint32_t retryCount = 0;
	char buffer[BUFFER_SIZE];
	ZeroMemory(buffer, BUFFER_SIZE);

	BIO_write(socketInfo->session->clientSSLReadBIO, socketInfo->recvBuffer.buf, socketInfo->recvbytes);

	ERR_clear_error();
	pending = BIO_pending(socketInfo->session->clientSSLReadBIO);

	retCode = SSL_do_handshake(socketInfo->session->clientSSLConnection);

	pending = BIO_pending(socketInfo->session->clientSSLWriteBIO);

	if (retCode <= 0)
	{
		errorCode = SSL_get_error(socketInfo->session->clientSSLConnection, retCode);

		if (errorCode != SSL_ERROR_NONE && errorCode != SSL_ERROR_WANT_READ)
		{
			ERR_error_string_n(ERR_get_error(), buffer, 512);
			//mLogger->error("ProcessSSLHandshake() : ssl accept failed, error message : {}", buffer);
			return -1;
		}
	}
	else
	{
		socketInfo->session->bIsSSLConnected = true;
		return 0;
	}
	
	if (pending > 0)
	{
		int ret = 0;
		DWORD flags = 0;
		DWORD recvBytes = 0;
		int count = BIO_read(socketInfo->session->clientSSLWriteBIO, socketInfo->sendBuffer.buf, pending);
		socketInfo->sendBuffer.len = count;
		socketInfo->sendPendingBytes = pending;
		ret = WSASend(socketInfo->socket, &socketInfo->sendBuffer, 1, 0, 0, &socketInfo->overlapped, nullptr);
		httpHelper::InterLockedIncrement(socketInfo);

		if (ret == SOCKET_ERROR)
		{
			assert(WSAGetLastError() != WSA_IO_PENDING);
		}

		if (ret == 0)
		{
			ret = WSARecv(socketInfo->socket, &socketInfo->recvBuffer, 1, &recvBytes, &flags, &socketInfo->overlapped, nullptr);

			if (ret == 0)
			{
				BIO_write(socketInfo->session->clientSSLReadBIO, socketInfo->recvBuffer.buf, recvBytes);

				ERR_clear_error();
				pending = BIO_pending(socketInfo->session->clientSSLReadBIO);

				retCode = SSL_do_handshake(socketInfo->session->clientSSLConnection);

				if (retCode <= 0)
				{
					errorCode = SSL_get_error(socketInfo->session->clientSSLConnection, retCode);

					if (errorCode != SSL_ERROR_NONE && errorCode != SSL_ERROR_WANT_READ)
					{
						ERR_error_string_n(ERR_get_error(), buffer, 512);
						//mLogger->error("ProcessSSLHandshake() : ssl accept failed, error message : {}", buffer);
						return -1;
					}
				}
				else
				{
					socketInfo->session->bIsSSLConnected = true;
					return 0;
				}
			}
		}

		return SSL_ERROR_WANT_READ;
	}

	if (errorCode == SSL_ERROR_WANT_READ)
	{
		return SSL_ERROR_WANT_READ;
	}
	else
	{
		socketInfo->session->bIsSSLConnected = true;
		return 0;
	}
}

ClientWork::STATUS ClientWork::ProcessRequest(SOCKETINFO* socketInfo)
{
	int ret = 0;
	DWORD flags = 0;

	if (socketInfo->session->bIsSSLConnected != true)
	{
		ret = ProcessSSLHandshake(socketInfo);

		switch (ret)
		{
		case SSL_ERROR_WANT_READ:
			return STATUS::HTTPS_CLIENT_SSL_HANDSHAKE;

		case 0:
			std::cout << "SSL connection" << std::endl;
			return STATUS::HTTPS_CLIENT_WANT_READ_DATA;

		default:
			return STATUS::HTTPS_CLIENT_ERROR;
		}
	}

	std::string buffer;
	buffer.reserve(BUFFER_SIZE * 2);

	int errorCode = ReceiveData(socketInfo, &buffer);

	if (errorCode == SSL_ERROR_WANT_READ)
	{
		return HTTPS_CLIENT_NO_AVAILABLE_DATA;
	}

	if (errorCode == SSL_ERROR_ZERO_RETURN)
	{
		return HTTPS_CLIENT_DISCONNECTED;
	}

	if (errorCode == STATUS::HTTPS_CLIENT_BIO_NOT_READY)
	{
		return STATUS::HTTPS_CLIENT_BIO_NOT_READY;
	}

	if (httpHelper::PrepareResponse(socketInfo->session->httpObject, buffer) == false)
	{
		return HTTPS_CLIENT_INVALID_HTTP_HEADER;
	}

	httpHelper::WriteHttpsResponseToSSL(socketInfo);

	return SendHttpResponse(socketInfo);
}

bool ClientWork::IsNull(SOCKETINFO* socketInfo)
{
	return socketInfo->session->httpObject == nullptr;
}

ClientWork::STATUS ClientWork::SendHttpResponse(SOCKETINFO* socketInfo)
{
	int ret = 0;
	int pendingBytes = BIO_pending(socketInfo->session->clientSSLWriteBIO);
	int writtenSize = 0;

	if (pendingBytes > BLOCK_SIZE)
	{
		pendingBytes = BLOCK_SIZE;
	}

	if (pendingBytes > 0)
	{
		socketInfo->sentbytes += pendingBytes;
		BIO_read(socketInfo->session->clientSSLWriteBIO, socketInfo->sendBuffer.buf, pendingBytes);
	}

	socketInfo->sendBuffer.len = pendingBytes;
	socketInfo->session->currentOperation = OPERATION::SEND;
	ret = WSASend(socketInfo->socket, &socketInfo->sendBuffer, 1, 0, 0, &socketInfo->overlapped, nullptr); //
	httpHelper::InterLockedIncrement(socketInfo);
	socketInfo->sendPendingBytes = pendingBytes;

	if (ret == SOCKET_ERROR)
	{
		ret = WSAGetLastError();

		if (ret != WSA_IO_PENDING)
		{
			httpHelper::InterLockedDecrement(socketInfo);
			socketInfo->isbClosed = true;
			return STATUS::HTTPS_CLIENT_ERROR;
		}
	}

	if (socketInfo->sendbytes - socketInfo->sentbytes > 0)
	{
		return STATUS::HTTPS_CLIENT_WANT_SEND_DATA;
	}

	/*if (socketInfo->recvbytes > 0)
	{
		return STATUS::HTTPS_CLIENT_WANT_READ_DATA;
	}*/

	return HTTPS_CLIENT_OK;
}

void ClientWork::CloseConnection(SOCKETINFO* socketInfo)
{
	socketInfo->session->bIsDisconnected = true;
	delete socketInfo->session->ip;
	delete socketInfo->session->sessionTimer;
	delete socketInfo->session->httpObject;
	SSL_shutdown(socketInfo->session->clientSSLConnection);
	SSL_free(socketInfo->session->clientSSLConnection);
	shutdown(socketInfo->session->clientSocket, SD_BOTH);
	closesocket(socketInfo->session->clientSocket);
	socketInfo->recvMemoryBlock = nullptr;
	socketInfo->sendMemoryBlock = nullptr;
	delete socketInfo->session;
	delete socketInfo;
}

uint64_t ClientWork::ReceiveData(SOCKETINFO* socketInfo, std::string* content)
{
	uint8_t buffer[BUFFER_SIZE];
	char errorBuffer[BUFFER_SIZE];

	uint64_t recordSize = 0;
	u_long avaliableDataSize = 0;
	int32_t sslErrorCode = 0;
	size_t receivedDataLength = 0;
	uint64_t recvLenSum = 0;
	int errorCode = 0;

	if (IsApplicationRecord(socketInfo) != true)
	{
		errorCode = 1;
	}

	recordSize = GetRecordSize(socketInfo);
	avaliableDataSize = BIO_pending(socketInfo->session->clientSSLReadBIO);

	if (avaliableDataSize == 0)
	{
		errorCode = BIO_write(socketInfo->session->clientSSLReadBIO, socketInfo->recvBuffer.buf, socketInfo->recvbytes);

		if (errorCode == 0)
		{
			errorCode = SSL_get_error(socketInfo->session->clientSSLConnection, errorCode);

			ERR_error_string_n(errorCode, errorBuffer, BUFFER_SIZE);
			std::cout << errorBuffer << std::endl;
		}
	}

	do
	{
		ERR_clear_error();
		ZeroMemory(buffer, sizeof(buffer));
		sslErrorCode = SSL_read_ex(socketInfo->session->clientSSLConnection, buffer, BUFFER_SIZE, &receivedDataLength);

		if (sslErrorCode <= 0)
		{
			sslErrorCode = SSL_get_error(socketInfo->session->clientSSLConnection, sslErrorCode);

			switch (sslErrorCode)
			{
			case SSL_ERROR_WANT_READ:
				return SSL_ERROR_WANT_READ;

			case SSL_ERROR_ZERO_RETURN:
				return SSL_ERROR_ZERO_RETURN;

			case SSL_ERROR_SYSCALL:
				ERR_error_string_n(sslErrorCode, errorBuffer, BUFFER_SIZE);
				std::cout << errorBuffer << std::endl;
				return SSL_ERROR_WANT_READ;

			case SSL_ERROR_SSL:
				ERR_error_string_n(sslErrorCode, errorBuffer, BUFFER_SIZE);
				std::cout << errorBuffer << std::endl;
				return SSL_ERROR_WANT_READ;
			}
		}

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
				AcquireSRWLockExclusive(&mSRWLock);
				std::cout << "ssl read failed, error Code : " << sslErrorCode << std::endl;
				ReleaseSRWLockExclusive(&mSRWLock);
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

		avaliableDataSize = BIO_pending(socketInfo->session->clientSSLReadBIO);
	} while (avaliableDataSize != 0);

	if (avaliableDataSize != 0)
	{
		socketInfo->recvbytes = 0;
	}

	return recvLenSum;
}

uint64_t ClientWork::GetRecordSize(const SOCKETINFO* socketInfo)
{
	// + 5 = Record Header Size
	uint16_t size = ((uint8_t)socketInfo->recvBuffer.buf[3] << 8) | (uint8_t)socketInfo->recvBuffer.buf[4] + 5;

	return size;
}

void ClientWork::RemoveData(const SOCKETINFO* socketInfo, uint64_t recordSize)
{
	std::memmove(socketInfo->recvBuffer.buf, &socketInfo->recvBuffer.buf[recordSize], socketInfo->recvBuffer.len - recordSize);
}

bool ClientWork::IsChangeCipherRecord(const SOCKETINFO* socketInfo)
{
	uint8_t type = socketInfo->recvBuffer.buf[0];

	return type == 0x14;
}

bool ClientWork::IsApplicationRecord(const SOCKETINFO* socketInfo)
{
	uint8_t type = socketInfo->recvBuffer.buf[0];

	return type == 0x17;
}
