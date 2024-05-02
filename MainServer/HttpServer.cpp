#include "HttpServer.h"

network::socket_t HttpServer::mHttpSocket = NULL;

int32_t HttpServer::Run()
{
    network::OpenSocket(mHttpSocket, network::HTTP_PORT_NUMBER, nullptr, false);

	reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, redirectToHttps, nullptr, 0, nullptr));

    return 0;
}

uint32_t __stdcall HttpServer::redirectToHttps(void*)
{
	HANDLE eventHandle = WSACreateEvent();
	WSANETWORKEVENTS netEvents;
	ZeroMemory(&netEvents, sizeof(netEvents));

	WSAEventSelect(mHttpSocket, eventHandle, FD_ACCEPT);

	while (true)
	{
		WSAWaitForMultipleEvents(1, &eventHandle, false, WSA_INFINITE, false);
		WSAEnumNetworkEvents(mHttpSocket, eventHandle, &netEvents);

		switch (netEvents.lNetworkEvents)
		{
		case FD_ACCEPT:
			network::ProcessRedirect(mHttpSocket);
			continue;
		}
	}

	WSACloseEvent(eventHandle);
}
