#pragma once

struct SOCKETINFO
{
	OVERLAPPED overlapped;

	OPERATION operation;
	uint64_t pendingCount;
	bool isbClosed;
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