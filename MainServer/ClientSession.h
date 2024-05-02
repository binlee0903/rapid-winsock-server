#include "network.h"
#include "ClientSessionType.h"

using socket_t = decltype(socket(0, 0, 0));

struct ClientSession
{
	uint32_t sessionID;
	int32_t processingCount;
	socket_t clientSocket;
	HANDLE eventHandle;
	SSL* clientSSLConnection;
	std::string* ip;
};