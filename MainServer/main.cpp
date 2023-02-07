#pragma once

#include "HttpsServer.h"

int32_t main()
{
	IServer* server = HttpsServer::GetServer();

	return server->Run();
}