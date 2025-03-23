#pragma once

constexpr uint32_t BASIC_SSL_CHUNK_SIZE = 16384;
constexpr uint32_t BUFFER_SIZE = 512;
constexpr uint32_t MAX_REQUEST_SIZE = 1000;
constexpr uint32_t KEEP_ALIVE_TIME = 5;

#include "ClientSessionType.h"
#include "ClientSession.h"

class ClientWork final
{
public:
	enum ERROR_CODE
	{
		ERROR_NONE,
		ERROR_CLOSE_BEFORE_WORK_DONE,
		ERROR_SSL,
		ERROR_ZERO_RETURN,
	};

	enum STATUS : int8_t
	{
		HTTPS_CLIENT_OK,
		HTTPS_CLIENT_ERROR,
		HTTPS_CLIENT_NO_AVAILABLE_DATA,
		HTTPS_CLIENT_INVALID_HTTP_HEADER,
		HTTPS_CLIENT_ZERO_RETURN,
	};

	ClientWork(ClientSession* clientSession, ClientSessionType sessionType);
	~ClientWork();

	// delete copy constructor and operator for safe
	ClientWork(const ClientWork& rhs) = delete;
	ClientWork& operator=(ClientWork& rhs) = delete;

	void FinishWork() const;
	/**
	 * when client is accepted, this function will run
	 *
	 * @param client pointer to client
	 */
	ERROR_CODE Run(void* clientArg);

	bool IsProcessing() const;
	bool IsThisWorkProcessing() const;
	ClientSession* GetClientSession() const;

	/**
	 * process https requests.
	 * but if ssl is not established, do ssl handshake and read bytes
	 *
	 * @return HTTPS_CLIENT_OK when success,
	 *  return HTTPS_CLIENT_ERROR when get error,
	 *  return HTTPS_CLIENT_NO_AVAILABLE_DATA when request is empty
	 */
	ClientWork::STATUS ProcessRequest();

	ClientSessionType GetType() const;

	bool IsNull();
private:
	void closeConnection();

	STATUS writeHttpResponse();
	uint64_t receiveData(std::string* content);

private:
	static SRWLOCK mSRWLock;

	HttpObject* mHttpObject;

	ClientSession* mClientSession;
	ClientSessionType mClientSessionType;

	bool misbProcessing;
};