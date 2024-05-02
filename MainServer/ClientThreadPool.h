#pragma once

#include "ClientWork.h"

constexpr uint16_t THREAD_COUNT = 1;

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

	bool IsQueueEmpty() const;

private:
	static DWORD __stdcall Run(LPVOID lpParam);

private:
	ClientThreadPool();

private:
	static ClientThreadPool* mInstance;

	std::queue<ClientWork*> mClientWorks;

	HANDLE* mThreads;
	uint16_t mThreadCount;
	std::vector<HANDLE> mEventHandles;
	SRWLOCK* mSRWLock;
};