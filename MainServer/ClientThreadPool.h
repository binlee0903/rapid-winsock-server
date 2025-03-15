#pragma once

#include "ClientWork.h"
#include "ThreadSafeQueue.h"

constexpr uint32_t THREAD_COUNT = 1;
constexpr uint32_t EVENT_COUNT = 2;

constexpr uint16_t MAX_IOCP_BUFFER_SIZE = 8192;
constexpr uint16_t TIME_OUT = 1000;

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

	static ClientThreadPool* GetInstance(MemoryPool* memoryPool);

	void Init(HANDLE ioCompletionPort);

private:
	static DWORD __stdcall Run(LPVOID lpParam);
	bool isChangeCipherRecord(char* data, int receivedBytes);
	bool isFullLengthOfRecord(char* data, int receivedBytes);
	void printSocketError();
private:
	ClientThreadPool(MemoryPool* memoryPool);

private:
	static ClientThreadPool* mInstance;
	static HANDLE mIOCPHandle;

	MemoryPool* mMemoryPool;
	HANDLE* mThreads;
	SRWLOCK mSRWLock;
};