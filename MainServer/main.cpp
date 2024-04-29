#pragma once

#include "HttpsServer.h"
#include "HttpServer.h"
#include "DebugHelper.h"

int32_t main()
{
	SetUnhandledExceptionFilter(CrashHandlerThatCreateDumpFileCallBack);
	HttpsServer* server = HttpsServer::GetServer();
	HttpServer::Run();

	return server->Run();
}