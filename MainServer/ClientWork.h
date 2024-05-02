#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdint>
#include <assert.h>
#include <queue>

#include <WinSock2.h>
#include <ws2tcpip.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include "HttpHelper.h"
#include "ClientSession.h"

using socket_t = decltype(socket(0, 0, 0));

constexpr uint32_t BASIC_SSL_CHUNK_SIZE = 16384;
constexpr uint32_t BUFFER_SIZE = 512;
constexpr uint32_t MAX_REQUEST_SIZE = 1000;
constexpr uint32_t KEEP_ALIVE_TIME = 5;

class ClientWork final
{
public:
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

	ClientWork(ClientSession* clientSession, ClientSessionType sessionType);
	~ClientWork();

	// delete copy constructor and operator for safe
	ClientWork(const ClientWork& rhs) = delete;
	ClientWork& operator=(ClientWork& rhs) = delete;

	/**
	 * when client is accepted, this function will run
	 *
	 * @param client pointer to client
	 */
	ERROR_CODE Run(void* clientArg);

	bool IsProcessing() const;

	/**
	 * process https requests.
	 * but if ssl is not established, do ssl handshake and read bytes
	 *
	 * @return HTTPS_CLIENT_OK when success,
	 *  return HTTPS_CLIENT_ERROR when get error,
	 *  return HTTPS_CLIENT_NO_AVAILABLE_DATA when request is empty
	 */
	int8_t ProcessRequest();
private:
	void closeConnection();
	void finishWork() const;

	int8_t writeHttpResponse();
	uint64_t receiveData(std::string* content);

private:
	static SRWLOCK mSRWLock;

	HttpHelper* mHttpHelper;
	HttpObject* mHttpObject;

	ClientSession* mClientSession;
	ClientSessionType mClientSessionType;
};