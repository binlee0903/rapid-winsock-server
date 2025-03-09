#pragma once

constexpr uint32_t BASIC_SSL_CHUNK_SIZE = 16384;
constexpr uint32_t BUFFER_SIZE = 512;
constexpr uint32_t MAX_REQUEST_SIZE = 1000;
constexpr uint32_t KEEP_ALIVE_TIME = 5;

#include "ClientSessionType.h"
#include "ClientSession.h"

namespace ClientWork
{
	enum ERROR_CODE
	{
		ERROR_NONE,
		ERROR_CLOSE_BEFORE_WORK_DONE
	};

	enum STATUS : int8_t
	{
		HTTPS_CLIENT_OK,
		HTTPS_CLIENT_ERROR,
		HTTPS_CLIENT_NO_AVAILABLE_DATA,
		HTTPS_CLIENT_INVALID_HTTP_HEADER
	};

	bool IsProcessing(ClientSession* session);

	int ProcessSSLHandshake(ClientSession* clientSession);

	/**
	 * process https requests.
	 * but if ssl is not established, do ssl handshake and read bytes
	 *
	 * @param httpObject parsed http request information
	 * 
	 * @return HTTPS_CLIENT_OK when success,
	 *  return HTTPS_CLIENT_ERROR when get error,
	 *  return HTTPS_CLIENT_NO_AVAILABLE_DATA when request is empty
	 */
	ClientWork::STATUS ProcessRequest(ClientSession* session);

	bool IsNull(ClientSession* session);

	void CloseConnection(ClientSession* session);

	STATUS WriteHttpResponse(ClientSession* session);
	uint64_t ReceiveData(ClientSession* session, std::string* content);
};