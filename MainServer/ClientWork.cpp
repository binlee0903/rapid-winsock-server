#include "stdafx.h"
#include "ClientWork.h"

bool ClientWork::IsProcessing(ClientSession* session)
{
	return session->processingCount > 0;
}

int ClientWork::ProcessSSLHandshake(ClientSession* clientSession)
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
			//mLogger->error("ProcessSSLHandshake() : ssl accept failed, error message : {}", buffer);
			return -1;
		}
	}

	return 0;
}

ClientWork::STATUS ClientWork::ProcessRequest(ClientSession* session)
{
	int ret = 0;

	if (session->bIsSSLConnected != true)
	{
		ret = ProcessSSLHandshake(session);
		assert(ret == 0);
		return STATUS::HTTPS_CLIENT_OK;
	}

	std::string buffer;
	buffer.reserve(BUFFER_SIZE * 2);

	uint64_t receivedDataLength = ReceiveData(session , &buffer);

	if (receivedDataLength == 0)
	{
		return HTTPS_CLIENT_NO_AVAILABLE_DATA;
	}

	if (httpHelper::PrepareResponse(session->httpObject, buffer) == false)
	{
		return HTTPS_CLIENT_INVALID_HTTP_HEADER;
	}

	return WriteHttpResponse(session);
}

bool ClientWork::IsNull(ClientSession* session)
{
	return session->httpObject == nullptr;
}

ClientWork::STATUS ClientWork::WriteHttpResponse(ClientSession* session)
{
	std::vector<int8_t> response;
	response.reserve(BUFFER_SIZE);

	httpHelper::CreateHttpResponse(session->httpObject, response);

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
			sslErrorCode = SSL_write_ex(session->clientSSLConnection, &response[wroteSize], BASIC_SSL_CHUNK_SIZE, &wroteSizeToSSL);
		}
		else
		{
			sslErrorCode = SSL_write_ex(session->clientSSLConnection, &response[wroteSize], responseSize - BASIC_SSL_CHUNK_SIZE * (chunkCount - 1), &wroteSizeToSSL);
		}

		sslErrorCode = SSL_get_error(session->clientSSLConnection, sslErrorCode);

		assert(sslErrorCode != SSL_ERROR_NONE);

		wroteSize += wroteSizeToSSL;
	}

	return HTTPS_CLIENT_OK;
}

void ClientWork::CloseConnection(ClientSession* session)
{
	assert(session->processingCount == 0);
	session->bIsDisconnected = true;
	delete session->ip;
	delete session->sessionTimer;
	SSL_shutdown(session->clientSSLConnection);
	WSACloseEvent(session->eventHandle);
	SSL_free(session->clientSSLConnection);
	shutdown(session->clientSocket, SD_BOTH);
	closesocket(session->clientSocket);
	delete session;
}

uint64_t ClientWork::ReceiveData(ClientSession* session, std::string* content)
{
	uint8_t buffer[BUFFER_SIZE];

	u_long avaliableDataSize = 0;
	int32_t sslErrorCode = 0;
	size_t receivedDataLength = 0;
	uint64_t recvLenSum = 0;

	if (ioctlsocket(session->clientSocket, FIONREAD, &avaliableDataSize) != 0)
	{
		return 0;
	}

	do
	{
		ERR_clear_error();
		ZeroMemory(buffer, sizeof(buffer));
		sslErrorCode = SSL_read_ex(session->clientSSLConnection, buffer, BUFFER_SIZE, &receivedDataLength);
		sslErrorCode = SSL_get_error(session->clientSSLConnection, sslErrorCode);

		if (receivedDataLength != 0)
		{
			for (uint32_t i = 0; i < receivedDataLength; ++i)
			{
				content->push_back(buffer[i]);
			}

			recvLenSum += receivedDataLength;
		}

		avaliableDataSize = SSL_pending(session->clientSSLConnection);
	} while (avaliableDataSize != 0);

	return recvLenSum;
}