#pragma once

#include <cstdint>
#include <cassert>

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <process.h>
#include <synchapi.h>

#include "network.h"

LONG CALLBACK CrashHandlerThatCreateDumpFileCallBack(EXCEPTION_POINTERS* exception);

class HttpServer final
{
public:
	HttpServer() = default;
	~HttpServer() = default;

	static int32_t Run();

private:
	static uint32_t __stdcall redirectToHttps(void*);

private:
	static network::socket_t mHttpSocket;
};