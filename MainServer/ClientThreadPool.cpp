#include "stdafx.h"
#include "ClientThreadPool.h"

ClientThreadPool* ClientThreadPool::mInstance = nullptr;
HANDLE ClientThreadPool::mIOCPHandle = nullptr;

ClientThreadPool::ClientThreadPool(MemoryPool* memoryPool)
	: mThreads(new HANDLE[THREAD_COUNT])
	, mSRWLock()
	, mMemoryPool(memoryPool)
{
}

ClientThreadPool::~ClientThreadPool()
{
	for (uint32_t i = 0; i < THREAD_COUNT; i++)
	{
		WaitForSingleObject(mThreads[i], INFINITE);
	}

	delete[] mThreads;
}

ClientThreadPool* ClientThreadPool::GetInstance(MemoryPool* memoryPool)
{
	if (mInstance == nullptr)
	{
		mInstance = new ClientThreadPool(memoryPool);
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
	SOCKETINFO* socketInfo;
	DWORD flags = 0;

	while (true)
	{
		ret = GetQueuedCompletionStatus(mIOCPHandle, &receivedByteCount, &clientSocket, reinterpret_cast<LPOVERLAPPED*>(&socketInfo), WSA_INFINITE);
		httpHelper::InterLockedDecrement(socketInfo);

		if (ret == 0)
		{
			ret = WSAGetLastError();

			if (ret != ERROR_IO_PENDING)
			{
				mInstance->printSocketError();
				assert(false);
			}
		}

		if (receivedByteCount == 0 && socketInfo->pendingCount == 0)
		{
			mInstance->mMemoryPool->DeAllocate(socketInfo->recvMemoryBlock);
			mInstance->mMemoryPool->DeAllocate(socketInfo->sendMemoryBlock);
			ClientWork::CloseConnection(socketInfo);
			continue;
		}

		if (socketInfo->isbClosed == false)
		{
			if (socketInfo->sendPendingBytes != receivedByteCount)
			{
				socketInfo->recvbytes = receivedByteCount;
				std::cout << "Recv" << std::endl;
				ret = ClientWork::ProcessRequest(socketInfo);

				switch (ret)
				{
				case ClientWork::STATUS::HTTPS_CLIENT_ERROR:
					mInstance->mMemoryPool->DeAllocate(socketInfo->recvMemoryBlock);
					mInstance->mMemoryPool->DeAllocate(socketInfo->sendMemoryBlock);
					ClientWork::CloseConnection(socketInfo);
					continue;

				case ClientWork::STATUS::HTTPS_CLIENT_NO_AVAILABLE_DATA:
					ret = WSARecv(socketInfo->socket, &socketInfo->recvBuffer, 1, &receivedByteCount, &flags, &socketInfo->overlapped, nullptr);
					httpHelper::InterLockedIncrement(socketInfo);

					if (ret == 0)
					{
						ret = PostQueuedCompletionStatus(mIOCPHandle, receivedByteCount, socketInfo->socket, &socketInfo->overlapped);
						if (ret == SOCKET_ERROR)
						{
							ret = WSAGetLastError();
						}
					}

					if (ret == SOCKET_ERROR)
					{
						int ret = WSAGetLastError();

						if (ret == WSAECONNRESET)
						{
							socketInfo->isbClosed = true;
						}
					}
					break;

				case ClientWork::STATUS::HTTPS_CLIENT_WANT_READ_DATA:
					ret = WSARecv(socketInfo->socket, &socketInfo->recvBuffer, 1, &receivedByteCount, &flags, &socketInfo->overlapped, nullptr);
					httpHelper::InterLockedIncrement(socketInfo);

					if (ret == 0)
					{
						ret = PostQueuedCompletionStatus(mIOCPHandle, receivedByteCount, socketInfo->socket, &socketInfo->overlapped);
						if (ret == SOCKET_ERROR)
						{
							ret = WSAGetLastError();
						}
					}

					if (ret == SOCKET_ERROR)
					{
						int ret = WSAGetLastError();

						if (ret == WSAECONNRESET)
						{
							socketInfo->isbClosed = true;
						}
					}
					break;

				case ClientWork::STATUS::HTTPS_CLIENT_DISCONNECTED:
					socketInfo->isbClosed = true;
					PostQueuedCompletionStatus(mIOCPHandle, receivedByteCount, clientSocket, (LPOVERLAPPED)socketInfo);
					break;

				case ClientWork::STATUS::HTTPS_CLIENT_BIO_NOT_READY:
					PostQueuedCompletionStatus(mIOCPHandle, receivedByteCount, clientSocket, (LPOVERLAPPED)socketInfo);

					if (ret == SOCKET_ERROR)
					{
						int ret = WSAGetLastError();

						if (ret != ERROR_IO_PENDING)
						{
							mInstance->printSocketError();
							assert(false);
						}
					}
					break;

				case ClientWork::STATUS::HTTPS_CLIENT_OK:
					ret = WSARecv(socketInfo->socket, &socketInfo->recvBuffer, 1, nullptr, &flags, &socketInfo->overlapped, nullptr);
					httpHelper::InterLockedIncrement(socketInfo);

					if (ret == 0)
					{
						ret = PostQueuedCompletionStatus(mIOCPHandle, receivedByteCount, socketInfo->socket, &socketInfo->overlapped);
						if (ret == SOCKET_ERROR)
						{
							ret = WSAGetLastError();
						}
					}

					if (ret == SOCKET_ERROR)
					{
						int ret = WSAGetLastError();

						if (ret != ERROR_IO_PENDING)
						{
							mInstance->printSocketError();
							assert(false);
						}
					}
					break;

				case ClientWork::STATUS::HTTPS_CLIENT_WANT_SEND_DATA:
					if (socketInfo->sendbytes == 0)
					{
						ret = WSARecv(socketInfo->socket, &socketInfo->recvBuffer, 1, nullptr, &flags, &socketInfo->overlapped, nullptr);
						httpHelper::InterLockedIncrement(socketInfo);

						if (ret == 0)
						{
							ret = PostQueuedCompletionStatus(mIOCPHandle, receivedByteCount, socketInfo->socket, &socketInfo->overlapped);
							if (ret == SOCKET_ERROR)
							{
								ret = WSAGetLastError();
							}
						}

						if (ret == SOCKET_ERROR)
						{
							int ret = WSAGetLastError();

							if (ret != ERROR_IO_PENDING)
							{
								mInstance->printSocketError();
								assert(false);
							}
						}
						break;
					}
				}

			}
			else
			{
				socketInfo->sendPendingBytes = 0;

				if (socketInfo->sentbytes < socketInfo->sendbytes)
				{
					ret = ClientWork::SendHttpResponse(socketInfo);

					if (ret == ClientWork::STATUS::HTTPS_CLIENT_OK)
					{
						socketInfo->sendBuffer.len = BLOCK_SIZE;
						socketInfo->sentbytes = 0;
						socketInfo->sendbytes = 0;

						ret = WSARecv(socketInfo->socket, &socketInfo->recvBuffer, 1, &receivedByteCount, &flags, &socketInfo->overlapped, nullptr);
						socketInfo->session->currentOperation = OPERATION::RECEIVE;
						httpHelper::InterLockedIncrement(socketInfo);

						if (ret == 0)
						{
							ret = PostQueuedCompletionStatus(mIOCPHandle, receivedByteCount, socketInfo->socket, &socketInfo->overlapped);
							if (ret == SOCKET_ERROR)
							{
								ret = WSAGetLastError();
							}
						}

						if (ret == SOCKET_ERROR)
						{
							int ret = WSAGetLastError();

							if (ret == WSAECONNRESET)
							{
								socketInfo->isbClosed = true;
							}
						}
					}
				}
				else
				{
					std::cout << "Send" << std::endl;
					ret = WSARecv(socketInfo->socket, &socketInfo->recvBuffer, 1, &receivedByteCount, &flags, &socketInfo->overlapped, nullptr);
					httpHelper::InterLockedIncrement(socketInfo);

					if (ret == 0)
					{
						ret = PostQueuedCompletionStatus(mIOCPHandle, receivedByteCount, socketInfo->socket, &socketInfo->overlapped);
						if (ret == SOCKET_ERROR)
						{
							ret = WSAGetLastError();
						}
					}

					if (ret == SOCKET_ERROR)
					{
						ret = WSAGetLastError();

						if (ret == WSAECONNRESET)
						{
							socketInfo->isbClosed = true;
						}
					}
				}
			}
		}
	}
}

void ClientThreadPool::Init(HANDLE ioCompletionPort)
{
	InitializeSRWLock(&mSRWLock);
	mIOCPHandle = ioCompletionPort;

	for (uint32_t i = 0; i < THREAD_COUNT; i++)
	{
		mThreads[i] = CreateThread(nullptr, NULL, &ClientThreadPool::Run, nullptr, NULL, nullptr);
	}
}

bool ClientThreadPool::isChangeCipherRecord(char* data, int receivedBytes)
{
	uint8_t type = data[0];

	if (type == 0x14)
	{
		return true;
	}

	return false;
}

bool ClientThreadPool::isFullLengthOfRecord(char* data, int receivedBytes)
{
	uint8_t type = data[0];
	uint16_t size = ((uint8_t)data[3] << 8) | (uint8_t)data[4] + 5;

	if (type == 0x16 || type == 0x15 || type == 0x14) // handshake data, alert so bypass it
	{
		return true;
	}

	if (size <= receivedBytes)
	{
		return true;
	}

	return false;
}

void ClientThreadPool::printSocketError()
{
	char* msg = nullptr;
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, WSAGetLastError(),
		LANG_SYSTEM_DEFAULT, reinterpret_cast<char*>(&msg), 0, nullptr);

	if (msg != nullptr)
	{
		std::cout << msg << std::endl;
	}
	LocalFree(msg);
}