#pragma once
#include "HTMLPageRouter.h"
#include "HttpFileContainer.h"

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <fstream>
#include <sstream>
#include <cassert>
#include <iostream>

class HttpHelper final
{
public:
	HttpHelper& operator=(const HttpHelper& rhs) = delete;
	HttpHelper(const HttpHelper& rhs) = delete;

	~HttpHelper();

	static HttpHelper* GetHttpHelper();
	static void DeleteHttpHelper();

	void ParseHttpHeader(HttpObject* httpObject, std::string& recv);
	void CreateHttpResponse(HttpObject* httpObject, std::string& response);

private:
	HttpHelper();

	void createHeader(HttpObject* httpObject, std::string& response);

private:
	static HttpHelper* mInstance;
	const char mServerHttpVersion[9];
	HttpFileContainer* mFileContainer;
};

