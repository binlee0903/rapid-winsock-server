#include "HttpServer.h"

int32_t HttpServer::Run()
{
    network::OpenSocket(mHttpSocket, network::HTTP_PORT_NUMBER, nullptr, false);

	reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, redirectToHttps, nullptr, 0, nullptr));

    return 0;
}

uint32_t HttpServer::redirectToHttps(void*)
{
	HANDLE eventHandle = WSACreateEvent();
	WSANETWORKEVENTS netEvents;
	ZeroMemory(&netEvents, sizeof(netEvents));

	WSAEventSelect(mHttpSocket, eventHandle, FD_ACCEPT);

	network::socket_t clientSocket = NULL;
	sockaddr_in sockaddrIn;

	while (true)
	{
		WSAWaitForMultipleEvents(1, &eventHandle, false, WSA_WAIT_TIMEOUT, false);
		WSAEnumNetworkEvents(mHttpSocket, eventHandle, &netEvents);

		switch (netEvents.lNetworkEvents)
		{
		case FD_ACCEPT:
			clientSocket = network::ProcessAccept(mHttpSocket, sockaddrIn);

			if (clientSocket == NULL)
			{
				//std::cout << "http Run() : clientSocket was NULL" << std::endl;
				continue;
			}
			else
			{
				//sendRedirectMessage(clientSocket);
				clientSocket = NULL;
			}
			break;
		default:
			continue;
		}
	}

	WSACloseEvent(eventHandle);
}
