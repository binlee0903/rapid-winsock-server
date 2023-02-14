#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>

#include <cstdint>
#include <string>

class IServer;

class IClient
{
public:
	IClient() = default;

	IClient(const IClient& rhs) = delete;
	IClient& operator=(IClient& rhs) = delete;

	virtual void IncreaseRequestCount() = 0;
	virtual HANDLE GetEventHandle() const = 0;
	virtual uint16_t GetRequestCount() const = 0;
	virtual std::string& GetClientIP() = 0;

	virtual int InitializeClient(IServer* server, SRWLOCK* srwLock, SOCKET clientSocket) = 0;
	virtual bool IsKeepAlive() = 0;
	virtual int ProcessRead() = 0;
	virtual int ProcessWrite() = 0;
	virtual int ProcessClose() = 0;
	virtual int ProcessOOB() = 0;
};

