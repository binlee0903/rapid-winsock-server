#include "stdafx.h"
#include "ClientThreadPool.h"

ClientThreadPool* ClientThreadPool::mInstance = nullptr;

ClientThreadPool::ClientThreadPool()
	: mThreads(new HANDLE[THREAD_COUNT])
	, mThreadCount(THREAD_COUNT)
	, mEventHandles()
	, mSRWLock(new SRWLOCK())
	, mThreadsRunningState(new bool[THREAD_COUNT])
{
	mEventHandles.reserve(THREAD_COUNT);
}

ClientThreadPool::~ClientThreadPool()
{
	for (uint16_t i = 0; i < mThreadCount; i++)
	{
		CloseHandle(mEventHandles[i]);
		WaitForSingleObject(mThreads[i], INFINITE);
	}

	delete mSRWLock;
	delete[] mThreads;
	delete[] mThreadsRunningState;
}

DWORD __stdcall ClientThreadPool::Run(LPVOID lpParam)
{
	bool* runningState = reinterpret_cast<bool*>(lpParam);
	ClientWork::ERROR_CODE error_code;
	ClientWork* clientWork;
	uint32_t index = 0;
	bool isClose = false;

	while (!isClose)
	{
		index = WaitForMultipleObjects(mInstance->mEventHandles.size(), mInstance->mEventHandles.data(), false, 200);

		if (index == WAIT_TIMEOUT && !mInstance->mClientWorks.empty())
		{
			index = 0;
		}

		switch (index)
		{
		case THREAD_SIGNAL:
			AcquireSRWLockExclusive(mInstance->mSRWLock);
			clientWork = mInstance->GetClientWork();
			ReleaseSRWLockExclusive(mInstance->mSRWLock);

			if (clientWork == nullptr)
			{
				continue;
			}

			*runningState = true;
			error_code = clientWork->Run(nullptr);

			if (error_code == ClientWork::ERROR_CODE::ERROR_CLOSE_BEFORE_WORK_DONE)
			{
				mInstance->QueueWork(clientWork);
				mInstance->Signal(THREAD_SIGNAL);
			}

			*runningState = false;
			// TODO: handle more error
			break;

		case THREAD_CLOSE:
			isClose = true;
			break;
		}
	}

	return 0;
}

void ClientThreadPool::QueueWork(ClientWork* clientWork)
{
	mClientWorks.push(clientWork);
}

void ClientThreadPool::Signal(THREAD_EVENT threadEvent)
{
	if (threadEvent == THREAD_CLOSE)
	{
		for (uint32_t i = 0; i < THREAD_COUNT; i++)
		{
			SetEvent(mEventHandles[threadEvent]);
		}

		return;
	}

	SetEvent(mEventHandles[threadEvent]);
}

void ClientThreadPool::Init()
{
	InitializeSRWLock(mSRWLock);

	mEventHandles.push_back(CreateEvent(nullptr, false, false, nullptr));
	mEventHandles.push_back(CreateEvent(nullptr, false, false, nullptr));

	for (uint16_t i = 0; i < mThreadCount; i++)
	{
		mThreads[i] = CreateThread(nullptr, NULL, &ClientThreadPool::Run, mThreadsRunningState + i, NULL, nullptr);
		mThreadsRunningState[i] = false;
	}
}

bool ClientThreadPool::IsThreadsRunning() const
{
	for (uint16_t i = 0; i < mThreadCount; i++)
	{
		if (mInstance->mThreadsRunningState[i] == true)
		{
			return true;
		}
	}

	return false;
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