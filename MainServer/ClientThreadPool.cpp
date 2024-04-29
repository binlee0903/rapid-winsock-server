#include "ClientThreadPool.h"

std::queue<ClientWork*> ClientThreadPool::mClientWorks;

ClientThreadPool::ClientThreadPool()
	: mThreadTerminateFlags(new bool* [THREAD_COUNT])
	, mThreads(new HANDLE[THREAD_COUNT])
	, mThreadIDs(new DWORD[THREAD_COUNT])
	, mThreadCount(THREAD_COUNT)
{
	for (uint16_t i = 0; i < mThreadCount; i++)
	{
		mThreadTerminateFlags[i] = new bool(false);
		mThreads[i] = CreateThread(nullptr, NULL, &ClientThreadPool::Run, mThreadTerminateFlags[i], NULL, &mThreadIDs[i]);
	}
}

ClientThreadPool::~ClientThreadPool()
{
	for (uint16_t i = 0; i < mThreadCount; i++)
	{
		*(mThreadTerminateFlags[i]) = true;
	}

	delete[] mThreadTerminateFlags;
	delete[] mThreads;
	delete[] mThreadIDs;
}

DWORD __stdcall ClientThreadPool::Run(LPVOID lpParam)
{
	ClientWork::ERROR_CODE error_code;
	bool* terminateFlag = reinterpret_cast<bool*>(lpParam);
	ClientWork* clientWork;

	while (*terminateFlag == false)
	{
		clientWork = ClientThreadPool::GetClientWork();

		if (clientWork != nullptr)
		{
			error_code = clientWork->Run(nullptr);
		}
		// TODO: handle error
	}

	delete terminateFlag;

	return 0;
}

void ClientThreadPool::QueueWork(ClientWork* clientWork)
{
	mClientWorks.push(clientWork);
}

ClientWork* ClientThreadPool::GetClientWork()
{
	if (mClientWorks.empty() == true)
	{
		return nullptr;
	}

	ClientWork* clientWork = mClientWorks.front();
	mClientWorks.pop();

	return clientWork;
}