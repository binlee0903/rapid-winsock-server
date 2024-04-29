#pragma once

#include "ClientWork.h"

constexpr uint16_t THREAD_COUNT = 2;

class ClientThreadPool final
{
public:
	ClientThreadPool();
	~ClientThreadPool();

	static ClientWork* GetClientWork();

	void QueueWork(ClientWork* clientWork);

private:
	static DWORD __stdcall Run(LPVOID lpParam);

private:
	static std::queue<ClientWork*> mClientWorks;

	bool** mThreadTerminateFlags;
	HANDLE* mThreads;
	DWORD* mThreadIDs;
	uint16_t mThreadCount;
};