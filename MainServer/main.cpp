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

	uint32_t clientSocket = 0;
	sockaddr_in clientAddr;
	int addrLength = sizeof(sockaddr_in);
	char buffer[2048];
	ZeroMemory(buffer, 2048);

	char clientAddrStr[INET_ADDRSTRLEN];

	while (true)
	{
		clientSocket = accept(listenSocket, (sockaddr*)&clientAddr, &addrLength);

		if (clientSocket == INVALID_SOCKET)
		{
			break;
		}

		inet_ntop(AF_INET, &clientAddr.sin_addr, clientAddrStr, sizeof(clientAddr));
		std::cout << "TCP Server Client : " << clientAddrStr << " PORT NUMBER : " << ntohs(clientAddr.sin_port) << std::endl;

		while (true)
		{
			returnValue = recv(clientSocket, buffer, 1024, 0);
			if (returnValue == SOCKET_ERROR)
			{
				break;
			}
			else if (returnValue == 0)
			{
				break;
			}

			buffer[returnValue] = '\0';
			std::cout << "content : " << buffer << std::endl;
		}

		closesocket(clientSocket);
	}

	closesocket(listenSocket);
	WSACleanup();

	return 0;
};