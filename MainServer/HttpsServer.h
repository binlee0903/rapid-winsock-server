/*****************************************************************//**
 * \file   HttpsServer.h
 * \brief  https server that support tls1.3,
 *         this class is singleton
 *
 * \author binlee0903
 * \date   February 2023
 *********************************************************************/

#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <iostream>
#include <conio.h>

#include <vector>
#include <unordered_set>
#include <process.h>
#include <synchapi.h>

#include "network.h"
#include "ClientThreadPool.h"
#include "HttpRouter.h"

#ifdef _DEBUG
	constexpr char SERVER_CERT_FILE[] = "C:\\Users\\egb35\\Documents\\server-cert\\server.crt";
	constexpr char SERVER_KEY_FILE[] = "C:\\Users\\egb35\\Documents\\server-cert\\server.key";
#else
	constexpr char SERVER_CERT_FILE[] = "C:\\Users\\Administrator\\Documents\\server-cert\\binlee-blog.com_20240206F089A.crt.pem";
	constexpr char SERVER_KEY_FILE[] = "C:\\Users\\Administrator\\Documents\\server-cert\\binlee-blog.com_20240206F089A.key.pem";
#endif

constexpr uint16_t MAX_CLIENT_CONNECTION_COUNT = 1000; // max clients count
constexpr uint16_t MAX_SOCKET_BUFFER_SIZE = 8192;
constexpr uint16_t HTTP_PORT_NUMBER = 80;
constexpr uint16_t HTTPS_PORT_NUMBER = 443;
constexpr uint16_t TIME_OUT = 3000;

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
private:
	static uint32_t __stdcall checkQuitMessage(void*);
private:
	HttpsServer();
	~HttpsServer();

	void sendRedirectMessage(socket_t clientSocket);
	void printSocketError();
private:
	static HttpsServer* mServer;

	bool mbIsQuitButtonPressed;

	socket_t mHttpsSocket;
	
	ClientThreadPool* mClientThreadPool;
	SSL* mSSL;
	SSL_CTX* mSSLCTX;
};

