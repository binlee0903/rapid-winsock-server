#include "stdafx.h"
#include "ClientThreadPool.h"

ClientThreadPool* ClientThreadPool::mInstance = nullptr;

ClientThreadPool::ClientThreadPool()
	: mThreads(new HANDLE[THREAD_COUNT])
	, mEventHandles(new HANDLE[EVENT_COUNT])
	, mSRWLock(new SRWLOCK())
	, mThreadRunningCount(0)
{
}

ClientThreadPool::~ClientThreadPool()
{
	for (uint16_t i = 0; i < EVENT_COUNT; i++)
	{
		CloseHandle(mEventHandles[i]);
	}

	for (uint16_t i = 0; i < THREAD_COUNT; i++)
	{
		WaitForSingleObject(mThreads[i], INFINITE);
	}

	delete mSRWLock;
	delete[] mEventHandles;
	delete[] mThreads;
}

DWORD __stdcall ClientThreadPool::Run(LPVOID lpParam)
{
	ClientWork::ERROR_CODE error_code;
	ClientWork* clientWork;
	uint32_t signalType = 0;

	while (true)
	{
		signalType = WaitForMultipleObjects(EVENT_COUNT, mInstance->mEventHandles, false, INFINITE);

		switch (signalType)
		{
		case THREAD_SIGNAL:
			AcquireSRWLockExclusive(mInstance->mSRWLock);
			clientWork = mInstance->GetClientWork();
			ReleaseSRWLockExclusive(mInstance->mSRWLock);

			if (clientWork == nullptr)
			{
				continue;
			}

			mInstance->mThreadRunningCount += 1;
			error_code = clientWork->Run(nullptr);

			if (error_code == ClientWork::ERROR_CODE::ERROR_CLOSE_BEFORE_WORK_DONE)
			{
				mInstance->QueueWork(clientWork);
				mInstance->Signal(THREAD_SIGNAL);
			}
			else
			{
				clientWork->FinishWork();
			}
			// TODO: handle more error
			mInstance->mThreadRunningCount -= 1;
			break;

		case THREAD_CLOSE:
			goto end;
		}

		clientWork = nullptr;
	}

end:
	return 0;
}

void ClientThreadPool::QueueWork(ClientWork* clientWork)
{
	mClientWorks.push(clientWork);
}

void ClientThreadPool::Signal(THREAD_EVENT threadEvent)
{
	for (uint32_t i = 0; i < THREAD_COUNT; i++)
	{
		SetEvent(mEventHandles[threadEvent]);
	}
}

void ClientThreadPool::Init()
{
	InitializeSRWLock(mSRWLock);

	for (uint32_t i = 0; i < THREAD_COUNT; i++)
	{
		mEventHandles[i] = CreateEvent(nullptr, false, false, nullptr);
	}

	for (uint16_t i = 0; i < THREAD_COUNT; i++)
	{
		mThreads[i] = CreateThread(nullptr, NULL, &ClientThreadPool::Run, nullptr, NULL, nullptr);
	}
}

bool ClientThreadPool::IsThreadsRunning() const
{
	return mThreadRunningCount > 0;
}

bool ClientThreadPool::IsWorkQueueEmpty() const
{
	return mClientWorks.empty();
}

ClientThreadPool* ClientThreadPool::GetInstance()
{
	if (mInstance == nullptr)
	{
		mInstance = new ClientThreadPool();
	}

	return mInstance;
}

ClientWork* ClientThreadPool::GetClientWork()
{
	if (mInstance->mClientWorks.empty() == true)
	{
		return nullptr;
	}

	ClientWork* clientWork = mInstance->mClientWorks.front();
	mInstance->mClientWorks.pop();

	return clientWork;
}