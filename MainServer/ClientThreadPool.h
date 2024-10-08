#pragma once

#include "ClientWork.h"

constexpr uint32_t THREAD_COUNT = 2;
constexpr uint32_t EVENT_COUNT = 2;

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
	static ClientWork* GetClientWork();

	void QueueWork(ClientWork* clientWork);
	void Signal(THREAD_EVENT threadEvent);
	void Init();

	bool IsThreadsRunning() const;
	bool IsWorkQueueEmpty() const;

private:
	static DWORD __stdcall Run(LPVOID lpParam);

private:
	ClientThreadPool();

private:
	static ClientThreadPool* mInstance;

	std::queue<ClientWork*> mClientWorks;
	uint32_t mThreadRunningCount;

	HANDLE* mThreads;
	HANDLE* mEventHandles;
	SRWLOCK* mSRWLock;
};