#pragma once

#include "HttpsServer.h"
#include "HttpServer.h"

int32_t main()
{
	IServer* server = HttpsServer::GetServer();
	HttpServer::Run();

	return server->Run();
}