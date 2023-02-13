#pragma once
#include "HTMLPageRouter.h"
#include "HttpFileContainer.h"

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <fstream>
#include <sstream>
#include <cassert>
#include <iostream>

const char HTTP_501_MESSAGE[] = "HTTP / 1.0 501 Not Implemented\r\n";
const char HTTP_404_MESSAGE[] = "HTTP / 1.0 404 Not Found\r\n";
const char HTTP_200_MESSAGE[] = "HTTP/1.1 200 OK\r\n";
const char HTTP_503_MESSAGE[] = "HTTP/1.1 503 Service Unavailable\r\n";

class HttpHelper final
{
public:
	HttpHelper& operator=(const HttpHelper& rhs) = delete;
	HttpHelper(const HttpHelper& rhs) = delete;

	~HttpHelper();

	static HttpHelper* GetHttpHelper();
	static void DeleteHttpHelper();

	void ParseHttpHeader(HttpObject* httpObject, std::string& recv);
	void CreateHttpResponse(HttpObject* httpObject, std::vector<int8_t>& response);

private:
	HttpHelper();

	void create404Response(HttpObject* httpObject, std::vector<int8_t>& response);
	void createHeader(HttpObject* httpObject, std::vector<int8_t>& response);
	void appendStringToVector(const char* str, int size, std::vector<int8_t>& v);

private:
	static HttpHelper* mInstance;
	const char mServerHttpVersion[9];
	HttpFileContainer* mTextFileContainer;
};

