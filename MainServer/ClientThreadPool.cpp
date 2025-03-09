#include "stdafx.h"
#include "ClientThreadPool.h"

ClientThreadPool* ClientThreadPool::mInstance = nullptr;

ClientThreadPool::ClientThreadPool()
	: mThreads(new HANDLE[THREAD_COUNT])
	, mSRWLock(new SRWLOCK())
{
}

ClientThreadPool::~ClientThreadPool()
{
	for (uint32_t i = 0; i < THREAD_COUNT; i++)
	{
		WaitForSingleObject(mThreads[i], INFINITE);
	}

	delete mSRWLock;
	delete[] mThreads;
}

ClientThreadPool* ClientThreadPool::GetInstance()
{
	if (mInstance == nullptr)
	{
		mInstance = new ClientThreadPool();
	}

	return mInstance;
}

DWORD __stdcall ClientThreadPool::Run(LPVOID lpParam)
{
	ClientWork::ERROR_CODE error_code;
	uint32_t signalType = 0;
	int32_t ret = 0;
	DWORD receivedByteCount = 0;
	socket_t clientSocket;
	SOCKETINFO* pSocketInfo;

	while (true)
	{
		ret = GetQueuedCompletionStatus(mIOCPHandle, &receivedByteCount, &clientSocket, reinterpret_cast<LPOVERLAPPED*>(&pSocketInfo), TIME_OUT);

		if (ret == 0 || receivedByteCount == 0)
		{
			ClientWork::CloseConnection(pSocketInfo->session);
			delete pSocketInfo;
			continue;
		}

		ClientWork::ProcessRequest(pSocketInfo->session);
	}
}

void ClientThreadPool::Init(HANDLE ioCompletionPort)
{
	InitializeSRWLock(mSRWLock);
	mIOCPHandle = ioCompletionPort;

	for (uint32_t i = 0; i < THREAD_COUNT; i++)
	{
		mThreads[i] = CreateThread(nullptr, NULL, &ClientThreadPool::Run, nullptr, NULL, nullptr);
	}
}