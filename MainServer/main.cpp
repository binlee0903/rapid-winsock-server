#pragma once
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <cstdint>

#include "Server.h"


int32_t main()
{
	Server& server = *Server::GetServer();

	return server.Run();
};