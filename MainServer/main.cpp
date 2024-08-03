#pragma once

#include "stdafx.h"
#include "HttpsServer.h"
#include "HttpServer.h"

int YourAllocHook(int allocType, void* userData, size_t size,
	int blockType, long requestNumber,
	const unsigned char* filename, int lineNumber)
{
	if (size == 4096) {
		size++;
	}

	return 1;
}

int32_t main()
{
	SET_CRT_DEBUG_FIELD(_CRTDBG_LEAK_CHECK_DF);
	_CrtSetAllocHook(YourAllocHook);
	SetUnhandledExceptionFilter(CrashHandlerThatCreateDumpFileCallBack);
	HttpsServer* server = HttpsServer::GetServer();
	HttpServer::Run();

	int ret = server->Run();
	_CrtDumpMemoryLeaks();

	return ret;
}