#pragma once

struct SOCKETINFO
{
	OVERLAPPED overlapped;
	uint64_t pendingCount;
	bool isbClosed;
	SRWLOCK srwLock;
	SOCKET socket;
	MemoryBlock* sendMemoryBlock;
	MemoryBlock* recvMemoryBlock;
	uint64_t recvbytes;
	uint64_t sendbytes;
	uint64_t sentbytes;
	uint64_t sendPendingBytes;
	ClientSession* session;
	WSABUF sendBuffer;
	WSABUF recvBuffer;
};