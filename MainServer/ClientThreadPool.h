#pragma once

#include "ClientWork.h"
#include "ThreadSafeQueue.h"

constexpr uint32_t THREAD_COUNT = 4;
constexpr uint32_t EVENT_COUNT = 2;

constexpr uint16_t MAX_IOCP_BUFFER_SIZE = 8192;
constexpr uint16_t TIME_OUT = 1000;

struct SOCKETINFO
{
	OVERLAPPED overlapped;
	SOCKET socket;
	char buffer[MAX_IOCP_BUFFER_SIZE + 1];
	uint32_t recvbytes;
	uint32_t sendbytes;
	ClientSession* session;
	WSABUF wsabuf;
};

class ClientThreadPool final
{
public:
	enum THREAD_EVENT
	{
		THREAD_SIGNAL,
		THREAD_CLOSE
	};

public:
	~ClientThreadPool();

	static ClientThreadPool* GetInstance();

	void Init(HANDLE ioCompletionPort);

private:
	static DWORD __stdcall Run(LPVOID lpParam);

private:
	ClientThreadPool();

private:
	static ClientThreadPool* mInstance;
	static HANDLE mIOCPHandle;

	HANDLE* mThreads;
	SRWLOCK* mSRWLock;
};