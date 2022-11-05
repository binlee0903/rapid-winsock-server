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

	server.Run();

	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		std::cout << L"Socket start up Failed(Server constructor)" << std::endl;

		return 0;
	}

	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	SOCKET returnValue = 0;

	if (listenSocket == INVALID_SOCKET)
	{
		return 0;
	}

	sockaddr_in serverAddr;
	ZeroMemory(&serverAddr, sizeof(sockaddr_in));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(80);

	returnValue = bind(listenSocket, (sockaddr*)&serverAddr, sizeof(sockaddr_in));

	if (returnValue == SOCKET_ERROR)
	{
		return 0;
	}

	returnValue = listen(listenSocket, SOMAXCONN);
	if (returnValue == SOCKET_ERROR)
	{
		return 0;
	}

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