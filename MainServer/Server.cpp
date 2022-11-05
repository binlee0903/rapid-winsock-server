#include "Server.h"

Server::Server()
	: mWsaData(nullptr)
	, mSockets(0)
{
	mSockets.reserve(100);

	if (WSAStartup(MAKEWORD(2, 2), mWsaData))
	{
		std::cout << L"Socket start up Failed(Server constructor)" << std::endl;

		return;
	}
}

Server::~Server()
{
	for (auto i : mSockets)
	{
		if (i != 0)
		{
			closeSocket(i);
		}
	}

	if (WSACleanup() != 0)
	{
		printSocketError();
	}

	delete mServer;
}

void Server::Run()
{


}

void Server::openSocket()
{
	if (socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) != 0)
	{
		printSocketError();
	}
}

void Server::closeSocket(SOCKET socket)
{
	if (closesocket(socket) != 0)
	{
		printSocketError();
	}
}

Server* Server::GetServer()
{
	if (mServer == nullptr)
	{
		mServer = new Server();
	}

	return mServer;
}

void Server::printSocketError()
{
	wchar_t* msg = nullptr;
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, WSAGetLastError(),
		LANG_SYSTEM_DEFAULT, msg, 0, nullptr);

	if (msg != nullptr)
	{
		std::cout << msg << std::endl;
	}
}
