#pragma once

#include "HttpsServer.h"
#include "HttpServer.h"
#include "DebugHelper.h"

int32_t main()
{
	SET_CRT_DEBUG_FIELD(_CRTDBG_LEAK_CHECK_DF);
	SetUnhandledExceptionFilter(CrashHandlerThatCreateDumpFileCallBack);
	HttpsServer* server = HttpsServer::GetServer();
	HttpServer::Run();

	int ret = server->Run();
	_CrtDumpMemoryLeaks();

	return ret;
}