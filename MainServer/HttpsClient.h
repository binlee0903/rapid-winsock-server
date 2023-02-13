#pragma once

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <string>
#include <iostream>

#include "IServer.h"
#include "IClient.h"

#include "HttpObject.h"
#include "HttpHelper.h"
//#include "HTMLPageRouter.h"

class HttpObject;

constexpr uint16_t BUFFER_SIZE = 512;

class HttpsClient final : public IClient
{
public:
	HttpsClient() = default;
	~HttpsClient();

	HttpsClient(const HttpsClient& rhs) = delete;
	HttpsClient& operator=(HttpsClient& rhs) = delete;

	virtual HANDLE GetEventHandle() const override;

	virtual int InitializeClient(IServer* server, SRWLOCK* srwLock, SOCKET clientSocket) override;
	virtual bool IsKeepAlive() const override;
	virtual int ProcessRead() override;
	virtual int ProcessWrite() override;
	virtual int ProcessClose() override;
	virtual int ProcessOOB() override;

private:
	void printSocketError();
	int processSSLHandshake();
	uint32_t receiveData(std::string* content); // returns received data length

private:
	/*HTMLPageRouter* mRouter;*/
	HttpHelper* mHttpHelper;
	SSL* mSSL;
	SSL_CTX* mSSLCTX;
	SRWLOCK* mSRWLock;
	IServer* mServer;
	HttpObject* mHttpObject;
	SOCKET mSocket;
	sockaddr_in mClientSockAddr;
	HANDLE mEventHandle;

	bool mbIsKeepAlive;
	bool mbIsSSLConnected;
	wchar_t mClientAddr[INET_ADDRSTRLEN];
};