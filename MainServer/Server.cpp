#include "Server.h"

Server::Server()
	: mWsaData(nullptr)
{
	mThreadHandles.reserve(100);
	mConnectionCount = 0;
	mSocket = 0;

	ZeroMemory(&mServerAddr, sizeof(sockaddr_in));
	mServerAddr.sin_family = AF_INET;
	mServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	mServerAddr.sin_port = htons(PORT_NUMBER);

	int result = WSAStartup(MAKEWORD(2, 2), mWsaData);

	assert(result == 0);

	if (result != 0)
	{
		std::cout << L"Socket start up Failed(Server constructor)" << std::endl;

		return;
	}

	std::cout << L"Vendor info : " << mWsaData->lpVendorInfo << L"Version : " << mWsaData->wVersion << std::endl;
}

Server::~Server()
{
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
	wchar_t buffer[MAX_SOCKET_BUFFER_SIZE];

	while (mConnectionCount < MAX_CONNECTION_COUNT)
	{
		clientSocket = accept(mSocket, (sockaddr*)&clientSockAddr, &addrLen);

		if (clientSocket == INVALID_SOCKET)
		{
			printSocketError();
			return -1;
		}

		wchar_t clientAddr[INET_ADDRSTRLEN];
		InetNtopW(AF_INET, &clientSockAddr.sin_addr, clientAddr, sizeof(clientAddr));

		std::cout << L"Client IP : " << clientAddr << L"Client Num : " << mConnectionCount << std::endl;

        HANDLE threadHandle = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0,
                                                                      &Server::processClient,
                                                                      reinterpret_cast<void*>(&clientSocket), 0, nullptr));

        threadHandle;


	}

    this->Terminate();

	return 0;
}

void Server::Terminate()
{
	delete this;
}

void Server::openSocket()
{
	SOCKET tempSocket = NULL;
	tempSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	assert(tempSocket == 0);
	assert(mSocket == 0);

	if (tempSocket == INVALID_SOCKET)
	{
		printSocketError();
	}
	else if (mSocket == 0)
	{
		mSocket = tempSocket;
	}
	else
	{
		std::cout << L"Server.cpp openSocket() attempted open socket twice" << std::endl;
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
}

// returns received data length
uint32_t Server::reveiveClientData()

uint32_t Server::processClient(void* clientSocketArg)
{
    constexpr int32_t BUFFER_SIZE = 512;

    SOCKET clientSocket = reinterpret_cast<SOCKET>(clientSocketArg);
    char buffer[BUFFER_SIZE];
    std::wstring content;
    content.reserve(BUFFER_SIZE * 2);

    int recvLen = 0;
    int recvLenSum = 0;

    do
    {
        recvLen = recv(clientSocket, buffer, BUFFER_SIZE, MSG_WAITALL);

        for (uint16_t i = 0; i < BUFFER_SIZE; ++i)
        {
            if (buffer[i] == '\r' && buffer[i + 1] == '\n')
            {
                buffer[i] = '\0';
                content.push_back(buffer[i]);
                break;
            }
            else
            {
                content.push_back(buffer[i]);
            }
        }

        recvLenSum += recvLen;
    } while (recvLen != 0);


    mConnectionCount++;

    return recvLenSum;
}
