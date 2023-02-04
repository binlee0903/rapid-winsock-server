#pragma once
#include "HttpObject.h"

#include <fstream>
#include <sstream>
#include <cassert>

class HttpHelper final
{
public:
	HttpHelper() = delete;
	~HttpHelper() = delete;

	static void ParseHttpHeader(HttpObject* httpObject, std::wstring& recv);
	static void CreateHttpResponse(HttpObject* httpObject, std::string& response);

};

