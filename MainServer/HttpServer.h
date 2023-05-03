#pragma once

#include <cstdint>
#include <cassert>

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <process.h>
#include <synchapi.h>

#include "network.h"

class HttpServer final
{
public:
	HttpServer();
	~HttpServer();

	static int32_t Run();

private:
	static uint32_t redirectToHttps(void*);

private:
	static network::socket_t mHttpSocket;
};