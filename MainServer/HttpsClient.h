/*****************************************************************//**
 * @file   HttpsClient.h
 * @brief  connected client via server
 * 
 * @author binlee0903
 * @date   February 2023
 *********************************************************************/

#pragma once

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <string>
#include <iostream>
#include <process.h>

#include "IServer.h"

#include "HttpObject.h"
#include "HttpHelper.h"

class HttpObject;

constexpr uint16_t BUFFER_SIZE = 512;
constexpr uint16_t MAX_REQUEST_SIZE = 1000;

using socket_t = decltype(socket(0, 0, 0));

class HttpsClient final
{
public:
	enum STATUS : int8_t
	{
		HTTPS_CLIENT_OK,
		HTTPS_CLIENT_ERROR,
		HTTPS_CLIENT_NO_AVAILABLE_DATA
	};

public:
	HttpsClient(IServer* server, SRWLOCK* srwLock, SSL_CTX* sslCTX, socket_t clientSocket, sockaddr_in clientSockAddr);
	~HttpsClient();

	// delete copy constructor and operator for safe
	HttpsClient(const HttpsClient& rhs) = delete;
	HttpsClient& operator=(HttpsClient& rhs) = delete;

	/**
	 * Increase mRequestCount when http request has arrived
	 */
	void IncreaseRequestCount();

	/**
	 * when client is accepted, this function will run
	 * 
	 * @param client pointer to client
	 */
	static uint32_t Run(void* clientArg);

	/**
	 * check client connection is keep-alive
	 * 
	 * @return return true if connection is keep-alive
	 */
	bool IsKeepAlive();

	/**
	 * process https requests.
	 * but if ssl is not established, do ssl handshake and read bytes
	 * 
	 * @return HTTPS_CLIENT_OK when success, 
	 *  return HTTPS_CLIENT_ERROR when get error,
	 *  return HTTPS_CLIENT_NO_AVAILABLE_DATA when request is empty
	 */
	int8_t ProcessRequest();

	/**
	 * this will call destructor,
	 * and close socket
	 */
	void ProcessClose();
private:
	int8_t writeHttpResponse();
	void printSocketError();
	int processSSLHandshake();
	uint64_t receiveData(std::string* content);

private:
	HttpHelper* mHttpHelper;
	SSL* mSSL;
	SRWLOCK* mSRWLock;
	IServer* mServer;
	HttpObject* mHttpObject;

	socket_t mSocket;
	HANDLE mEventHandle;

	bool mbIsKeepAlive;
	bool mbIsSSLConnected;
	uint16_t mRequestCount;
	std::string mClientAddr;
};
