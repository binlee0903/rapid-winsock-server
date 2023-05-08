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
#include "HttpsClient.h"
#include "HttpRouter.h"

#ifdef _DEBUG
	constexpr char SERVER_CERT_FILE[] = "C:\\Users\\egb35\\Documents\\server_cert\\binlee-blog.crt";
	constexpr char SERVER_KEY_FILE[] = "C:\\Users\\egb35\\Documents\\server_cert\\binlee-blog.key";
#else
	constexpr char SERVER_CERT_FILE[] = "C:\\Users\\Administrator\\Documents\\server-cert\\binlee-blog.com_20230212664E0.crt.pem";
	constexpr char SERVER_KEY_FILE[] = "C:\\Users\\Administrator\\Documents\\server-cert\\binlee-blog.com_20230212664E0.key.pem";
#endif

constexpr uint16_t MAX_CONNECTION_COUNT = 1000; // max clients count
constexpr uint16_t MAX_SOCKET_BUFFER_SIZE = 8192;
constexpr uint16_t HTTP_PORT_NUMBER = 80;
constexpr uint16_t HTTPS_PORT_NUMBER = 443;
constexpr uint16_t TIME_OUT = 3000;

class HttpsServer final : public IServer
{
public:
	/**
	 * return https server's pointer, if server is not constructed, creates it
	 *
	 * \return https server's pointer
	 */
	static HttpsServer* GetServer();

	/**
	 * if this function is called, server will start listen
	 * 
	 * \return 0 when q is pressed in console, but if this function return -1, there was an error
	 */
	virtual int32_t Run() override;

	virtual void PopClient(std::string& ip) override;
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
	std::unordered_map<std::string, HttpsClient*> mClients;
	std::unordered_set<std::string> mBlackLists;

	SRWLOCK* mSRWLock;
	SSL* mSSL;
	SSL_CTX* mSSLCTX;
};

