#include "stdafx.h"
#include "ClientThreadPool.h"

ClientThreadPool* ClientThreadPool::mInstance = nullptr;

ClientThreadPool::ClientThreadPool()
	: mThreads(new HANDLE[THREAD_COUNT])
	, mEventHandles(new HANDLE[EVENT_COUNT])
	, mSRWLock()
	, mThreadRunningCount(0)
{
	InitializeSRWLock(&mSRWLock);
}

ClientThreadPool::~ClientThreadPool()
{
	for (uint32_t i = 0; i < EVENT_COUNT; i++)
	{
		CloseHandle(mEventHandles[i]);
	}

	for (uint32_t i = 0; i < THREAD_COUNT; i++)
	{
		WaitForSingleObject(mThreads[i], INFINITE);
	}

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
			AcquireSRWLockExclusive(&mInstance->mSRWLock);
			clientWork = mInstance->GetClientWork();
			ReleaseSRWLockExclusive(&mInstance->mSRWLock);

			if (clientWork == nullptr)
			{
				continue;
			}

			AcquireSRWLockExclusive(&mInstance->mSRWLock);
			mInstance->mThreadRunningCount += 1;
			ReleaseSRWLockExclusive(&mInstance->mSRWLock);

			error_code = clientWork->Run(nullptr);

			if (error_code == ClientWork::ERROR_CODE::ERROR_CLOSE_BEFORE_WORK_DONE)
			{
				AcquireSRWLockExclusive(&mInstance->mSRWLock);
				mInstance->QueueWork(clientWork);
				mInstance->Signal(THREAD_SIGNAL);
				ReleaseSRWLockExclusive(&mInstance->mSRWLock);
			}
			else if (error_code == ClientWork::ERROR_CODE::ERROR_ZERO_RETURN)
			{
				AcquireSRWLockExclusive(&mInstance->mSRWLock);
				mInstance->cancelWorks(clientWork);
				clientWork->FinishWork();
				ReleaseSRWLockExclusive(&mInstance->mSRWLock);
			}
			else
			{
				clientWork->FinishWork();
			}

			AcquireSRWLockExclusive(&mInstance->mSRWLock);
			mInstance->mThreadRunningCount -= 1;
			ReleaseSRWLockExclusive(&mInstance->mSRWLock);

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
	mClientWorks.Push(clientWork);
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
	for (uint32_t i = 0; i < THREAD_COUNT; i++)
	{
		mEventHandles[i] = CreateEvent(nullptr, false, false, nullptr);
	}

	for (uint32_t i = 0; i < THREAD_COUNT; i++)
	{
		mThreads[i] = CreateThread(nullptr, NULL, &ClientThreadPool::Run, nullptr, NULL, nullptr);
	}
}

bool ClientThreadPool::IsThreadsRunning() const
{
	return mThreadRunningCount > 0;
}

bool ClientThreadPool::IsWorkQueueEmpty()
{
	return mClientWorks.IsQueueEmpty();
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
	ClientWork* clientWork = mInstance->mClientWorks.Pop();

	return clientWork;
}

void ClientThreadPool::cancelWorks(ClientWork* clientWork)
{
	ClientWork* queuedWork = nullptr;
	int size = mInstance->mClientWorks.size();

	for (int i = 0; i < size; i++)
	{
		queuedWork = mClientWorks.Pop();

		if (queuedWork->GetType() == ClientSessionType::SESSION_CLOSE)
		{
			mClientWorks.Push(queuedWork);
		}
		else if (queuedWork->GetClientSession()->sessionID != clientWork->GetClientSession()->sessionID)
		{
			mClientWorks.Push(queuedWork);
		}
		else
		{
			if (queuedWork->IsThisWorkProcessing() != true)
			{
				queuedWork->GetClientSession()->processingCount--;
				queuedWork->FinishWork();
			}
		}
	}
}