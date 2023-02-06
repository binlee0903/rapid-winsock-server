#include "Server.h"

Server* Server::mServer;
std::multimap<std::wstring, std::wstring> Server::mClientData;

Server::Server()
	: mWsaData(new WSADATA())
	, mSRWLock(new SRWLOCK())
{
	mServer = this;
	mThreadHandles.reserve(100);
	mConnectionCount = 0;
	mSocket = 0;
	InitializeSRWLock(mSRWLock);

	ZeroMemory(&mServerAddr, sizeof(sockaddr_in));
	mServerAddr.sin_family = AF_INET;
	mServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	mServerAddr.sin_port = htons(PORT_NUMBER);

	int result = WSAStartup(MAKEWORD(2, 2), mWsaData);
	assert(result == 0);

	if (result != 0)
	{
		std::wcout << L"Socket start up Failed(Server constructor)" << std::endl;

		return;
	}
}

Server::~Server()
{
	for (auto x : mThreadHandles)
	{
		CloseHandle(x);
	}

	delete mWsaData;
	delete mSRWLock;

	int result = WSACleanup();

	assert(result == 0);

	if (result != 0)
	{
		printSocketError();
	}
}

int32_t Server::Run()
{
	openSocket();

	int32_t returnValue = 0;
	returnValue = bind(mSocket, (sockaddr*)&mServerAddr, sizeof(sockaddr_in));

	assert(returnValue != SOCKET_ERROR);
	if (returnValue == SOCKET_ERROR)
	{
		printSocketError();
		return -1;
	}

	returnValue = listen(mSocket, MAX_CONNECTION_COUNT);
	if (returnValue == SOCKET_ERROR)
	{
		printSocketError();
		return -1;
	}

	SOCKET clientSocket = NULL;
	sockaddr_in clientSockAddr;
	int32_t addrLen = sizeof(sockaddr_in);

	while (mConnectionCount < MAX_CONNECTION_COUNT)
	{
		clientSocket = accept(mSocket, (sockaddr*)&clientSockAddr, &addrLen);

		if (clientSocket == INVALID_SOCKET)
		{
			printSocketError();
			return -1;
		}

		mConnectionCount++;

        HANDLE threadHandle = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0,
                                                                      &Server::processClient,
                                                                      reinterpret_cast<void*>(clientSocket), 0, nullptr));

		mThreadHandles.push_back(threadHandle);
		clientSocket = NULL;
		ZeroMemory(&clientSockAddr, sizeof(clientSockAddr));
	}

	delete this;
	return 0;
}

void Server::openSocket()
{
	SOCKET tempSocket = NULL;
	tempSocket = socket(AF_INET, SOCK_STREAM, 0);

	assert(tempSocket != 0);
	assert(mSocket == 0);

	if (tempSocket == INVALID_SOCKET)
	{
		printSocketError();
	}
	else if (mSocket == 0)
	{
		const DWORD optValue = 1;
		setsockopt(tempSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&optValue), sizeof(optValue));
		mSocket = tempSocket;
	}
	else
	{
		std::wcout << L"Server.cpp openSocket() attempted open socket twice" << std::endl;
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
		LANG_SYSTEM_DEFAULT, reinterpret_cast<wchar_t*>(&msg), 0, nullptr);

	if (msg != nullptr)
	{
		std::cout << msg << std::endl;
	}
	LocalFree(msg);
	delete this;
}

// returns received data length
uint32_t Server::receiveClientData(SOCKET clientSocket, std::wstring& content)
{
	char buffer[BUFFER_SIZE];

	u_long readLen = 0;
	int recvLen = 0;
	int recvLenSum = 0;

	do
	{
		/*HANDLE eventHandle = WSACreateEvent();
		WSANETWORKEVENTS netEvents;
		WSAEventSelect(clientSocket, eventHandle, FD_READ);
		WSAWaitForMultipleEvents(1, &eventHandle, false, INFINITE, false);*/

		int retValue = ioctlsocket(clientSocket, FIONREAD, &readLen);
		if (retValue != 0)
		{
			break;
		}

		if (readLen == 0)
		{
			break;
		}

		if (readLen > BUFFER_SIZE)
		{
			readLen = BUFFER_SIZE;
		}


		recvLen = recv(clientSocket, buffer, readLen, 0);

		if (recvLen != 0)
		{
			for (uint16_t i = 0; i < readLen; ++i)
			{
				if (buffer[i] == '\r' && buffer[i + 1] == '\n')
				{
					if (buffer[i + 2] == '\r' && buffer[i + 3] == '\n')
					{
						content.push_back(L'\0');
						break;
					}
					content.push_back(buffer[i]);
				}
				else
				{
					content.push_back(buffer[i]);
				}
			}

			recvLenSum += recvLen;
		}
	} while (recvLen != 0);

	return recvLenSum;
}

uint32_t Server::processClient(void* clientSocketArg)
{
	SOCKET clientSocket = reinterpret_cast<SOCKET>(clientSocketArg);
	sockaddr_in clientSockAddr;

	wchar_t clientAddr[INET_ADDRSTRLEN];
	int32_t clientAddrLen = sizeof(clientAddr);
	getpeername(clientSocket, reinterpret_cast<sockaddr*>(&clientSockAddr), &clientAddrLen);
	InetNtop(AF_INET, &clientSockAddr.sin_addr, clientAddr, sizeof(clientAddr) / 2);

	std::wstring content;
	content.reserve(BUFFER_SIZE * 2);
	uint32_t receivedDataLength = receiveClientData(clientSocket, content);

	if (receivedDataLength == 0)
	{
		mServer->closeSocket(clientSocket);
		_endthreadex(0);
		return 0;
	}

	mClientData.insert(std::pair<std::wstring, std::wstring>{ clientAddr, content }); // for debug

	HttpObject* httpObject = new HttpObject();
	HttpHelper::ParseHttpHeader(httpObject, content);

	std::string response;
	response.reserve(BUFFER_SIZE);
	HttpHelper::CreateHttpResponse(httpObject, response);

	int32_t returnValue = send(clientSocket, response.c_str(), response.length(), 0);
	if (returnValue == SOCKET_ERROR)
	{
		return -1;
	}

	delete httpObject;

    return 0;
}
