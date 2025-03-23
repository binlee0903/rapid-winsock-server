/*****************************************************************//**
 * \file   HttpsServer.h
 * \brief  https server that support tls1.3,
 *         this class is singleton
 *
 * \author binlee0903
 * \date   February 2023
 *********************************************************************/

#pragma once

#include "ClientThreadPool.h"

#ifdef _DEBUG
constexpr char SERVER_CERT_FILE[] = "C:\\Users\\egb35\\Documents\\server-cert\\server.crt";
constexpr char SERVER_KEY_FILE[] = "C:\\Users\\egb35\\Documents\\server-cert\\server.key";
#else
constexpr char SERVER_CERT_FILE[] = "C:\\Users\\egb3543\\Documents\\server-cert\\binlee-blog.com_20240206F089A.crt.pem";
constexpr char SERVER_KEY_FILE[] = "C:\\Users\\egb3543\\Documents\\server-cert\\binlee-blog.com_20240206F089A.key.pem";
#endif

constexpr uint16_t MAX_CLIENT_CONNECTION_COUNT = 1000; // max clients count
constexpr uint16_t MAX_SOCKET_BUFFER_SIZE = 8192;
constexpr uint16_t HTTP_PORT_NUMBER = 80;
constexpr uint16_t HTTPS_PORT_NUMBER = 443;
constexpr uint16_t TIME_OUT = 3000;
constexpr uint32_t MAX_LOGGER_SIZE = 1048576; // 1MB
constexpr uint32_t MAX_LOGGER_FILES = 5;

#ifdef   _DEBUG
#define  SET_CRT_DEBUG_FIELD(a) \
            _CrtSetDbgFlag((a) | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
#define  CLEAR_CRT_DEBUG_FIELD(a) \
            _CrtSetDbgFlag(~(a) & _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
#else
#define  SET_CRT_DEBUG_FIELD(a)   ((void) 0)
#define  CLEAR_CRT_DEBUG_FIELD(a) ((void) 0)
#endif

class HttpsServer final
{
public:
	/**
	 * return https server's pointer, if server is not constructed, creates it
	 *
	 * @return https server's pointer
	 */
	static HttpsServer* GetServer();

	/**
	 * if this function is called, server will start listen
	 *
	 * @return 0 when q is pressed in console, but if this function return -1, there was an error
	 */
	virtual int32_t Run();

	int ProcessSSLHandshake(ClientSession* clientSession);
private:
	static DWORD __stdcall checkQuitMessage(LPVOID lpParam);
private:
	HttpsServer();
	~HttpsServer();

	void printSocketError();
	void invalidateSession();
	void signalForRemainingWorks();
	void eraseClient(uint32_t index);

	ClientSession* createClientSession(socket_t clientSocket, HANDLE clientEventHandle, SSL* clientSSL, std::string& ip);
private:
	static HttpsServer* mServer;

	bool mbIsQuitButtonPressed;
	uint32_t mSessionIDSequence;

	socket_t mHttpsSocket;
	
	ClientThreadPool* mClientThreadPool;
	SSL* mSSL;
	SSL_CTX* mSSLCTX;

	std::vector<ClientSession*> mClientSessions;
	std::vector<HANDLE> mClientEventHandles;

	std::shared_ptr<spdlog::logger> mLogger;
};

